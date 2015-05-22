/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

/*
    This file is AUTO-GENERATED from 'instructions.yml'.

    `make` should trigger a rebuild when it is modified; if not, use:
    `python scripts/update_asm_instructions.py`.

    @AUTOGEN_DATE Fri May 22 00:55:44 2015 UTC
*/

/* @AUTOGEN_INST_BLOCK_START */

INST_FUNC(adc)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x8F)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x88)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x89)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x8A)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x8B)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x8C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x8C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x8C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x8D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x8D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x8D)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_BC)
            INST_RETURN(2, 0xED, 0x4A)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_DE)
            INST_RETURN(2, 0xED, 0x5A)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_HL)
            INST_RETURN(2, 0xED, 0x6A)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_SP)
            INST_RETURN(2, 0xED, 0x7A)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0xCE, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x8E)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x8E, INST_INDEX(1).offset)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(add)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x87)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x80)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x81)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x82)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x83)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x84)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x84)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x84)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x85)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x85)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x85)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_BC)
            INST_RETURN(1, 0x09)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_BC)
            INST_RETURN(2, INST_IX_PREFIX, 0x09)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_BC)
            INST_RETURN(2, INST_IY_PREFIX, 0x09)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_DE)
            INST_RETURN(1, 0x19)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_DE)
            INST_RETURN(2, INST_IX_PREFIX, 0x19)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_DE)
            INST_RETURN(2, INST_IY_PREFIX, 0x19)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_HL)
            INST_RETURN(1, 0x29)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0x29)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0x29)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_SP)
            INST_RETURN(1, 0x39)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_SP)
            INST_RETURN(2, INST_IX_PREFIX, 0x39)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_SP)
            INST_RETURN(2, INST_IY_PREFIX, 0x39)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0xC6, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x86)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x86, INST_INDEX(1).offset)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(and)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0xA7)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0xA0)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0xA1)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0xA2)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0xA3)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0xA4)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0xA4)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0xA4)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0xA5)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0xA5)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0xA5)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U8)
            INST_RETURN(2, 0xE6, INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0xA6)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0xA6, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(bit)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_REGISTER) {
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xCB, 0x47 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_B)
            INST_RETURN(2, 0xCB, 0x40 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_C)
            INST_RETURN(2, 0xCB, 0x41 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_D)
            INST_RETURN(2, 0xCB, 0x42 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_E)
            INST_RETURN(2, 0xCB, 0x43 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_H)
            INST_RETURN(2, 0xCB, 0x44 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_L)
            INST_RETURN(2, 0xCB, 0x45 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(2, 0xCB, 0x46 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDEXED) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x46 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(call)
{
    INST_TAKES_ARGS(
        AT_CONDITION|AT_IMMEDIATE,
        AT_IMMEDIATE|AT_OPTIONAL,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U16)
            INST_RETURN(3, 0xCD, INST_IMM_U16_B1(INST_IMM(0)), INST_IMM_U16_B2(INST_IMM(0)))
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_CONDITION && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_COND(0) == COND_NZ && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xC4, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_Z && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xCC, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_NC && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xD4, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_C && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xDC, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_PO && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xE4, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_PE && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xEC, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_P && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xF4, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_M && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xFC, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ccf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x3F)
}

INST_FUNC(cp)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0xBF)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0xB8)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0xB9)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0xBA)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0xBB)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0xBC)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0xBC)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0xBC)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0xBD)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0xBD)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0xBD)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0xBE)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0xBE, INST_INDEX(0).offset)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U8)
            INST_RETURN(2, 0xFE, INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(cpd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA9)
}

INST_FUNC(cpdr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB9)
}

INST_FUNC(cpi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA1)
}

INST_FUNC(cpir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB1)
}

INST_FUNC(cpl)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x2F)
}

INST_FUNC(daa)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x27)
}

