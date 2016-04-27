/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "psg.h"
#include "util.h"

/*
    Initialize the SN76489 Programmable Sound Generator (PSG).
*/
void psg_init(PSG *psg)
{
    // TODO
    (void) psg;
}

/*
    Free memory previously allocated by the PSG.
*/
void psg_free(PSG *psg)
{
    // TODO
    (void) psg;
}

/*
    Power on the PSG, setting up initial state.
*/
void psg_power(PSG *psg)
{
    psg->tone1 = psg->tone2 = psg->tone3 = 0x0000;
    psg->vol1 = psg->vol2 = psg->vol3 = 0x0F;
    psg->noise = 0x00;
}

/*
    Write a byte of input to the PSG.
*/
void psg_write(PSG *psg, uint8_t byte)
{
    // TODO
    (void) psg;
    TRACE("PSG ignoring write: 0x%02X", byte)
}
