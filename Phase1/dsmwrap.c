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
  int Taille_du_nom_de_la_machine = 100; // TODO pourquoi 100 ?
  char name[Taille_du_nom_de_la_machine];

  int i;
  for (i = 0; i< argc - 4; i++){
    finalargs[i] = argv[4 + i];
  }
  finalargs[argc-4] = NULL;

  /* creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */
  int port_avec_lanceur;
  int prop_avec_lanceur = 0;
  int fd1;

  fd1 = creer_socket(prop_avec_lanceur, &port_avec_lanceur);
  fprintf(stdout, "port avec lanceur : %i\n", port_avec_lanceur);

  /*  Envoi du nom de machine au lanceur */
  if (-1 == gethostname(name, Taille_du_nom_de_la_machine)){
    perror("gethostname");
    exit(EXIT_FAILURE);
  }


  /*  Envoi du pid au lanceur */

  /*  Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */
  int port_dsm;
  int prop_dsm = 0;
  int fd2;

  fd2 = creer_socket(prop_dsm, &port_dsm);
  fprintf(stdout, "port dsm : %i\n", port_dsm);
  /* Envoi du numero de port au lanceur */
  /* pour qu'il le propage à tous les autres */
  /* processus dsm */

  /* on execute la bonne commande */
  execvp(finalargs[0],finalargs);

  return EXIT_SUCCESS;
}
