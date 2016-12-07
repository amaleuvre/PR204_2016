#include "common_impl.h"

int main(int argc, char const *argv[]) {
  char * test1 = "toto";
  char * test2 = "toto\n";

  if (0 == strncmp(test1,test2, strlen(test1))) printf("ca marche\n");
  else printf("ca marche pas\n");
  return 0;
}
