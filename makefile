# Copyright (C) 2014 Ben Kurtovic <ben.kurtovic@gmail.com>
# Released under the terms of the MIT License. See LICENSE for details.

PROGRAM = crater
SOURCES = src
BUILD   = build
DEVEXT  = -dev

CC     = gcc
FLAGS  = -O2 -Wall -pedantic -std=c11
CFLAGS = $(shell sdl2-config --cflags)
LIBS   = $(shell sdl2-config --libs)
MKDIR  = mkdir -p
RM     = rm -rf

MODE = release
SRCS = $(foreach d,.,$(wildcard *.c)) $(foreach d,$(SOURCES),$(wildcard $(addprefix $(d)/*,.c)))
OBJS = $(patsubst %.c,%.o,$(addprefix $(BUILD)/$(MODE)/,$(SRCS)))
DEPS = $(OBJS:%.o=%.d)
DIRS = $(sort $(dir $(OBJS)))

ifdef DEBUG
	PROGRAM := $(PROGRAM)$(DEVEXT)
	FLAGS   += -g
	MODE     = debug
endif

.PHONY: all clean

all: $(PROGRAM)

clean:
	$(RM) $(PROGRAM) $(PROGRAM)$(DEVEXT) $(BUILD)

$(DIRS):
	$(MKDIR) $@

$(PROGRAM): $(OBJS)
	$(CC) $(FLAGS) $(LIBS) $(OBJS) -o $@

$(OBJS): | $(DIRS)

$(BUILD)/$(MODE)/%.o: %.c
	$(CC) $(FLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)
