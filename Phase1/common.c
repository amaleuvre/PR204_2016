#include "common_impl.h"

int creer_socket(int prop, int *port_num){
  int fd = 0;

  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  /* renvoie le numero de descripteur */
  /* et modifie le parametre port_num */

  int sock;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(0);

  bind(sock, (struct sockaddr *)&sin, sizeof(sin));

  getsockname(fd, (struct sockaddr *)&sin, (socklen_t *)sizeof(sin));
  char port_num_temp[100]; // TODO changer le 100, pourquoi 100 ?
  sprintf(port_num_temp, "%d",ntohs(sin.sin_port));
  port_num = (int *)port_num_temp;
  return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
