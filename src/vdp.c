/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <string.h>

#include "vdp.h"
#include "util.h"

#define CODE_VRAM_READ  0
#define CODE_VRAM_WRITE 1
#define CODE_REG_WRITE  2
#define CODE_CRAM_WRITE 3

/*
    Initialize the Video Display Processor (VDP).
*/
void vdp_init(VDP *vdp)
{
    vdp->vram = cr_malloc(sizeof(uint8_t) * VDP_VRAM_SIZE);
    vdp->cram = cr_malloc(sizeof(uint8_t) * VDP_CRAM_SIZE);
    memset(vdp->vram, 0x00, VDP_VRAM_SIZE);
    memset(vdp->cram, 0x00, VDP_CRAM_SIZE);
}

/*
    Free memory previously allocated by the VDP.
*/
void vdp_free(VDP *vdp)
{
    free(vdp->vram);
}

/*
    Power on the VDP, setting up initial state.
*/
void vdp_power(VDP *vdp)
{
    vdp->regs[0x00] = 0x00;
    vdp->regs[0x01] = 0x00;
    vdp->regs[0x02] = 0xFF;
    vdp->regs[0x03] = 0xFF;
    vdp->regs[0x04] = 0xFF;
    vdp->regs[0x05] = 0xFF;
    vdp->regs[0x06] = 0xFF;
    vdp->regs[0x07] = 0x00;
    vdp->regs[0x08] = 0x00;
    vdp->regs[0x09] = 0x00;
    vdp->regs[0x0A] = 0x01;

    vdp->h_counter = 0;
    vdp->v_counter = 0;
    vdp->v_count_jump = false;

    vdp->control_code = 0;
    vdp->control_addr = 0;
    vdp->control_flag = false;
    vdp->stat_int = vdp->stat_ovf = vdp->stat_col = 0;
    vdp->read_buf = 0;
    vdp->cram_latch = 0;
}

/*
    Return whether frame-completion interrupts are enabled.
*/
static bool should_frame_interrupt(const VDP *vdp)
{
    return vdp->regs[0x01] & 0x20;
}

/*
    Return the base address of the pattern name table.
*/
static uint16_t get_pnt_base(const VDP *vdp)
{
    return (vdp->regs[0x02] & 0x0E) << 10;
}

/*
    Return the base address of the sprite attribute table.
*/
static uint16_t get_sat_base(const VDP *vdp)
{
    return (vdp->regs[0x05] & 0x7E) << 7;
}

/*
    Return the base address of the sprite generator table.
*/
static uint16_t get_sgt_base(const VDP *vdp)
{
    return (vdp->regs[0x06] & 0x04) << 11;
}

/*
    Return the CRAM address of the backdrop color.
*/
static uint8_t get_backdrop_addr(const VDP *vdp)
{
    return ((vdp->regs[0x07] & 0x0F) << 1) + 0x20;
}

/*
    Advance the V counter for the next scanline.
*/
static void advance_scanline(VDP *vdp)
{
    if (vdp->v_counter == 0xDA)
        vdp->v_count_jump = !vdp->v_count_jump;

    if (vdp->v_counter == 0xDA && vdp->v_count_jump)
        vdp->v_counter = 0xD5;
    else
        vdp->v_counter++;
}

/*
    Simulate one scanline within the VDP.

    TODO: elaborate
*/
void vdp_simulate_line(VDP *vdp)
{
    if (vdp->v_counter >= 0x18 && vdp->v_counter < 0xA8) {
        // TODO: draw current line
    }
    if (vdp->v_counter == 0xC0)
        vdp->stat_int = true;
    advance_scanline(vdp);
}

/*
    Read a byte from the VDP's control port, revealing status flags.

    The status byte consists of:
    7  6  5  4  3  2  1  0
    F  9S C  *  *  *  *  *

    - F: Interrupt flag: set when the effective display area is completed
    - 9S: 9th sprite / Sprite overflow: more than eight sprites on a scanline
    - C: Collision flag: two sprites have an overlapping pixel
    - *: Unused

    The control flag is also reset.
*/
uint8_t vdp_read_control(VDP *vdp)
{
    uint8_t status =
        (vdp->stat_int << 8) + (vdp->stat_ovf << 7) + (vdp->stat_col << 6);
    vdp->stat_int = vdp->stat_ovf = vdp->stat_col = 0;
    vdp->control_flag = false;
    return status;
}

