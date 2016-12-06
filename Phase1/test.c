#include "common_impl.h"

int main(int argc, char const *argv[]) {
  char * newargv[1];
  newargv[0] = "truc";
  execvp("./bin/truc", newargv);
  printf("ca marche pas\n");
  return 0;
}
