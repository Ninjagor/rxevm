/*
    *
    * assembler.c - Main assembler implementation for RASM
    *
    * Copyright (c) Rohit Karthik
    *
    * This source code is licensed under the license found in the
    * LICENSE file in the root directory of this source tree.
    *
*/

#include "assembler/assembler.h"
#include "assembler/opmeta.h"
#include "assembler/labelmap.h"
#include "assembler/opcodegen.h"
#include "tools/fs/fs.h"
#include "types.h"
#include "tools/dynbuf.h"
#include "tools/parser.h"
#include "assembler/sprites.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

int mkdir_recursive(const char* dir, mode_t mode) {
    char tmp[1024];
    char* p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);

    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

static void init_string_table(StringTable* st) {
    st->strings = NULL;
    st->count = 0;
    st->capacity = 0;
}

static int add_string(StringTable* st, const char* str) {
    for (size_t i = 0; i < st->count; i++) {
        if (strcmp(st->strings[i], str) == 0) {
            return (int)i;
        }
    }
    if (st->count == st->capacity) {
        size_t new_cap = st->capacity ? st->capacity * 2 : 8;
        char** tmp = realloc(st->strings, new_cap * sizeof(char*));
        if (!tmp) return -1;
        st->strings = tmp;
        st->capacity = new_cap;
    }
    st->strings[st->count] = strdup(str);
    if (!st->strings[st->count]) return -1;
    return (int)(st->count++);
}

static void free_string_table(StringTable* st) {
    for (size_t i = 0; i < st->count; i++) {
        free(st->strings[i]);
    }
    free(st->strings);
    st->strings = NULL;
    st->count = 0;
    st->capacity = 0;
}

static void write_string_table(FILE* f, StringTable* st) {
    size_t total_written = 0;

    for (size_t i = 0; i < st->count; i++) {
        const char* s = st->strings[i];
        size_t len = strlen(s);
        for (size_t j = 0; j <= len; j++) {
            uint8_t b = (uint8_t)s[j];
            fwrite(&b, sizeof(uint8_t), 1, f);
            total_written++;
        }
    }

    if (total_written % 2 != 0) {
        uint8_t pad = 0x00;
        fwrite(&pad, 1, 1, f);
    }
}

static void appendByte(DynBuffer* buf, uint8_t byte) {
    if (buf->size + 1 > buf->capacity) {
        size_t new_cap = buf->capacity ? buf->capacity * 2 : 64;
        uint16_t* tmp = (uint16_t*)realloc(buf->data, new_cap * sizeof(uint16_t));
        if (!tmp) {
            fprintf(stderr, "Out of memory appending byte\n");
            exit(1);
        }
        buf->data = tmp;
        buf->capacity = new_cap;
    }
    ((uint8_t*)buf->data)[buf->size++] = byte;
}

static void write_operand(DynBuffer* buf, const char* operand) {
    uint8_t prefix;
    uint16_t val;
    if (operand[0] == '%') {
        prefix = 0x00;
        val = (uint16_t)strtol(operand + 1, NULL, 0);
    } else if (operand[0] == '#') {
        prefix = 0x01;
        val = (uint16_t)strtol(operand + 1, NULL, 0);
    } else {
        prefix = 0x01;
        val = (uint16_t)strtol(operand, NULL, 0);
    }

    appendByte(buf, prefix);
    appendByte(buf, (val >> 8) & 0xFF);
    appendByte(buf, val & 0xFF);
}

static size_t get_instruction_size(char** toks) {
    if (!toks[0]) return 0;
    char upper[16];
    snprintf(upper, sizeof(upper), "%s", toks[0]);
    for (char* p = upper; *p; p++) *p = toupper(*p);
    if (upper[0] == '_') return 0; // label
    int opcode = get_opcode_from_str(upper);
    if (opcode < 0) return 0;
    int operand_count = get_operand_count((enum OpCode)opcode);
    return 1 + operand_count * 3;
}

