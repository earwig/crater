/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "instructions.h"
#include "inst_support.h"

/* Instruction parser functions */

INST_FUNC(adc)
{
    INST_TAKES_ARGS(2, 2)
    INST_FORCE_TYPE(0, AT_REGISTER)
    switch (INST_REG(0)) {
        case REG_A:
            switch (INST_TYPE(1)) {
                case AT_REGISTER:
                    switch (INST_REG(1)) {
                        INST_HANDLE_MAIN_8_BIT_REGS(0x88)
                        default: INST_ERROR(ARG1_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    INST_CHECK_IMM(1, IMM_U8)
                    INST_RETURN(2, 0xCE, INST_IMM(1).uval)
                case AT_INDIRECT:
                    INST_INDIRECT_HL_ONLY(1)
                    INST_RETURN(1, 0x8E)
                case AT_INDEXED:
                    INST_RETURN(3, INST_INDEX_BYTES(1, 0x8E))
                default:
                    INST_ERROR(ARG1_TYPE)
            }
        case REG_HL:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(2, 0xED, 0x4A)
                case REG_DE: INST_RETURN(2, 0xED, 0x5A)
                case REG_HL: INST_RETURN(2, 0xED, 0x6A)
                case REG_SP: INST_RETURN(2, 0xED, 0x7A)
                default: INST_ERROR(ARG1_BAD_REG)
            }
        default:
            INST_ERROR(ARG0_TYPE)
    }
}

INST_FUNC(add)
{
    INST_TAKES_ARGS(2, 2)
    INST_FORCE_TYPE(0, AT_REGISTER)
    switch (INST_REG(0)) {
        case REG_A:
            switch (INST_TYPE(1)) {
                case AT_REGISTER:
                    switch (INST_REG(1)) {
                        INST_HANDLE_MAIN_8_BIT_REGS(0x80)
                        default: INST_ERROR(ARG1_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    INST_CHECK_IMM(1, IMM_U8)
                    INST_RETURN(2, 0xC6, INST_IMM(1).uval)
                case AT_INDIRECT:
                    INST_INDIRECT_HL_ONLY(1)
                    INST_RETURN(1, 0x86)
                case AT_INDEXED:
                    INST_RETURN(3, INST_INDEX_BYTES(1, 0x86))
                default:
                    INST_ERROR(ARG1_TYPE)
            }
        case REG_HL:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(1, 0x09)
                case REG_DE: INST_RETURN(1, 0x19)
                case REG_HL: INST_RETURN(1, 0x29)
                case REG_SP: INST_RETURN(1, 0x39)
                default: INST_ERROR(ARG1_BAD_REG)
            }
        case REG_IX:
        case REG_IY:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x09)
                case REG_DE: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x19)
                case REG_IX:
                case REG_IY:
                    if (INST_REG(0) != INST_REG(1))
                        INST_ERROR(ARG1_BAD_REG)
                    INST_RETURN(2, INST_INDEX_PREFIX(1), 0x29)
                case REG_SP: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x39)
                default: INST_ERROR(ARG1_BAD_REG)
            }
        default:
            INST_ERROR(ARG0_TYPE)
    }
}

INST_FUNC(and)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(bit)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(call)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ccf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x3F)
}

INST_FUNC(cp)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(di)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xF3)
}

INST_FUNC(djnz)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ei)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xFB)
}

