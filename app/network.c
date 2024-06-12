#include "network.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
size_t write_handler_mem(void *data, size_t size, size_t num_mem, void *arg);
size_t write_handler_disk(void *contents, size_t size, size_t num_items,
                          FILE *file);
char *get_response(char *uri, char *bearer_token) {
  // printf("Getting response from: %s with token: %s\n", uri, bearer_token);
  CURL *curl;
  char *content = NULL;
  char full_uri[200];
  strcpy(full_uri, uri);
  // Set the the response content
  struct response_content response;
  response.content = malloc(1);
  response.size = 0;
  // Initialize the curl handle
  curl = curl_easy_init();
  if (curl) {
    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, (void *)full_uri);
    // Set the response content
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_handler_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    // Set headers
    struct curl_slist *headers = NULL;
    if (bearer_token != NULL) {
      headers = curl_slist_append(headers, bearer_token);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    // Check for errors
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    } else {
      content = malloc(response.size + 1);
      strcpy(content, response.content);
    }
    // Clean up
    free(response.content);
    if (headers != NULL) {
      curl_slist_free_all(headers);
    }
    curl_easy_cleanup(curl);
  }
  return content;
}
int download_file(char *uri, char *file, char *bearer_token) {
  // printf("Download the docker image\n");
  int result = -1;
  FILE *file_ptr;
  CURL *curl = curl_easy_init();
  char full_uri[200];
  strcpy(full_uri, uri);
  if (curl) {
    if ((file_ptr = fopen(file, "wb")) == NULL) {
      fprintf(stderr, "Error opening file for writing\n");
      return -1;
    } else {
      curl_easy_setopt(curl, CURLOPT_URL, (void *)full_uri);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
      // Set headers
      struct curl_slist *headers = NULL;
      if (bearer_token != NULL) {
        headers = curl_slist_append(headers, bearer_token);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
      }
      // Retrieve the file from the URL
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_handler_disk);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)file_ptr);
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
      // Exam the return code
      CURLcode res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
      }
      // Clean up
      if (headers != NULL) {
        curl_slist_free_all(headers);
      }
      fflush(file_ptr);
      fclose(file_ptr);
    }
    curl_easy_cleanup(curl);
    result = 0;
  } else {
    fprintf(stderr, "Error initializing curl\n");
    result = -1;
  }
  return result;
}
size_t write_handler_mem(void *data, size_t size, size_t num_mem, void *arg) {
  size_t chunk_size = size * num_mem;
  struct response_content *response = (struct response_content *)arg;
  char *new_content =
      realloc(response->content, response->size + chunk_size + 1);
  if (new_content == NULL) {
    fprintf(stderr, "Failed to allocate memory\n");
    return 0;
  }
  response->content = new_content;
  memcpy(&(response->content[response->size]), data, chunk_size);
  response->size += chunk_size;
  response->content[response->size] = 0;
  return chunk_size;
}
size_t write_handler_disk(void *contents, size_t size, size_t num_items,
                          FILE *file) {
  return fwrite(contents, size, num_items, file);
}
