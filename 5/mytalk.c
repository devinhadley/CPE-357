/* mytalk.c
 *
 * This file is responsible for the retrieval of arguments and the validation of
 * ports. Depending on the user specified arguments it will either utilize
 * server.c or client.c.
 * */

#include "mytalk.h"
#include "client.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Initializes the flags to their default values.*/
void init_args(Flags *flags) {

  flags->host_name = NULL;
  flags->port = 0;
  flags->verbosity = 0;
  flags->use_ncurses = true;
  flags->auto_connect = false;
}

/* Returns -1 and prints error message if the passes port is not valid */
bool is_valid_port(Flags *flags, char *port) {
  char *strol_end = NULL;

  flags->port = strtol(port, &strol_end, 10);

  if (strol_end == port) {
    fprintf(stderr, "%s: port is not a number\n", port);
    return false;
  }

  if (*strol_end != '\0') {
    fprintf(stderr, "%s: malformed port\n", port);
    return false;
  }

  if (flags->port > PORT_MAX || flags->port < PORT_MIN) {
    fprintf(stderr, "Port not in range 0 - 65535 (inclusive)\n");
    return false;
  }

  return true;
}

/* Handles the cli of the program and either calls run_client or run_server. */
int main(int argc, char *argv[]) {
  int opt;
  int args_left;
  char *port_str;
  Flags args;
  init_args(&args);

  while ((opt = getopt(argc, argv, "vaN")) != -1) {
    switch (opt) {
    case 'v':
      args.verbosity++;
      break;
    case 'a':
      args.auto_connect = true;
      break;
    case 'N':
      args.use_ncurses = false;
      break;
    }
  }

  args_left = argc - optind;

  if (args_left == 1) {
    /* Handle the port (this is going to be a server) */
    port_str = argv[optind];

    if (!is_valid_port(&args, port_str)) {
      exit(EXIT_FAILURE);
    }

    run_server(&args);

  } else if (args_left == 2) {
    /* Handle the host name and port (this is going to be a client) */

    port_str = argv[optind + 1];

    if (!is_valid_port(&args, port_str)) {
      exit(EXIT_FAILURE);
    }

    args.host_name = argv[optind];
    run_client(&args);

  } else {
    fprintf(stderr, "usage: mytalk [-v] [-a] [-N] [hostname] port\n");
    exit(EXIT_SUCCESS);
  }

  return EXIT_SUCCESS;
}