/*
    Read a byte from the VDP's data port.

    This will return the contents of the read buffer, and then fill the buffer
    with the VRAM at the current control address, before incrementing the
    control address. The control flag is also reset.
*/
uint8_t vdp_read_data(VDP *vdp)
{
    uint8_t buffer = vdp->read_buf;
    vdp->read_buf = vdp->vram[vdp->control_addr];
    vdp->control_addr = (vdp->control_addr + 1) % 0x3FFF;
    vdp->control_flag = false;
    return buffer;
}

/*
    Write a byte into the VDP's control port.

    Depending on the status of the control flag, this will either update the
    lower byte of the control address, or the upper six bits of the control
    address and the control code. The flag is toggled by each control write,
    and reset by each control read and data read or write.

    If the control code indicates a VRAM read, the read buffer will be filled
    with the VRAM at the given control address, which is then incremented. If
    the code indicates a register write, the corresponding register
    (byte & 0x0F) will be written with the lower byte of the control address.
*/
void vdp_write_control(VDP *vdp, uint8_t byte)
{
    if (!vdp->control_flag) {  // First byte
        vdp->control_addr = (vdp->control_addr & 0x3F00) + byte;
    } else {  // Second byte
        vdp->control_addr = ((byte & 0x3F) << 8) + (vdp->control_addr & 0xFF);
        vdp->control_code = byte >> 6;
    }

    if (vdp->control_code == CODE_VRAM_READ) {
        vdp->read_buf = vdp->vram[vdp->control_addr];
        vdp->control_addr = (vdp->control_addr + 1) % 0x3FFF;
    } else if (vdp->control_code == CODE_REG_WRITE) {
        uint8_t reg = byte & 0x0F;
        if (reg <= VDP_REGS)
            vdp->regs[reg] = vdp->control_addr & 0xFF;
    }

    vdp->control_flag = !vdp->control_flag;
}

/*
    Write a byte into CRAM. Handles even/odd address latching.
*/
static void write_cram(VDP *vdp, uint8_t byte)
{
    if (!(vdp->control_addr % 2)) {
        vdp->cram_latch = byte;
    } else {
        vdp->cram[(vdp->control_addr - 1) % 0x3F] = vdp->cram_latch;
        vdp->cram[ vdp->control_addr      % 0x3F] = byte % 0x0F;
    }
}

/*
    Write a byte into the VDP's data port.

    Depending on the control code, this either writes into the VRAM or CRAM at
    the current control address, which is then incremented. The control flag is
    also reset, and the read buffer is squashed.
*/
void vdp_write_data(VDP *vdp, uint8_t byte)
{
    if (vdp->control_code == CODE_CRAM_WRITE)
        write_cram(vdp, byte);
    else
        vdp->vram[vdp->control_addr] = byte;

    vdp->control_addr = (vdp->control_addr + 1) % 0x3FFF;
    vdp->control_flag = false;
    vdp->read_buf = byte;
}

/*
    Return whether the VDP is currently asserting an interrupt.
*/
bool vdp_assert_irq(VDP *vdp)
{
    return vdp->stat_int && should_frame_interrupt(vdp);
}

/*
    @DEBUG_LEVEL
    Print out all register values to stdout.
*/
void vdp_dump_registers(const VDP *vdp)
{
    const uint8_t *regs = vdp->regs;
    DEBUG("Dumping VDP register values:")

    DEBUG("- $00:  0x%02X (" BINARY_FMT ")", regs[0x00], BINARY_VAL(regs[0]))
    DEBUG("- $01:  0x%02X (" BINARY_FMT ")", regs[0x01], BINARY_VAL(regs[1]))

    DEBUG("- $02:  0x%02X (PNT: 0x%04X)", regs[0x02], get_pnt_base(vdp))
    DEBUG("- $03:  0x%02X (CT)", regs[0x03])
    DEBUG("- $04:  0x%02X (BPG)", regs[0x04])
    DEBUG("- $05:  0x%02X (SAT: 0x%04X)", regs[0x05], get_sat_base(vdp))
    DEBUG("- $06:  0x%02X (SGT: 0x%04X)", regs[0x06], get_sgt_base(vdp))
    DEBUG("- $07:  0x%02X (BDC: 0x%02X)", regs[0x07], get_backdrop_addr(vdp))
    DEBUG("- $08:  0x%02X (HS)", regs[0x08])
    DEBUG("- $09:  0x%02X (VS)", regs[0x09])
    DEBUG("- $0A:  0x%02X (LC)", regs[0x0A])
}
