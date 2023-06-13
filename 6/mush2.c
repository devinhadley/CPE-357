/* mush2.c
 *
 * Mush2 is a minimally usefull shell which supports piping, i/o redirection,
 * and a built in cd command. mush2.c contains the code focused on process
 * execution, cd, and i/o redirection. Parsing is handeled by libmush.
 * */

#include "libmush/include/mush.h"
#include "stdio.h"
#include "stdlib.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define RWALL 0666

void sig_int_handler(int signum) {
  printf("\n");
  return;
}

/* Children should not inherit the sigint handler */
void clear_sigint_handler() {

  struct sigaction sa_clear;
  sa_clear.sa_handler = SIG_DFL;
  sigemptyset(&sa_clear.sa_mask);
  sa_clear.sa_flags = 0;

  if (sigaction(SIGINT, &sa_clear, NULL) == -1) {
    perror("Failed to clear signal handler");
    exit(EXIT_FAILURE);
  }
}

/* Mush2 should handle SIGINT gracefully and not exit. */
void initialize_sigint_handler() {

  struct sigaction sa;

  sa.sa_handler = sig_int_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("Failed to register signal handler for sigint");
    exit(EXIT_FAILURE);
  }
}

/* Displays the terminal promot if the input and stdout point to a valid
 * terminal interface. */
void display_prompt(FILE *input_file) {
  if (isatty(fileno(input_file)) && isatty(fileno(stdout))) {
    printf("8-P ");
    fflush(stdout);
  }
}

/* Returns the input file based on the user input, if the user doesnt specify
 * the file this function returns stdin. */
FILE *get_input_file(int argc, char *argv[]) {

  FILE *input_file = stdin;
  struct stat path_stat;

  /* was a file path specified? */
  if (argc == 2) {

    if (stat(argv[1], &path_stat) == -1) {
      perror("failed to stat path");
      exit(EXIT_FAILURE);
    }

    if (!S_ISREG(path_stat.st_mode)) {
      fprintf(stderr, "%s: is not a file.", argv[1]);
      exit(EXIT_FAILURE);
    }

    if ((input_file = fopen(argv[1], "r")) == NULL) {
      perror("Failed to open input file");
      exit(EXIT_FAILURE);
    }
  } else if (argc == 1) {
    return stdin;
  } else {
    fprintf(stderr, "usage: mush [input file]");
    exit(EXIT_FAILURE);
  }
  return input_file;
}

/* This function retrieves the current pipeline */
pipeline parse_pipeline(FILE *input_file) {
  char *current_line = readLongString(input_file);
  pipeline current_pipeline;

  if (current_line == NULL) {
    return NULL;
  }

  clerror = 0;
  current_pipeline = crack_pipeline(current_line);
  if (clerror) {
    free_pipeline(current_pipeline);
    free(current_line);
    return NULL;
  }

  free(current_line);
  return current_pipeline;
}

/* This function creates the pipes needed by the child processes. */
int **create_pipes(int length) {

  int i;
  int **pipes = malloc(length * sizeof(int *));

  if (pipes == NULL) {
    perror("Failed to malloc space needed for pipes.");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < length; i++) {
    pipes[i] = malloc(2 * sizeof(int));

    if (pipes[i] == NULL) {
      perror("Failed to malloc space for individual pipe.");
      exit(EXIT_FAILURE);
    }

    if (pipe(pipes[i]) == -1) {
      perror("Failed to create pipe.");
      exit(EXIT_FAILURE);
    }
  }

  return pipes;
}

/* Creates the pids that will be used to track the children. */
pid_t *create_children_pids(int length) {
  /* Allocate the space needed for the children pids */
  pid_t *children_pids = (pid_t *)malloc(sizeof(pid_t) * length);
  if (children_pids == NULL) {
    perror("Failed to malloc space needed for children pids.");
    exit(EXIT_FAILURE);
  }
  return children_pids;
}

/* Closes and frees all the pipes. */
void close_all_pipes(int **pipes, int length) {
  int i;

  for (i = 0; i < length; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
    free(pipes[i]);
  }
  free(pipes);
}

