/*
    *
    * opcodegen.h - Opcode matcher and generator header file
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef OPCODEGEN_H
#define OPCODEGEN_H

#include "assembler/assembler.h"

enum OpCode get_opcode_from_str(const char* instr);

#endif
