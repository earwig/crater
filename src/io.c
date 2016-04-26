/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "io.h"

/*
    Initialize an IO object.
*/
void io_init(IO *io, VDP *vdp)
{
    io->vdp = vdp;
}

/*
    "Power on" the IO object.
*/
void io_power(IO *io)
{
    io->except = false;
}

/*
    Return whether the IRQ line is currently active.
*/
bool io_check_irq(IO *io)
{
    return vdp_assert_irq(io->vdp);
}

/*
    Read and return a byte from the given port.
*/
uint8_t io_port_read(IO *io, uint8_t port)
{
    if (port <= 0x06) {
        // TODO: GG specific registers; initial state: C0 7F FF 00 FF 00 FF
    } else if (port <= 0x3F) {
        return 0xFF;
    } else if (port <= 0x7F && !(port % 2)) {
        return io->vdp->v_counter;
    } else if (port <= 0x7F) {
        return io->vdp->h_counter;
    } else if (port <= 0xBF && !(port % 2)) {
        return vdp_read_data(io->vdp);
    } else if (port <= 0xBF) {
        return vdp_read_control(io->vdp);
    } else if (port == 0xCD || port == 0xDC) {
        // TODO: Return the I/O port A/B register
    } else if (port == 0xC1 || port == 0xDD) {
        // TODO: Return the I/O port B/misc. register
    } else {
        return 0xFF;
    }

    io->except = true;
    io->exc_port = port;
    return 0;
}

/*
    Write a byte to the given port.
*/
void io_port_write(IO *io, uint8_t port, uint8_t value)
{
    if (port <= 0x06) {
        // TODO: GG specific registers; initial state: C0 7F FF 00 FF 00 FF
        goto except;
    } else if (port <= 0x3F && !(port % 2)) {
        // TODO: Write to memory control register
        goto except;
    } else if (port <= 0x3F) {
        // TODO: Write to I/O control register
        goto except;
    } else if (port <= 0x7F) {
        // TODO: Write to the SN76489 PSG
        goto except;
    } else if (port <= 0xBF && !(port % 2)) {
        vdp_write_data(io->vdp, value);
    } else if (port <= 0xBF) {
        vdp_write_control(io->vdp, value);
    } else {
        return;
    }

    return;

    except:
    io->except = true;
    io->exc_port = port;
}