INST_FUNC(dec)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x3D)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0x05)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0x0D)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0x15)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0x1D)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0x25)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x25)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x25)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0x2D)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x2D)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x2D)
        if (INST_REG(0) == REG_BC)
            INST_RETURN(1, 0x0B)
        if (INST_REG(0) == REG_DE)
            INST_RETURN(1, 0x1B)
        if (INST_REG(0) == REG_HL)
            INST_RETURN(1, 0x2B)
        if (INST_REG(0) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0x2B)
        if (INST_REG(0) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0x2B)
        if (INST_REG(0) == REG_SP)
            INST_RETURN(1, 0x3B)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0x35)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0x35, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(di)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xF3)
}

INST_FUNC(djnz)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_REL)
            INST_RETURN(2, 0x10, INST_IMM(0).sval - 2)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ei)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xFB)
}

INST_FUNC(ex)
{
    INST_TAKES_ARGS(
        AT_INDIRECT|AT_REGISTER,
        AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(0) == REG_AF && INST_REG(1) == REG_AF_)
            INST_RETURN(1, 0x08)
        if (INST_REG(0) == REG_DE && INST_REG(1) == REG_HL)
            INST_RETURN(1, 0xEB)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT && INST_TYPE(1) == AT_REGISTER) {
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_SP) && INST_REG(1) == REG_HL)
            INST_RETURN(1, 0xE3)
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_SP) && INST_REG(1) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0xE3)
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_SP) && INST_REG(1) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0xE3)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(exx)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xD9)
}

INST_FUNC(halt)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x76)
}

INST_FUNC(im)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if ((INST_IMM(0).mask & IMM_IM && INST_IMM(0).uval == 0))
            INST_RETURN(2, 0xED, 0x46)
        if ((INST_IMM(0).mask & IMM_IM && INST_IMM(0).uval == 1))
            INST_RETURN(2, 0xED, 0x56)
        if ((INST_IMM(0).mask & IMM_IM && INST_IMM(0).uval == 2))
            INST_RETURN(2, 0xED, 0x5E)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(in)
{
    INST_TAKES_ARGS(
        AT_PORT|AT_REGISTER,
        AT_OPTIONAL|AT_PORT,
        AT_NONE
    )
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_PORT) {
        if (INST_REG(0) == REG_A && INST_PORT(1).type == AT_IMMEDIATE)
            INST_RETURN(2, 0xDB, INST_PORT(1).port.imm.uval)
        if (INST_REG(0) == REG_A && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x78)
        if (INST_REG(0) == REG_B && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x40)
        if (INST_REG(0) == REG_C && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x48)
        if (INST_REG(0) == REG_D && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x50)
        if (INST_REG(0) == REG_E && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x58)
        if (INST_REG(0) == REG_H && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x60)
        if (INST_REG(0) == REG_L && INST_PORT(1).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x68)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_PORT) {
        if (INST_PORT(0).type == AT_REGISTER)
            INST_RETURN(2, 0xED, 0x70)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(inc)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x3C)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0x04)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0x0C)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0x14)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0x1C)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0x24)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x24)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x24)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0x2C)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x2C)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x2C)
        if (INST_REG(0) == REG_BC)
            INST_RETURN(1, 0x03)
        if (INST_REG(0) == REG_DE)
            INST_RETURN(1, 0x13)
        if (INST_REG(0) == REG_HL)
            INST_RETURN(1, 0x23)
        if (INST_REG(0) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0x23)
        if (INST_REG(0) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0x23)
        if (INST_REG(0) == REG_SP)
            INST_RETURN(1, 0x33)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0x34)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0x34, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ind)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xAA)
}

INST_FUNC(indr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xBA)
}

INST_FUNC(ini)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA2)
}

INST_FUNC(inir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB2)
}

