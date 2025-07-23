/*
    *
    * fs.h - Header file for related actions and utilities
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <stdint.h>

/* Read given file and write contents to a specified buffer */
uint8_t* read_file_to_buffer(const char* filepath, size_t* out_size);

/* Read given file and return a char buffer with the contents */
char* read_file_to_string(const char* filepath);

/* Write to a specified filepath */
int write_file(const char *filepath, char *contents);

#endif
