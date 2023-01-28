#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int stringsum(char *s){
  int sum = 0;

  for(char *ny = s; *ny != 0; ny++){
    if(!isalpha(*ny)){
      return -1;
    }
    if(isspace(*ny)){
      continue;
    }
    sum += tolower(*ny) - 'a' + 1;
  }
  return sum;
}

int distance_between(char *s, char c){
  char *start = index(s,c); //finner første occurence av c
  char *end = rindex(s,c); //finner siste occurence av c
  return (!start) ? -1 : end - start;
}

char *string_between(char *s, char c){
  char *start = index(s,c);
  char *end = rindex(s,c);

  if(!start && !end){
    return NULL;
  }

  char *mem = malloc(end - start + 1);

  if(!mem){
    fprintf(stderr, "malloc failed\n");
    return NULL;
  }
  char *mem_p = mem; // for å ikke miste peker til mem
  for(char *p = start + 1; p < end; p++){
    *mem_p = *p;
    mem_p++;
  }
  *mem_p = 0;

  return mem;
}

int stringsum2(char *s, int *res){
  int i = stringsum(s);
  *res = i;
  return 0;
}
