#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BLOCKSIZE 4096
static int entryCount = 0;

struct inode* create_file( struct inode* parent, char* name, char readonly, int size_in_bytes ){
  if(parent == NULL){
    return NULL;
  }
  //lager ny mappe og legger den i parents entries
  struct inode* ret = find_inode_by_name(parent,name);
  if(!ret){
    struct inode* nyNode;
    nyNode = malloc(sizeof(struct inode));
    if(nyNode == NULL){
      fprintf(stderr, "calloc failed");
      exit(EXIT_FAILURE);
    }
    nyNode->id = entryCount;
    entryCount++;
    nyNode->name = strdup(name);
    if(nyNode->name == NULL){
      fprintf(stderr, "strdup failed");
      exit(EXIT_FAILURE);
    }
    nyNode->is_directory = 0;
    nyNode->is_readonly = readonly;
    nyNode->filesize = size_in_bytes;
    nyNode->num_entries = (size_in_bytes + BLOCKSIZE - 1)/BLOCKSIZE;
    nyNode->entries = malloc(sizeof(size_t)*nyNode->num_entries);
    if(nyNode->entries == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    for(int i = 0; i < nyNode->num_entries; i++){
      nyNode->entries[i] = (size_t) allocate_block();
    }

    parent->entries = realloc(parent->entries,(parent->num_entries + 1)*sizeof(size_t));
    if(parent->entries == NULL){
      fprintf(stderr, "realloc failed");
      exit(EXIT_FAILURE);
    }
    parent->entries[parent->num_entries] = (size_t) nyNode;
    parent->num_entries += 1;

    return nyNode;
  }

  return NULL;

}

struct inode* create_dir( struct inode* parent, char* name ){

