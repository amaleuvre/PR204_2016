#include "common_impl.h"

/* variables globales */
#define PAGE_NUMBER
#define PAGE_SIZE  // défini avec _SC_PAGE_SIZE
#define DSM_NODE_ID
#define DSM_NODE_NUM
#define BASE_ADDR
#define TOP_ADDR
#define NB_MOTS_MAX 100
#define TAILLE_MOT_MAX 30

typedef struct info_machine { // stocke les infos des differentes machines
  char nom[256];
  char pid[6];
  char port[6];
  int rang;
}info_machine;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig){
  /* on traite tous les fils qui se terminent */
  /* pour eviter les zombies */
  waitpid(-1,NULL,WNOHANG);
  num_procs_creat--;
}

int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  } else {
    int demons = 1;
    pid_t pid; // pid du processus
    int num_procs = 0; // nombre de machines à utiliser
    struct sigaction act_chld;
    int fd; // descripteur pour lire machine_file
    char c; // charactere de lecture
    char* tableau_mots[NB_MOTS_MAX]; // stocke les mots lus
    int lu; // mot lu
    int i; // indice pour boucle de lecture
    int j; // indice pour boucle de lecture
    char **newargv = malloc((argc + 5)*sizeof(char *));
    int Taille_du_nom_de_la_machine = 100; // taille max des noms de machine, arbitraire
    char hostname[Taille_du_nom_de_la_machine]; // stocke le nom de la machine
    int o; // pour l'affichage des noms de machine
    int sock; // socket d'ecoute
    int port_num;
    int csock; // socket acceptée
    int h; // pour le numéro de rang
    char chaine[14] = ".pedago.ipb.fr"; // complete le nom des machines
    char *nom_complet; // stocke le nom complet des machines
    int taille_nom; // taille du nom complet des machines
    struct sockaddr_in csin; 

    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    memset(&act_chld, 0, sizeof(struct sigaction));
    act_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &act_chld, NULL);

    /* lecture du fichier de machines */
    fd = open(argv[1],O_RDONLY);

    /* 1- on recupere le nombre de processus a lancer */
    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */

    for(i = 0;i < NB_MOTS_MAX;i++){
      tableau_mots[i] = malloc(sizeof(char)*TAILLE_MOT_MAX);
      memset(tableau_mots[i],0,sizeof(char)*TAILLE_MOT_MAX);
    }
    lu = read(fd,&c,1);

    // Comptage des lignes + copie des mots
    i=0;
    j=0;
    while(lu == 1) {
      if(c == '\n') {
        j=0;
        i++;
        num_procs++;
        lu = read(fd,&c,1);
      }
      tableau_mots[i][j]=c;
      j++;
      lu = read(fd,&c,1);
    }
    close(fd);

    /* affichage des noms des machines qui seront utilisées */
    for (o = 0; o < num_procs; o++) {
      printf("%s\n", tableau_mots[o]);
    }

    /* creation de la socket d'ecoute */
    sock = creer_socket(0, &port_num);

    /* + ecoute effective */
    listen(sock, num_procs);

    /* creation des structures de stockage des infos */
    struct info_machine bdd[num_procs];
    struct info_machine machine;

    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      /* creation du tube pour rediriger stdout */
      int pip_out[2];
      pipe(pip_out);

      /* creation du tube pour rediriger stderr */
      int pip_err[2];
      pipe(pip_err);

      /* creation des fils */
      pid = fork();
      if(pid == -1){
        ERROR_EXIT("fork bug");
      }

      if (pid == 0) { /* fils */

        struct hostent* res;
        struct in_addr* addr;
        int k;

        /* redirection stdout */
        close(pip_out[0]);
        dup2(pip_out[1],STDOUT_FILENO); // masquer cette ligne pour avoir les print du programme a executer

        /* redirection stderr */
        close(pip_err[0]);
        dup2(pip_err[1],STDERR_FILENO); // masquer cette ligne pour avoir les print du programme a executer

        /* Creation du tableau d'arguments pour le ssh */
        newargv[0] = "ssh";
        newargv[1] = tableau_mots[i]; //nom de la machine
        newargv[2] = "/net/t/amaleuvre/Documents/S7/PR204/Phase1/bin/dsmwrap"; //chemin vers le programme a executer
        //newargv[2] = "/net/t/mfouque/Documents/pr204/Phase1/bin/dsmwrap";

        gethostname(hostname, Taille_du_nom_de_la_machine);
        res = gethostbyname(hostname);
        addr = (struct in_addr*) res->h_addr_list[0];
        newargv[3] = inet_ntoa(*addr); //adresse IP de la machine

        newargv[4] = malloc(6*sizeof(char));
        sprintf(newargv[4], "%d", port_num); //numero de port

        /* création de la nouvelle chaine d'argument pour le ssh */
        for (k = 1; k < argc - 1; k++){
          newargv [4+k] = argv[k+1];
        }
        newargv[argc+k] = NULL;

        /* jump to new prog : */
        execvp("ssh",newargv);

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(pip_out[1]);
        close(pip_err[1]);

        num_procs_creat++;
      }
    }

    for(i = 0; i < num_procs ; i++){

      memset(&machine, 0, sizeof(struct info_machine));

      /* on accepte les connexions des processus dsm */

      memset(&csin, 0, sizeof(struct sockaddr_in));
      socklen_t taille = sizeof(csin);

      do {
        csock = accept(sock, (struct sockaddr*)&csin, &taille);
      } while((csock == -1) && (errno == EINTR));

      /* On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      do_read(csock, &taille_nom, sizeof(int));

      /* 2- puis la chaine elle-meme */
      do_read(csock, machine.nom, taille_nom*sizeof(char));

      /* On recupere le pid du processus distant  */
      do_read(csock, machine.pid, 6*sizeof(char));

      /* On recupere le numero de port de la socket d'ecoute des processus distants */
      do_read(csock, machine.port, 6*sizeof(char));

      // attribution des numéros de rang;

      for(h=0; h < num_procs; h++){
        if (0 == strncmp(machine.nom, tableau_mots[h], strlen(machine.nom))) machine.rang = h;
        /* cas où le nom retourné est en .pedago... */
        nom_complet = malloc(strlen(machine.nom));
        strcpy(nom_complet, tableau_mots[h]);
        strcat(nom_complet, chaine);
        if (0 == strncmp(machine.nom, nom_complet, strlen(machine.nom))){
          machine.rang = h;
          strcpy(machine.nom, tableau_mots[h]);
        }
      }
      /* stockage des informations recueillies */
      bdd[machine.rang] = machine;
    }

    /* affichage de toutes les infos utiles */
    for(i = 0; i < num_procs ; i++){
      printf("Rang(%i): Nom(%s)  pid(%s)  port(%s)\n", bdd[i].rang, bdd[i].nom, bdd[i].pid, bdd[i].port);
    }
    /* envoi du nombre de processus aux processus dsm */

    /* envoi des rangs aux processus dsm */

    /* envoi des infos de connexion aux processus */


    /* gestion des E/S : on recupere les caracteres */
    /* sur les tubes de redirection de stdout/stderr */
    /* while(1)
    {
    je recupere les infos sur les tubes de redirection
    jusqu'à ce qu'ils soient inactifs (ie fermes par les
    processus dsm ecrivains de l'autre cote ...)

  };
  */

  /* on attend les processus fils */
  while(demons){
    if(num_procs_creat == 0) demons = 0;
  }

  /* on ferme les descripteurs proprement */
  // close....

  /* on ferme la socket d'ecoute */
  // int closesocket(int sock);

}
exit(EXIT_SUCCESS);
}
