#
SHELL := /bin/bash
#
CC := gcc
AR := gcc-ar rc
#
CFLAGS := -Wall -Wextra -Wl,-O1 \
-mtune=generic -std=gnu11 -O2 -Ofast \
-fexpensive-optimizations -ffunction-sections \
-I./include
#
LDFLAGS := -lz -larchive
#
SOURCES := $(wildcard src/*.c)
#
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))



all: default

%.o: %.c
	@printf "\t\t\t\t$$(tput el1 && tput setaf 2)"
	@printf "\r compiling: %s" $^
	@printf "$$(tput sgr0)"
	@$(CC) $(CFLAGS) -c $^ -o $@

droidimg: $(OBJECTS)
	@printf "\t\t\t\t$$(tput el1 && tput setaf 42)"
	@printf "\r linking: %s" $^
	@printf "$$(tput sgr0)"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


default: androidimg clean


clean:
	@find * -type f -name '*.o' -delete


#
.PHONY: xosd.o