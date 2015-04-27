# Copyright (C) 2014 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

PROGRAM = crater
SOURCES = src
BUILD   = build
DEVEXT  = -dev

CC     = clang
FLAGS  = -O2 -Wall -Wextra -pedantic -std=c11
CFLAGS = $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs)
MKDIR  = mkdir -p
RM     = rm -rf

MODE = release
BNRY = $(PROGRAM)
SDRS = $(shell find $(SOURCES) -type d | xargs echo)
SRCS = $(filter-out %.inc.c,$(foreach d,. $(SDRS),$(wildcard $(addprefix $(d)/*,.c))))
OBJS = $(patsubst %.c,%.o,$(addprefix $(BUILD)/$(MODE)/,$(SRCS)))
DEPS = $(OBJS:%.o=%.d)
DIRS = $(sort $(dir $(OBJS)))

ifdef DEBUG
	BNRY  := $(BNRY)$(DEVEXT)
	FLAGS += -g -fsanitize=address -DDEBUG_MODE
	MODE   = debug
endif

.PHONY: all clean test test-all test-z80 test-asm test-dasm

all: $(BNRY)

clean:
	$(RM) $(BUILD) $(PROGRAM) $(PROGRAM)$(DEVEXT)

$(DIRS):
	$(MKDIR) $@

$(BNRY): $(OBJS)
	$(CC) $(FLAGS) $(LIBS) $(OBJS) -o $@

$(OBJS): | $(DIRS)

$(BUILD)/$(MODE)/%.o: %.c
	$(CC) $(FLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

test: test-all test-z80 test-asm test-dasm

test-all:
	@echo "running all tests"

test-z80:
	@echo "running Z80 CPU tests"

test-asm:
	@echo "running assembler tests"

test-dasm:
	@echo "running disassembler tests"
