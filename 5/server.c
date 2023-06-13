
/* server.c
 *
 * This file is responsible for handling the client side implementation of
 * mytalk. This program will create a host socket, which will bind to the
 * user-specified port. The server will wait for a connection and accept it if
 * desired by the user. The chat functionality is shared with the client.
 */

#include "server.h"
#include "Talk/include/talk.h"
#include "mytalk.h"
#include "shared.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* Creates the server's listening socket. */
int create_socket() {
  int sock;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error\n");
    exit(EXIT_FAILURE);
  }
  return sock;
}

/* Binds the server's socket to a given port */
void bind_socket(int sock, int port) {
  struct sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock, (struct sockaddr *)&sa, sizeof(sa))) {
    perror("Couldn't bind to the port.\n");
    exit(EXIT_FAILURE);
  }
}

/* Accepts a connection and returns the accepted socket. */
int accept_connection(int sock, struct sockaddr_in *peerinfo,
                      unsigned int *peer_len) {
  int client_sock;

  if (listen(sock, 1)) {
    perror("Failed to listen for connection on the socket.\n");
    exit(EXIT_FAILURE);
  }

  if ((client_sock = accept(sock, (struct sockaddr *)peerinfo, peer_len)) ==
      -1) {
    perror("Failed to accept the client connection.");
    exit(EXIT_FAILURE);
  };

  return client_sock;
}

/* Initializes the pollfd structs to the proper values */
void initialize_polls(struct pollfd *polls, int client_sock) {

  polls[LOCAL].fd = STDIN_FILENO;
  polls[LOCAL].events = POLLIN;
  polls[LOCAL].revents = 0;

  polls[REMOTE].fd = client_sock;
  polls[REMOTE].events = POLLIN;
  polls[REMOTE].revents = 0;
}

/* Reads the username the client sends. This function expects the mytalk
 * protocol is being adhered to. */
void receive_username(int client_sock, char *remote_username, size_t size) {
  int read_length;

  if ((read_length = recv(client_sock, remote_username, size, 0)) == -1) {
    perror("Couldn't receive username from the client.");
    exit(EXIT_FAILURE);
  }

  remote_username[read_length] = '\0';
}

/*  Notifies the server of incoming connection, and handles the connection if
 * allowed by the user or auto accept is enabled. */
void handle_connection(int client_sock, Flags *flags, char *remote_username,
                       char *remote_address, int sock) {
  struct in_addr addr;
  struct hostent *host;

  if (inet_aton(remote_address, &addr) == 0) {
    perror("Invalid remote adress.");
    exit(EXIT_FAILURE);
  }

  host = gethostbyaddr(&addr, sizeof(addr), AF_INET);

  if (host == NULL) {
    perror("Failed to get host from remote address.");
    exit(EXIT_FAILURE);
  }

  printf("Mytalk request from %s@%s. Accept (y/n)?\n", remote_username,
         host->h_name);

  if (flags->auto_connect) {
    printf("Auto accepting....\n");
  }

  if (flags->auto_connect || tolower(getchar()) == 'y') {
    if (send(client_sock, "ok", sizeof("ok"), 0) == -1) {
      perror("Failed to send ok to client.\n");
      exit(EXIT_FAILURE);
    }
  } else {
    if (send(client_sock, DECLINE_STR, sizeof(DECLINE_STR), 0) == -1) {
      perror("Failed to send decline to the client.");
      exit(EXIT_FAILURE);
    }
    close(sock);
    close(client_sock);
    exit(EXIT_SUCCESS);
  }
}

/* Creates a server instance of mytalk. */
void run_server(Flags *flags) {
  struct sockaddr_in peerinfo;
  unsigned int peer_len = sizeof(peerinfo);
  char remote_address[INET_ADDRSTRLEN];
  char remote_username[BUFFER_SIZE];
  struct pollfd polls[2];

  set_verbosity(flags->verbosity);

  /* create the socket and wait for client connection */
  int sock = create_socket();
  bind_socket(sock, flags->port);
  int client_sock = accept_connection(sock, &peerinfo, &peer_len);

  /* Create polls for local and remote */
  initialize_polls(polls, client_sock);

  /* retrieve the adress in string representation */
  inet_ntop(AF_INET, &peerinfo.sin_addr.s_addr, remote_address,
            sizeof(remote_address));

  receive_username(client_sock, remote_username, sizeof(remote_username));

  handle_connection(client_sock, flags, remote_username, remote_address, sock);

  if (flags->use_ncurses) {
    start_windowing();
  }

  handle_chat(polls, client_sock);
  close(sock);
}