INST_FUNC(jp)
{
    INST_TAKES_ARGS(
        AT_CONDITION|AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT,
        AT_IMMEDIATE|AT_OPTIONAL,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U16)
            INST_RETURN(3, 0xC3, INST_IMM_U16_B1(INST_IMM(0)), INST_IMM_U16_B2(INST_IMM(0)))
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_CONDITION && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_COND(0) == COND_NZ && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xC2, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_Z && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xCA, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_NC && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xD2, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_C && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xDA, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_PO && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xE2, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_PE && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xEA, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_P && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xF2, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_COND(0) == COND_M && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0xFA, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0xE9)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0xE9, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(jr)
{
    INST_TAKES_ARGS(
        AT_CONDITION|AT_IMMEDIATE,
        AT_IMMEDIATE|AT_OPTIONAL,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_REL)
            INST_RETURN(2, 0x18, INST_IMM(0).sval - 2)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_CONDITION && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_COND(0) == COND_NZ && INST_IMM(1).mask & IMM_REL)
            INST_RETURN(2, 0x20, INST_IMM(1).sval - 2)
        if (INST_COND(0) == COND_Z && INST_IMM(1).mask & IMM_REL)
            INST_RETURN(2, 0x28, INST_IMM(1).sval - 2)
        if (INST_COND(0) == COND_NC && INST_IMM(1).mask & IMM_REL)
            INST_RETURN(2, 0x30, INST_IMM(1).sval - 2)
        if (INST_COND(0) == COND_C && INST_IMM(1).mask & IMM_REL)
            INST_RETURN(2, 0x38, INST_IMM(1).sval - 2)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ld)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x7F)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x78)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x79)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x7A)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x7B)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x7C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x7C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x7C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x7D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x7D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x7D)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x47)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x40)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x41)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x42)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x43)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x44)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x44)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x44)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x45)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x45)
        if (INST_REG(0) == REG_B && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x45)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x4F)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x48)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x49)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x4A)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x4B)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x4C)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x4C)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x4C)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x4D)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x4D)
        if (INST_REG(0) == REG_C && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x4D)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x57)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x50)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x51)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x52)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x53)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x54)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x54)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x54)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x55)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x55)
        if (INST_REG(0) == REG_D && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x55)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x5F)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x58)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x59)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x5A)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x5B)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x5C)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x5C)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x5C)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x5D)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x5D)
        if (INST_REG(0) == REG_E && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x5D)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x67)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_A)
            INST_RETURN(2, INST_IX_PREFIX, 0x67)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_A)
            INST_RETURN(2, INST_IY_PREFIX, 0x67)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x60)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_B)
            INST_RETURN(2, INST_IX_PREFIX, 0x60)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_B)
            INST_RETURN(2, INST_IY_PREFIX, 0x60)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x61)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_C)
            INST_RETURN(2, INST_IX_PREFIX, 0x61)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_C)
            INST_RETURN(2, INST_IY_PREFIX, 0x61)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x62)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_D)
            INST_RETURN(2, INST_IX_PREFIX, 0x62)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_D)
            INST_RETURN(2, INST_IY_PREFIX, 0x62)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x63)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_E)
            INST_RETURN(2, INST_IX_PREFIX, 0x63)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_E)
            INST_RETURN(2, INST_IY_PREFIX, 0x63)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x64)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x64)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x64)
        if (INST_REG(0) == REG_H && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x65)
        if (INST_REG(0) == REG_IXH && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x65)
        if (INST_REG(0) == REG_IYH && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x65)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x6F)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_A)
            INST_RETURN(2, INST_IX_PREFIX, 0x6F)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_A)
            INST_RETURN(2, INST_IY_PREFIX, 0x6F)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x68)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_B)
            INST_RETURN(2, INST_IX_PREFIX, 0x68)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_B)
            INST_RETURN(2, INST_IY_PREFIX, 0x68)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x69)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_C)
            INST_RETURN(2, INST_IX_PREFIX, 0x69)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_C)
            INST_RETURN(2, INST_IY_PREFIX, 0x69)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x6A)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_D)
            INST_RETURN(2, INST_IX_PREFIX, 0x6A)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_D)
            INST_RETURN(2, INST_IY_PREFIX, 0x6A)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x6B)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_E)
            INST_RETURN(2, INST_IX_PREFIX, 0x6B)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_E)
            INST_RETURN(2, INST_IY_PREFIX, 0x6B)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x6C)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x6C)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x6C)
        if (INST_REG(0) == REG_L && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x6D)
        if (INST_REG(0) == REG_IXL && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x6D)
        if (INST_REG(0) == REG_IYL && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x6D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IX)
            INST_RETURN(3, INST_IX_PREFIX, 0xED, 0x57)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IY)
            INST_RETURN(3, INST_IY_PREFIX, 0xED, 0x57)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_A)
            INST_RETURN(3, INST_IX_PREFIX, 0xED, 0x47)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_A)
            INST_RETURN(3, INST_IY_PREFIX, 0xED, 0x47)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_R)
            INST_RETURN(2, 0xED, 0x5F)
        if (INST_REG(0) == REG_R && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xED, 0x4F)
        if (INST_REG(0) == REG_SP && INST_REG(1) == REG_HL)
            INST_RETURN(1, 0xF9)
        if (INST_REG(0) == REG_SP && INST_REG(1) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0xF9)
        if (INST_REG(0) == REG_SP && INST_REG(1) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0xF9)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x3E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_B && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x06, INST_IMM(1).uval)
        if (INST_REG(0) == REG_C && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x0E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_D && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x16, INST_IMM(1).uval)
        if (INST_REG(0) == REG_E && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x1E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_H && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x26, INST_IMM(1).uval)
        if (INST_REG(0) == REG_IXH && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(3, INST_IX_PREFIX, 0x26, INST_IMM(1).uval)
        if (INST_REG(0) == REG_IYH && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(3, INST_IY_PREFIX, 0x26, INST_IMM(1).uval)
        if (INST_REG(0) == REG_L && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x2E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_IXL && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(3, INST_IX_PREFIX, 0x2E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_IYL && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(3, INST_IY_PREFIX, 0x2E, INST_IMM(1).uval)
        if (INST_REG(0) == REG_BC && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0x01, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_REG(0) == REG_DE && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0x11, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_REG(0) == REG_HL && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0x21, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_REG(0) == REG_IX && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(4, INST_IX_PREFIX, 0x21, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_REG(0) == REG_IY && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(4, INST_IY_PREFIX, 0x21, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        if (INST_REG(0) == REG_SP && INST_IMM(1).mask & IMM_U16)
            INST_RETURN(3, 0x31, INST_IMM_U16_B1(INST_IMM(1)), INST_IMM_U16_B2(INST_IMM(1)))
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x7E)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0x46)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0x4E)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0x56)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0x5E)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0x66)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0x6E)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x7E, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_B)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x46, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_C)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x4E, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_D)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x56, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_E)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x5E, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_H)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x66, INST_INDEX(1).offset)
        if (INST_REG(0) == REG_L)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x6E, INST_INDEX(1).offset)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT) {
        if (INST_REG(0) == REG_A && (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_BC))
            INST_RETURN(1, 0x0A)
        if (INST_REG(0) == REG_A && (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_DE))
            INST_RETURN(1, 0x1A)
        if (INST_REG(0) == REG_HL && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(3, 0x2A, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_IX && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(4, INST_IX_PREFIX, 0x2A, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_IY && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(4, INST_IY_PREFIX, 0x2A, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_A && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(3, 0x3A, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_BC && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(4, 0xED, 0x4B, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_DE && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(4, 0xED, 0x5B, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        if (INST_REG(0) == REG_SP && INST_INDIRECT(1).type == AT_IMMEDIATE)
            INST_RETURN(4, 0xED, 0x7B, INST_IMM_U16_B1(INST_INDIRECT(1).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(1).addr.imm))
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT && INST_TYPE(1) == AT_REGISTER &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(1, 0x77)
        if (INST_REG(1) == REG_B)
            INST_RETURN(1, 0x70)
        if (INST_REG(1) == REG_C)
            INST_RETURN(1, 0x71)
        if (INST_REG(1) == REG_D)
            INST_RETURN(1, 0x72)
        if (INST_REG(1) == REG_E)
            INST_RETURN(1, 0x73)
        if (INST_REG(1) == REG_H)
            INST_RETURN(1, 0x74)
        if (INST_REG(1) == REG_L)
            INST_RETURN(1, 0x75)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x77, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_B)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x70, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_C)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x71, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_D)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x72, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_E)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x73, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_H)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x74, INST_INDEX(0).offset)
        if (INST_REG(1) == REG_L)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x75, INST_INDEX(0).offset)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT && INST_TYPE(1) == AT_IMMEDIATE &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        if (INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0x36, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_IMM(1).mask & IMM_U8)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0x36, INST_INDEX(0).offset, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT && INST_TYPE(1) == AT_REGISTER) {
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_BC) && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x02)
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_DE) && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x12)
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_HL)
            INST_RETURN(3, 0x22, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_IX)
            INST_RETURN(4, INST_IX_PREFIX, 0x22, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_IY)
            INST_RETURN(4, INST_IY_PREFIX, 0x22, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_A)
            INST_RETURN(3, 0x32, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_BC)
            INST_RETURN(4, 0xED, 0x43, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_DE)
            INST_RETURN(4, 0xED, 0x53, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        if (INST_INDIRECT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_SP)
            INST_RETURN(4, 0xED, 0x73, INST_IMM_U16_B1(INST_INDIRECT(0).addr.imm), INST_IMM_U16_B2(INST_INDIRECT(0).addr.imm))
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ldd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA8)
}

