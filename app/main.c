#define _GNU_SOURCE
#include "init_docker_image.h"
#include "util.h"
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <sched.h>  // for clone
#include <signal.h> // for SIGCHLD
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#define BUFFER_SIZE 4096
char child_stack[1024 * 1024];
struct child_args {
  int *out_pipe;
  int *err_pipe;
  char *command;
  char **argv;
  char docker_image[PATH_MAX];
};
/*
        Copy files from the src to the tmp_dir, don't need at final stage
*/
int copy_files(char *src, char *dest) {
  FILE *src_files = fopen(src, "rb");
  FILE *dest_files = fopen(dest, "wb");
  // Print the source file names
  // printf("Source file: %s\n", src);
  if (src_files == NULL || dest_files == NULL) {
    perror("Error opening files!\n");
    return EXIT_FAILURE;
  }
  // Read the source file and write to the destination file
  char buffer[BUFFER_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_files)) > 0) {
    fwrite(buffer, 1, bytes_read, dest_files);
  }
  fclose(src_files);
  fclose(dest_files);
  chmod(dest, S_IRWXU); // Set the permission of the file
  return EXIT_SUCCESS;
}
int create_and_change_docker_directory(char *curr_dir, char *image) {
  // Create a temporary directory
  char dir_name[] = "/tmp/mydockerXXXXXX";
  char *tmp_dir = mkdtemp(dir_name);
  if (tmp_dir == NULL) {
    perror("Error creating temporary directory!\n");
    return EXIT_FAILURE;
  }
  // Initialize the docker image
  if (init_docker_image(image, tmp_dir) == -1) {
    perror("Error initializing docker image!\n");
    return EXIT_FAILURE;
  }
  // Get the destination path
  char *file_name = basename(curr_dir);
  char *dest_path = malloc(strlen(tmp_dir) + strlen(file_name) + 2);
  sprintf(dest_path, "%s/%s", tmp_dir, file_name);
  // The make_dir has some tricky behavior,
  // it will create /sh directory if the command has /sh
  // make_dir(dest_path);
  // We don't need to copy the files, because we download the docker image
  /*if (copy_files(curr_dir, dest_path) == EXIT_FAILURE) {
          perror("Error copying files!\n");
          return EXIT_FAILURE;
  }*/
  // Change the current directory to the temporary directory
  if (chdir(tmp_dir) == -1) {
    perror("Error changing directory!\n");
    return EXIT_FAILURE;
  }
  // Change the current root to the temporary directory using chroot
  char *new_dir = malloc(strlen(tmp_dir) + 2);
  getcwd(new_dir, strlen(tmp_dir) + 2);
  if (chroot(new_dir) != 0) {
    perror("Error changing root");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int child_function(void *arg) {
  struct child_args *args = (struct child_args *)arg;
  if (create_and_change_docker_directory(args->command, args->docker_image) ==
      EXIT_FAILURE) {
    perror("Error creating and changing docker directory!\n");
    return EXIT_FAILURE;
  }
  // Redirect the stdout and stderr
  dup2(args->out_pipe[1], STDOUT_FILENO);
  dup2(args->err_pipe[1], STDERR_FILENO);
  // Close the read end of the pipes
  close(args->out_pipe[0]);
  close(args->err_pipe[0]);
  /*printf("Executing %s\n",  (char*)args->command);
  int i = 0;
  while(args->argv[i] != NULL) {
          printf("Command %s\n", (char*)args->argv[i]);
          i++;
  }*/

  if (execv(args->command, args->argv) == -1) {
    perror("execv failed");
    return EXIT_FAILURE;
  }
}
int main(int argc, char *argv[]) {
  setbuf(stdout, NULL);
  char docker_image[PATH_MAX];
  char *command = argv[3];
  strcpy(docker_image, argv[2]);
  // Set the output and error pipes
  int out_pipe[2];
  int err_pipe[2];
  pipe(out_pipe);
  pipe(err_pipe);
  // Revise the argv for the child process
  int len = argc - 3 + 2;
  char **new_args = calloc(len, sizeof(char *));
  memcpy(new_args, &argv[3], (len - 1) * sizeof(char *));
  struct child_args args;

  args.out_pipe = out_pipe;
  args.err_pipe = err_pipe;
  args.command = command;
  args.argv = new_args;
  strcpy(args.docker_image, docker_image);
  // int child_pid = fork();
  int child_pid = clone(child_function, child_stack + (1024 * 1024),
                        CLONE_NEWPID | SIGCHLD, (void *)&args);
  if (child_pid == -1) {
    perror("Error forking!");
    return 1;
  }
  // Examines the exit status of the child process
  int status, exit_status;
  waitpid(child_pid, &status, 0);
  exit_status = WEXITSTATUS(status);
  close(out_pipe[1]);
  close(err_pipe[1]);
  // Read the output and error
  char out[BUFFER_SIZE];
  char err[BUFFER_SIZE];
  int out_bytes_read = read(out_pipe[0], out, sizeof(out));
  int err_bytes_read = read(err_pipe[0], err, sizeof(err));
  // Write the output and error
  if (out_bytes_read != -1) {
    out[out_bytes_read] = '\0';
    write(STDOUT_FILENO, out, out_bytes_read);
  }
  if (err_bytes_read != -1) {
    err[err_bytes_read] = '\0';
    write(STDERR_FILENO, err, err_bytes_read);
  }
  exit(exit_status);
  return EXIT_SUCCESS;
}
