/*
 * bitwriter.c
 * This file abstracts the bitwriting process required by hendcode.c
 * It works by utilizing a struct which represents the bitwriter and performing
 * writes to the destination_fd defined in the struct using the buffer defined
 * in the struct.
 */
#include "bitwriter.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define HEADER_BUFFER_SIZE 2048

/* Initialize the BitWriter structure */
void bitwriter_init(BitWriter *bw, int destination_fd) {
  bw->buffer_position = 0;
  bw->destination_fd = destination_fd;
  bw->bit_count = 0;
  bw->byte = 0;
}

/* Write the buffer to the destination fd */
void bitwriter_write_buffer(BitWriter *bw) {
  ssize_t write_ret =
      write(bw->destination_fd, bw->buffer, bw->buffer_position);
  if (write_ret != (ssize_t)bw->buffer_position) {
    perror("Error writing to file when endoder writing buffer.");
    exit(EXIT_FAILURE);
  }
  bw->buffer_position = 0;
}

/* Write a Huffman code string. Will not write until buffer is full or write
 * buffer is called */
void bitwriter_write_huffman_code(BitWriter *bw, const char *code) {
  size_t code_len = strlen(code);
  size_t i;

  for (i = 0; i < code_len; i++) {
    bw->byte = (bw->byte << 1) | ((code[i] - '0') & 1);
    bw->bit_count++;

    if (bw->bit_count == 8) {
      bw->buffer[bw->buffer_position++] = bw->byte;
      bw->byte = 0;
      bw->bit_count = 0;

      if (bw->buffer_position == BUFFER_SIZE) {
        bitwriter_write_buffer(bw);
      }
    }
  }
}

/* Flush any remaining bits in the byte variable and pad with zeros if
 * necessary
 */
void bitwriter_flush(BitWriter *bw) {
  int padding_bits;
  if (bw->bit_count > 0) {
    padding_bits = 8 - bw->bit_count;
    bw->buffer[bw->buffer_position++] = (bw->byte << padding_bits) & 0xFF;
  }
  bitwriter_write_buffer(bw);
  bw->buffer_position = 0;
}

void bitwriter_translate_file(BitWriter *bw, int in_fd, char *codes[]) {
  /* Fills the frequency table argument with the frequencies found in the
   * provided file. */
  unsigned char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  size_t i;

  while ((bytes_read = read(in_fd, buffer, BUFFER_SIZE)) > 0) {
    for (i = 0; i < bytes_read; i++) {
      bitwriter_write_huffman_code(bw, codes[buffer[i]]);
    }
  }
  bitwriter_flush(bw);
}

/* Write the header containing the number of codes and the frequency table to
 * the file */
/* refactor this to write in one call */
void bitwriter_write_header(BitWriter *bw, int num_codes,
                            unsigned int frequency_table[]) {
  int i;
  unsigned int character_count;
  unsigned char buffer[2048];
  unsigned char cleaned_count;
  unsigned int buffer_offset = 0;
  int write_ret;

  if (num_codes == 0) {
    return;
  }

  num_codes -= 1;
  cleaned_count = (unsigned char)num_codes;

  memcpy(buffer + buffer_offset, &cleaned_count, sizeof(unsigned char));
  buffer_offset += sizeof(unsigned char);

  for (i = 0; i <= 255; i++) {
    character_count = htonl(frequency_table[i]);
    if (character_count != 0) {
      memcpy(buffer + buffer_offset, &i, sizeof(unsigned char));
      buffer_offset += sizeof(unsigned char);

      memcpy(buffer + buffer_offset, &character_count, sizeof(unsigned int));
      buffer_offset += sizeof(unsigned int);
    }
  }

  write_ret = write(bw->destination_fd, buffer, buffer_offset);
  if (write_ret != (ssize_t)buffer_offset) {
    perror("Error writing header to file.");
    exit(EXIT_FAILURE);
  }
}
