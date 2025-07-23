/*
    *
    * memory.h - Memory definitions header file
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdalign.h>

struct Block {
    size_t size;
    int free;
    struct Block *next;
    struct Block *prev;
    unsigned int magic;
};

struct Memory {
    void *memory;
    size_t mem_capacity;
    bool has_initialized;
    struct Block *free_list;
};

#define ALIGN(size) (((size) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))
#define BLOCK_SIZE sizeof(Block)
#define MIN_BLOCK_SIZE 16
#define BLOCK_MAGIC 0xDEADBEEF

#endif
