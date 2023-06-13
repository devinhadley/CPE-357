#ifndef SERVER_H
#define SERVER_H

#include "mytalk.h"
#include <netinet/in.h>
#include <poll.h>

#define LOCAL 0
#define REMOTE 1
#define BUFFER_SIZE 4096
#define DECLINE_STR "You're not my friend."
#define CLOSE_MSG "Connection closed. ^C to terminate."

void handle_interrupt();

int create_socket();
void bind_socket(int sock, int port);
int accept_connection(int sock, struct sockaddr_in *peerinfo,
                      unsigned int *peer_len);
void initialize_polls(struct pollfd *polls, int client_sock);
void receive_username(int client_sock, char *remote_username, size_t size);
void handle_connection(int client_sock, Flags *flags, char *remote_username,
                       char *remote_address, int sock);

void run_server(Flags *flags);

#endif /* SERVER_H */
