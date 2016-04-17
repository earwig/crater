/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stddef.h>

#include "arguments.h"
#include "../logging.h"
#include "../util.h"

#define MAX_ARG_SIZE 256

typedef enum {
    AT_NONE = 0,
    /* Register */
    AT_REG_A,   AT_REG_B,   AT_REG_C,   AT_REG_D,   AT_REG_E,   AT_REG_H,
    AT_REG_L,   AT_REG_I,   AT_REG_R,   AT_REG_AF,  AT_REG_BC,  AT_REG_DE,
    AT_REG_HL,  AT_REG_IXY, AT_REG_SP,  AT_REG_AF_, AT_REG_IH,  AT_REG_IL,
    /* Immediate */
    AT_IMM_U16, AT_IMM_U8,  AT_IMM_REL, AT_IMM_BIT, AT_IMM_RST, AT_IMM_IM,
    /* Indirect */
    AT_IDR_HL,  AT_IDR_BC,  AT_IDR_DE,  AT_IDR_SP,  AT_IDR_IXY, AT_IDR_IMM,
    /* Indexed */
    AT_IX_IY,
    /* Condition */
    AT_COND_NZ, AT_COND_Z,  AT_COND_NC, AT_COND_C,  AT_COND_PO, AT_COND_PE,
    AT_COND_P,  AT_COND_M,
    /* Port */
    AT_PORT_C,  AT_PORT_IM, AT_PORT_0
} ArgType;

#define __ AT_NONE
#define A_ AT_REG_A
#define B_ AT_REG_B
#define C_ AT_REG_C
#define D_ AT_REG_D
#define E_ AT_REG_E
#define H_ AT_REG_H
#define L_ AT_REG_L
#define I_ AT_REG_I
#define R_ AT_REG_R
#define AF AT_REG_AF
#define BC AT_REG_BC
#define DE AT_REG_DE
#define HL AT_REG_HL
#define XY AT_REG_IXY
#define SP AT_REG_SP
#define AS AT_REG_AF_
#define IH AT_REG_IH
#define IL AT_REG_IL
#define M2 AT_IMM_U16
#define M1 AT_IMM_U8
#define ML AT_IMM_REL
#define MB AT_IMM_BIT
#define MR AT_IMM_RST
#define MI AT_IMM_IM
#define NH AT_IDR_HL
#define NB AT_IDR_BC
#define ND AT_IDR_DE
#define NS AT_IDR_SP
#define NI AT_IDR_IXY
#define NM AT_IDR_IMM
#define II AT_IX_IY
#define NZ AT_COND_NZ
#define Z_ AT_COND_Z
#define NC AT_COND_NC
#define CC AT_COND_C
#define PO AT_COND_PO
#define PE AT_COND_PE
#define P_ AT_COND_P
#define M_ AT_COND_M
#define PC AT_PORT_C
#define PM AT_PORT_IM
#define R0 AT_PORT_0

