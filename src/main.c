/*
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#include <stdio.h>
#include <stdlib.h>
#include "assembler/assembler.h"
#include "tools/memory/alloc.h"
#include "tools/memory/memory.h"
#include <unistd.h>

int main(int argc, char** argv)
{
    struct Memory memory = {0};

    initialize_memory(&memory);

    char *alloc_test = (char*)s_malloc(&memory, 5*sizeof(char));

    alloc_test[0] = '1';
    alloc_test[1] = '2';
    alloc_test[2] = '3';
    alloc_test[3] = '4';
    alloc_test[4] = '\0';

    printf("String: %s\n", alloc_test);

    char *leaky = (char*)s_malloc(&memory, 100);

    for (int i = 0; i < 99; i++) {
        leaky[i] = 'A' + (i % 26);
    }
    leaky[99] = '\0';

    printf("Leaky string: %s\n", leaky);

    s_free(&memory, alloc_test);
    s_free(&memory, leaky);

    s_DEBUG_report_leaks(&memory);

    free(memory.memory);
    printf("Exited Successfully\n");
    _exit(0);
}
