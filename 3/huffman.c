/* huffman.c
 * This file is used in the construction of a huffman tree.
 * It defines functions to create an ordered link list of HuffmanNodes.
 * Also contains  functions which form a huffman tree based on an ordered linked
 * list of HuffmanNodes.
 */
#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>

/* Creates a huffman tree given an ordered list of huffman nodes.
 * Will empty list and populate tree.
 */
void populate_huffman_tree(HuffmanNode **tree, HuffmanNode **list) {
  HuffmanNode *current_node;
  HuffmanNode *next_node;
  HuffmanNode *newNode;

  if (tree == NULL || list == NULL || *list == NULL) {
    return;
  }

  /* Handle the case when there are only two nodes left in the list */
  if ((*list)->next && (*list)->next->next == NULL) {
    current_node = pop_linked_list(list);
    next_node = pop_linked_list(list);

    newNode = (HuffmanNode *)malloc(sizeof(HuffmanNode));
    newNode->key = -1;
    newNode->frequency = current_node->frequency + next_node->frequency;
    newNode->left = current_node;
    newNode->right = next_node;
    newNode->next = NULL;

    *tree = newNode;
    return;
  }

  while ((*list) && (*list)->next) {
    current_node = pop_linked_list(list);
    next_node = pop_linked_list(list);

    newNode = insert_linked_list(
        list, -1, current_node->frequency + next_node->frequency);

    newNode->left = current_node;
    newNode->right = next_node;
  }

  *tree = pop_linked_list(list);
}

/* Populates a linked list passed as argument list using the provided
 * frequency_table. */
void populate_linked_list(HuffmanNode **list, unsigned int *frequency_table) {
  int i;

  for (i = 0; i < 256; i++) {
    if (frequency_table[i] != 0) {
      insert_linked_list(list, i, frequency_table[i]);
    }
  }
}

/* Removes and returns the head of the linked list. */
HuffmanNode *pop_linked_list(HuffmanNode **head) {

  HuffmanNode *to_return;
  if (head == NULL || *head == NULL) {
    return NULL;
  }

  to_return = *head;
  *head = (*head)->next;

  return to_return;
}

/* Creates a new huffman node with key and frequency and inserts into the
 * passed linked list and returns the address of the newly created huffman
 * node.*/
HuffmanNode *insert_linked_list(HuffmanNode **head, int key, int frequency) {

  HuffmanNode *current = *head;
  HuffmanNode *to_insert = malloc(sizeof(HuffmanNode));
  HuffmanNode *prev = NULL;

  to_insert->key = key;
  to_insert->frequency = frequency;
  to_insert->next = NULL;
  to_insert->left = NULL;
  to_insert->right = NULL;

  /* if list empty */
  if (*head == NULL) {
    *head = to_insert;
    return to_insert;
  }

  /* iterate until spot found */
  while (current != NULL) {
    if (to_insert->frequency > current->frequency ||
        (to_insert->frequency == current->frequency &&
         to_insert->key > current->key)) {
      prev = current;
      current = current->next;
    } else {
      break;
    }
  }
  /* insert at beggining */
  if (prev == NULL) {
    to_insert->next = current;
    *head = to_insert;
    return to_insert;
  }

  prev->next = to_insert;
  to_insert->next = current;

  return to_insert;
}

/* Frees a HuffmanTree */
void free_tree(HuffmanNode *node) {
  if (node == NULL) {
    return;
  }

  /* Recursively free child nodes */
  free_tree(node->left);
  free_tree(node->right);

  /* Free current node */
  free(node);
}
