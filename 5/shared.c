/* shared.c
 *
 * This file is responsible for holding the function handle_chat, which is
 * shared between client and server. The handle_chat is more of a state of which
 * client and server go into after a valid connection is achieved.
 *
 */

#include "shared.h"
#include "Talk/include/talk.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Puts the caller into a state of chat with the socket and also adheres to
 * mytalk protocols. */
void handle_chat(struct pollfd *polls, int sock) {
  char buffer[BUFFER_SIZE];
  int read_length;

  while (true) {
    if ((poll(polls, NUM_POLLS * sizeof(struct pollfd), -1)) == -1) {
      perror("Failed to poll after establishing connection with the client.\n");
      exit(EXIT_FAILURE);
    }
    /* Was stdin modified? */
    if (polls[LOCAL].revents & POLLIN) {
      update_input_buffer();
      if (has_whole_line()) {
        if ((read_length = read_from_input(buffer, sizeof(buffer))) > -1) {
          if (send(sock, buffer, read_length, 0) == -1) {
            perror("Failed to send input buffer to client.\n");
            exit(EXIT_FAILURE);
          }
          if (has_hit_eof()) {
            stop_windowing();
            close(sock);
            return;
          }
        } else if (read_length == -1) {
          fprintf(stderr, "Failed to read from stdin\n");
          exit(EXIT_FAILURE);
        }
      }
    }

    /* Was the socket modified? */
    if (polls[REMOTE].revents & POLLIN) {
      if ((read_length = recv(sock, buffer, sizeof(buffer), 0)) == -1) {
        fprintf(stderr, "Failed to read from remote socket\n");
        exit(EXIT_FAILURE);
      }
      if (read_length == 0) {

        if (write_to_output(CLOSE_MSG, sizeof(CLOSE_MSG) - 1) == ERR) {
          fprintf(stderr, "Couldn't write close message to output\n");
          exit(EXIT_FAILURE);
        }

        while (getchar() != EOF)
          /* nothing */;

        close(sock);
        stop_windowing();
        exit(EXIT_SUCCESS);

      } else {
        if (write_to_output(buffer, read_length) == ERR) {
          fprintf(stderr, "Couldn't write remote message.\n");
          exit(EXIT_FAILURE);
        }
      }
    }
  }
}