  //denne delen lager root hvis ikke denne finnes
  if(parent == NULL){
    struct inode* nyNode = malloc(sizeof(struct inode));
    if(nyNode == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    nyNode->id = entryCount;
    entryCount++;
    nyNode->name = strdup(name);
    if(nyNode->name == NULL){
      fprintf(stderr, "strdup failed");
      exit(EXIT_FAILURE);
    }
    nyNode->is_directory = 1;
    nyNode->is_readonly = 0;
    nyNode->filesize = 0;
    nyNode->num_entries = 0;
    nyNode->entries = malloc(sizeof(size_t));
    if(nyNode->entries == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    return nyNode;
  }

  struct inode* ret = find_inode_by_name(parent,name);
  //lager ny mappe og legger den i parents entries
  if(!ret){
    struct inode* nyNode;
    nyNode = malloc(sizeof(struct inode));
    if(nyNode == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    nyNode->id = entryCount;
    entryCount++;
    nyNode->name = strdup(name);
    nyNode->is_directory = 1;
    nyNode->is_readonly = 0;
    nyNode->filesize = 0;
    nyNode->num_entries = 0;
    nyNode->entries = malloc(sizeof(size_t));
    if(nyNode->entries == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }

    parent->entries = realloc(parent->entries,(parent->num_entries + 1)*sizeof(size_t));
    if(parent->entries == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    parent->entries[parent->num_entries] = (size_t) nyNode;
    parent->num_entries += 1;

    return nyNode;
  }

  return NULL;

}

//Finn inode ved navn
struct inode* find_inode_by_name(struct inode* parent, char* name){
  for(int i = 0; i < parent->num_entries; i++){
    struct inode* temp;
    size_t ent = parent->entries[i];
    temp = (struct inode*) ent;
    if(!strcmp(temp->name,name)){
      return temp;
    }
  }
  return NULL;
}

struct inode* load_inodes(){
  FILE* fp = fopen("superblock","r");
  if(!fp){
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  struct inode *root = malloc(sizeof(struct inode));
  if(root == NULL){
    fprintf(stderr,"malloc failed");
  }
  int id = 0;
  int name_length = 0;
  char *name = 0;
  char is_directory = 0;
  char is_readonly = 0;
  int filesize = 0;
  int num_entries = 0;
  size_t* entries = 0;

  fread(&id,sizeof(int),1,fp);
  fread(&name_length,sizeof(int),1,fp);
  name = malloc(name_length);
  if(name == NULL){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  fread(name,name_length,1,fp);
  fread(&is_directory,sizeof(char),1,fp);
  fread(&is_readonly,sizeof(char),1,fp);
  fread(&filesize,sizeof(int),1,fp);
  fread(&num_entries,sizeof(int),1,fp);
  entries = malloc(sizeof(size_t) * num_entries);
  fread(entries,sizeof(size_t),num_entries,fp);

  if(is_directory){

    if(root == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    root->id = id;
    root->name = name;
    root->is_directory = 1;
    root->is_readonly = 0;
    root->filesize = 0;
    root->num_entries = 0;
    root->entries = malloc(sizeof(size_t));
    if(root->entries == NULL){
      fprintf(stderr, "malloc failed");
      exit(EXIT_FAILURE);
    }
    for(int i = 0; i < num_entries; i++){
      loading_recursive(root,fp);
    }
    free(entries);
  }
  fclose(fp);
  return root;
}

void loading_recursive(struct inode* parent, FILE* fp){

  struct inode *nyNode = malloc(sizeof(struct inode));

  int id = 0;
  int name_length = 0;
  char* name;
  char is_directory = 0;
  char is_readonly = 0;
  int filesize = 0;
  int num_entries = 0;
  size_t* entries;

  fread(&id,sizeof(int),1,fp);
  fread(&name_length,sizeof(int),1,fp);
  name = malloc(name_length);
  if(name == NULL){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  fread(name,name_length,1,fp);
  fread(&is_directory,sizeof(char),1,fp);
  fread(&is_readonly,sizeof(char),1,fp);
  fread(&filesize,sizeof(int),1,fp);
  fread(&num_entries,sizeof(int),1,fp);
  entries = malloc(num_entries*sizeof(size_t));
  if(entries == NULL){
    fprintf(stderr, "malloc failed");
    exit(EXIT_FAILURE);
  }
  fread(entries,sizeof(size_t),num_entries,fp);

  if(is_directory){
    struct inode* ret = find_inode_by_name(parent,name);
    if(!ret){
      if(nyNode == NULL){
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
      }
      nyNode->id = id;
      nyNode->name = name;
      nyNode->is_directory = 1;
      nyNode->is_readonly = 0;
      nyNode->filesize = 0;
      nyNode->num_entries = 0;
      nyNode->entries = malloc(sizeof(size_t));
      if(nyNode->entries == NULL){
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
      }
      parent->entries = realloc(parent->entries,(parent->num_entries + 1)*sizeof(size_t));
      parent->entries[parent->num_entries] = (size_t) nyNode;
      parent->num_entries += 1;

      for(int i = 0; i < num_entries; i++){
        loading_recursive(nyNode,fp);
      }
    }
    free(entries);
  }

  else{ //hvis fil
    struct inode* ret = find_inode_by_name(parent,name);
    if(!ret){
      if(nyNode == NULL){
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
      }
      nyNode->id = id;
      nyNode->name = strdup(name);
      free(name);
      nyNode->is_directory = 0;
      nyNode->is_readonly = is_readonly;
      nyNode->filesize = filesize;
      nyNode->num_entries = num_entries;
      nyNode->entries = entries;

      parent->entries = realloc(parent->entries,(parent->num_entries + 1)*sizeof(size_t));
      parent->entries[parent->num_entries] = (size_t) nyNode;
      parent->num_entries += 1;
    }
  }
}

void fs_shutdown(struct inode* parent){

  int num_ent = parent->num_entries;

  if(parent->is_directory){
    for(int i = 0; i < num_ent; i++){
      size_t address = parent->entries[i];
      struct inode* child = (struct inode*) address;
      fs_shutdown(child);
    }
  }

  free(parent->name);
  free(parent->entries);
  free(parent);
}


/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

void debug_fs( struct inode* node )
{
  //printf("kjører debug");
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");
    if( node->is_directory )
    {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_entries; i++ )
        {
            struct inode* child = (struct inode*)node->entries[i];
            debug_fs( child );
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_entries; i++ )
        {
            printf("%d ", (int)node->entries[i]);
        }
        printf(")\n");
    }
}
