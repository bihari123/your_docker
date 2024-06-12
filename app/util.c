#include "util.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/**
 * Exec shell command to untar a file
 */
int untar(char *file, int remove, int suppress_output) {
  char *file_name = basename(file);
  char *path = dirname(file);
  // construct the command
  size_t size = strlen("cd ") + strlen(path) + strlen(" && tar -xf ") +
                strlen(file_name) + strlen(" > /dev/null") + strlen(" && rm ") +
                strlen(file_name);
  char *command = malloc(size + 1);
  strcpy(command, "cd ");
  strcat(command, path);
  strcat(command, " && tar -xvhzvf ");
  strcat(command, file_name);
  if (suppress_output) {
    strcat(command, " > /dev/null");
  }
  if (remove) {
    strcat(command, " && rm ");
    strcat(command, file_name);
  }
  // printf("tar command: %s\n", command);
  return system(command);
}
/**
 * Super-hacky way to create a folder - BEWARE!
 *
 */
int makedir(char *dir) {
  size_t size = strlen("mkdir -p ") + strlen(dir);
  char *command = malloc(size + 1);
  strcpy(command, "mkdir -p ");
  strcat(command, dir);
  return system(command);
}
