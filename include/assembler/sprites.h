/*
    *
    * sprites.h - Header file for sprite tables
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef SPRITES_H
#define SPRITES_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint8_t** sprites;
    size_t count;
    size_t capacity;
} SpriteTable;

void init_sprite_table(SpriteTable* st);
int add_sprite(SpriteTable* st, uint8_t* sprite_data);
void free_sprite_table(SpriteTable* st);
void write_sprite_table(FILE* f, SpriteTable* st);

#endif
