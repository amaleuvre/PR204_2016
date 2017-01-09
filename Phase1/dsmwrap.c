#include "common_impl.h"

int main(int argc, char **argv)
{
  /*  processus intermediaire pour "nettoyer" */
  /* la liste des arguments qu'on va passer */
  /* a la commande a executer vraiment */
  /* on passe de "exec(./dsmwrap hostIP hostport truc args)" */
  /* à : "truc args" */

  char *hostIP = argv[1];
  char *hostport = argv[2];
  //char * rank = argv[3]; NON PAS ICI
  char * finalargs[argc - 3];
  int name_len = 100; // TODO pourquoi 100 ?
  char name[name_len];

  int i;
  for (i = 0; i< argc - 3; i++){
    finalargs[i] = argv[3 + i];
  }
  finalargs[argc-3] = NULL;

  /* creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */


  //printf("hostIP : %s\n",hostIP);
  //printf("hostport : %s\n",hostport);

  int fd1;
  fd1 = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in lanceur_sin;
  memset(&lanceur_sin, 0, sizeof(struct sockaddr_in));
  inet_aton(hostIP,&lanceur_sin.sin_addr); // attention format
  lanceur_sin.sin_family = AF_INET;
  lanceur_sin.sin_port = htons(atoi(hostport)); // attention format

  int test_connect;
  do {test_connect = connect(fd1, (struct sockaddr *)&lanceur_sin, sizeof(struct sockaddr_in));
  } while(test_connect == -1);

  /*  Envoi du nom de machine au lanceur */
  if (-1 == gethostname(name, name_len)){
    perror("gethostname");
    exit(EXIT_FAILURE);
  }

  int taille_nom = 0;
  while(name[taille_nom] != '\0'){
    taille_nom = taille_nom + 1;
  }

  do_write(fd1, &taille_nom, sizeof(int));

  do_write(fd1, &name, taille_nom);

  /*  Envoi du pid au lanceur */
  pid_t pid;
  pid = getpid();
  char pid_to_send[6];
  sprintf(pid_to_send, "%d", pid);
  do_write(fd1, &pid_to_send, 6*sizeof(char));

  /*  Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */
  int port_dsm;
  int prop_dsm = 0;
  int fd2;

  fd2 = creer_socket(prop_dsm, &port_dsm);
  if (-1 == fd2){
    fprintf(stdout, "Error in creer_socket\n");
    fflush(stdout);
    exit(EXIT_FAILURE);
  }
  /* Envoi du numero de port au lanceur */
  /* pour qu'il le propage à tous les autres */
  /* processus dsm */

  char port_to_send[6];
  sprintf(port_to_send, "%d", port_dsm);
  do_write(fd1, &port_to_send, 6*sizeof(char));


  fflush(stdout);
  /* on execute la bonne commande */
  execvp(finalargs[0],finalargs);

  return EXIT_SUCCESS;
}
