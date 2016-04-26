/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "z80.h"
#include "disassembler.h"
#include "logging.h"
#include "util.h"

#define REG_AF  0
#define REG_BC  1
#define REG_DE  2
#define REG_HL  3
#define REG_SP  4
#define REG_AF_ 5
#define REG_BC_ 6
#define REG_DE_ 7
#define REG_HL_ 8

#define FLAG_CARRY     0
#define FLAG_SUBTRACT  1
#define FLAG_PARITY    2
#define FLAG_OVERFLOW  2
#define FLAG_UNDOC_3   3
#define FLAG_HALFCARRY 4
#define FLAG_UNDOC_5   5
#define FLAG_ZERO      6
#define FLAG_SIGN      7

/*
    Initialize a Z80 object.

    Register values are invalid until z80_power() is called. No other Z80
    functions should be called before it.
*/
void z80_init(Z80 *z80, MMU *mmu, IO *io)
{
    z80->mmu = mmu;
    z80->io = io;
    z80->except = true;
    z80->exc_code = Z80_EXC_NOT_POWERED;
    z80->exc_data = 0;
}

/*
    Power on the Z80, setting registers to their default values.

    This also clears the exception flag, which is necessary before the Z80 can
    begin emulation.
*/
void z80_power(Z80 *z80)
{
    Z80RegFile *regfile = &z80->regfile;

    regfile->a  = regfile->f  = 0xFF;
    regfile->b  = regfile->c  = 0xFF;
    regfile->d  = regfile->e  = 0xFF;
    regfile->h  = regfile->l  = 0xFF;
    regfile->a_ = regfile->f_ = 0xFF;
    regfile->b_ = regfile->c_ = 0xFF;
    regfile->d_ = regfile->e_ = 0xFF;
    regfile->h_ = regfile->l_ = 0xFF;

    regfile->ix = 0xFFFF;
    regfile->iy = 0xFFFF;
    regfile->sp = 0xFFF0;
    regfile->pc = 0x0000;

    regfile->i = 0xFF;
    regfile->r = 0xFF;

    regfile->im_a = regfile->im_b = 0;
    regfile->iff1 = regfile->iff2 = 0;

    z80->except = false;
    z80->pending_cycles = 0;

    z80->trace.fresh = true;
    z80->trace.last_addr = 0;
    z80->trace.counter = 0;
}

/*
    Get the value of a register pair.
*/
static inline uint16_t get_pair(Z80 *z80, uint8_t pair)
{
    switch (pair) {
        case REG_AF:  return (z80->regfile.a  << 8) + z80->regfile.f;
        case REG_BC:  return (z80->regfile.b  << 8) + z80->regfile.c;
        case REG_DE:  return (z80->regfile.d  << 8) + z80->regfile.e;
        case REG_HL:  return (z80->regfile.h  << 8) + z80->regfile.l;
        case REG_AF_: return (z80->regfile.a_ << 8) + z80->regfile.f_;
        case REG_BC_: return (z80->regfile.b_ << 8) + z80->regfile.c_;
        case REG_DE_: return (z80->regfile.d_ << 8) + z80->regfile.e_;
        case REG_HL_: return (z80->regfile.h_ << 8) + z80->regfile.l_;
        case REG_SP:  return z80->regfile.sp;
    }
    FATAL("invalid call: get_pair(z80, %u)", pair)
}

/*
    Set the value of a register pair.
*/
static inline void set_pair(Z80 *z80, uint8_t pair, uint16_t value)
{
    switch (pair) {
        case REG_AF:  z80->regfile.a  = value >> 8; z80->regfile.f  = value; break;
        case REG_BC:  z80->regfile.b  = value >> 8; z80->regfile.c  = value; break;
        case REG_DE:  z80->regfile.d  = value >> 8; z80->regfile.e  = value; break;
        case REG_HL:  z80->regfile.h  = value >> 8; z80->regfile.l  = value; break;
        case REG_AF_: z80->regfile.a_ = value >> 8; z80->regfile.f_ = value; break;
        case REG_BC_: z80->regfile.b_ = value >> 8; z80->regfile.c_ = value; break;
        case REG_DE_: z80->regfile.d_ = value >> 8; z80->regfile.e_ = value; break;
        case REG_HL_: z80->regfile.h_ = value >> 8; z80->regfile.l_ = value; break;
        case REG_SP:  z80->regfile.sp = value; break;
        default:
            FATAL("invalid call: set_pair(z80, %u, 0x%04X)", pair, value)
    }
}

/*
    Return whether a particular flag is set in the F register.
*/
static inline bool get_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regfile.f & (1 << flag);
}

/*
    Return whether a particular flag is set in the F' register.
*/
static inline bool get_shadow_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regfile.f_ & (1 << flag);
}

