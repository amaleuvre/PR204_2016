#include "common_impl.h"

/* variables globales */
#define PAGE_NUMBER
#define PAGE_SIZE  // défini avec _SC_PAGE_SIZE
#define DSM_NODE_ID
#define DSM_NODE_NUM
#define BASE_ADDR
#define TOP_ADDR

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

void sigchld_handler(int sig)
{
  /* on traite les fils qui se terminent */
  /* pour eviter les zombies */
  waitpid(-1,NULL,WNOHANG);
}


int main(int argc, char *argv[])
{
  if (argc < 3){
    usage();
  } else {
    pid_t pid;
    int num_procs = 0;
    int i;

    /* Mise en place d'un traitant pour recuperer les fils zombies*/
    /* XXX.sa_handler = sigchld_handler; */
    struct sigaction act_chld;
    memset(&act_chld, 0, sizeof(struct sigaction));
    act_chld.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &act_chld, NULL);

    /* lecture du fichier de machines */
    FILE *fp;
    fp = fopen("machine_file","r");
    /* 1- on recupere le nombre de processus a lancer */
    int character;
    int nb_line = 0;
    int nb_proc;
    while ((character = getc(fp)) != EOF){
      if (character == '\n')
      ++nb_line;
    }
    nb_proc = nb_line;
    /* 2- on recupere les noms des machines : le nom de */
    int reset = fseek(fp, 0, SEEK_SET);
    // TODO a finir

    /* la machine est un des elements d'identification */




    /* creation de la socket d'ecoute */
    /* + ecoute effective */

    /* creation des fils */
    for(i = 0; i < num_procs ; i++) {

      /* creation du tube pour rediriger stdout */
      int pip_out[2];
      pipe(pip_out);

      /* creation du tube pour rediriger stderr */
      int pip_err[2];
      pipe(pip_err);


      pid = fork();
      if(pid == -1) ERROR_EXIT("fork");

      if (pid == 0) { /* fils */

        /* redirection stdout */
        close(pip_out[0]);
        dup2(pip_out[1],STDOUT_FILENO);

        /* redirection stderr */
        close(pip_err[0]);
        dup2(pip_err[1],STDERR_FILENO);

        /* Creation du tableau d'arguments pour le ssh */

        /* jump to new prog : */
        /* execvp("ssh",newargv); */

      } else  if(pid > 0) { /* pere */
        /* fermeture des extremites des tubes non utiles */
        close(pip_out[1]);
        close(pip_err[1]);

        num_procs_creat++;
      }
    }


    for(i = 0; i < num_procs ; i++){

      /* on accepte les connexions des processus dsm */

      /* On recupere le nom de la machine distante */
      /* 1- d'abord la taille de la chaine */
      /* 2- puis la chaine elle-meme */

      /* On recupere le pid du processus distant  */

      /* On recupere le numero de port de la socket d'ecoute des processus distants */
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

  /* on ferme la socket d'ecoute */
}
exit(EXIT_SUCCESS);
}
