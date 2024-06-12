#ifndef DOCKER_REGISTRY_H
#define DOCKER_REGISTRY_H

#define DOCKER_REGISTRY_AUTH_URI                                               \
  "https://auth.docker.io/token?service=registry.docker.io"
#define DOCKER_REGISTRY_IMAGES_URI "https://registry.hub.docker.com/v2"
char *make_file_from_id(char *id);
char **parse_layers(char *response_content);
char *parse_token(char *response_content);
char *docker_registry_auth(char *scope);
char **docker_enumerate_layers(char *token, char *repo, char *image, char *tag);
int docker_get_layer(char *token, char *dir, char *repo, char *image, char *id);
char **add_string_to_array(char **array, int *size, char *string);

#endif // ! DOCKER_REGISTRY_H
