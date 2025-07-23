/*
    *
    * alloc.c - General purpose memory allocator for RXEVM
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#include "tools/memory/memory.h"
#include "tools/memory/alloc.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* malloc vm memory to initialize */
int initialize_memory(struct Memory *memory)
{
    if (memory->has_initialized)
        return 0;

    memory->memory = malloc(VM_MEMORY_CAPACITY);
    if (memory->memory == NULL)
    {
        perror("\nUnable to allocate memory\n");
        return 1;
    }

    memory->mem_capacity = VM_MEMORY_CAPACITY;
    memory->has_initialized = true;

    memory->free_list = (struct Block *)memory->memory;
    memory->free_list->size = VM_MEMORY_CAPACITY - sizeof(struct Block);
    memory->free_list->free = 1;
    memory->free_list->next = NULL;

    return 0;
}

/* Split a block if it's large enough */
static void split_block(struct Block *block, size_t size)
{
    if (block->size >= size + sizeof(struct Block) + MIN_BLOCK_SIZE)
    {
        struct Block *new_block = (struct Block *)((char *)block + sizeof(struct Block) + size);
        new_block->size = block->size - size - sizeof(struct Block);
        new_block->free = 1;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

/* Allocate memory from the custom heap */
void *s_malloc(struct Memory *memory, size_t size)
{
    size = ALIGN(size);
    struct Block *curr = memory->free_list;

    while (curr)
    {
        if (curr->free && curr->size >= size)
        {
            split_block(curr, size);
            curr->free = 0;
            return (char *)curr + sizeof(struct Block);
        }
        curr = curr->next;
    }

    return NULL;
}

/* Merge adjacent free blocks */
static void coalesce(struct Memory *memory)
{
    struct Block *curr = memory->free_list;
    while (curr && curr->next)
    {
        if (curr->free && curr->next->free)
        {
            curr->size += sizeof(struct Block) + curr->next->size;
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}

/* Free a previously allocated block */
void s_free(struct Memory *memory, void *ptr)
{
    if (!ptr)
        return;

    struct Block *block = (struct Block *)((char *)ptr - sizeof(struct Block));
    block->free = 1;

    coalesce(memory);
}

/* Allocate zero-initialized memory */
void *s_calloc(struct Memory *memory, size_t nmemb, size_t size)
{
    size_t total = nmemb * size;
    void *ptr = s_malloc(memory, total);
    if (ptr)
        memset(ptr, 0, total);
    return ptr;
}

/* Resize an allocated block */
void *s_realloc(struct Memory *memory, void *ptr, size_t size)
{
    if (!ptr)
        return s_malloc(memory, size);

    if (size == 0)
    {
        s_free(memory, ptr);
        return NULL;
    }

    struct Block *block = (struct Block *)((char *)ptr - sizeof(struct Block));
    size = ALIGN(size);

    if (block->size >= size)
    {
        split_block(block, size);
        return ptr;
    }

    /* Allocate new block */
    void *new_ptr = s_malloc(memory, size);
    if (!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, block->size < size ? block->size : size);
    s_free(memory, ptr);

    return new_ptr;
}


/* DEBUG leak reporter */
void s_DEBUG_report_leaks(struct Memory *memory)
{
    struct Block *curr = memory->free_list;
    int leak_count = 0;
    size_t total_leaked = 0;

    printf("[Leak Report]\n");

    while (curr)
    {
        if (!curr->free)
        {
            void *user_ptr = (void *)((char *)curr + sizeof(struct Block));
            printf("  Leak at %p | Size: %zu bytes\n", user_ptr, curr->size);
            leak_count++;
            total_leaked += curr->size;
        }
        curr = curr->next;
    }

    if (leak_count == 0)
        printf("  No leaks detected.\n");
    else
        printf("  %d leaks found, total leaked: %zu bytes\n", leak_count, total_leaked);
}