static ArgType instr_args[3][256] = {
    {
        __, BC, NB, BC, B_, B_, B_, __, AF, HL, A_, BC, C_, C_, C_, __,
        ML, DE, ND, DE, D_, D_, D_, __, ML, HL, A_, DE, E_, E_, E_, __,
        NZ, HL, NM, HL, H_, H_, H_, __, Z_, HL, HL, HL, L_, L_, L_, __,
        NC, SP, NM, SP, NH, NH, NH, __, CC, HL, A_, SP, A_, A_, A_, __,
        B_, B_, B_, B_, B_, B_, B_, B_, C_, C_, C_, C_, C_, C_, C_, C_,
        D_, D_, D_, D_, D_, D_, D_, D_, E_, E_, E_, E_, E_, E_, E_, E_,
        H_, H_, H_, H_, H_, H_, H_, H_, L_, L_, L_, L_, L_, L_, L_, L_,
        NH, NH, NH, NH, NH, NH, __, NH, A_, A_, A_, A_, A_, A_, A_, A_,
        A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, A_, A_, A_, A_, A_, A_, A_, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        NZ, BC, NZ, M2, NZ, BC, A_, MR, Z_, __, Z_, __, Z_, M2, A_, MR,
        NC, DE, NC, PM, NC, DE, M1, MR, CC, __, CC, A_, CC, __, A_, MR,
        PO, HL, PO, NS, PO, HL, M1, MR, PE, NH, PE, DE, PE, __, M1, MR,
        P_, AF, P_, __, P_, AF, M1, MR, M_, SP, M_, __, M_, __, M1, MR
    },
    {
        __, M2, A_, __, __, __, M1, __, AS, BC, NB, __, __, __, M1, __,
        __, M2, A_, __, __, __, M1, __, __, DE, ND, __, __, __, M1, __,
        ML, M2, HL, __, __, __, M1, __, ML, HL, NM, __, __, __, M1, __,
        ML, M2, A_, __, __, __, M1, __, ML, SP, NM, __, __, __, M1, __,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        __, __, __, __, __, __, __, __, B_, C_, D_, E_, H_, L_, NH, A_,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, M2, M2, M2, __, M1, __, __, __, M2, __, M2, __, M1, __,
        __, __, M2, A_, M2, __, __, __, __, __, M2, PM, M2, __, M1, __,
        __, __, M2, HL, M2, __, __, __, __, __, M2, HL, M2, __, __, __,
        __, __, M2, __, M2, __, __, __, __, HL, M2, __, M2, __, __, __
    },
    { __ }
};

static ArgType instr_args_extended[3][256] = {
    {
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        B_, PC, HL, NM, __, __, MI, I_, C_, PC, HL, BC, __, __, MI, R_,
        D_, PC, HL, NM, __, __, MI, A_, E_, PC, HL, DE, __, __, MI, A_,
        H_, PC, HL, NM, __, __, MI, __, L_, PC, HL, HL, __, __, MI, __,
        PC, PC, HL, NM, __, __, MI, __, A_, PC, HL, SP, __, __, MI, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
    },
    {
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        PC, B_, BC, BC, __, __, __, A_, PC, C_, BC, NM, __, __, __, A_,
        PC, D_, DE, DE, __, __, __, I_, PC, E_, DE, NM, __, __, __, R_,
        PC, H_, HL, HL, __, __, __, __, PC, L_, HL, NM, __, __, __, __,
        __, R0, SP, SP, __, __, __, __, PC, A_, SP, NM, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
    },
    { __ }
};

static ArgType instr_args_bits[3][256] = {
    {
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB
    },
    {
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_,
        B_, C_, D_, E_, H_, L_, NH, A_, B_, C_, D_, E_, H_, L_, NH, A_
    },
    { __ }
};

static ArgType instr_args_index[3][256] = {
    {
        __, __, __, __, __, __, __, __, __, XY, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, XY, __, __, __, __, __, __,
        __, XY, NM, XY, IH, IH, IH, __, __, XY, XY, XY, IL, IL, IL, __,
        __, __, __, __, II, II, II, __, __, XY, __, __, __, __, __, __,
        __, __, __, __, B_, B_, B_, __, __, __, __, __, C_, C_, C_, __,
        __, __, __, __, D_, D_, D_, __, __, __, __, __, E_, E_, E_, __,
        IH, IH, IH, IH, IH, IH, H_, IH, IL, IL, IL, IL, IL, IL, L_, IL,
        II, II, II, II, II, II, __, II, __, __, __, __, A_, A_, A_, __,
        __, __, __, __, A_, A_, A_, __, __, __, __, __, A_, A_, A_, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, A_, A_, A_, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, IH, IL, II, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, IH, IL, II, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, XY, __, NS, __, XY, __, __, __, NI, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, SP, __, __, __, __, __, __
    },
    {
        __, __, __, __, __, __, __, __, __, BC, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, DE, __, __, __, __, __, __,
        __, M2, XY, __, __, __, M1, __, __, XY, NM, __, __, __, M1, __,
        __, __, __, __, __, __, M1, __, __, SP, __, __, __, __, __, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, IH, IL, II, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, IH, IL, II, __,
        B_, C_, D_, E_, IH, IL, II, A_, B_, C_, D_, E_, IH, IL, II, A_,
        B_, C_, D_, E_, H_, L_, __, A_, __, __, __, __, IH, IL, II, __,
        __, __, __, __, IH, IL, II, __, __, __, __, __, IH, IL, II, __,
        __, __, __, __, __, __, __, __, __, __, __, __, IH, IL, II, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, XY, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, XY, __, __, __, __, __, __
    },
    { __ }
};