INST_FUNC(lddr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB8)
}

INST_FUNC(ldi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA0)
}

INST_FUNC(ldir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB0)
}

INST_FUNC(neg)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x44)
}

INST_FUNC(nop)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x00)
}

INST_FUNC(or)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0xB7)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0xB0)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0xB1)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0xB2)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0xB3)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0xB4)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0xB4)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0xB4)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0xB5)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0xB5)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0xB5)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U8)
            INST_RETURN(2, 0xF6, INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0xB6)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0xB6, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(otdr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xBB)
}

INST_FUNC(otir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB3)
}

INST_FUNC(out)
{
    INST_TAKES_ARGS(
        AT_PORT,
        AT_IMMEDIATE|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_PORT && INST_TYPE(1) == AT_REGISTER) {
        if (INST_PORT(0).type == AT_IMMEDIATE && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xD3, INST_PORT(0).port.imm.uval)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xED, 0x79)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_B)
            INST_RETURN(2, 0xED, 0x41)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_C)
            INST_RETURN(2, 0xED, 0x49)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_D)
            INST_RETURN(2, 0xED, 0x51)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_E)
            INST_RETURN(2, 0xED, 0x59)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_H)
            INST_RETURN(2, 0xED, 0x61)
        if (INST_PORT(0).type == AT_REGISTER && INST_REG(1) == REG_L)
            INST_RETURN(2, 0xED, 0x69)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_PORT && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_PORT(0).type == AT_REGISTER && (INST_IMM(1).mask & IMM_U8 && INST_IMM(1).uval == 0))
            INST_RETURN(2, 0xED, 0x71)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(outd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xAB)
}

