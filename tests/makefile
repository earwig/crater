# Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

RUNNER     = runner
COMPONENTS = cpu vdp psg asm dis integrate

.PHONY: all clean $(COMPONENTS)

all: $(COMPONENTS)

clean:
	$(RM) $(RUNNER)

$(RUNNER): $(RUNNER).c
	$(CC) $(FLAGS) $< -o $@

$(COMPONENTS): $(RUNNER)
	./$(RUNNER) $@
