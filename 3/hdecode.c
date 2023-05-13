/*
 * hdecode.c
 * It reads Huffman encoded data from an input file, decodes it, and writes the
 * decoded data to an output file.
 * The program reads the Huffman frequency table from the header of the input
 * file and uses it to build a Huffman tree. The encoded data is then read from
 * the file, decoded using the Huffman tree, and written to the output file.
 * The program handles input and output file errors, and also allows data to be
 * read from standard input and written to standard output.
 */

#include "huffman.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FREQUENCY_TABLE_SIZE 256
#define HEADER_BUFFER_SIZE 2048
#define READ_BUFFER_SIZE 4096
#define WRITE_BUFFER_SIZE 8192

/* mutates frequency_table to contain the frequencency of each byte */
int retrieve_table_from_header(unsigned int frequency_table[], int input_fd) {

  unsigned char buffer[HEADER_BUFFER_SIZE];
  int bytes_read;
  int buffer_offset = 0;
  int count;
  unsigned char current_byte;
  unsigned int current_count;
  int i;

  /* we know for certain a valid header wont be greater than HEADER_BUFFER */
  bytes_read = read(input_fd, buffer, HEADER_BUFFER_SIZE);

  if (bytes_read == -1) {
    perror("Error reading from input file");
    exit(EXIT_FAILURE);
  }

  if (bytes_read == 0) {
    return 0;
  }

  /* retrieve the number of unique bytes */
  memcpy(&count, buffer + buffer_offset, sizeof(unsigned char));
  buffer_offset += sizeof(unsigned char);
  count += 1;

  /* Populate the frequency table with each byte and the amount of times it
   * occurrs.*/
  for (i = 0; i < count; i++) {
    memcpy(&current_byte, buffer + buffer_offset, sizeof(unsigned char));
    buffer_offset += sizeof(unsigned char);

    memcpy(&current_count, buffer + buffer_offset, sizeof(unsigned int));
    buffer_offset += sizeof(unsigned int);
    current_count = ntohl(current_count);

    frequency_table[current_byte] = current_count;
  }

  return buffer_offset;
}

/* Returns false if not at leaf and true if at leaf. Mutates decoded_char to
 * contain the current byte if node is a leaf.*/
bool decode_bit(HuffmanNode **node, uint8_t bit, unsigned char *decoded_char) {
  if (node == NULL || *node == NULL) {
    return false;
  }

  if (bit == 0) {
    *node = (*node)->left;
  } else {
    *node = (*node)->right;
  }

  /* If node is a leaf */
  if (*node != NULL && (*node)->left == NULL && (*node)->right == NULL) {
    *decoded_char = (*node)->key;
    return true;
  }

  return false;
}

/* Parses the input file past the header, and converts every code back into its
 * corresponding byte and writes it to the output file. */
void decode_and_write(HuffmanNode **tree, int input_fd, int output_fd,
                      int header_offset, int num_bytes) {

  unsigned char buffer[READ_BUFFER_SIZE];
  unsigned char write_buffer[WRITE_BUFFER_SIZE];

  int bytes_read;
  int write_buffer_offset = 0;
  int i;

  unsigned char byte;
  HuffmanNode *current_node = *tree;
  int j;

  unsigned char bit;
  unsigned char decoded_char;

  unsigned int processed_bytes = 0;
  bool is_decoded;

  if (tree == NULL || *tree == NULL) {
    return;
  }
  /* handle the edge case if there is one character */
  if ((*tree)->left == NULL && (*tree)->right == NULL) {
    unsigned char single_char = (*tree)->key;
    int remaining_bytes = num_bytes;

    while (remaining_bytes > 0) {
      int bytes_to_write = remaining_bytes > WRITE_BUFFER_SIZE
                               ? WRITE_BUFFER_SIZE
                               : remaining_bytes;
      memset(write_buffer, single_char, bytes_to_write);
      if (write(output_fd, write_buffer, bytes_to_write) == -1) {
        perror("Failed to write to output file when handling one character");
        exit(EXIT_FAILURE);
      };
      remaining_bytes -= bytes_to_write;
    }
    return;
  }
  /* Skip the header */
  if (lseek(input_fd, header_offset, SEEK_SET) == -1) {
    perror("Error seeking input file.");
    exit(EXIT_FAILURE);
  }

  /* Convert the codes in the input file into their corresponding bytes. */
  while ((bytes_read = read(input_fd, buffer, READ_BUFFER_SIZE)) > 0) {
    for (i = 0; i < bytes_read; i++) {
      byte = buffer[i];

      for (j = 7; j >= 0; j--) {

        bit = (byte >> j) & 1;
        is_decoded = decode_bit(&current_node, bit, &decoded_char);

        if (is_decoded) {
          memcpy(write_buffer + write_buffer_offset, &decoded_char,
                 sizeof(char));
          write_buffer_offset += sizeof(char);
          processed_bytes += 1;

          if (write_buffer_offset == WRITE_BUFFER_SIZE) {
            if (write(output_fd, write_buffer,
                      sizeof(char) * write_buffer_offset) == -1) {
              perror("failed to write with max buffer when decoding");
              exit(EXIT_FAILURE);
            };
            write_buffer_offset = 0;
          }

          if (processed_bytes == num_bytes) {
            if (write(output_fd, write_buffer,
                      sizeof(char) * write_buffer_offset) == -1) {
              perror("Failed to write buffer when processing last byte.");
              exit(EXIT_FAILURE);
            }
            return;
          }
          current_node = *tree;
        }
      }
    }
  }

  if (bytes_read == -1) {
    perror("Failed to read input file when decoding");
    exit(EXIT_FAILURE);
  }

  if (write(output_fd, write_buffer, sizeof(char) * write_buffer_offset) ==
      -1) {
    perror("Failed to write when flushing decode write buffer");
    exit(EXIT_FAILURE);
  }
}

/* Returns the total amount of bytes in the frequency table */
unsigned int get_number_of_bytes(unsigned int frequency_table[]) {
  int i;
  unsigned int count = 0;
  for (i = 0; i < 256; i++) {
    if (frequency_table[i] != 0) {
      count += frequency_table[i];
    }
  }

  return count;
}

int main(int argc, char *argv[]) {

  int input_fd = 0;
  int output_fd = 1;
  unsigned int frequency_table[FREQUENCY_TABLE_SIZE] = {0};
  int header_offset;
  HuffmanNode *list = NULL;
  HuffmanNode *tree = NULL;
  unsigned int num_bytes;

  if (argc != 2 && argc != 3) {
    fprintf(stderr, "usage: hdecode [ ( infile | - ) [ outfile ] ]");
    exit(1);
  }

  /* Retrieve the input file. */
  if (strcmp(argv[1], "-") != 0) {
    input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
      fprintf(stderr, "Failed to open file: %s", argv[1]);
      exit(1);
    }
  }

  /* Retrieve the output file. */
  if (argc == 3) {
    output_fd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (output_fd == -1) {
      fprintf(stderr, "Failed to open file: %s", argv[2]);
      exit(1);
    }
  }

  header_offset = retrieve_table_from_header(frequency_table, input_fd);
  num_bytes = get_number_of_bytes(frequency_table);
  populate_linked_list(&list, frequency_table);
  populate_huffman_tree(&tree, &list);
  decode_and_write(&tree, input_fd, output_fd, header_offset, num_bytes);

  free_tree(tree);

  return 0;
}
