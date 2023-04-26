/*
 * File: fw.h
 * This header file contains function prototypes and necessary includes
 * for the functions implemented in fw.c, which handles the command line
 * interface of fw, along with the retrieval and display of the top n words.
 * The program uses a positived-valued hash table to store the frequency of each
 * word.
 */

#ifndef FW_H
#define FW_H

#include "hash.h"
#include <stdbool.h>
#include <stdio.h>

/* Function prototypes */
bool is_valid_number(char *param);
void set_arguments(int argc, char *argv[], int *number_of_words, char ***paths,
                   int *num_paths);
char *read_next_word_lower(FILE *file);
void extract_words_from_file(char *file_name, HashTable **table);
Entry **get_top_n_entries(int n, HashTable *table);
void display_top_n_entries(int n, int total_words, Entry **top_n_entries);
void extract_words_from_stdin(HashTable **table);
void extract_words_from_path(char *path, HashTable **table);

#endif
