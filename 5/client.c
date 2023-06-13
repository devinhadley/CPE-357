
/* client.c
 *
 * This file is responsible for handling the client side implementation of
 * mytalk. This program will create a socket and seek a connection with the
 * server. The chat functionality is the same as the servers.
 */

#include "Talk/include/talk.h"
#include "mytalk.h"
#include "server.h"
#include "shared.h"
#include <netdb.h>
#include <poll.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Populates a sockaddr_in struct with proper values and returns the struct. */
struct sockaddr_in create_socket_address(Flags *flags,
                                         struct hostent *hostent) {
  struct sockaddr_in sa;

  sa.sin_family = AF_INET;
  sa.sin_port = htons(flags->port);
  sa.sin_addr.s_addr = *(uint32_t *)hostent->h_addr_list[0];

  return sa;
}

/* Populates a sockaddr_in struct with proper values and returns the struct. */
int create_and_connect_socket(struct sockaddr_in sa) {
  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Couldnt create the client socket.\n");
    exit(EXIT_FAILURE);
  }

  if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    perror("Failed to connect client socket to the specified address.\n");
    exit(EXIT_FAILURE);
  }

  return sock;
}

/* Sends the username of the current uid to the server to adhere to the mytalk
 * protocol. */
void send_username_to_server(int sock) {

  struct passwd *pwd = getpwuid(getuid());
  char *username = pwd->pw_name;

  if (send(sock, username, strlen(username), 0) == -1) {
    perror("Failed to send username to the host.\n");
    exit(EXIT_FAILURE);
  }
}

/* Initializes the polling struct to prepare for running the chat. */
void initialize_polling_struct(struct pollfd *polls, int sock) {
  polls[LOCAL].fd = STDIN_FILENO;
  polls[LOCAL].events = POLLIN;
  polls[LOCAL].revents = 0;

  polls[REMOTE].fd = sock;
  polls[REMOTE].events = POLLIN;
  polls[REMOTE].revents = 0;
}

/* Exits if the server rejects our connection or the client failed to recieve
 * data. */
void check_server_response(int sock, char *hostname) {
  char buffer[BUFFER_SIZE];

  if (recv(sock, &buffer, sizeof("ok"), 0) == -1) {
    perror("Failed to receive server response when validating connection.\n");
    exit(EXIT_FAILURE);
  }

  if (strcmp((char *)buffer, "ok") != 0) {
    printf("%s declined connection.\n", hostname);
    exit(EXIT_SUCCESS);
  }
}

/* Creates a client instance of mytalk. */
void run_client(Flags *flags) {
  struct hostent *hostent;
  struct sockaddr_in sa;
  struct pollfd polls[2];
  int sock;

  set_verbosity(flags->verbosity);
  hostent = gethostbyname(flags->host_name);

  sa = create_socket_address(flags, hostent);
  sock = create_and_connect_socket(sa);

  send_username_to_server(sock);

  printf("Waiting for response from %s\n", flags->host_name);

  initialize_polling_struct(polls, sock);

  if (poll(&polls[REMOTE], 1, -1) == -1) {
    perror("Failed to poll remote.\n");
    exit(EXIT_FAILURE);
  }

  check_server_response(sock, flags->host_name);

  if (flags->use_ncurses) {
    start_windowing();
  }

  handle_chat(polls, sock);
}
