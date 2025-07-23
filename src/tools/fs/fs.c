/*
    *
    * fs.c - File related actions and utilities
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "tools/fs/fs.h"

/* Read given file and write contents to a specified buffer */
uint8_t* read_file_to_buffer(const char* filepath, size_t* out_size) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    uint8_t* buffer = malloc(size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, size, f);
    fclose(f);

    *out_size = size;
    return buffer;
}

/* Read given file and return a char buffer with the contents */
char* read_file_to_string(const char* filepath) {
  FILE* file = fopen(filepath,"r"); 

  if (file == NULL) {
      perror("Error opening file");
      return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buffer = (char*)malloc(file_size + 1);
  if (buffer == NULL) {
      perror("Error allocating memory");
      fclose(file);
      return NULL;
  }

  fread(buffer, 1, file_size, file);
  buffer[file_size] = '\0';

  fclose(file);
  return buffer;
}

/* Write to a specified filepath */
int write_file(const char *filepath, char *contents) {
    return 0;
}