static Result first_pass(LabelMap* labels, ParsedLines* plines, uint16_t* instr_addrs) {
    uint16_t pc = 0;
    for (size_t i = 0; i < plines->count; i++) {
        char** toks = plines->lines[i];
        if (!toks[0]) continue;
        for (char* p = toks[0]; *p; p++) *p = toupper(*p);
        if (toks[0][0] == '_') {
            if (!add_label(labels, toks[0], pc)) {
                printf("Linking label %s failed\n", toks[0]);
                return ERROR;
            }
            continue;
        }
        instr_addrs[i] = pc;
        pc += (uint16_t)get_instruction_size(toks);
    }
    return SUCCESS;
}

static Result second_pass(DynBuffer* opcodes, LabelMap* labels, ParsedLines* plines, StringTable* ststrings, SpriteTable* stsprites) {
    for (size_t i = 0; i < plines->count; i++) {
        char** toks = plines->lines[i];
        if (!toks[0]) continue;
        if (toks[0][0] == '_') continue;
        for (char* p = toks[0]; *p; p++) *p = toupper(*p);

        int opcode = get_opcode_from_str(toks[0]);


        if (opcode == -3) { // DSTR
            if (!toks[1] || !toks[2] || !toks[3]) {
                printf("COMPILATION ERROR: DSTR requires register, string literal, and address\n");
                return ERROR;
            }
            int regnum = -1;
            if (toks[1][0] == 'R') regnum = atoi(toks[1] + 1);
            if (regnum < 0) {
                printf("COMPILATION ERROR: DSTR requires a valid register\n");
                return ERROR;
            }
            const char* str_literal = toks[2];
            size_t len = strlen(str_literal);
            if (len < 2 || str_literal[0] != '"' || str_literal[len - 1] != '"') {
                printf("COMPILATION ERROR: DSTR string literal must be quoted\n");
                return ERROR;
            }
            char* string_content = strndup(str_literal + 1, len - 2);
            if (!string_content) {
                printf("COMPILATION ERROR: Memory allocation failed\n");
                return ERROR;
            }
            uint16_t start_addr = (uint16_t)strtol(toks[3], NULL, 0);
            for (size_t idx = 0; idx <= strlen(string_content); idx++) {
                appendByte(opcodes, MOV);
                write_operand(opcodes, (char[]){'%', '0' + regnum, 0});
                write_operand(opcodes, (char[]){'#', (char)string_content[idx], 0});
                appendByte(opcodes, 0); appendByte(opcodes, 0); appendByte(opcodes, 0); // pad operand 3rd bytes (if needed)
                appendByte(opcodes, STORE);
                write_operand(opcodes, (char[]){'%', '0' + regnum, 0});
                char addr_operand[8];
                snprintf(addr_operand, sizeof(addr_operand), "#%u", start_addr + (uint16_t)idx);
                write_operand(opcodes, addr_operand);
                appendByte(opcodes, 0); appendByte(opcodes, 0); appendByte(opcodes, 0);
            }
            free(string_content);
            continue;
        }

        if (opcode == -2) {
            if (!toks[1] || !toks[2]) {
                printf("COMPILATION ERROR: STRS requires register and string literal\n");
                return ERROR;
            }
            int regnum = -1;
            if (toks[1][0] == '%') regnum = atoi(toks[1] + 1);
            if (regnum < 0) {
                printf("COMPILATION ERROR: STRS requires valid register\n");
                return ERROR;
            }
            const char* str_literal = toks[2];
            size_t len = strlen(str_literal);
            if (len < 2 || str_literal[0] != '"' || str_literal[len - 1] != '"') {
                printf("COMPILATION ERROR: STRS string literal must be quoted\n");
                return ERROR;
            }
            char* string_content = strndup(str_literal + 1, len - 2);
            if (!string_content) {
                printf("COMPILATION ERROR: Memory allocation failed\n");
                return ERROR;
            }
            int idx = add_string(ststrings, string_content);
            free(string_content);
            if (idx < 0) {
                printf("COMPILATION ERROR: Failed to add string to table\n");
                return ERROR;
            }
            uint16_t dest_addr = 0x0000;
            if (toks[3]) {
                dest_addr = (uint16_t)strtol(toks[3], NULL, 0);
            }
            appendByte(opcodes, 0x80);
            write_operand(opcodes, (char[]){'%', '0' + regnum, 0});
            write_operand(opcodes, (char[]){'#', (char)idx, 0});
            write_operand(opcodes, (char[]){'#', (char)dest_addr, 0});
            continue;
        }

        if (opcode == -4) { // LDS
            if (!toks[1] || !toks[2]) {
                printf("COMPILATION ERROR: LDS requires register and sprite data\n");
                return ERROR;
            }
            int regnum = -1;
            if (toks[1][0] == '%') regnum = atoi(toks[1] + 1);
            if (regnum < 0) {
                printf("COMPILATION ERROR: LDS requires valid register\n");
                return ERROR;
            }
            const char* bracketed = toks[2];
            if (bracketed[0] != '[' || bracketed[strlen(bracketed) - 1] != ']') {
                printf("COMPILATION ERROR: Sprite data must be in [brackets]\n");
                return ERROR;
            }
            char* inner = strndup(bracketed + 1, strlen(bracketed) - 2);
            if (!inner) return ERROR;
            uint8_t sprite[8] = {0};
            int filled = 0;
            char* tok = strtok(inner, " ");
            while (tok && filled < 8) {
                uint16_t val = (uint16_t)strtol(tok, NULL, 0);
                sprite[filled++] = (uint8_t)(val & 0xFF);
                tok = strtok(NULL, " ");
            }
            free(inner);
            if (filled != 8) {
                printf("COMPILATION ERROR: Sprite must have exactly 8 bytes\n");
                return ERROR;
            }
            int sprite_idx = add_sprite(stsprites, sprite);
            if (sprite_idx < 0) {
                printf("COMPILATION ERROR: Failed to add sprite\n");
                return ERROR;
            }
            uint16_t dest_addr = 0x0000;
            if (toks[3]) {
                dest_addr = (uint16_t)strtol(toks[3], NULL, 0);
            }
            appendByte(opcodes, 0x83);
            write_operand(opcodes, (char[]){'%', '0' + regnum, 0});
            write_operand(opcodes, (char[]){'#', (char)sprite_idx, 0});
            write_operand(opcodes, (char[]){'#', (char)dest_addr, 0});
            continue;
        }


        if (opcode < 0) {
            printf("COMPILATION ERROR: Invalid opcode %s\n", toks[0]);
            return ERROR;
        }

        int operand_count = get_operand_count((enum OpCode)opcode);
        int actual_operands = 0;
        for (int j = 1; j <= operand_count; j++) {
            if (toks[j]) actual_operands++;
        }
        if (actual_operands < operand_count) {
            printf("COMPILATION ERROR: Missing operands for %s\n", toks[0]);
            return ERROR;
        }
        if (actual_operands > operand_count) {
            printf("COMPILATION ERROR: Too many operands for %s\n", toks[0]);
            return ERROR;
        }

        appendByte(opcodes, (uint8_t)opcode);

        for (int j = 1; j <= operand_count; j++) {
            if (!toks[j]) {
                // pad empty operand with zeros
                appendByte(opcodes, 0);
                appendByte(opcodes, 0);
                appendByte(opcodes, 0);
                continue;
            }
            // label patch for jump/call/compare ops
            if ((opcode == JMP || opcode == CALL || opcode == JE || opcode == JNE || opcode == JL ||
                 opcode == JLE || opcode == JG || opcode == JGE || opcode == CEQ || opcode == CLE || opcode == CL ||
                 opcode == CNE || opcode == CG || opcode == CGE) && toks[j][0] == '_') {
                int addr = get_label_address(labels, toks[j]);
                if (addr == -1) {
                    printf("COMPILATION ERROR: Undefined label %s\n", toks[j]);
                    return ERROR;
                }
                char label_imm[16];
                snprintf(label_imm, sizeof(label_imm), "#%d", addr);
                write_operand(opcodes, label_imm);
            } else {
                write_operand(opcodes, toks[j]);
            }
        }
    }
    return SUCCESS;
}

