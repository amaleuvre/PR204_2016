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
  pid_t pid;
  int port;
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

void sigchld_handler(int sig) // TODO en démasquant les sigchild
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
    char **newargv = malloc((argc + 4)*sizeof(char *));
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
    sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);

    bind(sock, (struct sockaddr *)&sin, sizeof(sin));

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
        gethostname(hostname, Taille_du_nom_de_la_machine);

        newargv[0] = "ssh";
        newargv[1] = tableau_mots[i]; //nom de la machine en question

        //fprintf(stdout,"Exec[%i] ============== %s\n",i,tableau_mots[i]);

        newargv[2] = "~/Documents/S7/PR204/Phase1/bin/dsmwrap"; //chemin vers le programme a executer
        fprintf(stdout,"Exec[%i] ============== %s %i\n",i,newargv[1],getpid());

        //  char *ptr =  inet_ntoa(sin.sin_addr);
        //newargv[3] = "coucou";//malloc(64);
        //strcpy(newargv[3],ptr);
        newargv[3] = inet_ntoa(sin.sin_addr); //adresse IP de la machine


        //char temp[100]; // TODO changer le 100, pourquoi 100 ?
        //sprintf(temp, "%d",ntohs(sin.sin_port));
        //newargv[4] = temp; //numero de port



        int k;
        for (k = 1; k < argc - 1; k++){
          newargv [4+k] = argv[k+1];
        }
        newargv[argc+k] = NULL;



        int t;
        for (t = 0; t < argc + 4; t++) {
          printf("%s\n",newargv[t]);
        }
        fflush(stdout);

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

    for(i = 0; i < num_procs ; i++){

      memset(&machine, 0, sizeof(struct info_machine));

      // TODO fd[i] = accept ....

      /* on accepte les connexions des processus dsm */
      struct sockaddr_in csin;
      memset(&csin, 0, sizeof(struct sockaddr_in));
      socklen_t taille = sizeof(csin);
      int csock = accept(sock, (struct sockaddr*)&csin, &taille);
      if (csock == -1) printf("Erreur à l'accept\n");
      /* On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */

      /* 2- puis la chaine elle-meme */
      read(csock, &machine.nom, sizeof(machine.nom));

      /* On recupere le pid du processus distant  */
      read(csock, &machine.pid, sizeof(machine.pid));

      /* On recupere le numero de port de la socket d'ecoute des processus distants */
      read(csock, &machine.port, sizeof(machine.port));

      // attribution des numéros de rang;
      int h;
      for(h=0; h < num_procs; h++){
        if (0 == strncmp(machine.nom, tableau_mots[h], strlen(machine.nom))) machine.rang = h;
      }


      bdd[i] = machine;
      printf("Machine[%i] : nom(%s)  pid(%i)  port(%i)  rang(%i)\n", i, machine.nom, machine.pid, machine.port, machine.rang);
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
