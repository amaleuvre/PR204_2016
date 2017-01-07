#include "common_impl.h"

int creer_socket(int prop, int *port_num){
  int fd = 0;

  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  /* renvoie le numero de descripteur */
  /* et modifie le parametre port_num */

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == fd){
    perror("socket");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(0);

  // setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable))

  if (-1 == bind(fd, (struct sockaddr *)&sin, sizeof(sin))){
    perror("bind");
    exit(EXIT_FAILURE);
  }
  int taille = sizeof(sin);
  if (-1 == getsockname(fd, (struct sockaddr *)&sin, (socklen_t *)&taille)){
    perror("getsockname");
    exit(EXIT_FAILURE);
  }
  *port_num = ntohs(sin.sin_port);
  printf("PORT : %i\n",*port_num);
  return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
