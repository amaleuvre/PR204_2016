#include "common_impl.h"

int main(int argc, char const *argv[]) {
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

  char *name = NULL;
  size_t name_len = 0;
  ssize_t read;
  char *mach_array[nb_proc];
  int i;
  for(i=0; i < nb_line; i++){
    read = getline(&name, &name_len, fp);
    mach_array[i] = name;
  }
  free(name);
  int j;
  for (j=0; j< nb_line; j++) fprintf(stdout, "%s\n", mach_array[j]);
  return 0;
}
