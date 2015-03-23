crater
======

**crater** is an emulator for the [Sega Game Gear][game gear], written in C.

[game gear]: https://en.wikipedia.org/wiki/Sega_Game_Gear

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

Only OS X and Linux are tested. You'll need a decent compiler that supports C11
(clang preferred) and SDL 2. Using Homebrew, you can `brew install sdl2`; using
apt, you can `apt-get install libsdl2-dev`.

Run `make` to create `./crater`. To build the development version with debug
symbols (they can exist simultaneously), run `make DEBUG=1`, which creates
`./crater-dev`.

Usage
-----

Running `./crater`  without arguments will display a list of ROM images located
in the `roms/` directory, and then ask the user to pick one, or enter their own
ROM path. You can provide a path directly with `./crater path/to/rom`.

Add or symlink ROMs to `roms/` at your leisure. Note that they should end in
`.gg` or `.bin`.

Add `--fullscreen` (`-f`) to enable fullscreen mode, or `--scale <n>`
(`-s <n>`) to scale the game screen by an integer factor.

`./crater -h` gives (fairly basic) command-line usage, and `./crater -v` gives
the current version.

### Advanced options

crater supports several advanced features. Add `--debug` (`-g`) to display
detailed information about emulation state while running, including register
values and memory contents. You can also pause emulation to set breakpoints and
change state.

`--assemble <input> [<output>]` (`-a`) converts z80 assembly source code into a
`.gg` binary that can be run by crater. `--disassemble <input> [<output>]`
(`-d`) executes the opposite operation. If no output file is given, crater will
use the name of the input file, with the extension replaced with `.gg` for `-a`
and `.s` for `-d`. By default, this will never overwrite the original filename;
pass `--overwrite` (`-r`) to let crater do so.
