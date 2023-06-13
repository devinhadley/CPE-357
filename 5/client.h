

#ifndef CLIENT
#define CLIENT

#include "mytalk.h"
#include <netdb.h>
#include <poll.h>

void run_client(Flags *flags);
void check_server_response(int sock, char *hostname);
struct sockaddr_in create_socket_address(Flags *flags, struct hostent *hostent);
int create_and_connect_socket(struct sockaddr_in sa);
void send_username_to_server(int sock);
void initialize_polling_struct(struct pollfd *polls, int sock);

#endif