/* Creates a child process, dups pipe to stdin/stdout if needed, then closes all
 * pipes.  */
void create_child_process(int **pipes, pid_t *children_pids, int i,
                          pipeline current_pipeline) {
  int fd;

  /* create the child process */
  if ((children_pids[i] = fork()) == -1) {
    perror("failed to fork.");
    exit(EXIT_FAILURE);
  }

  /* is this a child instance? */
  if (children_pids[i] == 0) {
    /* we now need to change this processes stdin/out to the pipeline spec */

    /* Children should not handle sigint */
    clear_sigint_handler();

    /* Change the subprocesses stdin to the pipe specification */
    if (current_pipeline->stage->inname == NULL) {
      /* Pipe should remain standard in. */
      if (i != 0) {
        /* stdin becomes the stdout of the pipe before it */
        if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
          perror("Failed to dup2 pipe to stdin.");
          exit(EXIT_FAILURE);
        }
      }
    } else {
      /* Handle file redirection */
      fd = open(current_pipeline->stage->inname, O_RDONLY);

      if (fd == -1) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
      }

      /* Set the file descriptor to be the stdin descriptor */
      if (dup2(fd, STDIN_FILENO) == -1) {
        perror("Failed to dup2 input file to stdin.");
        exit(EXIT_FAILURE);
      }

      close(fd);
    }

    if (current_pipeline->stage->outname == NULL) {
      if (i != (current_pipeline->length - 1)) {
        /* stdout becomes the input of the next pipe */
        if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
          perror("Failed to dup2 pipe to stdin.");
          exit(EXIT_FAILURE);
        }
      }
    } else {

      fd = open(current_pipeline->stage->outname, O_CREAT | O_TRUNC | O_WRONLY,
                RWALL);

      if (fd == -1) {
        perror("Failed to open ouput file");
        exit(EXIT_FAILURE);
      }

      chmod(current_pipeline->stage->outname, RWALL);

      /* Set the file descriptor to be the stdin descriptor */
      if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("Failed to dup2 input file to stdin.");
        exit(EXIT_FAILURE);
      }

      close(fd);
    }

    /* we can close all other pipes since we duped the ones we needed */
    close_all_pipes(pipes, current_pipeline->length - 1);

    /* Pipes have been configured as needed, time to exec. */
    execvp(current_pipeline->stage[i].argv[0], current_pipeline->stage[i].argv);

    /* The child should termianate after execvp, if we reach here execvp failed.
     */

    perror(current_pipeline->stage[i].argv[0]);

    exit(errno);
  }
}

/* Function exits when all children have terminated. If a child fails it will
 * print an error message, and exit.*/
void wait_for_children(int length, pid_t *child_pids) {
  /* Wait until all children are finished. */
  int i;

  for (i = 0; i < (length); i++) {
    if (waitpid(child_pids[i], NULL, 0) == -1) {
      if (errno != EINTR) {
        perror("Failed to wait for child");
        return;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *input_file = get_input_file(argc, argv);
  int **pipes;
  pid_t *children_pids;
  pipeline current_pipeline;
  int i;

  initialize_sigint_handler();

  /* Run until EOF encountered in the input stream */
  while (!feof(input_file)) {

    display_prompt(input_file);

    /* Parse the pipeline. */
    current_pipeline = parse_pipeline(input_file);

    if (current_pipeline == NULL) {
      continue;
    }

    if (strcmp(current_pipeline->stage->argv[0], "cd") == 0) {
      /* Change directories to the user specified dir */
      if (chdir(current_pipeline->stage->argv[1]) == -1) {
        perror("cd");
      }
      free_pipeline(current_pipeline);
      continue;
    }

    pipes = create_pipes(current_pipeline->length - 1);
    children_pids = create_children_pids(current_pipeline->length);

    for (i = 0; i < current_pipeline->length; i++) {
      create_child_process(pipes, children_pids, i, current_pipeline);
    }

    close_all_pipes(pipes, current_pipeline->length - 1);

    wait_for_children(current_pipeline->length, children_pids);

    free_pipeline(current_pipeline);
    free(children_pids);
  }

  yylex_destroy();
  fclose(input_file);

  return 0;
}
