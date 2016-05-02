/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <string.h>
#include <SDL.h>

#include "vdp.h"
#include "util.h"

#define FLAG_CONTROL   0x01
#define FLAG_FRAME_INT 0x02
#define FLAG_LINE_INT  0x04
#define FLAG_SPR_OVF   0x08
#define FLAG_SPR_COL   0x10

#define CODE_VRAM_READ  0
#define CODE_VRAM_WRITE 1
#define CODE_REG_WRITE  2
#define CODE_CRAM_WRITE 3

/*
    Initialize the Video Display Processor (VDP).

    The VDP will write to its pixels array whenever it draws a scanline. It
    defaults to NULL, but you should set it to something if you want to see its
    output.
*/
void vdp_init(VDP *vdp)
{
    vdp->pixels = NULL;
    vdp->vram = cr_malloc(sizeof(uint8_t) * VDP_VRAM_SIZE);
    vdp->cram = cr_malloc(sizeof(uint8_t) * VDP_CRAM_SIZE);
}

/*
    Free memory previously allocated by the VDP.
*/
void vdp_free(VDP *vdp)
{
    free(vdp->vram);
    free(vdp->cram);
}

/*
    Power on the VDP, setting up initial state.
*/
void vdp_power(VDP *vdp)
{
    memset(vdp->vram, 0x00, VDP_VRAM_SIZE);
    memset(vdp->cram, 0x00, VDP_CRAM_SIZE);

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

    vdp->flags = 0;
    vdp->control_code = 0;
    vdp->control_addr = 0;
    vdp->line_count = 0x01;
    vdp->read_buf = 0;
    vdp->cram_latch = 0;
}

