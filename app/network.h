
#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <sys/types.h>
struct response_content {
  char *content;
  size_t size;
};

size_t write_handler_disk(void *contents, size_t size, size_t nitems,
                          FILE *file);
size_t write_handler_mem(void *data, size_t size, size_t nmemb, void *arg);
int download_file(char *uri, char *file, char *bearer_token);
char *get_response(char *uri, char *bearer_token);

#endif // !NETWORK_H