INST_FUNC(outi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA3)
}

INST_FUNC(pop)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_BC)
            INST_RETURN(1, 0xC1)
        if (INST_REG(0) == REG_DE)
            INST_RETURN(1, 0xD1)
        if (INST_REG(0) == REG_HL)
            INST_RETURN(1, 0xE1)
        if (INST_REG(0) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0xE1)
        if (INST_REG(0) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0xE1)
        if (INST_REG(0) == REG_AF)
            INST_RETURN(1, 0xF1)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(push)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_BC)
            INST_RETURN(1, 0xC5)
        if (INST_REG(0) == REG_DE)
            INST_RETURN(1, 0xD5)
        if (INST_REG(0) == REG_HL)
            INST_RETURN(1, 0xE5)
        if (INST_REG(0) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0xE5)
        if (INST_REG(0) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0xE5)
        if (INST_REG(0) == REG_AF)
            INST_RETURN(1, 0xF5)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(res)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER
    )
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_REGISTER) {
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xCB, 0x87 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_B)
            INST_RETURN(2, 0xCB, 0x80 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_C)
            INST_RETURN(2, 0xCB, 0x81 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_D)
            INST_RETURN(2, 0xCB, 0x82 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_E)
            INST_RETURN(2, 0xCB, 0x83 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_H)
            INST_RETURN(2, 0xCB, 0x84 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_L)
            INST_RETURN(2, 0xCB, 0x85 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(2, 0xCB, 0x86 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDEXED) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x86 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 3 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDEXED && INST_TYPE(2) == AT_REGISTER) {
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x87 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x80 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x81 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x82 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x83 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x84 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0x85 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ret)
{
    INST_TAKES_ARGS(
        AT_CONDITION|AT_OPTIONAL,
        AT_NONE,
        AT_NONE
    )
    if (INST_NARGS == 0) {
        INST_RETURN(1, 0xC9)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_CONDITION) {
        if (INST_COND(0) == COND_NZ)
            INST_RETURN(1, 0xC0)
        if (INST_COND(0) == COND_Z)
            INST_RETURN(1, 0xC8)
        if (INST_COND(0) == COND_NC)
            INST_RETURN(1, 0xD0)
        if (INST_COND(0) == COND_C)
            INST_RETURN(1, 0xD8)
        if (INST_COND(0) == COND_PO)
            INST_RETURN(1, 0xE0)
        if (INST_COND(0) == COND_PE)
            INST_RETURN(1, 0xE8)
        if (INST_COND(0) == COND_P)
            INST_RETURN(1, 0xF0)
        if (INST_COND(0) == COND_M)
            INST_RETURN(1, 0xF8)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(reti)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x4D)
}

