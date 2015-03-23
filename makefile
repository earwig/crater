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
SRCS = $(foreach d,.,$(wildcard *.c)) $(foreach d,$(SOURCES),$(wildcard $(addprefix $(d)/*,.c)))
OBJS = $(patsubst %.c,%.o,$(addprefix $(BUILD)/$(MODE)/,$(SRCS)))
DEPS = $(OBJS:%.o=%.d)
DIRS = $(sort $(dir $(OBJS)))

ifdef DEBUG
	BNRY  := $(BNRY)$(DEVEXT)
	FLAGS += -g -DDEBUG_MODE
	MODE   = debug
endif

.PHONY: all clean test

all: $(BNRY)

clean:
	$(RM) $(BUILD) $(PROGRAM) $(PROGRAM)$(DEVEXT)

test:
	@echo "not implemented yet"

$(DIRS):
	$(MKDIR) $@

$(BNRY): $(OBJS)
	$(CC) $(FLAGS) $(LIBS) $(OBJS) -o $@

$(OBJS): | $(DIRS)

$(BUILD)/$(MODE)/%.o: %.c
	$(CC) $(FLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)
