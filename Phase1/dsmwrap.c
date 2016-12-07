#include "common_impl.h"

int main(int argc, char **argv) // TODO dsmwrap.c
{
  /*  processus intermediaire pour "nettoyer" */
  /* la liste des arguments qu'on va passer */
  /* a la commande a executer vraiment */
  /* on passe de "exec(./dsmwrap hostIP hostport rank truc args)" */
  /* à : "truc args" */
  char *hostIP = argv[1];
  char *hostport = argv[2];
  char * rank = argv[3];
  char * finalargs[argc - 4];
  int name_len = 100; // TODO pourquoi 100 ?
  char name[name_len];

  int i;
  for (i = 0; i< argc - 4; i++){
    finalargs[i] = argv[4 + i];
  }
  finalargs[argc-4] = NULL;

  /* creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */

  int fd1;
  fd1 = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in lanceur_sin;
  memset(&lanceur_sin, 0, sizeof(struct sockaddr_in));
  lanceur_sin.sin_addr.s_addr = htonl(atol(hostIP)); // attention format
  lanceur_sin.sin_family = AF_INET;
  lanceur_sin.sin_port = htons(atoi(hostport)); // attention format

  if (-1 == connect(fd1, (struct sockaddr *)&lanceur_sin, sizeof(lanceur_sin))){
    perror("connect");
    exit(EXIT_FAILURE);
  }

  /*  Envoi du nom de machine au lanceur */
  if (-1 == gethostname(name, name_len)){
    perror("gethostname");
    exit(EXIT_FAILURE);
  }

  if (-1 == write(fd1, &name, name_len)) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  /*  Envoi du pid au lanceur */
  pid_t pid;
  pid = getpid();

  if (-1 == write(fd1, &pid, sizeof(pid_t))) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  /*  Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */
  int port_dsm;
  int prop_dsm = 0;
  int fd2;

  fd2 = creer_socket(prop_dsm, &port_dsm);
  if (-1 == fd2){
    fprintf(stdout, "Error in creer_socket\n");
    exit(EXIT_FAILURE);
  }
  /* Envoi du numero de port au lanceur */
  /* pour qu'il le propage à tous les autres */
  /* processus dsm */

  if (write(fd1, &port_dsm, sizeof(port_dsm)) == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  /* on execute la bonne commande */
  execvp(finalargs[0],finalargs);

  return EXIT_SUCCESS;
}