INST_FUNC(retn)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x45)
}

INST_FUNC(rl)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x17)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x10)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x11)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x12)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x13)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x14)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x15)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x16)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x16)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x17)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x10)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x11)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x12)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x13)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x14)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x15)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(rla)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x17)
}

INST_FUNC(rlc)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x07)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x00)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x01)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x02)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x03)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x04)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x05)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x06)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x06)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x07)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x00)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x01)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x02)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x03)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x04)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x05)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(rlca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x07)
}

INST_FUNC(rld)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x6F)
}

INST_FUNC(rr)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x1F)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x18)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x19)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x1A)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x1B)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x1C)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x1D)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x1E)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1E)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1F)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x18)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x19)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1A)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1B)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1C)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x1D)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(rra)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x1F)
}

INST_FUNC(rrc)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x0F)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x08)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x09)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x0A)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x0B)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x0C)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x0D)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x0E)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0E)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0F)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x08)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x09)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0A)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0B)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0C)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x0D)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(rrca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x0F)
}

INST_FUNC(rrd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x67)
}

INST_FUNC(rst)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 0))
            INST_RETURN(1, 0xC7)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 8))
            INST_RETURN(1, 0xCF)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 16))
            INST_RETURN(1, 0xD7)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 24))
            INST_RETURN(1, 0xDF)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 32))
            INST_RETURN(1, 0xE7)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 40))
            INST_RETURN(1, 0xEF)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 48))
            INST_RETURN(1, 0xF7)
        if ((INST_IMM(0).mask & IMM_RST && INST_IMM(0).uval == 56))
            INST_RETURN(1, 0xFF)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sbc)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_A)
            INST_RETURN(1, 0x9F)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_B)
            INST_RETURN(1, 0x98)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_C)
            INST_RETURN(1, 0x99)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_D)
            INST_RETURN(1, 0x9A)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_E)
            INST_RETURN(1, 0x9B)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_H)
            INST_RETURN(1, 0x9C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x9C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x9C)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x9D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x9D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x9D)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_BC)
            INST_RETURN(2, 0xED, 0x42)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_DE)
            INST_RETURN(2, 0xED, 0x52)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_HL)
            INST_RETURN(2, 0xED, 0x62)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_SP)
            INST_RETURN(2, 0xED, 0x72)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0xDE, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x9E)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x9E, INST_INDEX(1).offset)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(scf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x37)
}