static ArgType instr_args_index_bits[3][256] = {
    {
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB,
        MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB, MB
    },
    {
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II,
        II, II, II, II, II, II, II, II, II, II, II, II, II, II, II, II
    },
    {
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_,
        B_, C_, D_, E_, H_, L_, __, A_, B_, C_, D_, E_, H_, L_, __, A_
    }
};

#undef __
#undef A_
#undef B_
#undef C_
#undef D_
#undef E_
#undef H_
#undef L_
#undef I_
#undef R_
#undef AF
#undef BC
#undef DE
#undef HL
#undef XY
#undef SP
#undef AS
#undef IH
#undef IL
#undef M2
#undef M1
#undef ML
#undef MB
#undef MR
#undef MI
#undef NH
#undef NB
#undef ND
#undef NS
#undef NI
#undef NM
#undef II
#undef NZ
#undef Z_
#undef NC
#undef CC
#undef PO
#undef PE
#undef P_
#undef M_
#undef PC
#undef PM
#undef R0

/*
    Decode an immediate argument.
*/
static void decode_immediate(
    char *arg, ArgType type, const uint8_t *bytes, size_t shift)
{
    char *format;
    size_t take;
    uint8_t b = bytes[shift];

    switch (type) {
        case AT_IMM_U16:  // 16-bit immediate
            sprintf(arg, "$%04X", bytes[shift + 1] + (bytes[shift + 2] << 8));
            break;
        case AT_IMM_U8:  // 8-bit unsigned immediate
            if ((bytes[0] == 0xDD || bytes[0] == 0xFD) && bytes[1] == 0x36)
                take = 2;
            else
                take = 1;
            sprintf(arg, "$%02X", bytes[shift + take]);
            break;
        case AT_IMM_REL:  // 8-bit relative offset (JP and DJNZ)
            sprintf(arg, "%hhd", (int8_t) (bytes[shift + 1] + 2));
            break;
        case AT_IMM_BIT:  // Bit position
            sprintf(arg, "%d", (b & 0x38) >> 3);
            break;
        case AT_IMM_RST:  // Reset
            sprintf(arg, "$%02X", b & 0x38);
            break;
        case AT_IMM_IM:  // Interrupt mode
            sprintf(arg, "%d", !(b & (1 << 4)) ? 0 : !(b & (1 << 3)) ? 1 : 2);
            break;
        case AT_IDR_IMM:  // Indirect immediate
            sprintf(arg, "($%04X)", bytes[shift + 1] + (bytes[shift + 2] << 8));
            break;
        case AT_IX_IY:  // Indexed offset
            format = bytes[0] == 0xDD ? "(ix%+hhd)" : "(iy%+hhd)";
            sprintf(arg, format, (int8_t) bytes[shift + 1]);
            break;
        case AT_PORT_IM:  // Immediate port
            sprintf(arg, "(%u)", bytes[shift + 1]);
            break;
        default:
            FATAL("invalid call: decode_immediate(arg, %d, ...)", type)
            return;
    }
}