INST_FUNC(ex)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(in)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(inc)
{
    INST_TAKES_ARGS(1, 1)
    switch (INST_TYPE(0)) {
        case AT_REGISTER:
            switch (INST_REG(0)) {
                case REG_A:   INST_RETURN(1, 0x3C)
                case REG_B:   INST_RETURN(1, 0x04)
                case REG_C:   INST_RETURN(1, 0x0C)
                case REG_D:   INST_RETURN(1, 0x14)
                case REG_E:   INST_RETURN(1, 0x1C)
                case REG_H:   INST_RETURN(1, 0x24)
                case REG_L:   INST_RETURN(1, 0x2C)
                case REG_BC:  INST_RETURN(1, 0x03)
                case REG_DE:  INST_RETURN(1, 0x13)
                case REG_HL:  INST_RETURN(1, 0x23)
                case REG_SP:  INST_RETURN(1, 0x33)
                case REG_IX:  INST_RETURN(2, 0xDD, 0x23)
                case REG_IY:  INST_RETURN(2, 0xFD, 0x23)
                case REG_IXH: INST_RETURN(2, 0xDD, 0x2C)
                case REG_IXL: INST_RETURN(2, 0xFD, 0x2C)
                case REG_IYH: INST_RETURN(2, 0xDD, 0x2C)
                case REG_IYL: INST_RETURN(2, 0xFD, 0x2C)
                default: INST_ERROR(ARG0_BAD_REG)
            }
        case AT_INDIRECT:
            INST_INDIRECT_HL_ONLY(0)
            INST_RETURN(1, 0x34)
        case AT_INDEXED:
            INST_RETURN(3, INST_INDEX_BYTES(0, 0x34))
        default:
            INST_ERROR(ARG0_TYPE)
    }
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(jr)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ld)
{
    uint8_t base;
    INST_TAKES_ARGS(2, 2)
    switch (INST_TYPE(0)) {
        case AT_REGISTER:
            switch (INST_REG(0)) {
                case REG_A:
                    switch (INST_TYPE(1)) {
                        case AT_REGISTER:
                            switch (INST_REG(1)) {
                                INST_HANDLE_MAIN_8_BIT_REGS(0x78)
                                case REG_I:   INST_RETURN(2, 0xED, 0x57)
                                case REG_R:   INST_RETURN(2, 0xED, 0x5F)
                                default: INST_ERROR(ARG1_BAD_REG)
                            }
                        case AT_IMMEDIATE:
                            INST_CHECK_IMM(1, IMM_U8)
                            INST_RETURN(2, 0x3E, INST_IMM(1).uval)
                        case AT_INDIRECT:
                            switch (INST_INDIRECT(1).type) {
                                case AT_REGISTER:
                                    switch (INST_INDIRECT(1).addr.reg) {
                                        case REG_BC: INST_RETURN(1, 0x0A)
                                        case REG_DE: INST_RETURN(1, 0x1A)
                                        case REG_HL: INST_RETURN(1, 0x7E)
                                        default: INST_ERROR(ARG0_BAD_REG)
                                    }
                                case AT_IMMEDIATE:
                                    INST_RETURN(3, 0x3A, INST_INDIRECT_IMM(1))
                                case AT_LABEL:
                                    INST_RETURN_INDIRECT_LABEL(1, 3, 0x3A)
                                default:
                                    INST_ERROR(ARG1_TYPE)
                            }
                        case AT_INDEXED:
                            INST_RETURN(3, INST_INDEX_BYTES(1, 0x7E))
                        default:
                            INST_ERROR(ARG1_TYPE)
                    }
                case REG_B:
                    base = 0x00;
                case REG_C:
                    base = 0x08;
                case REG_D:
                    base = 0x10;
                case REG_E:
                    base = 0x18;
                    switch (INST_TYPE(1)) {
                        case AT_REGISTER:
                            switch (INST_REG(1)) {
                                INST_HANDLE_MAIN_8_BIT_REGS(base + 0x40)
                                default: INST_ERROR(ARG1_BAD_REG)
                            }
                        case AT_IMMEDIATE:
                            INST_CHECK_IMM(1, IMM_U8)
                            INST_RETURN(2, base + 0x06, INST_IMM(1).uval)
                        case AT_INDIRECT:
                            INST_INDIRECT_HL_ONLY(1)
                            INST_RETURN(1, base + 0x46)
                        case AT_INDEXED:
                            INST_RETURN(3, INST_INDEX_BYTES(1, base + 0x46))
                        default:
                            INST_ERROR(ARG1_TYPE)
                    }
                case REG_H:   // TODO (11 cases)
                case REG_L:   // TODO (11 cases)
                case REG_I:
                    INST_REG_ONLY(1, REG_A)
                    INST_RETURN(2, 0xED, 0x47)
                case REG_R:
                    INST_REG_ONLY(1, REG_A)
                    INST_RETURN(2, 0xED, 0x4F)
                case REG_BC:  // TODO ( 2 cases)
                case REG_DE:  // TODO ( 2 cases)
                case REG_HL:  // TODO ( 3 cases)
                case REG_IX:  // TODO ( 2 cases)
                case REG_IY:  // TODO ( 2 cases)
                case REG_SP:  // TODO ( 5 cases)
                case REG_IXH: // TODO ( 8 cases)
                case REG_IXL: // TODO ( 8 cases)
                case REG_IYH: // TODO ( 8 cases)
                case REG_IYL: // TODO ( 8 cases)
                default: INST_ERROR(ARG0_BAD_REG)
            }
        case AT_INDIRECT:
            switch (INST_INDIRECT(0).type) {
                case AT_REGISTER:
                    switch (INST_INDIRECT(0).addr.reg) {
                        case REG_BC: // TODO (1 case )
                        case REG_DE: // TODO (1 case )
                        case REG_HL: // TODO (8 cases)
                        default: INST_ERROR(ARG0_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    // TODO (8 cases)
                case AT_LABEL:
                    // TODO (same 8 cases)
                default:
                    INST_ERROR(ARG0_TYPE)
            }
        case AT_INDEXED:
            // TODO (16 cases)
        default:
            INST_ERROR(ARG0_TYPE)
    }
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(nop)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x00)
}

INST_FUNC(or)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(push)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(res)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ret)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rla)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x17)
}

INST_FUNC(rlc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rra)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x1F)
}

INST_FUNC(rrc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rrca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x0F)
}

INST_FUNC(rrd)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rst)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sbc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(scf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x37)
}

INST_FUNC(set)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sl1)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sla)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sll)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sls)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sra)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(srl)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sub)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(xor)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

/*
    Return the relevant ASMInstParser function for a given mnemonic.

    NULL is returned if the mnemonic is not known.
*/
ASMInstParser get_inst_parser(char mstr[MAX_MNEMONIC_SIZE])
{
    // Exploit the fact that we can store the entire mnemonic string as a
    // single 32-bit value to do fast lookups:
    uint32_t key = (mstr[0] << 24) + (mstr[1] << 16) + (mstr[2] << 8) + mstr[3];

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

    return NULL;
}
