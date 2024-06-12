#include "docker_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int init_docker_image(char *image, char *dir) {
  // printf("Initializing docker image: %s\n", image);
  size_t size = 0;
  char *tag = NULL;
  char *p_start = strstr(image, ":");
  if (p_start != NULL) {
    size = strlen(image) - strlen(p_start);
    tag = malloc(size + 2);
    strncpy(tag, p_start + 1, size);
    char *temp = malloc(strlen(p_start) + 1);
    strncpy(temp, image, strlen(p_start) - 1);
    image = temp;
  } else {
    tag = malloc(strlen("latest") + 1);
    strcpy(tag, "latest");
  }
  // Get the token
  size = strlen("repository:library/") + strlen(image) + strlen(":") +
         strlen(tag) + strlen(",pull");
  char *scope = malloc(size + 1);
  strcpy(scope, "repository:library/");
  strcat(scope, image);
  strcat(scope, ":");
  strcat(scope, tag);
  strcat(scope, ",pull");
  char *token = NULL;
  if ((token = docker_registry_auth(scope)) == NULL) {
    free(scope);
    return -1;
  }
  // printf("Token: %s\n", token);
  // Pull the image layers
  char **layer_ids = docker_enumerate_layers(token, "library", image, tag);
  if (layer_ids == NULL) {
    if (tag != NULL)
      free(tag);
    free(token);
    return -1;
  }
  // Download the layer blobs
  int result = 0;
  int index = 0;
  while (1) {
    // printf("Downloading layer: %s\n", layer_ids[index]);
    char *id = layer_ids[index];
    if (id == NULL) {
      break;
    }
    if (docker_get_layer(token, dir, "library", image, id) != 0) {
      result = -1;
    }
    free(layer_ids[index]);
    index++;
  }
  if (tag != NULL)
    free(tag);
  free(layer_ids);
  free(scope);
  free(token);
  return result;
}
