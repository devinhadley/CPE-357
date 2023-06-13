#ifndef MYTALK
#define MYTALK

#include <stdbool.h>

#define PORT_MAX 65535
#define PORT_MIN 0

typedef struct {
  char *host_name;
  long port;
  int verbosity;
  bool use_ncurses;
  bool auto_connect;

} Flags;

void init_args(Flags *);
bool is_valid_port(Flags *flags, char *port);

#endif
