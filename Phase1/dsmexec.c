#include "common_impl.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* variables globales */
#define PAGE_NUMBER
#define PAGE_SIZE  // défini avec _SC_PAGE_SIZE
#define DSM_NODE_ID
#define DSM_NODE_NUM
#define BASE_ADDR
#define TOP_ADDR
#define NB_MOTS_MAX 100
#define TAILLE_MOT_MAX 30

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
     waitpid(-1,NULL,WNOHANG); // ne fait que attendre, il faut le terminer
     // il peut en avoir plusieur, faire une boucle décroissante sur un compteur volatile
     // pour tous les traiter
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
          int count = 0;

          char c;
          char** tableau_mots;
          int i;
          int j=0;
          char * newargv[argc + 4];
          int Taille_du_nom_de_la_machine_distante = 100;
          char hostname[Taille_du_nom_de_la_machine_distante];
          /* 1- on recupere le nombre de processus a lancer */

          /* 2- on recupere les noms des machines : le nom de */
          /* la machine est un des elements d'identification */

          // int reset = fseek(fd, 0, SEEK_SET);
          // TODO enlever l'espace si besoin

          tableau_mots = malloc(sizeof(char*)*NB_MOTS_MAX);
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
                    count++;
               }
               tableau_mots[i][j]=c;
               j++;
               lu = read(fd,&c,1);
          }

          /* DEBUG // Affichage du résultat
          printf("Nombre de lignes : %d\n",count);
          for(i = 0; i < NB_MOTS_MAX; i++){
               if(tableau_mots[i][0] == 0)
               break;
               printf("%s\n", tableau_mots[i]);
          }*/

          close(fd);

          // Libération de la mémoire
          for(i = 0; i < NB_MOTS_MAX; i++) {
               free(tableau_mots[i]);
          }
          free(tableau_mots);



          /* creation de la socket d'ecoute */
          int sock;
          sock = socket(AF_INET, SOCK_STREAM, 0);

          //inet_addr("127.0.0.1");
          /*struct sockaddr_in
          {
          short      sin_family;
          unsigned short   sin_port;
          struct   in_addr   sin_addr;
          char   sin_zero[8];
     };*/
     struct sockaddr_in sin;
     memset(&sin, 0, sizeof(struct sockaddr_in));
     sin.sin_addr.s_addr = htonl(INADDR_ANY);
     sin.sin_family = AF_INET;
     sin.sin_port = htons(0);

     bind(sock, (struct sockaddr *)&sin, sizeof(sin));

     /* + ecoute effective */

     listen(sock, count);

     /* creation des fils */
     for(i = 0; i < num_procs ; i++) {

          /* creation du tube pour rediriger stdout */
          int pip_out[2];
          pipe(pip_out);

          /* creation du tube pour rediriger stderr */
          int pip_err[2];
          pipe(pip_err);


          pid = fork();
          if(pid == -1) ERROR_EXIT("fork ok");

          if (pid == 0) { /* fils */

               /* redirection stdout */
               close(pip_out[0]);
               dup2(pip_out[1],STDOUT_FILENO);

               /* redirection stderr */
               close(pip_err[0]);
               dup2(pip_err[1],STDERR_FILENO);

               /* Creation du tableau d'arguments pour le ssh */
               gethostname(hostname, Taille_du_nom_de_la_machine_distante);

               newargv[0] = "ssh";
               newargv[1] = tableau_mots[i]; //nom de la machine en question
               newargv[2] = "./bin/dsmwrap"; //chemin vers le programme a executer
               newargv[3] = inet_ntoa(sin.sin_addr); //adresse IP de la machine
               sprintf(newargv[4], "%d", ntohs(sin.sin_port)); //numero de port
               int k;
               for (k = 1; k < argc - 1; k++){
                    newargv [4+k] = argv[k+1];
               }
               newargv[argc+k] = NULL;
               /* jump to new prog : */
               printf("just avant ssh\n"); // DEBUG
               execvp("ssh",newargv);

          } else  if(pid > 0) { /* pere */
               /* fermeture des extremites des tubes non utiles */
               close(pip_out[1]);
               close(pip_err[1]);

               num_procs_creat++;
          }
     }


     for(i = 0; i < num_procs ; i++){

          /* on accepte les connexions des processus dsm */


          /*struct sockaddr_in csin;
          socklen_t taille = sizeof(csin);
          int csock = accept(sock, (sockaddr_in)&csin, &taille);
          */

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
// close....

/* on ferme la socket d'ecoute */
// int closesocket(int sock);

}
exit(EXIT_SUCCESS);
}
