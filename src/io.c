/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "io.h"
#include "logging.h"

/*
    Initialize an IO object.
*/
void io_init(IO *io, MMU *mmu, VDP *vdp, PSG *psg)
{
    io->vdp = vdp;
    io->mmu = mmu;
    io->psg = psg;
}

/*
    "Power on" the IO object.
*/
void io_power(IO *io)
{
    io->ports[0x00] = 0xC0;  // Overseas mode, NTSC
    io->ports[0x01] = 0x7F;
    io->ports[0x02] = 0xFF;
    io->ports[0x03] = 0x00;
    io->ports[0x04] = 0xFF;
    io->ports[0x05] = 0x00;

    io->buttons = 0xFF;
    io->start = true;
}

/*
    Return whether the IRQ line is currently active.
*/
bool io_check_irq(IO *io)
{
    return vdp_assert_irq(io->vdp);
}

/*
    Set the state of the given joystick button.
*/
void io_set_button(IO *io, uint8_t button, bool state)
{
    io->buttons = (io->buttons & ~(1 << button)) | ((!state) << button);
}

/*
    Set the state of the start button.
*/
void io_set_start(IO *io, bool state)
{
    io->start = !state;
}

/*
    Read from one of the system ports, which are numbered from 0x00 to 0x06.
*/
static uint8_t read_system_port(IO *io, uint8_t port)
{
    switch (port) {
        case 0x00:
            return (io->ports[port] & 0x7F) | (io->start << 7);
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            return io->ports[port];
    }
    return 0xFF;
}

/*
    Write to one of the system ports, which are numbered from 0x00 to 0x06.
*/
static void write_system_port(IO *io, uint8_t port, uint8_t value)
{
    switch (port) {
        case 0x01:
        case 0x02:
        case 0x03:
            io->ports[port] = value;
            break;
        case 0x05:
            io->ports[port] = value & 0xF8;
            break;
        case 0x06:
            psg_stereo(io->psg, value);
            break;
    }
}

/*
    Write to the memory control port, $3E.
*/
static void write_memory_control(IO *io, uint8_t value)
{
    io->mmu->bios_enabled = !(value & 0x08);
}

/*
    Read and return a byte from the given port.
*/
uint8_t io_port_read(IO *io, uint8_t port)
{
    if (port <= 0x06)
        return read_system_port(io, port);
    else if (port <= 0x3F)
        return 0xFF;
    else if (port <= 0x7F && !(port % 2))
        return io->vdp->v_counter;
    else if (port <= 0x7F)
        return io->vdp->h_counter;
    else if (port <= 0xBF && !(port % 2))
        return vdp_read_data(io->vdp);
    else if (port <= 0xBF)
        return vdp_read_control(io->vdp);
    else if (port == 0xCD || port == 0xDC)
        return io->buttons;
    else if (port == 0xC1 || port == 0xDD)
        return 0xFF;  // B/Misc port, always set (unless in SMS mode?)
    else
        return 0xFF;
}

/*
    Write a byte to the given port.
*/
void io_port_write(IO *io, uint8_t port, uint8_t value)
{
    if (port <= 0x06)
        write_system_port(io, port, value);
    else if (port <= 0x3F && !(port % 2))
        write_memory_control(io, value);
    else if (port <= 0x3F)
        return;  // TODO: Write to I/O control register
    else if (port <= 0x7F)
        psg_write(io->psg, value);
    else if (port <= 0xBF && !(port % 2))
        vdp_write_data(io->vdp, value);
    else if (port <= 0xBF)
        vdp_write_control(io->vdp, value);
}