Result assemble(const char* filepath, char* outDir, bool isQuiet, bool isVerbose) {
    char* raw_code = read_file_to_string(filepath);
    if (!raw_code) return ERROR;

    const char* preamble = "JMP _START\n";
    size_t total_len = strlen(preamble) + strlen(raw_code) + 1;

    if (!isQuiet) {
        printf("Running codemods...\n");
    }

    char* modified_code = malloc(total_len);
    if (!modified_code) {
        free(raw_code);
        return ERROR;
    }

    strcpy(modified_code, preamble);
    strcat(modified_code, raw_code);

    size_t line_count = 0;
    char** lines = split_lines(modified_code, &line_count);
    if (!lines) {
        free(modified_code);
        free(raw_code);
        return ERROR;
    }

    if (!isQuiet) {
        printf("Parsing...\n");
    }

    ParsedLines plines = parse_all_lines(lines, line_count);

    if (!isQuiet) {
        printf("Mapping labels...\n");
    }

    LabelMap labels;
    init_label_map(&labels);
    DynBuffer opcodes;
    initBuffer(&opcodes, 64); // buffer of bytes now

    if (!isQuiet) {
        printf("Generating static string table...\n");
    }

    StringTable strtable;
    init_string_table(&strtable);

    if (!isQuiet) {
        printf("Encoding sprites...\n");
    }

    SpriteTable stsprites;
    init_sprite_table(&stsprites);

    if (!isQuiet) {
        printf("Linking labels...\n");
    }

    // Allocate instr_addrs for first pass label tracking
    uint16_t* instr_addrs = malloc(sizeof(uint16_t) * plines.count);
    if (!instr_addrs) {
        printf("Memory allocation failed\n");
        free_label_map(&labels);
        free_parsed_lines(&plines);
        free(lines);
        free(modified_code);
        free(raw_code);
        return ERROR;
    }

    Result res = first_pass(&labels, &plines, instr_addrs);
    if (res == SUCCESS) res = second_pass(&opcodes, &labels, &plines, &strtable, &stsprites);

    free(instr_addrs);
    free_label_map(&labels);
    free_parsed_lines(&plines);
    for (size_t i = 0; i < line_count; i++) free(lines[i]);
    free(lines);
    free(modified_code);
    free(raw_code);

    if (res == ERROR) {
        freeBuffer(&opcodes);
        free_string_table(&strtable);
        free_sprite_table(&stsprites);
        return ERROR;
    }

    struct stat st = {0};
    if (stat(outDir, &st) == -1) {
        if (mkdir_recursive(outDir, 0755) != 0) {
            perror("Failed to create output directory");
            freeBuffer(&opcodes);
            free_string_table(&strtable);
            free_sprite_table(&stsprites);
            return ERROR;
        }
    }

    char outFilePath[1024];
    snprintf(outFilePath, sizeof(outFilePath), "%s/a.bin", outDir);

    FILE *f = fopen(outFilePath, "wb");
    if (!f) {
        printf("\nINVALID FILE %s\n", outFilePath);
        freeBuffer(&opcodes);
        free_string_table(&strtable);
        free_sprite_table(&stsprites);
        return ERROR;
    }

    if (isVerbose) {
        printf("\nAssembled bytes:");
        for (size_t i = 0; i < opcodes.size; i++) {
            if (i % 4 == 0) printf("\n");
            printf("0x%02X ", ((uint8_t*)opcodes.data)[i]);
        }
    }

    if (!isQuiet) {
        printf("\n\nCompiled: %zu Bytes\n", opcodes.size);
    }

    fwrite(opcodes.data, sizeof(uint8_t), opcodes.size, f);

    uint16_t delim = 0xFFFF;
    for (int i = 0; i < 4; i++) {
        fwrite(&delim, sizeof(uint16_t), 1, f);
    }

    write_string_table(f, &strtable);

    for (int i = 0; i < 4; i++) {
        fwrite(&delim, sizeof(uint16_t), 1, f);
    }

    write_sprite_table(f, &stsprites);

    fclose(f);

    freeBuffer(&opcodes);
    free_string_table(&strtable);
    free_sprite_table(&stsprites);

    return SUCCESS;
}
