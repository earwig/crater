/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    AT_REGISTER,
    AT_IMMEDIATE,
    AT_INDIRECT,
    AT_INDEXED,
    AT_LABEL,
    AT_CONDITION
} ASMArgType;

typedef enum {
    REG_A, REG_F, REG_B, REG_C, REG_D, REG_E, REG_H, REG_L, REG_I, REG_R,
    REG_AF, REG_BC, REG_DE, REG_HL, REG_IX, REG_IY,
    REG_PC, REG_SP,
    REG_AF_, REG_IXH, REG_IXL, REG_IYH, REG_IYL
} ASMArgRegister;

typedef struct {
    uint16_t value;
    bool is_negative;
} ASMArgImmediate;

typedef struct {
    bool is_reg;
    ASMArgRegister reg;
    ASMArgImmediate imm;
} ASMArgIndirect;

typedef struct {
    ASMArgRegister reg;
    ASMArgImmediate imm;
} ASMArgIndexed;

typedef char* ASMArgLabel;

typedef enum {
    COND_NZ, COND_N, COND_NC, COND_C, COND_PO, COND_PE, COND_P, COND_M
} ASMArgCondition;

typedef union {
    ASMArgRegister reg;
    ASMArgImmediate imm;
    ASMArgIndirect indirect;
    ASMArgIndexed index;
    ASMArgLabel label;
    ASMArgCondition cond;
} ASMArgData;

typedef struct {
    ASMArgType type;
    ASMArgData data;
} ASMInstArg;
