#include "part3-utils.h"

char xor_talon(char c){
  return c ^ 5;
}

char id_talon(char c){
  return c;
}

void vector_print(printer_type printer, vector* v){
  printf("[");
  VEC_FOR_EACH(v, {printer(elem); if(it+1!=iend) printf(", ");});
  printf("]\n");
}

void dragon_printer(void *elem){
  dragon *d = (dragon*)elem;
  printf("(%s)", d->name);
}

void string_printer(void *elem){
  printf("%s", (char*)elem);
}

int rand_range(int low, int high){
  assert(high > low);
  int length = high - low + 1;
  return rand() % length + low;
}

char *gen_new_dragon_name(int length){
  char *name = malloc(6);
  name[5] = 0;
  name[0] = (char)rand_range(65, 90); // uppercase first letter
  for(int i = 1; i < length; i++){
    name[i] = (char)rand_range(97, 122); // lowercase other letters
  }
  return name;
}

char *readfile(char *filename){
  FILE *f = fopen(filename, "r");

  char *buff = malloc(1);
  buff[0] = 0;
  size_t size = 1;

  char *line = NULL;
  size_t n = 0;

  while(getline(&line, &n, f) > 0){
    size += strlen(line);
    buff = realloc(buff, size);
    strcat(buff, line);
    free(line);
    line = NULL;
    n = 0;
  }
  if(line)
    free(line);
  fclose(f);

  return buff;
}
