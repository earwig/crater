/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

/*
    This file is AUTO-GENERATED from 'instructions.yml'.

    `make` should trigger a rebuild when it is modified; if not, use:
    `python scripts/update_asm_instructions.py`.

    @AUTOGEN_DATE Sun May 17 03:37:44 2015 UTC
*/

/* @AUTOGEN_INST_BLOCK_START */

INST_FUNC(adc)
{
    INST_TAKES_ARGS(
        AT_REGISTER,
        AT_IMMEDIATE|AT_INDIRECT|AT_INDEXED|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
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
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x8D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x8D)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x8D)
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
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0xCE, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT) {
        if (INST_REG(0) == REG_A && (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL))
            INST_RETURN(1, 0x8E)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
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
        AT_IMMEDIATE|AT_INDIRECT|AT_INDEXED|AT_REGISTER,
        AT_NONE
    )
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_REGISTER) {
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
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x85)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_L)
            INST_RETURN(1, 0x85)
        if (INST_REG(0) == REG_A && INST_REG(1) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x85)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_BC)
            INST_RETURN(2, INST_IY_PREFIX, 0x09)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_BC)
            INST_RETURN(2, INST_IX_PREFIX, 0x09)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_BC)
            INST_RETURN(1, 0x09)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_DE)
            INST_RETURN(2, INST_IY_PREFIX, 0x19)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_DE)
            INST_RETURN(2, INST_IX_PREFIX, 0x19)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_DE)
            INST_RETURN(1, 0x19)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_HL)
            INST_RETURN(2, INST_IY_PREFIX, 0x29)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_HL)
            INST_RETURN(2, INST_IX_PREFIX, 0x29)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_HL)
            INST_RETURN(1, 0x29)
        if (INST_REG(0) == REG_IY && INST_REG(1) == REG_SP)
            INST_RETURN(2, INST_IY_PREFIX, 0x39)
        if (INST_REG(0) == REG_IX && INST_REG(1) == REG_SP)
            INST_RETURN(2, INST_IX_PREFIX, 0x39)
        if (INST_REG(0) == REG_HL && INST_REG(1) == REG_SP)
            INST_RETURN(1, 0x39)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_IMMEDIATE) {
        if (INST_REG(0) == REG_A && INST_IMM(1).mask & IMM_U8)
            INST_RETURN(2, 0xC6, INST_IMM(1).uval)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDIRECT) {
        if (INST_REG(0) == REG_A && (INST_INDIRECT(1).type == AT_REGISTER && INST_INDIRECT(1).addr.reg == REG_HL))
            INST_RETURN(1, 0x86)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 2 && INST_TYPE(0) == AT_REGISTER && INST_TYPE(1) == AT_INDEXED) {
        if (INST_REG(0) == REG_A)
            INST_RETURN(3, INST_INDEX_PREFIX(1), 0x86, INST_INDEX(1).offset)
        INST_ERROR(ARG_VALUE)
    }
    INST_ERROR(ARG_TYPE)
}

INST_FUNC(ccf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x3F)
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

INST_FUNC(di)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xF3)
}

INST_FUNC(ei)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xFB)
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

INST_FUNC(inc)
{
    INST_TAKES_ARGS(
        AT_INDEXED|AT_INDIRECT|AT_REGISTER,
        AT_NONE,
        AT_NONE
    )
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_REGISTER) {
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
        if (INST_REG(0) == REG_IYL)
            INST_RETURN(2, INST_IY_PREFIX, 0x2C)
        if (INST_REG(0) == REG_L)
            INST_RETURN(1, 0x2C)
        if (INST_REG(0) == REG_IXL)
            INST_RETURN(2, INST_IX_PREFIX, 0x2C)
        if (INST_REG(0) == REG_BC)
            INST_RETURN(1, 0x03)
        if (INST_REG(0) == REG_DE)
            INST_RETURN(1, 0x13)
        if (INST_REG(0) == REG_IY)
            INST_RETURN(2, INST_IY_PREFIX, 0x23)
        if (INST_REG(0) == REG_IX)
            INST_RETURN(2, INST_IX_PREFIX, 0x23)
        if (INST_REG(0) == REG_HL)
            INST_RETURN(1, 0x23)
        if (INST_REG(0) == REG_SP)
            INST_RETURN(1, 0x33)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDIRECT) {
        if ((INST_INDIRECT(0).type == AT_REGISTER && INST_INDIRECT(0).addr.reg == REG_HL))
            INST_RETURN(1, 0x34)
        INST_ERROR(ARG_VALUE)
    }
    if (INST_NARGS == 1 && INST_TYPE(0) == AT_INDEXED) {
        if (1)
            INST_RETURN(3, INST_INDEX_PREFIX(0), 0x34, INST_INDEX(0).offset)
        INST_ERROR(ARG_VALUE)
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

INST_FUNC(rla)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x17)
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

INST_FUNC(rra)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x1F)
}

INST_FUNC(rrca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x0F)
}

INST_FUNC(scf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x37)
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
    HANDLE(ccf)
    HANDLE(cpd)
    HANDLE(cpdr)
    HANDLE(cpi)
    HANDLE(cpir)
    HANDLE(cpl)
    HANDLE(daa)
    HANDLE(di)
    HANDLE(ei)
    HANDLE(exx)
    HANDLE(halt)
    HANDLE(inc)
    HANDLE(ind)
    HANDLE(indr)
    HANDLE(ini)
    HANDLE(inir)
    HANDLE(ldd)
    HANDLE(lddr)
    HANDLE(ldi)
    HANDLE(ldir)
    HANDLE(neg)
    HANDLE(nop)
    HANDLE(otdr)
    HANDLE(otir)
    HANDLE(outd)
    HANDLE(outi)
    HANDLE(reti)
    HANDLE(retn)
    HANDLE(rla)
    HANDLE(rlca)
    HANDLE(rld)
    HANDLE(rra)
    HANDLE(rrca)
    HANDLE(scf)
/* @AUTOGEN_LOOKUP_BLOCK_END */
    return NULL;
}