INST_FUNC(set)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE,
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER
    )
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_REGISTER) {
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_A)
            INST_RETURN(2, 0xCB, 0xC7 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_B)
            INST_RETURN(2, 0xCB, 0xC0 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_C)
            INST_RETURN(2, 0xCB, 0xC1 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_D)
            INST_RETURN(2, 0xCB, 0xC2 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_E)
            INST_RETURN(2, 0xCB, 0xC3 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_H)
            INST_RETURN(2, 0xCB, 0xC4 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(1) == REG_L)
            INST_RETURN(2, 0xCB, 0xC5 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDIRECT &&
            (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL)) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(2, 0xCB, 0xC6 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDEXED) {
        if (INST_IMM(0).mask & IMM_BIT)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC6 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 3 && INST_TYPE(0) == AT_IMMEDIATE && INST_TYPE(1) == AT_INDEXED && INST_TYPE(2) == AT_REGISTER) {
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC7 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC0 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC1 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC2 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC3 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC4 + 8 * INST_IMM(0).uval)
        if (INST_IMM(0).mask & IMM_BIT && INST_REG(2) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(1), 0xCB, INST_INDEX(1).offset, 0xC5 + 8 * INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sl1)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x37)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x30)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x31)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x32)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x33)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x34)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x36)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x36)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x37)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x30)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x31)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x32)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x33)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x34)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sla)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x27)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x20)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x21)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x22)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x23)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x24)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x25)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x26)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x26)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x27)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x20)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x21)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x22)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x23)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x24)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x25)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sll)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x37)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x30)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x31)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x32)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x33)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x34)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT) {
        INST_RETURN(2, 0xCB, 0x36)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x37)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x30)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x31)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x32)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x33)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x34)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sls)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x37)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x30)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x31)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x32)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x33)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x34)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT) {
        INST_RETURN(2, 0xCB, 0x36)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x37)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x30)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x31)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x32)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x33)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x34)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x35)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sra)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x2F)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x28)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x29)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x2A)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x2B)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x2C)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x2D)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x2E)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2E)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2F)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x28)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x29)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2A)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2B)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2C)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x2D)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(srl)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_OPTIONAL|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(2, 0xCB, 0x3F)
        if (INST_REG(0) == REG_B)
            INST_RETURN(2, 0xCB, 0x38)
        if (INST_REG(0) == REG_C)
            INST_RETURN(2, 0xCB, 0x39)
        if (INST_REG(0) == REG_D)
            INST_RETURN(2, 0xCB, 0x3A)
        if (INST_REG(0) == REG_E)
            INST_RETURN(2, 0xCB, 0x3B)
        if (INST_REG(0) == REG_H)
            INST_RETURN(2, 0xCB, 0x3C)
        if (INST_REG(0) == REG_L)
            INST_RETURN(2, 0xCB, 0x3D)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(2, 0xCB, 0x3E)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3E)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_INDEXED && INST_TYPE(1) == AT_REGISTER) {
        if (INST_REG(1) == REG_A)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3F)
        if (INST_REG(1) == REG_B)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x38)
        if (INST_REG(1) == REG_C)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x39)
        if (INST_REG(1) == REG_D)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3A)
        if (INST_REG(1) == REG_E)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3B)
        if (INST_REG(1) == REG_H)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3C)
        if (INST_REG(1) == REG_L)
            INST_RETURN(4, INST_INDEX_PREFIX(0), 0xCB, INST_INDEX(0).offset, 0x3D)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(sub)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0x97)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0x90)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0x91)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0x92)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0x93)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0x94)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0x94)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0x94)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0x95)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x95)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x95)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U8)
            INST_RETURN(2, 0xD6, INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0x96)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0x96, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(xor)
{
    INST_TAKES_ARGS(
        AT_IMMEDIATE|AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_TYPE(0) == AT_REGISTER) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(1, 0xAF)
        if (INST_REG(0) == REG_B)
            INST_RETURN(1, 0xA8)
        if (INST_REG(0) == REG_C)
            INST_RETURN(1, 0xA9)
        if (INST_REG(0) == REG_D)
            INST_RETURN(1, 0xAA)
        if (INST_REG(0) == REG_E)
            INST_RETURN(1, 0xAB)
        if (INST_REG(0) == REG_H)
            INST_RETURN(1, 0xAC)
        if (INST_REG(0) == REG_IXH)
            INST_RETURN(2, INST_IX_PREFIX, 0xAC)
        if (INST_REG(0) == REG_IYH)
            INST_RETURN(2, INST_IY_PREFIX, 0xAC)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0xAD)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0xAD)
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0xAD)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_IMMEDIATE) {
        if (INST_IMM(0).mask & IMM_U8)
            INST_RETURN(2, 0xEE, INST_IMM(0).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_TYPE(0) == AT_INDIRECT &&
            (INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL)) {
        INST_RETURN(1, 0xAE)
    }
    if (INST_TYPE(0) == AT_INDEXED) {
        INST_RETURN(3, INST_INDEX_PREFIX(0), 0xAE, INST_INDEX(0).offset)
    }
    INST_ERROR(ARG_TYPE)
}

