/*
 *File: hash.c
 *This file contains the implementation of a hash table with separate chaining.
 *The hash table is designed to store strings as keys and integers as values.
 *The hash function used is the djb2 hash function.
 *The hash table automatically resizes itself when the load factor exceeds 1.
 * This hash table is designed for positive values only.
 */

#include "hash.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *strdup(const char *string);

unsigned long hash_string(char *key) {
  unsigned long hash = 5381;
  int c;

  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

bool is_prime(int num) {
  int i;

  if (num <= 1) {
    return false;
  }

  for (i = 2; i * i <= num; i++) {
    if (num % i == 0) {
      return false;
    }
  }

  return true;
}

int next_prime_number(int num) {
  while (!is_prime(++num))
    ;
  /*do nothing */

  return num;
}

int hash_table_get(HashTable *table, char *key) {
  /*Returns -1 if not found since hash table only supports positive values */

  Entry *currentEntry;
  int index;

  index = hash_string(key) % table->size;
  currentEntry = table->entries[index];

  while (currentEntry != NULL) {
    if (strcmp(currentEntry->key, key) == 0) { /*element was found */
      return currentEntry->value;
    }

    currentEntry = currentEntry->next;
  }

  return -1;
}

void hash_table_add(HashTable **ptr_table, char *key, int value) {
  /*
   *Adds an element to the hash table.
   *
   *
   *The pointer may be modified during this function call.
   *Takes a double pointer to the table as it will automatically resize the
   *table if the load factor exceeds 1.
   *
   *
   */

  int index;
  Entry *newEntry;
  Entry *currentEntry;
  Entry *previousEntry;
  HashTable *table = *ptr_table;

  index = hash_string(key) % table->size;

  currentEntry = table->entries[index];

  /*Add the first entry */
  if (currentEntry == NULL) {
    if (!(newEntry = (Entry *)malloc(sizeof(Entry)))) {
      perror("failed malloc in hash_table_add when adding first entry");
      exit(EXIT_FAILURE);
    }

    newEntry->key = strdup(key);
    newEntry->value = value;
    newEntry->next = NULL;

    table->entries[index] = newEntry;
    table->num_entries++;

    return;
  }

  while (currentEntry != NULL) {
    /*Traverse the linked list checking for element */

    if (strcmp(currentEntry->key, key) == 0) {
      currentEntry->value = value;
      return;
    }

    previousEntry = currentEntry;
    currentEntry = currentEntry->next;
  }

  if (!(newEntry = (Entry *)malloc(sizeof(Entry)))) {
    perror(
        "failed malloc in hash_table_add when appending to end of linked list");
    exit(EXIT_FAILURE);
  }

  /*Create new element and append to end of linked list. */

  newEntry->key = strdup(key);
  newEntry->value = value;
  newEntry->next = NULL;

  previousEntry->next = newEntry;
  table->num_entries++;

  if (table->num_entries / (float)table->size > 1) {
    resize_hash_table(ptr_table);
  }
}

HashTable *create_hash_table(unsigned int size) {
  HashTable *table;

  if (!(table = (HashTable *)malloc(sizeof(HashTable)))) {
    perror("failed malloc when creating HashTable");
    exit(EXIT_FAILURE);
  }

  table->size = size;
  table->num_entries = 0;

  if (!(table->entries = (Entry **)calloc(size, sizeof(Entry *)))) {
    perror("failed malloc when creting entry pointers for table");
    exit(EXIT_FAILURE);
  }

  return table;
}

void resize_hash_table(HashTable **table) {
  /*
   *Allocates a new hash table, deallocates old, and sets table to newly
   *allocated adress.
   **/

  HashTable *new_table;
  unsigned int new_size;
  unsigned int i;
  Entry *current;
  Entry *temp;

  new_size = next_prime_number((*table)->size * 2);
  new_table = create_hash_table(new_size);
  for (i = 0; i < (*table)->size; i++) {
    current = (*table)->entries[i];
    while (current != NULL) {
      hash_table_add(&new_table, current->key, current->value);
      temp = current;
      current = current->next;
      free(temp->key);
      free(temp);
    }
  }

  free((*table)->entries);
  free(*table);

  *table = new_table;
}

void print_hash_table(HashTable *table) {
  int i;

  if (table == NULL) {
    fprintf(stderr, "Error: NULL hash table.\n");
    return;
  }

  for (i = 0; i < table->size; i++) {
    Entry *entry = table->entries[i];
    while (entry != NULL) {
      printf("Key: %s, Value: %d\n", entry->key, entry->value);
      entry = entry->next;
    }
  }
}

void hash_table_remove(HashTable *table, char *key) {
  /*
   *Parse the hash table, if head is item set head to next;
   *Traverse list, remove and set as needed.
   **/

  unsigned int index;
  Entry *current;
  Entry *previous;

  index = hash_string(key) % table->size;
  current = table->entries[index];

  /*The head is the item. */
  if (strcmp(current->key, key) == 0) {
    free(current->key);
    table->entries[index] = current->next;
    table->num_entries--;
    free(current);
    return;
  }

  /*Iterate through the linked list */
  previous = current;
  current = current->next;
  while (current) {
    /*If item found, set previous next to current next */
    if (strcmp(current->key, key) == 0) {
      free(current->key);
      previous->next = current->next;
      free(current);
      table->num_entries--;
      return;
    }

    previous = current;
    current = current->next;
  }

  /*Item not found (nothing to remove) */
  return;
}

int compare_entries(Entry *a, Entry *b) {
  /* Returns positive if a greater than b, negative if b greater than a.
   * If equal value, returns negative if a before b or positive if a after b.
   */

  if (a->value != b->value) {
    return a->value - b->value;
  }
  return strcmp(a->key, b->key);
}

Entry *get_max_entry(HashTable *table) {
  /*
   *Returns a copy of the max-valued entry.
   */

  Entry *current_entry;
  Entry *max_entry = NULL;
  Entry *entry_copy = NULL;
  int i;

  for (i = 0; i < table->size; i++) {
    current_entry = table->entries[i];
    while (current_entry != NULL) {
      if (max_entry == NULL || compare_entries(current_entry, max_entry) > 0) {
        max_entry = current_entry;
      }

      current_entry = current_entry->next;
    }
  }

  if (max_entry != NULL) {
    if (!(entry_copy = (Entry *)malloc(sizeof(Entry)))) {
      perror("failed malloc in get_max_entry");
      exit(EXIT_FAILURE);
    }

    entry_copy->key = strdup(max_entry->key);
    entry_copy->value = max_entry->value;
    entry_copy->next = NULL;
  }

  /*return null if the table is empty */
  return entry_copy;
}

void free_hash_table(HashTable *table) {
  unsigned int i;
  Entry *current;
  Entry *temp;

  for (i = 0; i < table->size; i++) {
    current = table->entries[i];
    while (current != NULL) {
      temp = current;
      current = current->next;
      free(temp->key);
      free(temp);
    }
  }

  free(table->entries);
  free(table);
}
