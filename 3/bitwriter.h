#ifndef BITWRITER_H
#define BITWRITER_H

#include <stdint.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

typedef struct {
  uint8_t buffer[BUFFER_SIZE];
  size_t buffer_position;
  int destination_fd;
  int bit_count;
  uint8_t byte;
} BitWriter;

void bitwriter_init(BitWriter *bw, int destination_fd);
void bitwriter_write_buffer(BitWriter *bw);
void bitwriter_write_huffman_code(BitWriter *bw, const char *code);
void bitwriter_flush(BitWriter *bw);
void bitwriter_write_header(BitWriter *bw, int num_codes,
                            unsigned int frequency_table[]);
void bitwriter_translate_file(BitWriter *bw, int in_fd, char *codes[]);
#endif
