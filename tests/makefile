# Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

RUNNER     = runner
COMPONENTS = cpu vdp psg asm dis integrate

.PHONY: all clean $(COMPONENTS)

all: $(COMPONENTS)

clean:
	$(RM) $(RUNNER)
	$(RM) asm/*.gg

$(RUNNER): $(RUNNER).c
	$(CC) $(FLAGS) $< -o $@

$(COMPONENTS): $(RUNNER)
	./$(RUNNER) $@

asm: asm-unarchive

asm-archive:
	tar -czf asm/roms.tar.gz asm/*.gg

asm-unarchive:
	tar -xf asm/roms.tar.gz
