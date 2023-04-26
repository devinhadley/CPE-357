#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fw.h"
#include "hash.h"
#include "test.h"

void test_hash() {
  assert(hash_string("") == 5381);

  assert(hash_string("a") != hash_string("b"));

  assert(hash_string("hello") != hash_string("world"));

  assert(hash_string("aaaaa") != hash_string("bbbbb"));

  assert(hash_string("abcdef") != hash_string("ghijkl"));

  assert(hash_string("Hello!") == hash_string("Hello!"));
}

void test_hash_add_get() {
  char *key = "Hello";
  int value = 1;

  HashTable *table = create_hash_table(10);
  HashTable *table_of_one = create_hash_table(1);

  hash_table_add(&table, key, value);
  hash_table_add(&table, "Man", 25);
  assert(table->num_entries == 2);

  assert(hash_table_get(table, key) == value);
  assert(hash_table_get(table, "Man") == 25);

  /* Test if entry doesnt exist */

  assert(hash_table_get(table, "bruh") == -1);

  /* Test seperate chaining traversal */
  hash_table_add(&table_of_one, "1", 1);
  hash_table_add(&table_of_one, "2", 2);
  hash_table_add(&table_of_one, "3", 3);

  assert(hash_table_get(table_of_one, "1") == 1);
  assert(hash_table_get(table_of_one, "2") == 2);
  assert(hash_table_get(table_of_one, "3") == 3);
  assert(table_of_one->num_entries == 3);

  /* Test replacing existing element */
  hash_table_add(&table_of_one, "3", 4);
  assert(hash_table_get(table_of_one, "3") == 4);

  free_hash_table(table);
  free_hash_table(table_of_one);
}

void test_hash_create() {
  unsigned int test_size = 13;
  unsigned int i;

  HashTable *table = create_hash_table(13);

  assert(table != NULL);

  assert(table->size == test_size);
  assert(table->num_entries == 0);

  for (i = 0; i < test_size; i++) {
    assert(table->entries[i] == NULL);
  }

  free_hash_table(table);
}

void test_hash_remove() {

  HashTable *table = create_hash_table(3);

  hash_table_add(&table, "hello!", 1);
  hash_table_add(&table, "my", 2);
  hash_table_add(&table, "friend!", 3);

  hash_table_remove(table, "hello!");

  assert(table->size == 3);
  assert(table->num_entries == 2);
  assert(hash_table_get(table, "hello!") == -1);
  assert(hash_table_get(table, "my") == 2);
  assert(hash_table_get(table, "friend!") == 3);

  hash_table_remove(table, "friend");

  assert(hash_table_get(table, "friend") == -1);
  assert(hash_table_get(table, "my") == 2);

  free_hash_table(table);
}

void test_hash_resize() {

  /* test manual call of resize */
  HashTable *table = create_hash_table(3);

  hash_table_add(&table, "hello!", 1);
  hash_table_add(&table, "my", 2);
  hash_table_add(&table, "friend!", 3);

  resize_hash_table(&table);

  assert(table->size == 7);
  assert(table->num_entries == 3);
  assert(hash_table_get(table, "hello!") == 1);
  assert(hash_table_get(table, "my") == 2);
  assert(hash_table_get(table, "friend!") == 3);

  free_hash_table(table);

  /* Test auto resize */
  table = create_hash_table(3);

  hash_table_add(&table, "my", 1);
  hash_table_add(&table, "name", 2);
  hash_table_add(&table, "is", 3);
  hash_table_add(&table, "devin", 4);

  assert(table->num_entries == 4);
  assert(table->size == 7);
  assert(hash_table_get(table, "my") == 1);
  assert(hash_table_get(table, "name") == 2);
  assert(hash_table_get(table, "is") == 3);
  assert(hash_table_get(table, "devin") == 4);

  free_hash_table(table);
}

void test_extract_words_from_file() {
  char *path = "files/test_fw.txt";
  HashTable *table = create_hash_table(11);

  extract_words_from_path(path, &table);

  assert(hash_table_get(table, "hello") == 1);

  assert(hash_table_get(table, "devin") == 1);
  assert(hash_table_get(table, "my") == 2);

  free_hash_table(table);
}

void test_get_max_entry() {
  HashTable *table = create_hash_table(11);
  Entry *max_entry;

  hash_table_add(&table, "yeet", 1);
  hash_table_add(&table, "meat", 2);
  hash_table_add(&table, "sheet", 3);
  hash_table_add(&table, "feet", 4);
  hash_table_add(&table, "street", 5);

  max_entry = get_max_entry(table);

  assert(strcmp(max_entry->key, "street") == 0);
  assert(max_entry->value == 5);

  hash_table_remove(table, "street");
  assert(max_entry->value == 5);

  free(max_entry->key);
  free(max_entry);
  free_hash_table(table);
}

void test_get_top_n_entries() {

  int i;
  HashTable *table = create_hash_table(11);
  Entry **top_5_entries;

  hash_table_add(&table, "yeet", 1);
  hash_table_add(&table, "meat", 2);
  hash_table_add(&table, "sheet", 3);
  hash_table_add(&table, "feet", 4);
  hash_table_add(&table, "street", 5);

  top_5_entries = get_top_n_entries(5, table);

  assert(top_5_entries[0]->value == 5);
  assert(top_5_entries[1]->value == 4);
  assert(top_5_entries[2]->value == 3);
  assert(top_5_entries[3]->value == 2);
  assert(top_5_entries[4]->value == 1);

  assert(table->num_entries == 0);

  free_hash_table(table);
  for (i = 0; i < 5; i++) {

    free(top_5_entries[i]->key);
    free(top_5_entries[i]);
  }

  free(top_5_entries);
}

void test_fw() {
  test_extract_words_from_file();
  test_get_top_n_entries();
}

void test_hash_map() {
  test_hash_create();
  test_hash();
  test_hash_add_get();
  test_hash_resize();
  test_hash_remove();
  test_get_max_entry();
}

int main(void) {
  test_hash_map();
  test_fw();
  return 0;
}
