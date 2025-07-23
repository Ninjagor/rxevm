/*
    *
    * dynbuf.h - Dynamic buffer header file
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef DYNBUFFER_H
#define DYNBUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t* data;
    size_t size;
    size_t capacity;
} DynBuffer;

void initBuffer(DynBuffer* buf, size_t initial_capacity);
void freeBuffer(DynBuffer* buf);
void appendWord(DynBuffer* buf, uint16_t word);
void appendWords(DynBuffer* buf, const uint16_t* words, size_t len);

#endif
