# Version from file
VERSION := $(shell cat version.txt)

# Folder and file names
ODIR = ./obj
SOURCES_DIR = ./src
BUILD_DIR = ./build
DIST_DIR = ./dist
EXE = SIDECART.TOS

# VASM PARAMETERS
# _DEBUG: 1 to enable debug, 0 to disable them
# To disable debug, make target DEBUG_MODE=0
VASMFLAGS=-Faout -quiet -x -m68000 -spaces -showopt -devpac -D_DEBUG=$(DEBUG_MODE)
VASM = vasm 
VLINK =  vlink

# LIBCMINI PARAMETERS
# IMPORTANT! There is functional verson of the LIBCMINI library in the docker image
# To reference the library, it must set the absolute path inside the container image
# The library is stored in /freemint/libcmini 
# More info about the library: https://github.com/freemint/libcmini
LIBCMINI ?= /freemint/libcmini

# GCC PARAMETERS
CC = m68k-atari-mint-gcc
CFLAGS=-c -std=gnu99 -I$(LIBCMINI)/include -Os -DVERSION=\"$(VERSION)\"

# Check if DEBUG_MODE is not empty
ifneq ($(DEBUG_MODE),)
    CFLAGS += -D_DEBUG=$(DEBUG_MODE)
endif

# LINKER PARAMETERS
# Add the -s option to strip the binary
LINKFLAGS=-nostdlib -L$(LIBCMINI)/lib -lcmini -lgcc -Wl,--traditional-format -O2 -fomit-frame-pointer -s 

_OBJS = 

OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

.PHONY: all
all: dist prepare

.PHONY: release
release: dist prepare

.PHONY: prepare
prepare: clean
	mkdir -p $(BUILD_DIR)

clean-compile : clean main.o screen.o

# All C files
screen.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/screen.c -o $(BUILD_DIR)/screen.o

helper.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/helper.c -o $(BUILD_DIR)/helper.o

romselector.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/romselector.c -o $(BUILD_DIR)/romselector.o

floppyselector.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/floppyselector.c -o $(BUILD_DIR)/floppyselector.o

floppydb.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/floppydb.c -o $(BUILD_DIR)/floppydb.o

config.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/config.c -o $(BUILD_DIR)/config.o

reset.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/reset.c -o $(BUILD_DIR)/reset.o

network.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/network.c -o $(BUILD_DIR)/network.o

storage.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/storage.c -o $(BUILD_DIR)/storage.o

rtc.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/rtc.c -o $(BUILD_DIR)/rtc.o

main.o: prepare
	$(CC) $(CFLAGS) $(SOURCES_DIR)/main.c -o $(BUILD_DIR)/main.o

main: screen.o helper.o network.o config.o reset.o romselector.o floppyselector.o floppydb.o storage.o rtc.o main.o
	$(CC) $(LIBCMINI)/lib/crt0.o \
		  $(BUILD_DIR)/screen.o \
		  $(BUILD_DIR)/romselector.o \
		  $(BUILD_DIR)/floppyselector.o \
		  $(BUILD_DIR)/floppydb.o \
		  $(BUILD_DIR)/config.o \
		  $(BUILD_DIR)/reset.o \
		  $(BUILD_DIR)/helper.o \
		  $(BUILD_DIR)/network.o \
		  $(BUILD_DIR)/storage.o \
		  $(BUILD_DIR)/rtc.o \
		  $(BUILD_DIR)/main.o \
		  -o $(BUILD_DIR)/$(EXE) $(LINKFLAGS);

.PHONY: dist
dist: main
	mkdir -p $(DIST_DIR)
	cp $(BUILD_DIR)/$(EXE) $(DIST_DIR) 	

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

## Tag this version
.PHONY: tag
tag:
	git tag v$(VERSION) && git push origin v$(VERSION) && \
	echo "Tagged: $(VERSION)"
