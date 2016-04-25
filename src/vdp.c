/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "vdp.h"

/*
    Initialize the Video Display Processor (VDP).
*/
void vdp_init(VDP *vdp)
{
    // TODO
}

/*
    Free memory previously allocated by the VDP.
*/
void vdp_free(VDP *vdp)
{
    // TODO
}

/*
    Power on the VDP, setting up initial state.
*/
void vdp_power(VDP *vdp)
{
    vdp->control_code = 0;
    vdp->control_addr = 0;
    vdp->control_flag = false;
}

/*
    Read a byte from the VDP's control port.
*/
uint8_t vdp_read_control(VDP *vdp)
{
    vdp->control_flag = false;

    // TODO
    return 0;
}

/*
    Read a byte from the VDP's data port.
*/
uint8_t vdp_read_data(VDP *vdp)
{
    vdp->control_flag = false;

    // TODO
    return 0;
}

/*
    Write a byte into the VDP's control port.
*/
void vdp_write_control(VDP *vdp, uint8_t byte)
{
    if (!vdp->control_flag) {  // First byte
        vdp->control_addr = (vdp->control_addr & 0x3F00) + byte;
    } else {  // Second byte
        vdp->control_addr = ((byte & 0x3F) << 8) + (vdp->control_addr & 0xFF);
        vdp->control_code = byte >> 6;
    }
    vdp->control_flag = !vdp->control_flag;
}

/*
    Write a byte into the VDP's data port.
*/
void vdp_write_data(VDP *vdp, uint8_t byte)
{
    vdp->control_flag = false;

    // TODO
}
