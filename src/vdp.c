/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <string.h>
#include <SDL.h>

#include "vdp.h"
#include "util.h"

#define CODE_VRAM_READ  0
#define CODE_VRAM_WRITE 1
#define CODE_REG_WRITE  2
#define CODE_CRAM_WRITE 3

extern SDL_Renderer* renderer;

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
    TODO: ...
*/
static void draw_pixel(uint8_t y, uint8_t x, uint16_t color)
{
    uint8_t r = 0x11 *  (color & 0x000F);
    uint8_t g = 0x11 * ((color & 0x00F0) >> 4);
    uint8_t b = 0x11 * ((color & 0x0F00) >> 8);

    SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);

    uint8_t i, j;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++)
            SDL_RenderDrawPoint(renderer, 3 * x + i, 3 * y + j);
    }
}

/*
    Draw the background of the current scanline.
*/
static void draw_background(VDP *vdp)
{
    uint8_t *pnt = vdp->vram + get_pnt_base(vdp);

    uint8_t row = (vdp->v_counter + vdp->regs[0x09]) % (28 * 8);
    uint8_t col;

    for (col = 6; col < 32 - 6; col++) {
        uint16_t index = (row >> 3) * 32 + col;
        uint16_t tile = pnt[2 * index] + (pnt[2 * index + 1] << 8);
        uint16_t pattern = tile & 0x01FF;
        bool palette  = tile & 0x0800;
        bool priority = tile & 0x1000;
        bool vflip    = tile & 0x0400;
        bool hflip    = tile & 0x0200;

        uint8_t offy = vflip ? (7 - row % 8) : (row % 8);
        uint8_t *pixels = &vdp->vram[pattern * 32 + 4 * offy];
        uint8_t bp0 = pixels[0], bp1 = pixels[1],
                bp2 = pixels[2], bp3 = pixels[3];

        uint8_t idx, i, offx;
        uint16_t color;

        for (i = 0; i < 8; i++) {
            idx =  ((bp0 >> i) & 1) +
                   (((bp1 >> i) & 1) << 1) +
                   (((bp2 >> i) & 1) << 2) +
                   (((bp3 >> i) & 1) << 3);
            color = vdp->cram[2 * idx] + (vdp->cram[2 * idx + 1] << 8);
            offx = hflip ? (col * 8 + (7 - i)) : (col * 8 + i);
            draw_pixel(vdp->v_counter, offx, color);
        }
    }
}

/*
    Draw sprites in the current scanline.
*/
static void draw_sprites(VDP *vdp)
{
    uint8_t *sat = vdp->vram + get_sat_base(vdp);
    uint8_t spritebuf[8], nsprites = 0, i;

    for (i = 0; i < 64; i++) {
        uint8_t y = sat[i] - 1;
        if (vdp->v_counter >= y && vdp->v_counter < y + 8) {
            DEBUG("sprite draw!!!")
            // TODO
        }
    }
}

/*
    Draw the current scanline.
*/
static void draw_scanline(VDP *vdp)
{
    draw_background(vdp);
    draw_sprites(vdp);
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
    Simulate one line within the VDP.
*/
void vdp_simulate_line(VDP *vdp)
{
    if (vdp->v_counter >= 0x18 && vdp->v_counter < 0xA8)
        draw_scanline(vdp);
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
        vdp->control_flag = true;
        return;
    }

    vdp->control_addr = ((byte & 0x3F) << 8) + (vdp->control_addr & 0xFF);
    vdp->control_code = byte >> 6;

    if (vdp->control_code == CODE_VRAM_READ) {
        vdp->read_buf = vdp->vram[vdp->control_addr];
        vdp->control_addr = (vdp->control_addr + 1) % 0x3FFF;
    } else if (vdp->control_code == CODE_REG_WRITE) {
        uint8_t reg = byte & 0x0F;
        if (reg <= VDP_REGS)
            vdp->regs[reg] = vdp->control_addr & 0xFF;
    }

    vdp->control_flag = false;
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
    // TODO: line interrupts
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

    // TODO: show flags
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

    // TODO: remove me!
    DEBUG("Dumping CRAM:")
    for (uint8_t i = 0x00; i < 0x40; i += 0x10) {
        uint16_t w[8];
        for (uint8_t j = 0; j < 8; j++)
            w[j] = vdp->cram[i + j * 2] + (vdp->cram[i + j * 2 + 1] << 8);

        DEBUG("- %04X %04X %04X %04X %04X %04X %04X %04X",
            w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7])
    }

    return;

    DEBUG("Dumping PNT:")
    for (uint16_t i = 0; i < 28 * 32; i += 32) {
        uint16_t w[32];
        for (uint8_t j = 0; j < 32; j++)
            w[j] = vdp->vram[get_pnt_base(vdp) + 2 * (i + j)] +
                  (vdp->vram[get_pnt_base(vdp) + 2 * (i + j) + 1] << 8);

        DEBUG("- %03X %03X %03X %03X %03X %03X %03X %03X"
               " %03X %03X %03X %03X %03X %03X %03X %03X"
               " %03X %03X %03X %03X %03X %03X %03X %03X"
               " %03X %03X %03X %03X %03X %03X %03X %03X",
            w[0x00], w[0x01], w[0x02], w[0x03], w[0x04], w[0x05], w[0x06], w[0x07],
            w[0x08], w[0x09], w[0x0A], w[0x0B], w[0x0C], w[0x0D], w[0x0E], w[0x0F],
            w[0x10], w[0x11], w[0x12], w[0x13], w[0x14], w[0x15], w[0x16], w[0x17],
            w[0x18], w[0x19], w[0x1A], w[0x1B], w[0x1C], w[0x1D], w[0x1E], w[0x1F])
    }

    DEBUG("Dumping PGT:")
    for (uint16_t i = 0; i < /* 512 */ 16; i++) {
        uint32_t w[8];
        for (uint8_t j = 0; j < 8; j++)
            w[j] = vdp->vram[32 * i + 4 * j] +
                  (vdp->vram[32 * i + 4 * j + 1] <<  8) +
                  (vdp->vram[32 * i + 4 * j + 2] << 16) +
                  (vdp->vram[32 * i + 4 * j + 3] << 24);

        DEBUG("- 0x%04X: %08X %08X %08X %08X %08X %08X %08X %08X", i,
            w[0], w[1], w[2], w[3], w[4], w[5], w[6], w[7])
    }
}