/*
    Return whether line-completion interrupts are enabled.
*/
static bool should_line_interrupt(const VDP *vdp)
{
    return vdp->regs[0x00] & 0x10;
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
    Return the backdrop color as a CRAM index.
*/
static uint8_t get_backdrop_color(const VDP *vdp)
{
    return ((vdp->regs[0x07] & 0x0F) << 1) + 0x20;
}

/*
    Return the horizontal background scroll value.
*/
static uint8_t get_bg_hscroll(const VDP *vdp)
{
    return vdp->regs[0x08];
}

/*
    Return the vertical background scroll value.
*/
static uint8_t get_bg_vscroll(const VDP *vdp)
{
    return vdp->regs[0x09];
}

/*
    Return the packed background tile at the given row and column.
*/
static uint16_t get_background_tile(const VDP *vdp, uint8_t row, uint8_t col)
{
    uint8_t *pnt = vdp->vram + get_pnt_base(vdp);
    uint16_t index = row * 32 + col;
    return pnt[2 * index] + (pnt[2 * index + 1] << 8);
}

/*
    Get the CRAM color index of the given (row, col) in the given pattern.
*/
static uint8_t read_pattern(const VDP *vdp, uint16_t pattern,
    uint8_t row, uint8_t col)
{
    uint8_t *planes = &vdp->vram[32 * pattern + 4 * row];
    return ((planes[0] >> (7 - col)) & 1) +
          (((planes[1] >> (7 - col)) & 1) << 1) +
          (((planes[2] >> (7 - col)) & 1) << 2) +
          (((planes[3] >> (7 - col)) & 1) << 3);
}

/*
    Return the BGR444 color at the given CRAM index.

    The index should be between 0 and 15, as there are 16 colors per palette.
*/
static uint16_t get_color(const VDP *vdp, uint8_t index, bool palette)
{
    uint8_t offset = 2 * (index + 16 * palette);
    return vdp->cram[offset] + (vdp->cram[offset + 1] << 8);
}

/*
    Draw a pixel onto our pixel array at the given coordinates.

    The color should be in BGR444 format, as returned by get_color().
*/
static void draw_pixel(VDP *vdp, uint8_t y, uint8_t x, uint16_t color)
{
    uint8_t r = 0x11 *  (color & 0x000F);
    uint8_t g = 0x11 * ((color & 0x00F0) >> 4);
    uint8_t b = 0x11 * ((color & 0x0F00) >> 8);

    uint32_t argb = (0xFF << 24) + (r << 16) + (g << 8) + b;
    vdp->pixels[y * 160 + x] = argb;
}

/*
    Draw the background of the current scanline.
*/
static void draw_background(VDP *vdp)
{
    uint8_t src_row = (vdp->v_counter + get_bg_vscroll(vdp)) % (28 << 3);
    uint8_t dst_row = vdp->v_counter - 0x18;
    uint8_t vcell = src_row >> 3;
    uint8_t hcell, col;

    uint8_t start_col   = get_bg_hscroll(vdp) >> 3;
    uint8_t fine_scroll = get_bg_hscroll(vdp) % 8;

    for (col = 6; col < 20 + 6; col++) {
        hcell = (32 - start_col + col) % 32;
        uint16_t tile = get_background_tile(vdp, vcell, hcell);
        uint16_t pattern  = tile & 0x01FF;
        bool     palette  = tile & 0x0800;
        bool     priority = tile & 0x1000;
        bool     vflip    = tile & 0x0400;
        bool     hflip    = tile & 0x0200;

        uint8_t vshift = vflip ? (7 - src_row % 8) : (src_row % 8), hshift;
        uint8_t pixel, dst_col, index;
        uint16_t color;

        for (pixel = 0; pixel < 8; pixel++) {
            dst_col = ((col - 6) << 3) + pixel + fine_scroll;
            hshift = hflip ? (7 - pixel) : pixel;

            index = read_pattern(vdp, pattern, vshift, hshift);
            color = get_color(vdp, index, palette);
            draw_pixel(vdp, dst_row, dst_col, color);
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
    if (!vdp->pixels)
        return;

    draw_background(vdp);
    draw_sprites(vdp);
}

/*
    Update the line counter, which triggers line interrupts.
*/
static void update_line_counter(VDP *vdp)
{
    if (vdp->v_counter < 0xC0) {
        if (vdp->line_count == 0x00) {
            vdp->flags |= FLAG_LINE_INT;
            vdp->line_count = vdp->regs[0x0A];
        } else {
            vdp->line_count--;
        }
    } else {
        vdp->line_count = vdp->regs[0x0A];
    }
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
        vdp->flags |= FLAG_FRAME_INT;
    update_line_counter(vdp);
    advance_scanline(vdp);
}

/*
    Read a byte from the VDP's control port, revealing status flags.

    The status byte consists of:
    7  6  5  4  3  2  1  0
    F  9S C  *  *  *  *  *

    - F: Frame interrupt: set when the effective display area is completed
    - 9S: 9th sprite / Sprite overflow: more than eight sprites on a scanline
    - C: Sprite collision: two sprites have an overlapping pixel
    - *: Unused

    The control and line interrupt flags are also reset.
*/
uint8_t vdp_read_control(VDP *vdp)
{
    uint8_t status =
        (!!(vdp->flags & FLAG_FRAME_INT) << 7) +
        (!!(vdp->flags & FLAG_SPR_OVF)   << 6) +
        (!!(vdp->flags & FLAG_SPR_COL)   << 5);
    vdp->flags = 0;
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
    vdp->control_addr = (vdp->control_addr + 1) & 0x3FFF;
    vdp->flags &= ~FLAG_CONTROL;
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
    vdp->flags ^= FLAG_CONTROL;
    if (vdp->flags & FLAG_CONTROL) {  // First byte
        vdp->control_addr = (vdp->control_addr & 0x3F00) + byte;
        return;
    }

    vdp->control_addr = ((byte & 0x3F) << 8) + (vdp->control_addr & 0xFF);
    vdp->control_code = byte >> 6;

    if (vdp->control_code == CODE_VRAM_READ) {
        vdp->read_buf = vdp->vram[vdp->control_addr];
        vdp->control_addr = (vdp->control_addr + 1) & 0x3FFF;
    } else if (vdp->control_code == CODE_REG_WRITE) {
        uint8_t reg = byte & 0x0F;
        if (reg <= VDP_REGS)
            vdp->regs[reg] = vdp->control_addr & 0xFF;
    }
}

/*
    Write a byte into CRAM. Handles even/odd address latching.
*/
static void write_cram(VDP *vdp, uint8_t byte)
{
    if (!(vdp->control_addr % 2)) {
        vdp->cram_latch = byte;
    } else {
        vdp->cram[(vdp->control_addr - 1) & 0x3F] = vdp->cram_latch;
        vdp->cram[ vdp->control_addr      & 0x3F] = byte & 0x0F;
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

    vdp->control_addr = (vdp->control_addr + 1) & 0x3FFF;
    vdp->flags &= ~FLAG_CONTROL;
    vdp->read_buf = byte;
}

/*
    Return whether the VDP is currently asserting an interrupt.
*/
bool vdp_assert_irq(VDP *vdp)
{
    return (vdp->flags & FLAG_FRAME_INT && should_frame_interrupt(vdp)) ||
           (vdp->flags & FLAG_LINE_INT  && should_line_interrupt(vdp));
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
    DEBUG("- $07:  0x%02X (BDC: 0x%02X)", regs[0x07], get_backdrop_color(vdp))
    DEBUG("- $08:  0x%02X (HS)", regs[0x08])
    DEBUG("- $09:  0x%02X (VS)", regs[0x09])
    DEBUG("- $0A:  0x%02X (LC)", regs[0x0A])
}
