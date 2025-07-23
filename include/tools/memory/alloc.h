/*
    *
    * alloc.h - General purpose memory allocator Header File
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/


#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "tools/memory/memory.h"

/* Initialize and allocate memory to be sub-allocated */
int initialize_memory(struct Memory *memory);

/* Allocate memory from the custom heap */
void *s_malloc(struct Memory *memory, size_t size);

/* Free a previously allocated block */
void s_free(struct Memory *memory, void *ptr);

/* Allocate zero-initialized memory */
void *s_calloc(struct Memory *memory, size_t nmemb, size_t size);

/* Resize an allocated block */
void *s_realloc(struct Memory *memory, void *ptr, size_t size);

/* DEBUG leak reporter */
void s_DEBUG_report_leaks(struct Memory *memory);

#endif
