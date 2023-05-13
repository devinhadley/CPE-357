#ifndef LINKED_LIST
#define LINKED_LIST

#include <stdlib.h>
typedef struct HuffmanNode {
  int key;
  int frequency;
  struct HuffmanNode *next;
  struct HuffmanNode *left;
  struct HuffmanNode *right;
} HuffmanNode;

void insert_node(HuffmanNode *prev, HuffmanNode *to_insert,
                 HuffmanNode **current);
HuffmanNode *insert_linked_list(HuffmanNode **head, int key, int frequency);
HuffmanNode *pop_linked_list(HuffmanNode **head);
void print_linked_list(HuffmanNode *head);
void populate_linked_list(HuffmanNode **list, unsigned int *frequency_table);
void populate_huffman_tree(HuffmanNode **tree, HuffmanNode **list);
void free_tree(HuffmanNode *node);
#endif
