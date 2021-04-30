#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "linked_list.h"

#define BUFFER_SIZE 1024
#define MAX_LINES 100

void init_list(list_t* list) {
    list->head = NULL;
    list->tail = NULL;
}

void add_to_list(list_t *list, char* inLine) {
    node_t* newNode = malloc(sizeof(node_t));
    newNode->line = strdup(inLine);
    if (list->head == NULL) {
        list->head = newNode;
        list->head->next = list->tail;
    } else if (list->tail == NULL) {
        list->head->next = newNode;
        list->tail = newNode;
        list->tail->next = NULL;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
        newNode->next = NULL;
    }
}

void print_list(list_t* list) {
    node_t* curr = list->head;
    int i = 1;
    while (curr != NULL) {
      printf("  %i: %s", i, curr->line);
      curr = curr->next;
      i++;
    }
}

void pop_node(list_t* list) {
    node_t* curr = list->head;
    if (curr != NULL) {
        list->head = curr->next;
        free(curr->line);
        free(curr);
    }
}

void free_list(list_t* list) {
    node_t* curr = list->head;
    while (curr != NULL) {
        pop_node(list);
        curr = list->head;
    }
    free(list);
}

int main(int argc, char *argv[]) {
  FILE *ifile = stdin;
  int line_count = 0;
  int max_lines = MAX_LINES;
  char buf[BUFFER_SIZE] = {0};
  char *file_name = NULL;
  int i;
  list_t* linkedList = malloc(sizeof(list_t));
  init_list(linkedList);

  //if command line has at least one parameter
  if (argc > 1) {
    for (i=1; i<argc; i++) {
      file_name = argv[i];
      ifile = fopen(file_name, "r");
      if (ifile == NULL) {
        perror("failed to open file");
        fprintf(stderr, " could not open file: %s\n", file_name);
      } else {
        line_count = 0;
        while ((line_count++ < max_lines) && (fgets(buf, BUFFER_SIZE, ifile) != NULL)) {
          add_to_list(linkedList, buf);
        }
        //printf("\n");
        fclose(ifile);
      }
    }
  }
  print_list(linkedList);
  free_list(linkedList);
  return(EXIT_SUCCESS);
}
