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
#include <time.h>

int main(int argc, char** argv)
{
    struct Memory memory = {0};

    initialize_memory(&memory);

    char* test = (char*)s_malloc(&memory, 5*sizeof(char));
    char* test2 = (char*)s_malloc(&memory, 5*sizeof(char));

    s_free(test);
    s_free(test2);

    s_DEBUG_dump_memory(&memory);
    s_DEBUG_report_leaks(&memory);

    free(memory.memory);
    printf("Exited Successfully\n");

    return 0;
}
