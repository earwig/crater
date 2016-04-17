/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stddef.h>

#include "arguments.h"
#include "../logging.h"
#include "../util.h"

#define MAX_ARG_SIZE 256

typedef enum {
    AT_NONE,
    /* Register */
    AT_REG_A,   AT_REG_B,   AT_REG_C,   AT_REG_D,   AT_REG_E,   AT_REG_H,
    AT_REG_L,   AT_REG_I,   AT_REG_R,   AT_REG_AF,  AT_REG_BC,  AT_REG_DE,
    AT_REG_HL,  AT_REG_IX,  AT_REG_IY,  AT_REG_SP,  AT_REG_AF_, AT_REG_IXH,
    AT_REG_IXL, AT_REG_IYH, AT_REG_IYL,
    /* Immediate */
    AT_IMM_U16, AT_IMM_U8,  AT_IMM_S8,  AT_IMM_REL, AT_IMM_BIT, AT_IMM_RST,
    AT_IMM_IM,
    /* Indirect */
    AT_IDR_HL,  AT_IDR_BC,  AT_IDR_DE,  AT_IDR_SP,  AT_IDR_IMM,
    /* Indexed */
    AT_IDX_IX,  AT_IDX_IY,
    /* Condition */
    AT_COND_NZ, AT_COND_Z,  AT_COND_NC, AT_COND_C,  AT_COND_PO, AT_COND_PE,
    AT_COND_P,  AT_COND_M,
    /* Port */
    AT_PORT_C,  AT_PORT_IM
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
#define IX AT_REG_IX
#define IY AT_REG_IY
#define SP AT_REG_SP
#define AS AT_REG_AF_
#define XH AT_REG_IXH
#define XL AT_REG_IXL
#define YH AT_REG_IYH
#define YL AT_REG_IYL
#define M2 AT_IMM_U16
#define M1 AT_IMM_U8
#define MS AT_IMM_S8
#define ML AT_IMM_REL
#define MB AT_IMM_BIT
#define MR AT_IMM_RST
#define MI AT_IMM_IM
#define NH AT_IDR_HL
#define NB AT_IDR_BC
#define ND AT_IDR_DE
#define NS AT_IDR_SP
#define NM AT_IDR_IMM
#define DX AT_IDX_IX
#define DY AT_IDX_IY
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

static ArgType instr_args[3][256] = {
    {
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, A_, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
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
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
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
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
        __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
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
#undef IX
#undef IY
#undef SP
#undef AS
#undef XH
#undef XL
#undef YH
#undef YL
#undef M2
#undef M1
#undef MS
#undef ML
#undef MB
#undef MR
#undef MI
#undef NH
#undef NB
#undef ND
#undef NS
#undef NM
#undef DX
#undef DY
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

/*
    Decode a single argument, given its type.
*/
static void decode_argument(char *arg, ArgType type, const uint8_t *bytes)
{
    switch (type) {
        case AT_NONE:    arg[0] = '\0';       break;
        case AT_REG_A:   strcpy(arg, "a");    break;
        case AT_REG_B:   strcpy(arg, "b");    break;
        case AT_REG_C:   strcpy(arg, "c");    break;
        case AT_REG_D:   strcpy(arg, "d");    break;
        case AT_REG_E:   strcpy(arg, "e");    break;
        case AT_REG_H:   strcpy(arg, "h");    break;
        case AT_REG_L:   strcpy(arg, "l");    break;
        case AT_REG_I:   strcpy(arg, "i");    break;
        case AT_REG_R:   strcpy(arg, "r");    break;
        case AT_REG_AF:  strcpy(arg, "af");   break;
        case AT_REG_BC:  strcpy(arg, "bc");   break;
        case AT_REG_DE:  strcpy(arg, "de");   break;
        case AT_REG_HL:  strcpy(arg, "hl");   break;
        case AT_REG_IX:  strcpy(arg, "ix");   break;
        case AT_REG_IY:  strcpy(arg, "iy");   break;
        case AT_REG_SP:  strcpy(arg, "sp");   break;
        case AT_REG_AF_: strcpy(arg, "af'");  break;
        case AT_REG_IXH: strcpy(arg, "ixh");  break;
        case AT_REG_IXL: strcpy(arg, "ixl");  break;
        case AT_REG_IYH: strcpy(arg, "iyh");  break;
        case AT_REG_IYL: strcpy(arg, "iyl");  break;
        case AT_IMM_U16: strcpy(arg, "???");  break;  // TODO
        case AT_IMM_U8:  strcpy(arg, "???");  break;  // TODO
        case AT_IMM_S8:  strcpy(arg, "???");  break;  // TODO
        case AT_IMM_REL: strcpy(arg, "???");  break;  // TODO
        case AT_IMM_BIT: strcpy(arg, "???");  break;  // TODO
        case AT_IMM_RST: strcpy(arg, "???");  break;  // TODO
        case AT_IMM_IM:  strcpy(arg, "???");  break;  // TODO
        case AT_IDR_HL:  strcpy(arg, "(hl)"); break;
        case AT_IDR_BC:  strcpy(arg, "(bc)"); break;
        case AT_IDR_DE:  strcpy(arg, "(de)"); break;
        case AT_IDR_SP:  strcpy(arg, "(sp)"); break;
        case AT_IDR_IMM: strcpy(arg, "???");  break;  // TODO
        case AT_IDX_IX:  strcpy(arg, "???");  break;  // TODO
        case AT_IDX_IY:  strcpy(arg, "???");  break;  // TODO
        case AT_COND_NZ: strcpy(arg, "nz");   break;
        case AT_COND_Z:  strcpy(arg, "z");    break;
        case AT_COND_NC: strcpy(arg, "nc");   break;
        case AT_COND_C:  strcpy(arg, "c");    break;
        case AT_COND_PO: strcpy(arg, "po");   break;
        case AT_COND_PE: strcpy(arg, "pe");   break;
        case AT_COND_P:  strcpy(arg, "p");    break;
        case AT_COND_M:  strcpy(arg, "m");    break;
        case AT_PORT_C:  strcpy(arg, "(c)");  break;
        case AT_PORT_IM: strcpy(arg, "???");  break;  // TODO
        default:
            FATAL("invalid call: decode_argument(%u, ...)", type)
    }
}

/*
    Extract the arguments for the given instruction.

    The return value must be free()d.
*/
char* decode_arguments(const uint8_t *bytes)
{
    char args[3][MAX_ARG_SIZE], *result;
    ArgType type;
    size_t i;

    for (i = 0; i < 3; i++) {
        type = instr_args[i][bytes[0]];
        decode_argument(args[i], type, bytes);
    }

    if (!*args[0])
        return NULL;
    if (!*args[1])
        return cr_strdup(args[0]);
    if (!*args[2])
        // TODO
        return NULL;
    // TODO
    return NULL;
}