/*
    Update the F register flags according to the set bits in the mask.
*/
static inline void update_flags(Z80 *z80, bool c, bool n, bool pv, bool f3,
                                bool h, bool f5, bool z, bool s, uint8_t mask)
{
    z80->regfile.f = (~mask & z80->regfile.f) | (mask & (
        c  << FLAG_CARRY     |
        n  << FLAG_SUBTRACT  |
        pv << FLAG_PARITY    |
        f3 << FLAG_UNDOC_3   |
        h  << FLAG_HALFCARRY |
        f5 << FLAG_UNDOC_5   |
        z  << FLAG_ZERO      |
        s  << FLAG_SIGN));
}

/*
    Push a two-byte value onto the stack.
*/
static inline void stack_push(Z80 *z80, uint16_t value)
{
    z80->regfile.sp -= 2;
    mmu_write_double(z80->mmu, z80->regfile.sp, value);
}

/*
    Pop a two-byte value from the stack.
*/
static inline uint16_t stack_pop(Z80 *z80)
{
    uint16_t value = mmu_read_double(z80->mmu, z80->regfile.sp);
    z80->regfile.sp += 2;
    return value;
}

/*
    Check for errors after an I/O operation.
*/
static void handle_io_errors(Z80 *z80)
{
    if (z80->io->except) {
        z80->except = true;
        z80->exc_code = Z80_EXC_IO_ERROR;
        z80->exc_data = z80->io->exc_port;
    }
}

/*
    Read and return a byte from the given port, and check for errors.
*/
static uint8_t read_port(Z80 *z80, uint8_t port)
{
    uint8_t value = io_port_read(z80->io, port);
    handle_io_errors(z80);
    return value;
}

/*
    Write a byte to the given port, and check for errors.
*/
static void write_port(Z80 *z80, uint8_t port, uint8_t value)
{
    io_port_write(z80->io, port, value);
    handle_io_errors(z80);
}

/*
    Extract an 8-bit register from the given opcode and return a pointer to it.
*/
static inline uint8_t* extract_reg(Z80 *z80, uint8_t opcode)
{
    switch (opcode & 0x38) {
        case 0x00: return &z80->regfile.b;
        case 0x08: return &z80->regfile.c;
        case 0x10: return &z80->regfile.d;
        case 0x18: return &z80->regfile.e;
        case 0x20: return &z80->regfile.h;
        case 0x28: return &z80->regfile.l;
        case 0x38: return &z80->regfile.a;
    }
    FATAL("invalid call: extract_reg(z80, 0x%02X)", opcode)
}

/*
    Extract a register pair from the given opcode and return its identifer.
*/
static inline uint8_t extract_pair(uint8_t opcode)
{
    switch (opcode & 0x30) {
        case 0x00: return REG_BC;
        case 0x10: return REG_DE;
        case 0x20: return REG_HL;
        case 0x30: return REG_SP;
    }
    FATAL("invalid call: extract_pair(0x%02X)", opcode)
}

/*
    Extract a condition from the given opcode.
*/
static inline bool extract_cond(const Z80 *z80, uint8_t opcode)
{
    switch (opcode & 0x38) {
        case 0x00: return !get_flag(z80, FLAG_ZERO);
        case 0x08: return  get_flag(z80, FLAG_ZERO);
        case 0x10: return !get_flag(z80, FLAG_CARRY);
        case 0x18: return  get_flag(z80, FLAG_CARRY);
        case 0x20: return !get_flag(z80, FLAG_PARITY);
        case 0x28: return  get_flag(z80, FLAG_PARITY);
        case 0x30: return !get_flag(z80, FLAG_SIGN);
        case 0x38: return  get_flag(z80, FLAG_SIGN);
    }
    FATAL("invalid call: extract_cond(z80, 0x%02X)", opcode)
}

/*
    Extract the current index register.
*/
static inline uint16_t* extract_index(Z80 *z80)
{
    uint8_t prefix = mmu_read_byte(z80->mmu, z80->regfile.pc - 1);
    if (prefix != 0xDD && prefix != 0xFD)
        FATAL("invalid call: extract_index(z80, 0x%02X)", prefix)
    return prefix == 0xDD ? &z80->regfile.ix : &z80->regfile.iy;
}

/*
    Return the CPU's current interrupt mode.
*/
static inline uint8_t get_interrupt_mode(const Z80 *z80)
{
    if (!z80->regfile.im_a)
        return 0;
    if (!z80->regfile.im_b)
        return 1;
    return 2;
}

/*
    Increment the refresh counter register, R.
*/
static inline void increment_refresh_counter(Z80 *z80)
{
    z80->regfile.r = (z80->regfile.r & 0x80) | ((z80->regfile.r + 1) & 0x7F);
}

#include "z80_ops.inc.c"