/*
    Decode a single argument, given its type.
*/
static void decode_argument(
    char *arg, ArgType type, const uint8_t *bytes, size_t shift)
{
    const char *value;

    switch (type) {
        case AT_NONE:
            arg[0] = '\0';
            return;
        case AT_REG_A:   value = "a";    break;
        case AT_REG_B:   value = "b";    break;
        case AT_REG_C:   value = "c";    break;
        case AT_REG_D:   value = "d";    break;
        case AT_REG_E:   value = "e";    break;
        case AT_REG_H:   value = "h";    break;
        case AT_REG_L:   value = "l";    break;
        case AT_REG_I:   value = "i";    break;
        case AT_REG_R:   value = "r";    break;
        case AT_REG_AF:  value = "af";   break;
        case AT_REG_BC:  value = "bc";   break;
        case AT_REG_DE:  value = "de";   break;
        case AT_REG_HL:  value = "hl";   break;
        case AT_REG_SP:  value = "sp";   break;
        case AT_REG_AF_: value = "af'";  break;
        case AT_IDR_HL:  value = "(hl)"; break;
        case AT_IDR_BC:  value = "(bc)"; break;
        case AT_IDR_DE:  value = "(de)"; break;
        case AT_IDR_SP:  value = "(sp)"; break;
        case AT_COND_NZ: value = "nz";   break;
        case AT_COND_Z:  value = "z";    break;
        case AT_COND_NC: value = "nc";   break;
        case AT_COND_C:  value = "c";    break;
        case AT_COND_PO: value = "po";   break;
        case AT_COND_PE: value = "pe";   break;
        case AT_COND_P:  value = "p";    break;
        case AT_COND_M:  value = "m";    break;
        case AT_PORT_C:  value = "(c)";  break;
        case AT_PORT_0:  value = "0";    break;
        case AT_REG_IXY: value = (bytes[0] == 0xDD) ? "ix"   : "iy";   break;
        case AT_REG_IH:  value = (bytes[0] == 0xDD) ? "ixh"  : "iyh";  break;
        case AT_REG_IL:  value = (bytes[0] == 0xDD) ? "ixl"  : "iyl";  break;
        case AT_IDR_IXY: value = (bytes[0] == 0xDD) ? "(ix)" : "(iy)";  break;
        case AT_IMM_U16:
        case AT_IMM_U8:
        case AT_IMM_REL:
        case AT_IMM_BIT:
        case AT_IMM_RST:
        case AT_IMM_IM:
        case AT_IDR_IMM:
        case AT_IX_IY:
        case AT_PORT_IM:
            decode_immediate(arg, type, bytes, shift);
            return;
        default:
            FATAL("invalid call: decode_argument(arg, %d, ...)", type)
            return;
    }
    strcpy(arg, value);
}

/*
    Return the appropriate argument table for the given instruction.

    This function also adds the number of instruction bytes to skip to its
    shift argument (corresponding to any prefix bits), so bytes[*shift] will
    always be the instruction opcode.

    This function return type is ridiculous, but it returns a pointer to an
    array of 3 arrays of 256 ArgTypes.
*/
static inline ArgType (*get_table_and_shift(
    const uint8_t *bytes, size_t *shift))[3][256]
{
    uint8_t b = bytes[0];

    if (b == 0xED)
        return (*shift)++, &instr_args_extended;
    if (b == 0xCB)
        return (*shift)++, &instr_args_bits;
    if (b == 0xDD || b == 0xFD) {
        if (bytes[1] == 0xCB)
            return (*shift) += 2, &instr_args_index_bits;
        return (*shift)++, &instr_args_index;
    }
    return &instr_args;
}

/*
    Extract the arguments for the given instruction.

    The return value must be free()d.
*/
char* decode_arguments(const uint8_t *bytes)
{
    char args[3][MAX_ARG_SIZE], *result;
    ArgType (*table)[3][256], type;
    size_t shift = 0, i, len;

    table = get_table_and_shift(bytes, &shift);
    for (i = 0; i < 3; i++) {
        type = (*table)[i][bytes[shift]];
        decode_argument(args[i], type, bytes, shift);
    }

    if (!*args[0])
        return NULL;
    if (!*args[1])
        return cr_strdup(args[0]);

    // Two or three arguments; need to add commas:
    len = strlen(args[0]) + strlen(args[1]) + strlen(args[2]);
    result = malloc(sizeof(char) * (len + 2 * (*args[2] ? 3 : 2) + 1));
    if (*args[2])
        sprintf(result, "%s, %s, %s", args[0], args[1], args[2]);
    else
        sprintf(result, "%s, %s", args[0], args[1]);
    return result;
}
