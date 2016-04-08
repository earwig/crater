# Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

PROGRAM = crater
SOURCES = src
BUILD   = build
DEVEXT  = -dev
TESTS   = cpu vdp psg asm dis integrate

CC     = clang
FLAGS  = -O2 -Wall -Wextra -pedantic -std=c11
CFLAGS = $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs)
DFLAGS = -g -DDEBUG_MODE
MKDIR  = mkdir -p
RM     = rm -rf
ASM_UP = scripts/update_asm_instructions.py

MODE = release
BNRY = $(PROGRAM)
FLGS = $(FLAGS)
SDRS = $(shell find $(SOURCES) -type d | xargs echo)
SRCS = $(filter-out %.inc.c,$(foreach d,. $(SDRS),$(wildcard $(addprefix $(d)/*,.c))))
OBJS = $(patsubst %.c,%.o,$(addprefix $(BUILD)/$(MODE)/,$(SRCS)))
DEPS = $(OBJS:%.o=%.d)
DIRS = $(sort $(dir $(OBJS)))
TCPS = $(addprefix test-,$(TESTS))

ifdef DEBUG
	BNRY := $(BNRY)$(DEVEXT)
	FLGS += $(DFLAGS)
	MODE  = debug
endif

export CC
export FLAGS
export RM

.PHONY: all clean test tests test-prereqs test-make-prereqs $(TCPS)

all: $(BNRY)

clean:
	$(RM) $(BUILD) $(PROGRAM) $(PROGRAM)$(DEVEXT)
	@$(MAKE) -C tests clean

$(DIRS):
	$(MKDIR) $@

$(BNRY): $(OBJS)
	$(CC) $(FLGS) $(LIBS) $(OBJS) -o $@

$(OBJS): | $(DIRS)

$(BUILD)/$(MODE)/%.o: %.c
	$(CC) $(FLGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

ASM_INST = $(SOURCES)/assembler/instructions
$(ASM_INST).inc.c: $(ASM_INST).yml $(ASM_UP)
	python $(ASM_UP)

test-prereqs: $(PROGRAM)
	@: # No-op; prevents make from cluttering output with "X is up to date"

test-make-prereqs:
	@$(MAKE) test-prereqs DEBUG=

test: test-make-prereqs
	@$(MAKE) -C tests -s all

tests: test

$(TCPS): test-make-prereqs
	@$(MAKE) -C tests -s $(subst test-,,$@)
