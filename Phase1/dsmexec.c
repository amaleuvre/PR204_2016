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

typedef struct info_machine {
  char nom[256];
  char pid[6];
  char port[6];
  int rang;
}info_machine;

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig) // TODO faire un variable volatile pour bien recupérer tous les fils
{
  /* on traite les fils qui se terminent */
  /* pour eviter les zombies */
  waitpid(-1,NULL,WNOHANG); // ne fait que attendre, il faut le terminer
  // il peut en avoir plusieur, faire une boucle décroissante sur un compteur volatile
  // pour tous les traiter
}

int toto(void)
{
  int a = 0;
  return a;
}

int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  } else {
    pid_t pid;
    int num_procs = 0;


    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    /* XXX.sa_handler = sigchld_handler; */
    struct sigaction act_chld;
    memset(&act_chld, 0, sizeof(struct sigaction));
    act_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &act_chld, NULL);

    /* lecture du fichier de machines */

    int fd = open(argv[1],O_RDONLY);

    char c;
    char* tableau_mots[NB_MOTS_MAX];
    int i;
    int j=0;
    char **newargv = malloc((argc + 5)*sizeof(char *));
    int Taille_du_nom_de_la_machine = 100;
    char hostname[Taille_du_nom_de_la_machine];

    /* 1- on recupere le nombre de processus a lancer */

    /* 2- on recupere les noms des machines : le nom de */
    /* la machine est un des elements d'identification */

    // int reset = fseek(fd, 0, SEEK_SET);
    // TODO enlever l'espace si besoin

    //tableau_mots = malloc(sizeof(char*)*NB_MOTS_MAX);
    for(i = 0;i < NB_MOTS_MAX;i++){
      tableau_mots[i] = malloc(sizeof(char)*TAILLE_MOT_MAX);
      memset(tableau_mots[i],0,sizeof(char)*TAILLE_MOT_MAX);
    }
    int lu = read(fd,&c,1);
    i=0;

    // Comptage des lignes + copie des mots
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

    int o;
    for (o = 0; o < num_procs; o++) {
      printf("%s\n", tableau_mots[o]);
    }


    /* creation de la socket d'ecoute */
    int sock;
    int port_num = 9999;
    sock = creer_socket(0, &port_num);
    /*struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);

    bind(sock, (struct sockaddr *)&sin, sizeof(sin));*/

    /* + ecoute effective */

    listen(sock, num_procs);

    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      /* creation du tube pour rediriger stdout */
      int pip_out[2];
      pipe(pip_out);

      /* creation du tube pour rediriger stderr */
      int pip_err[2];
      pipe(pip_err);


      pid = fork();
      if(pid == -1){
        ERROR_EXIT("fork bug");
      }

      if (pid == 0) { /* fils */

        /* redirection stdout */
        close(pip_out[0]);
        //dup2(pip_out[1],STDOUT_FILENO);

        /* redirection stderr */
        close(pip_err[0]);
        //dup2(pip_err[1],STDERR_FILENO);

        /* Creation du tableau d'arguments pour le ssh */

        newargv[0] = "ssh";
        newargv[1] = tableau_mots[i]; //nom de la machine en question

        newargv[2] = "/net/t/amaleuvre/Documents/S7/PR204/Phase1/bin/dsmwrap"; //chemin vers le programme a executer

        gethostname(hostname, Taille_du_nom_de_la_machine);
        struct hostent* res;
        struct in_addr* addr;
        res = gethostbyname(hostname);
        addr = (struct in_addr*) res->h_addr_list[0];
        newargv[3] = inet_ntoa(*addr); //adresse IP de la machine

        newargv[4] = malloc(6*sizeof(char));
        sprintf(newargv[4], "%d", port_num); //numero de port

        int k;
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

    struct info_machine bdd[num_procs];
    struct info_machine machine;

    int taille_nom;

    for(i = 0; i < num_procs ; i++){

      memset(&machine, 0, sizeof(struct info_machine));

      /* on accepte les connexions des processus dsm */
      struct sockaddr_in csin;
      memset(&csin, 0, sizeof(struct sockaddr_in));
      socklen_t taille = sizeof(csin);

      int csock;
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
      int h;
      char chaine[14] = ".pedago.ipb.fr";
      char *nom_complet;
      for(h=0; h < num_procs; h++){
        if (0 == strncmp(machine.nom, tableau_mots[h], strlen(machine.nom))) machine.rang = h;
        nom_complet = malloc(strlen(machine.nom));
        strcpy(nom_complet, tableau_mots[h]);
        strcat(nom_complet, chaine);
        if (0 == strncmp(machine.nom, nom_complet, strlen(machine.nom))){
          machine.rang = h;
          strcpy(machine.nom, tableau_mots[h]);
        }
      }

      bdd[machine.rang] = machine;
      //printf("Nom(%s)  pid(%s)  port(%s)  rang(%i)\n", machine.nom, machine.pid, machine.port, machine.rang);

    }

    for(i = 0; i < num_procs ; i++){
      printf("Rang(%i): Nom(%s)  pid(%s)  port(%s)\n", bdd[i].rang, bdd[i].nom, bdd[i].pid, bdd[i].port);
    }
    /* envoi du nombre de processus aux processus dsm*/

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
  wait(NULL);

  /* on ferme les descripteurs proprement */
  // close....

  /* on ferme la socket d'ecoute */
  // int closesocket(int sock);

}
exit(EXIT_SUCCESS);
}
