/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <unistd.h>

#include "gamegear.h"
#include "logging.h"
#include "util.h"

/* Clock speed in Hz was taken from the official Sega GG documentation */
#define CPU_CLOCK_SPEED (3579545.)
#define CYCLES_PER_FRAME (CPU_CLOCK_SPEED / GG_FPS)
#define CYCLES_PER_LINE (CYCLES_PER_FRAME / VDP_LINES_PER_FRAME)
#define NS_PER_FRAME (1000 * 1000 * 1000 / GG_FPS)

#define SET_EXC(...) snprintf(gg->exc_buffer, GG_EXC_BUFF_SIZE, __VA_ARGS__);

/*
    Create and return a pointer to a new GameGear object.
*/
GameGear* gamegear_create()
{
    GameGear *gg = cr_malloc(sizeof(GameGear));
    mmu_init(&gg->mmu);
    vdp_init(&gg->vdp);
    psg_init(&gg->psg);
    io_init(&gg->io, &gg->vdp, &gg->psg);
    z80_init(&gg->cpu, &gg->mmu, &gg->io);

    gg->powered = false;
    gg->callback = NULL;
    gg->exc_buffer[0] = '\0';
    return gg;
}

/*
    Destroy a previously-allocated GameGear object.

    Does *not* destroy any loaded ROM objects.
*/
void gamegear_destroy(GameGear *gg)
{
    mmu_free(&gg->mmu);
    vdp_free(&gg->vdp);
    psg_free(&gg->psg);
    free(gg);
}

/*
    Load a ROM image into the GameGear object.

    Does *not* steal a reference to the ROM object, so it must be kept alive
    until another ROM is loaded or the GameGear is destroyed. Calling this
    function while the GameGear is powered on has no effect.
*/
void gamegear_load(GameGear *gg, const ROM *rom)
{
    if (gg->powered)
        return;

    mmu_load_rom(&gg->mmu, rom->data, rom->size);
}

/*
    Power on the GameGear.

    This clears the exception buffer and executes boot code (e.g. clearing
    memory and setting initial register values).
*/
static void power_on(GameGear *gg)
{
    gg->exc_buffer[0] = '\0';
    gg->powered = true;

    mmu_power(&gg->mmu);
    vdp_power(&gg->vdp);
    io_power(&gg->io);
    z80_power(&gg->cpu);
}

/*
    Power off the GameGear.

    This function *may* be used while the GameGear is running to trigger a safe
    shutdown at the next opportunity. It is also reentrant. If the GameGear is
    already off, it will do nothing.
*/
void gamegear_power_off(GameGear *gg)
{
    gg->powered = false;
}

/*
    Set a callback to be triggered whenever the GameGear completes a frame.
*/
void gamegear_set_callback(GameGear *gg, GGFrameCallback callback)
{
    gg->callback = callback;
}

/*
    Reset the GameGear's frame callback function.
*/
void gamegear_clear_callback(GameGear *gg)
{
    gg->callback = NULL;
}

/*
    Simulate the GameGear for one frame.

    This function simulates the number of clock cycles corresponding to 1/60th
    of a second. The return value indicates whether an exception flag has been
    set somewhere. If true, emulation must be stopped.
*/
static bool simulate_frame(GameGear *gg)
{
    size_t line;
    bool except;

    for (line = 0; line < VDP_LINES_PER_FRAME; line++) {
        vdp_simulate_line(&gg->vdp);
        except = z80_do_cycles(&gg->cpu, CYCLES_PER_LINE);
        if (except)
            return true;
    }
    return false;
}

/*
    Simulate the GameGear.

    The GameGear must start out in an unpowered state; it will be powered only
    during the simulation. This function blocks until the simulation ends,
    either by an exception occurring or someone setting the GameGear's
    'powered' flag to false.

    If a callback has been set with gamegear_set_callback(), then we'll trigger
    it after every frame has been simulated (sixty times per second).

    Exceptions can be retrieved after this call with gamegear_get_exception().
    If the simulation ended normally, then that function will return NULL.
*/
void gamegear_simulate(GameGear *gg)
{
    if (gg->powered)
        return;

    DEBUG("GameGear: powering on")
    power_on(gg);

    while (gg->powered) {
        uint64_t start = get_time_ns(), delta;

        if (simulate_frame(gg) || !gg->powered)
            break;
        if (gg->callback)
            gg->callback(gg);

        delta = get_time_ns() - start;
        if (delta < NS_PER_FRAME)
            usleep((NS_PER_FRAME - delta) / 1000);
    }

    DEBUG("GameGear: powering off")
    gamegear_power_off(gg);
}

/*
    If an exception flag has been set in the GameGear, return the reason.

    This function returns a const pointer to a buffer holding a human-readable
    exception string (although it may be cryptic to an end-user). The buffer is
    owned by the GameGear object and should not be freed - its contents last
    until the GameGear is powered on. If no exception flag is set, this
    function returns NULL.
*/
const char* gamegear_get_exception(GameGear *gg)
{
    if (!gg->exc_buffer[0]) {
        if (gg->cpu.except) {
            switch (gg->cpu.exc_code) {
                case Z80_EXC_NOT_POWERED:
                    SET_EXC("CPU not powered")
                    break;
                case Z80_EXC_UNIMPLEMENTED_OPCODE:
                    SET_EXC("unimplemented opcode: 0x%02X", gg->cpu.exc_data)
                    break;
                case Z80_EXC_IO_ERROR:
                    SET_EXC("I/O error on port 0x%02X", gg->cpu.exc_data)
                    break;
                default:
                    SET_EXC("unknown exception")
                    break;
            }
        } else {
            return NULL;
        }
    }
    return gg->exc_buffer;
}

/*
    @DEBUG_LEVEL
    Print out some state info to stdout: Z80 and VDP register values, etc.
*/
void gamegear_print_state(const GameGear *gg)
{
    z80_dump_registers(&gg->cpu);
    vdp_dump_registers(&gg->vdp);
}
