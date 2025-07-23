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
#include "types.h"
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv)
{
    Result r = assemble("/Users/rohit/rohit-project-work/rxevm/samples/test1.rasm", "/Users/rohit/rohit-project-work/rxevm/rasm-build/", false, true);
    return 0;
}
