#include "fw.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>

#define HASH_STARTING_SIZE 5381

int main(int argc, char *argv[]) {
  int number_of_words = 10;
  int num_paths;
  int total_words;
  int i;
  char **paths;
  HashTable *table;
  Entry **top_n_entries;

  /* Set command line arguments */
  set_arguments(argc, argv, &number_of_words, &paths, &num_paths);

  /* Create and initialize the hash table */
  table = create_hash_table(HASH_STARTING_SIZE);

  /* Process standard input or file paths */
  if (num_paths == 0) {
    extract_words_from_stdin(&table);
  } else {
    for (i = 0; i < num_paths; i++) {
      extract_words_from_path(paths[i], &table);
    }
  }

  /* Calculate the total number of words in the table */
  total_words = table->num_entries;

  /* Get the top n entries */
  top_n_entries = get_top_n_entries(number_of_words, table);

  /* Display the top n entries */
  display_top_n_entries(number_of_words, total_words, top_n_entries);

  /* Free allocated memory */
  free_hash_table(table);
  free(top_n_entries);

  /* Exit the program */
  return 0;
}
