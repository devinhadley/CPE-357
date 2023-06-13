
#ifndef SHARED
#define SHARED
#define NUM_POLLS 2

#include <poll.h>

void handle_chat(struct pollfd *polls, int sock);

#endif
