/*

* File: fw.c
* Handles the command line interface of fw, along with the retrieval
* and display of the top n words.
* The program uses a hash table to store the frequency of each word.
* The program performs the following tasks:
* Parses command-line arguments to get the number of words to display and the
file paths.
* Reads words from the files, converting them to lowercase.
* Extracts words from the files and updates their frequency in a hash table.
* Retrieves the top n entries from the hash table.
* Displays the top n entries and their frequencies.
*/

#include "hash.h"
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

extern int getopt(int argc, char *const argv[], const char *optstring);
extern char *optarg;
extern int optind;

#define WORD_HUNK 100

bool is_valid_number(char *param) {
  /*
   * This function ensures that a given string is a valid number.
   * Also ensures the number is not negative as - is not digit.
   */
  int i;
  for (i = 0; param[i] != '\0'; i++) {
    if (!isdigit(param[i])) {
      return false;
    }
  }

  return true;
}

void set_arguments(int argc, char *argv[], int *number_of_words, char ***paths,
                   int *num_paths) {

  /*
   * This function retrieves the number of words expected and the paths
   * the user wants parsed and sets the corresponding parameters accordingly.
   */
  int opt;

  while ((opt = getopt(argc, argv, "n:")) != -1) {
    switch (opt) {
    case 'n':
      if (!is_valid_number(optarg)) {
        fprintf(stderr, "usage: fw [-n num] [file 1 [file 2 ...] ]\n");
        exit(1);
      }

      *number_of_words = atoi(optarg);
      break;
    default:
      fprintf(stderr, "usage: fw [-n num] [file 1 [file 2 ...] ]\n");
      exit(1);
    }
  }

  /*Need to support list of files */
  *num_paths = argc - optind;
  *paths = &argv[optind];
}

/*
 *This program reads lines of text from standard input, and outputs the lines
 *while filtering out consecutive duplicate lines. Only unique consecutive lines
 *are printed to standard output.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_next_word_lower(FILE *file) {
  /*
   * Reads the next word in a file.
   * Dynamically allocates a string and reurns the pointer.
   * Returns NULL when the end of the file is reached.
   * */
  int character;
  int length = 0;
  char *word = NULL;
  int reservedSize = 0;
  while ((character = fgetc(file)) != EOF) {
    /* If not a character, skip it */
    if (!isalpha(character)) {
      /* If a word is already being processed, stop reading */
      if (length > 0) {
        break;
      }

      continue;
    }

    /* If reserved size is equal to length, reallocate memory */
    if (reservedSize == length) {
      if (!(word = (char *)realloc(word, sizeof(char) *
                                             (reservedSize += WORD_HUNK)))) {
        perror("failed realloc when loading word");
        exit(EXIT_FAILURE);
      }
    }

    word[length++] = tolower(character);
  }

  if (length == 0 && character == EOF) {
    return NULL;
  }

  /* Reallocate memory for null terminator if necessary */
  if (length == reservedSize) {
    if (!(word = (char *)realloc(word, sizeof(char) * (reservedSize + 1)))) {
      perror("failed to realloc when allocating memory for null terminator");
      exit(EXIT_FAILURE);
    }
  }

  word[length] = '\0';

  return word;
}

void extract_words_from_file(char *file_name, HashTable **table) {
  /*
   * Parses a file and stores all of its words into a HashTable.
   */
  FILE *file;
  char *word;
  int value;

  file = fopen(file_name, "r");

  if (file == NULL) {
    fprintf(stderr, "%s: failed to open file\n", file_name);
    return;
  }

  while ((word = read_next_word_lower(file)) != NULL) {
    value = hash_table_get(*table, word);

    if (value == -1) {
      hash_table_add(table, word, 1);
      free(word);
      continue;
    }

    hash_table_add(table, word, value + 1);
    free(word);
  }

  fclose(file);
  return;
}

Entry **get_top_n_entries(int n, HashTable *table) {
  /*Returns top n entries from a HashTable.
   *THIS FUNCTION WILL MUTATE TABLE.
   * N entries are removed from the table if possible.
   **/

  Entry **top_n;
  Entry *current;
  int i;

  if (!(top_n = (Entry **)malloc(sizeof(Entry *) * n))) {
    perror("failed malloc in get_top_n_entries");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < n; i++) {
    current = get_max_entry(table);
    /*table is empty */
    if (current == NULL) {
      break;
    }

    hash_table_remove(table, current->key);
    top_n[i] = current;
  }

  return top_n;
}

void display_top_n_entries(int n, int total_words, Entry **top_n_entries) {
  /*
   * Takes a dynamically allocated list of entries and displays them.
   * This function expects that top_n_entries is already sorted.
   * */

  Entry *entry;
  int i;

  printf("The top %d words (out of %d) are:\n", n, total_words);

  if (n > total_words)
    n = total_words;

  for (i = 0; i < n; i++) {
    entry = top_n_entries[i];

    if (entry == NULL)
      break;

    printf("%9d %s\n", entry->value, entry->key);
    free(entry->key);
    free(entry);
  }
}

void extract_words_from_stdin(HashTable **table) {
  /*
   *  Stores the words extracted into stdin into HashTable parameter.
   */

  char *word;
  int value;

  while ((word = read_next_word_lower(stdin)) != NULL) {
    value = hash_table_get(*table, word);

    if (value == -1) {
      hash_table_add(table, word, 1);
      free(word);
      continue;
    }

    hash_table_add(table, word, value + 1);
    free(word);
  }

  return;
}

void extract_words_from_path(char *path, HashTable **table) {
  /*
   * Calls extract_words_from_path if the specified path is a file.
   *  Directries will be skipped, and outputted as such.
   *  Files will be processed if possible.
   */
  struct stat path_stat;
  int res = stat(path, &path_stat);

  if (res == -1) {
    perror(path);
    return;
  }

  if (S_ISREG(path_stat.st_mode)) {
    /*treat as file */
    extract_words_from_file(path, table);
  } else if (S_ISDIR(path_stat.st_mode)) {
    fprintf(stderr, "%s: is a directory not a file\n", path);
  }
}
