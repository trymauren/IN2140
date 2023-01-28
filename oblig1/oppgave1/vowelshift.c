#include <stdio.h>

int main(int argc, char *argv[]){
  if(argc < 3){
    printf("Too few arguments");
  }
  for(int i = 1; i < argc - 1; i++){
    char *word = argv[i];
    for(int k = 0; word[k] != '\0'; k++){
        if(word[k]=='a' || word[k]=='e' || word[k]=='i' || word[k]=='o' || word[k]=='u'){
          word[k] = *argv[argc - 1];
        }
    }
    printf("%s ",word);
  }
  printf("\n");
  return 0;
}
