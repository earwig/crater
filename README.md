crater
======

**crater** is an emulator for the [Sega Game Gear][game gear], written in C.

[game gear]: https://en.wikipedia.org/wiki/Sega_Game_Gear

Why?
----

While the internet is full of emulators for retro game systems, writing one is
nevertheless a fun learning project.

Crater is named after [31 Crateris][crateris], a star that was – for a short
time in 1974 – misidentified as [a moon of Mercury][moon]. Mercury was Sega's
codename for the Game Gear during development.

[crateris]: http://www.astrostudio.org/xhip.php?hip=58587
[moon]: https://en.wikipedia.org/wiki/Mercury%27s_moon

Installing
----------

Only OS X and Linux are tested. You'll need a decent compiler that supports C11
(gcc, clang) and SDL 2. Using Homebrew, you can `brew install sdl2`; using apt,
you can `apt-get install libsdl2-dev`.

Run `make` and then `./crater`. To build the development version with debug
symbols (they can exist simultaneously), run `make DEBUG=1` and then
`./crater-dev`.
