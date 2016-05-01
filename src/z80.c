/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "z80.h"
#include "disassembler.h"
#include "logging.h"
#include "util.h"

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
    z80->regs.af = 0xFFFF;
    z80->regs.bc = 0xFFFF;
    z80->regs.de = 0xFFFF;
    z80->regs.hl = 0xFFFF;

    z80->regs.af_ = 0xFFFF;
    z80->regs.bc_ = 0xFFFF;
    z80->regs.de_ = 0xFFFF;
    z80->regs.hl_ = 0xFFFF;

    z80->regs.ix = 0xFFFF;
    z80->regs.iy = 0xFFFF;
    z80->regs.sp = 0xFFF0;
    z80->regs.pc = 0x0000;

    z80->regs.i = 0xFF;
    z80->regs.r = 0xFF;

    z80->regs.im_a = z80->regs.im_b = 0;
    z80->regs.iff1 = z80->regs.iff2 = 0;

    z80->regs.ixy = NULL;
    z80->regs.ih = z80->regs.il = NULL;

    z80->except = false;
    z80->pending_cycles = 0;
    z80->irq_wait = false;

    z80->trace.fresh = true;
    z80->trace.last_addr = 0;
    z80->trace.counter = 0;
}

/*
    Return whether a particular flag is set in the F register.
*/
static inline bool get_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regs.f & (1 << flag);
}

/*
    Return whether a particular flag is set in the F' register.
*/
static inline bool get_shadow_flag(const Z80 *z80, uint8_t flag)
{
    return z80->regs.f_ & (1 << flag);
}

/*
    Update the F register flags according to the set bits in the mask.
*/
static inline void set_flags(Z80 *z80,
    bool c, bool n, bool pv, bool f3, bool h, bool f5, bool z, bool s,
    uint8_t mask)
{
    uint8_t new = (
        c  << FLAG_CARRY     |
        n  << FLAG_SUBTRACT  |
        pv << FLAG_PARITY    |
        f3 << FLAG_UNDOC_3   |
        h  << FLAG_HALFCARRY |
        f5 << FLAG_UNDOC_5   |
        z  << FLAG_ZERO      |
        s  << FLAG_SIGN
    );
    z80->regs.f = (~mask & z80->regs.f) | (mask & new);
}

#include "z80_flags.inc.c"

/*
    Push a two-byte value onto the stack.
*/
static inline void stack_push(Z80 *z80, uint16_t value)
{
    z80->regs.sp -= 2;
    mmu_write_double(z80->mmu, z80->regs.sp, value);
}

/*
    Pop a two-byte value from the stack.
*/
static inline uint16_t stack_pop(Z80 *z80)
{
    uint16_t value = mmu_read_double(z80->mmu, z80->regs.sp);
    z80->regs.sp += 2;
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
static inline uint8_t read_port(Z80 *z80, uint8_t port)
{
    uint8_t value = io_port_read(z80->io, port);
    handle_io_errors(z80);
    return value;
}

/*
    Write a byte to the given port, and check for errors.
*/
static inline void write_port(Z80 *z80, uint8_t port, uint8_t value)
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
        case 0x00: return &z80->regs.b;
        case 0x08: return &z80->regs.c;
        case 0x10: return &z80->regs.d;
        case 0x18: return &z80->regs.e;
        case 0x20: return &z80->regs.h;
        case 0x28: return &z80->regs.l;
        case 0x38: return &z80->regs.a;
    }
    FATAL("invalid call: extract_reg(z80, 0x%02X)", opcode)
}

/*
    Extract a register pair from the given opcode and return a pointer to it.
*/
static inline uint16_t* extract_pair(Z80 *z80, uint8_t opcode)
{
    switch (opcode & 0x30) {
        case 0x00: return &z80->regs.bc;
        case 0x10: return &z80->regs.de;
        case 0x20: return &z80->regs.hl;
        case 0x30: return &z80->regs.sp;
    }
    FATAL("invalid call: extract_pair(z80, 0x%02X)", opcode)
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
    Return the address signified by a indirect index instruction.
*/
static inline uint16_t get_index_addr(Z80 *z80, uint16_t offset_addr)
{
    return *z80->regs.ixy + ((int8_t) mmu_read_byte(z80->mmu, offset_addr));
}

/*
    Return the CPU's current interrupt mode.
*/
static inline uint8_t get_interrupt_mode(const Z80 *z80)
{
    if (!z80->regs.im_a)
        return 0;
    if (!z80->regs.im_b)
        return 1;
    return 2;
}

/*
    Handle an active IRQ line. Return the number of cycles consumed.
*/
static inline uint8_t accept_interrupt(Z80 *z80)
{
    TRACE("Z80 triggering mode-%d interrupt", get_interrupt_mode(z80))
    z80->regs.iff1 = z80->regs.iff2 = 0;
    stack_push(z80, z80->regs.pc);

    if (get_interrupt_mode(z80) < 2) {
        z80->regs.pc = 0x0038;
        return 13;
    } else {
        uint16_t addr = (z80->regs.i << 8) + 0xFF;
        z80->regs.pc = mmu_read_double(z80->mmu, addr);
        return 19;
    }
}

/*
    Increment the refresh counter register, R.
*/
static inline void increment_refresh_counter(Z80 *z80)
{
    z80->regs.r = (z80->regs.r & 0x80) | ((z80->regs.r + 1) & 0x7F);
}

#include "z80_ops.inc.c"

/*
    @TRACE_LEVEL
    Trace the instruction about to be executed by the CPU.
*/
static inline void trace_instruction(Z80 *z80)
{
    if (z80->regs.pc == z80->trace.last_addr && !z80->trace.fresh) {
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

    z80->trace.last_addr = z80->regs.pc;
    z80->trace.counter = 0;

    uint32_t quad = mmu_read_quad(z80->mmu, z80->regs.pc);
    uint8_t bytes[4] = {quad, quad >> 8, quad >> 16, quad >> 24};
    DisasInstr *instr = disassemble_instruction(bytes);

    TRACE("0x%04X:  %-14s        %s",
        z80->regs.pc, instr->bytestr, instr->line)
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
        if (io_check_irq(z80->io) && z80->regs.iff1 && !z80->irq_wait) {
            cycles -= accept_interrupt(z80);
            continue;
        }
        if (z80->irq_wait)
            z80->irq_wait = false;

        uint8_t opcode = mmu_read_byte(z80->mmu, z80->regs.pc);
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
    const Z80RegFile *rf = &z80->regs;
    DEBUG("Dumping Z80 register values:")

    DEBUG("- AF:   0x%04X (%03d, %03d)", rf->af, rf->a, rf->f)
    DEBUG("- BC:   0x%04X (%03d, %03d)", rf->bc, rf->b, rf->c)
    DEBUG("- DE:   0x%04X (%03d, %03d)", rf->de, rf->d, rf->e)
    DEBUG("- HL:   0x%04X (%03d, %03d)", rf->hl, rf->h, rf->l)

    DEBUG("- AF':  0x%04X (%03d, %03d)", rf->af_, rf->a_, rf->f_)
    DEBUG("- BC':  0x%04X (%03d, %03d)", rf->bc_, rf->b_, rf->c_)
    DEBUG("- DE':  0x%04X (%03d, %03d)", rf->de_, rf->d_, rf->e_)
    DEBUG("- HL':  0x%04X (%03d, %03d)", rf->hl_, rf->h_, rf->l_)

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