/*
    @TRACE_LEVEL
    Trace the instruction about to be executed by the CPU.
*/
static inline void trace_instruction(Z80 *z80)
{
    if (z80->regfile.pc == z80->trace.last_addr && !z80->trace.fresh) {
        z80->trace.counter++;
        if (!(z80->trace.counter % (1 << 14)))
            TRACE_NOEOL("repeat last: %llu times\r", z80->trace.counter);
        return;
    }
    if (z80->trace.fresh) {
        TRACE("PC ADDR  P1 P2 OP A1 A2        INSTR\tARGS")
        TRACE("-------  --------------        -----\t----")
        z80->trace.fresh = false;
    }

    z80->trace.last_addr = z80->regfile.pc;
    z80->trace.counter = 0;

    uint32_t quad = mmu_read_quad(z80->mmu, z80->regfile.pc);
    uint8_t bytes[4] = {quad, quad >> 8, quad >> 16, quad >> 24};
    DisasInstr *instr = disassemble_instruction(bytes);

    TRACE("0x%04X:  %-14s        %s",
        z80->regfile.pc, instr->bytestr, instr->line)
    disas_instr_free(instr);
}

/*
    Emulate the given number of cycles of the Z80, or until an exception.

    The return value indicates whether the exception flag is set. If it is,
    then emulation must be stopped because further calls to z80_do_cycles()
    will have no effect. The exception flag can be reset with z80_power().
*/
bool z80_do_cycles(Z80 *z80, double cycles)
{
    cycles += z80->pending_cycles;
    while (cycles > 0 && !z80->except) {
        uint8_t opcode = mmu_read_byte(z80->mmu, z80->regfile.pc);
        increment_refresh_counter(z80);
        if (TRACE_LEVEL)
            trace_instruction(z80);
        cycles -= (*instruction_table[opcode])(z80, opcode);
    }

    z80->pending_cycles = cycles;
    return z80->except;
}

/*
    @DEBUG_LEVEL
    Print out all register values to stdout.
*/
void z80_dump_registers(const Z80 *z80)
{
    const Z80RegFile *rf = &z80->regfile;
    DEBUG("Dumping Z80 register values:")

    DEBUG("- AF:   0x%02X%02X (%03d, %03d)", rf->a, rf->f, rf->a, rf->f)
    DEBUG("- BC:   0x%02X%02X (%03d, %03d)", rf->b, rf->c, rf->b, rf->c)
    DEBUG("- DE:   0x%02X%02X (%03d, %03d)", rf->d, rf->e, rf->d, rf->e)
    DEBUG("- HL:   0x%02X%02X (%03d, %03d)", rf->h, rf->l, rf->h, rf->l)

    DEBUG("- AF':  0x%02X%02X (%03d, %03d)", rf->a_, rf->f_, rf->a_, rf->f_)
    DEBUG("- BC':  0x%02X%02X (%03d, %03d)", rf->b_, rf->c_, rf->b_, rf->c_)
    DEBUG("- DE':  0x%02X%02X (%03d, %03d)", rf->d_, rf->e_, rf->d_, rf->e_)
    DEBUG("- HL':  0x%02X%02X (%03d, %03d)", rf->h_, rf->l_, rf->h_, rf->l_)

    DEBUG("- IX:   0x%04X (%05d)", rf->ix, rf->ix)
    DEBUG("- IY:   0x%04X (%05d)", rf->iy, rf->iy)
    DEBUG("- SP:   0x%04X (%05d)", rf->sp, rf->sp)
    DEBUG("- PC:   0x%04X (%05d)", rf->pc, rf->pc)

    DEBUG("- I:    0x%2X (%03d)", rf->i, rf->i)
    DEBUG("- R:    0x%2X (%03d)", rf->r, rf->r)

    DEBUG("- F:    "BINARY_FMT" (C: %u, N: %u, P/V: %u, H: %u, Z: %u, S: %u)",
          BINARY_VAL(rf->f),
          get_flag(z80, FLAG_CARRY),
          get_flag(z80, FLAG_SUBTRACT),
          get_flag(z80, FLAG_PARITY),
          get_flag(z80, FLAG_HALFCARRY),
          get_flag(z80, FLAG_ZERO),
          get_flag(z80, FLAG_SIGN))

    DEBUG("- F':   "BINARY_FMT" (C: %u, N: %u, P/V: %u, H: %u, Z: %u, S: %u)",
          BINARY_VAL(rf->f_),
          get_shadow_flag(z80, FLAG_CARRY),
          get_shadow_flag(z80, FLAG_SUBTRACT),
          get_shadow_flag(z80, FLAG_PARITY),
          get_shadow_flag(z80, FLAG_HALFCARRY),
          get_shadow_flag(z80, FLAG_ZERO),
          get_shadow_flag(z80, FLAG_SIGN))

    DEBUG("- IM:   0b%u%u (mode: %u)", rf->im_a, rf->im_b,
          get_interrupt_mode(z80))
    DEBUG("- IFF:  1: %u, 2: %u", rf->iff1, rf->iff2)
}
