/*
 * hash.h
 *
 * This header file contains the declarations of a hash table with separate
 * chaining. The hash table is designed to store strings as keys and integers as
 * values. The hash function used is the djb2 hash function. This hash table is
 * designed to only store positive values. This table also supports custom
 * functionality for retrieving a copy of the max value entry.
 *
 * The hash table automatically resizes itself when the load factor exceeds 1.
 */

#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stddef.h> /* For size_t */

/* Structure definition for Entry */
typedef struct Entry {
  char *key;
  int value;
  struct Entry *next;
} Entry;

/* Structure definition for HashTable */
typedef struct HashTable {
  unsigned int size;
  unsigned int num_entries;
  Entry **entries;
} HashTable;

/* Function prototypes */
HashTable *create_hash_table(unsigned int size);
void hash_table_add(HashTable **table, char *key, int value);
int hash_table_get(HashTable *table, char *key);
unsigned long hash_string(char *key);
bool is_prime(int num);
int next_prime_number(int num);
void resize_hash_table(HashTable **table);
void free_hash_table(HashTable *table);
void print_hash_table(HashTable *table);
void hash_table_remove(HashTable *table, char *key);
Entry *get_max_entry(HashTable *table);

#endif
