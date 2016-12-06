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
  int i;
  for (i = 0; i< argc - 4; i++){
    finalargs[i] = argv[4 + i];
  }
  finalargs[argc-4] = NULL;


  /*  creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */

  /*  Envoi du nom de machine au lanceur */

  /*  Envoi du pid au lanceur */

  /*  Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */

  /* Envoi du numero de port au lanceur */
  /* pour qu'il le propage à tous les autres */
  /* processus dsm */

  /* on execute la bonne commande */
  execvp(finalargs[0],finalargs);
  
  return 0;
}
