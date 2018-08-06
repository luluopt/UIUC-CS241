#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int main(int argc, char const *argv[]) {
  char * str = (char*)malloc(4);
  *str = "big";
  char *a = strtok(str, ",");
  a = strtok(NULL, ",");
  printf("%s", a);
  return 0;
}
