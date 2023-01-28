#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "the_apple.h"

int locateworm(char* apple);
int removeworm(char *apple);

int main(void){
  locateworm(apple);
  removeworm(apple);
  return 0;
}

int locateworm(char* apple){
  int i = 0;
  while(*apple != 'w' && *apple != '\0'){
    i++;
    apple++;
  }
  return i;
}
int removeworm(char *apple){
  int offset = locateworm(apple);
  char *newapple = strdup(apple) + offset;

  int count = 0;

  while(*newapple != '\0'){
    if(*newapple == 'w' || *newapple == 'o' || *newapple == 'r' || *newapple == 'm'){
      *newapple = ' ';
    }
    else{
      break;
    }
    newapple++;
    count++;
  }
  free(newapple - offset - count);


  return count;
}