/* @AUTOGEN_INST_BLOCK_END */

/*
    Return the relevant ASMInstParser function for the given encoded mnemonic.
*/
static ASMInstParser lookup_parser(uint32_t key)
{
/* @AUTOGEN_LOOKUP_BLOCK_START */
    HANDLE(adc)
    HANDLE(add)
    HANDLE(and)
    HANDLE(bit)
    HANDLE(call)
    HANDLE(ccf)
    HANDLE(cp)
    HANDLE(cpd)
    HANDLE(cpdr)
    HANDLE(cpi)
    HANDLE(cpir)
    HANDLE(cpl)
    HANDLE(daa)
    HANDLE(dec)
    HANDLE(di)
    HANDLE(djnz)
    HANDLE(ei)
    HANDLE(ex)
    HANDLE(exx)
    HANDLE(halt)
    HANDLE(im)
    HANDLE(in)
    HANDLE(inc)
    HANDLE(ind)
    HANDLE(indr)
    HANDLE(ini)
    HANDLE(inir)
    HANDLE(jp)
    HANDLE(jr)
    HANDLE(ld)
    HANDLE(ldd)
    HANDLE(lddr)
    HANDLE(ldi)
    HANDLE(ldir)
    HANDLE(neg)
    HANDLE(nop)
    HANDLE(or)
    HANDLE(otdr)
    HANDLE(otir)
    HANDLE(out)
    HANDLE(outd)
    HANDLE(outi)
    HANDLE(pop)
    HANDLE(push)
    HANDLE(res)
    HANDLE(ret)
    HANDLE(reti)
    HANDLE(retn)
    HANDLE(rl)
    HANDLE(rla)
    HANDLE(rlc)
    HANDLE(rlca)
    HANDLE(rld)
    HANDLE(rr)
    HANDLE(rra)
    HANDLE(rrc)
    HANDLE(rrca)
    HANDLE(rrd)
    HANDLE(rst)
    HANDLE(sbc)
    HANDLE(scf)
    HANDLE(set)
    HANDLE(sl1)
    HANDLE(sla)
    HANDLE(sll)
    HANDLE(sls)
    HANDLE(sra)
    HANDLE(srl)
    HANDLE(sub)
    HANDLE(xor)
/* @AUTOGEN_LOOKUP_BLOCK_END */
    return NULL;
}
