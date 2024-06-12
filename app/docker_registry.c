#include "docker_registry.h"
#include "network.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define DOCKER_REGISTRY_AUTH_URI                                               \
  "https://auth.docker.io/token?service=registry.docker.io"
#define DOCKER_REGISTRY_IMAGES_URI "https://registry.hub.docker.com/v2"
char *make_file_from_id(char *id);
char **parse_layers(char *response_content);
char *parse_token(char *response_content);
char *docker_registry_auth(char *scope) {
  // printf("Docker registry auth for scope: %s\n", scope);
  size_t len = strlen(DOCKER_REGISTRY_AUTH_URI) + strlen(scope);
  char *full_uri = malloc(len + 1);
  // form the image auth url
  strcpy(full_uri, DOCKER_REGISTRY_AUTH_URI);
  strcat(full_uri, "&scope=");
  strcat(full_uri, scope);
  char *content, *token;
  if ((content = get_response(full_uri, NULL)) != NULL) {
    token = parse_token(content);
    free(content);
  }
  free(full_uri);
  return token;
}
char **docker_enumerate_layers(char *token, char *repo, char *image,
                               char *tag) {
  // printf("Enumerating layers for image: %s\n", image);
  char **layer_ids = NULL;
  size_t len = strlen(DOCKER_REGISTRY_IMAGES_URI) + strlen(repo) +
               strlen(image) + strlen(tag) + strlen("////") +
               strlen("manifests");
  char *full_uri = malloc(len + 1);
  strcpy(full_uri, DOCKER_REGISTRY_IMAGES_URI);
  strcat(full_uri, "/");
  strcat(full_uri, repo);
  strcat(full_uri, "/");
  strcat(full_uri, image);
  strcat(full_uri, "/");
  strcat(full_uri, "manifests");
  strcat(full_uri, "/");
  strcat(full_uri, tag);
  // printf("Full URI: %s\n", full_uri);
  char *bearer_token = NULL;
  if (token != NULL) {
    len = strlen("Authorization: Bearer ") + strlen(token);
    bearer_token = malloc(len + 1);
    strcpy(bearer_token, "Authorization: Bearer ");
    strcat(bearer_token, token);
  }
  // printf("Bearer token: %s\n", bearer_token);
  char *content;
  int idx = 0;
  if ((content = get_response(full_uri, bearer_token)) != NULL) {
    // printf("Content: %s\n", content);
    layer_ids = parse_layers(content);
    // printf("Layer ids: %s\n", layer_ids[idx++]);
    free(content);
  }
  if (bearer_token != NULL) {
    free(bearer_token);
  }
  return layer_ids;
}
int docker_get_layer(char *token, char *dir, char *repo, char *image,
                     char *id) {
  // printf("Getting layer: %s\n", id);
  size_t len = strlen(DOCKER_REGISTRY_IMAGES_URI) + strlen(repo) +
               strlen(image) + strlen("blobs") + strlen(id) + strlen("////");
  char *full_uri = malloc(len + 1);
  strcpy(full_uri, DOCKER_REGISTRY_IMAGES_URI);
  strcat(full_uri, "/");
  strcat(full_uri, repo);
  strcat(full_uri, "/");
  strcat(full_uri, image);
  strcat(full_uri, "/");
  strcat(full_uri, "blobs");
  strcat(full_uri, "/");
  strcat(full_uri, id);
  char *file_name = make_file_from_id(id);
  len = strlen(dir) + strlen(file_name);
  char *file = malloc(len + 2);
  strcpy(file, dir);
  strcat(file, "/");
  strcat(file, file_name);
  len = strlen("Authorization: Bearer ") + strlen(token);
  char *bearer_token = malloc(len + 1);
  strcpy(bearer_token, "Authorization: Bearer ");
  strcat(bearer_token, token);
  if (download_file(full_uri, file, bearer_token) == -1) {
    perror("Error downloading file!\n");
    return -1;
  } else {
    untar(file, 1, 1);
  }
  free(file_name);
  free(full_uri);
  return 0;
}
char *make_file_from_id(char *id) {
  char *file = malloc(strlen(id) + 1);
  char *p = strstr(id, ":");
  strcpy(file, p + 1);
  return file;
}
char **add_string_to_array(char **array, int *size, char *string) {
  char **new_array = realloc(array, (*size + 1) * sizeof(char *));
  new_array[*size] = malloc(strlen(string) + 1);
  strcpy(new_array[*size], string);
  *size += 1;
  return new_array;
}
char **parse_layers(char *response_content) {
  // printf("Parsing layers, response_content: %s\n", response_content);
  if (response_content == NULL) {
    return NULL;
  }
  /* Bugs */
  char **list = NULL;
  int list_size = 0;
  while (1) {
    // Find the open segment
    char *pstart = strstr(response_content, "blobSum");
    if (pstart == NULL) {
      break;
    }
    // Find the close segment
    char *pend = strstr(pstart + 11, "\"");
    if (pend == NULL) {
      break;
    }
    int size = pend - pstart + 1;
    char *id = malloc(size + 1);
    memset(id, 0, size + 1);
    strncpy(id, pstart + 11, size - 12);
    list = add_string_to_array(list, &list_size, id);
    response_content = pend;
  }
  list = add_string_to_array(list, &list_size, "end");
  list[list_size - 1] = NULL;
  return list;
}
char *parse_token(char *response_content) {
  if (response_content == NULL) {
    return NULL;
  }
  // Find the token segment
  char *pstart = strstr(response_content, "token");
  if (pstart == NULL) {
    return NULL;
  }
  pstart += 8;
  char *pend = strstr(pstart, "\"");
  if (pend == NULL) {
    return NULL;
  }
  int size = pend - pstart;
  char *token = malloc(size + 1);
  strncpy(token, pstart, size);
  return token;
}
