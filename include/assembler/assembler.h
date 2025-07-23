/*
    *
    * assembler.h - Assembler header file
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "types.h"
#include <stdbool.h>
#include <stddef.h>
enum OpCode {
    /* Reg and Math ops */
    MOV       = 0x01,
    ADD       = 0x03,
    SUB       = 0x04,
    MUL       = 0x05,
    DIV       = 0x06,
    MOD       = 0x07,
    AND       = 0x08,
    OR        = 0x09,
    XOR       = 0x0A,
    NOT       = 0x0B,

    /* Comparison and Conditional */
    CMP       = 0x10,
    JE        = 0x11,
    JNE       = 0x12,
    JL        = 0x13,
    JLE       = 0x14,
    JG        = 0x15,
    JGE       = 0x16,

    /* Control Flow */
    JMP       = 0x20,
    CALL      = 0x21,
    RET       = 0x22,
    HALT      = 0x23,

    CEQ = 0x24,
    CNE = 0x25,
    CL = 0x26,
    CG = 0x27,
    CLE = 0x28,
    CGE = 0x29,

    PUSH      = 0x30,
    POP       = 0x31,

    /* Shift and Rotate */
    SHL       = 0x40,
    SHR       = 0x41,
    SAR       = 0x42,
    ROL       = 0x43,
    ROR       = 0x44,

    /* Mem access */
    LOAD      = 0x50,
    STORE     = 0x51,
    LOADR     = 0x52,
    LOADB     = 0x53,

    STRB  = 0x54,

    SPRB = 0x55,

    PRINT     = 0x60,
    PUTC      = 0x61,

    DRAW      = 0x70,
    CLS       = 0x71,
    INITDISPLAY = 0x72,

    POLL = 0x73,

    RDI = 0x74,

    STD = 0x75,
    STDI = 0x76,

    RTD = 0x77,

    STS = 0x78,
    STSI = 0x79,

    CLSM = 0x81,

    ENABLESMOD = 0x90,
    DISABLESMOD = 0x91
};

Result assemble(const char* filepath, char* outDir, bool isQuiet, bool isVerbose);


typedef struct {
  char** strings;
  size_t count;
  size_t capacity;
} StringTable;

#endif
