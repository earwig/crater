crater
======

**crater** is an emulator for the [Sega Game Gear][game gear], with an included
[Z80][z80] assembler/disassembler, written in C.

<img src="/docs/tailsadventure.png?raw=true" title="Tails Adventure (1995)" alt="Tails Adventure (1995)" width="320px">

[game gear]: https://en.wikipedia.org/wiki/Sega_Game_Gear
[z80]: https://en.wikipedia.org/wiki/Zilog_Z80

Why?
----

While the internet is full of emulators for retro game systems, writing one is
nevertheless a fun learning project.

crater is named after [31 Crateris][crateris], a star that was – for a short
time in 1974 – misidentified as [a moon of Mercury][moon]. Mercury was Sega's
codename for the Game Gear during development.

[crateris]: http://www.astrostudio.org/xhip.php?hip=58587
[moon]: https://en.wikipedia.org/wiki/Mercury%27s_moon

Installing
----------

Only OS X and Linux are tested. You'll need a modern compiler that supports C11
([clang][clang] preferred) and [SDL 2][sdl2]. Using Homebrew, you can
`brew install sdl2`; using apt, you can `apt-get install libsdl2-dev`.

Run `make` to create `./crater`. To build the development version with debug
symbols and no optimizations, run `make DEBUG=1`, which creates `./crater-dev`.

crater has a number of test cases. Run the entire suite with `make test`;
individual components can be tested by doing `make test-{component}`, where
`{component}` is one of `cpu`, `vdp`, `psg`, `asm`, `dis`, or `integrate`.

[clang]: http://clang.llvm.org/
[sdl2]: https://www.libsdl.org/

Usage
-----

Running `./crater` without arguments will display a list of ROM images located
in the `roms/` directory, and then ask the user to pick one, or enter their own
ROM path. You can provide a path directly with `./crater path/to/rom`.

Add or symlink ROMs to `roms/` at your leisure. Note that they must end in
`.gg` or `.bin` to be auto-detected.

Add `--fullscreen` (`-f`) to enable fullscreen mode, or `--scale <n>`
(`-x <n>`) to scale the game screen by an integer factor in windowed mode (this
only sets the starting configuration; the window should be resizeable).

For games that support it, crater will save cartridge RAM ("battery saves";
these are distinct from save states, which are not yet supported) to a file
named `<rom>.sav`, where `<rom>` is the path to the ROM file. You can set a
custom save location with `--save <path>` (`-s <path>`) or disable saving
entirely with `--no-save`.

Add `--debug` (`-g`) to show logging information while running. Pass it twice
(`-gg`) to show more detailed logs, including an emulator trace.

crater tries to reproduce the Game Gear's native display resolution, which had
a pixel aspect ratio (PAR) of [8:7][par]; this means the pixels were slightly
wider than square, unlike modern LCD displays with a 1:1 PAR. Add `--square`
(`-q`) to force square pixels.

`./crater -h` gives (fairly basic) command-line usage, and `./crater -v` gives
the current version.

[par]: https://pineight.com/mw/index.php?title=Dot_clock_rates

### Input

crater supports keyboard and joystick/controller input.

For keyboards, custom mappings are not yet supported. There are two primary
configurations I like:

- `Return`/`Esc` for `Start`; `WASD` for D-pad; `J` for `1`/left trigger;
  `K` for `2`/right trigger

- `Return`/`Esc` for `Start`; arrow keys for D-pad; `Z` for `1`/left trigger;
  `X` for `2`/right trigger

You can switch between them freely.

For controllers, crater uses SDL controller mappings. Many common controllers
are supported out of the box, but you may define your own mappings by creating
a file named `gamecontrollerdb.txt` in the working directory. For more info,
see [this community mapping database][gcdb].

[gcdb]: https://github.com/gabomdq/SDL_GameControllerDB

### Assembler/Disassembler

crater has built-in support for converting Z80 assembly into ROM images, as
well as attempting the reverse process (i.e., disassembling).

`--assemble <input> [<output>]` (`-a`) converts source code into a `.gg` binary
that can be run by crater. `--disassemble <input> [<output>]` (`-d`) executes
the opposite operation. If no output file is given, crater will use the name of
the input file, with the extension replaced with `.gg` for `-a` and `.asm` for
`-d`. By default, this will never overwrite the original filename; pass
`--overwrite` (`-r`) to let crater do so.

Status
------

The emulator is almost fully functional, lacking only audio support, a few
uncommon CPU instructions, and some advanced graphics features. Most games are
playable with only minor bugs. Future goals include full save states and a more
sophisticated debugging mode.

The assembler is complete. Future goals include more documentation, macros, and
additional directives.

The disassembler works, but can't differentiate between code and data yet, so
it's not very useful.

The testing infrastructure is limited. The assembler has decent coverage, other
components minimal.

Credits
-------

Special thanks to [SMS Power!][sms_power]'s excellent [development][sms_dev]
section, which has been invaluable in figuring out many of the Game Gear's
details, including ROM header structure and the memory mapping system. Various
source code comments reference their pages.

Also thanks to [Thomas Scherrer's Z80 website][scherrer] for many useful
resources about the Game Gear's CPU, including info about
[undocumented opcodes][undoc_ops] and [flags][undoc_flags]. Finally, credit
goes to [ClrHome][clrhome] for their helpful Z80 [instruction table][clrtab].

[sms_power]: http://www.smspower.org/
[sms_dev]: http://www.smspower.org/Development/Index
[scherrer]: http://z80.info/
[undoc_ops]: http://www.z80.info/z80undoc.htm
[undoc_flags]: http://z80.info/z80sflag.htm
[clrhome]: http://clrhome.org/
[clrtab]: http://clrhome.org/table/
