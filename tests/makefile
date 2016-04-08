# Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

RUNNER     = runner
COMPONENTS = cpu vdp psg asm dis integrate

.PHONY: all clean $(COMPONENTS)

all: $(COMPONENTS)

clean:
	$(RM) $(RUNNER)
	rm asm/full/*.gg

$(RUNNER): $(RUNNER).c
	$(CC) $(FLAGS) $< -o $@

$(COMPONENTS): $(RUNNER)
	./$(RUNNER) $@

asm: asm-unarchive

asm-archive:
	tar -czf asm/full/roms.tar.gz asm/full/*.gg

asm-unarchive:
	tar -xf asm/full/roms.tar.gz
