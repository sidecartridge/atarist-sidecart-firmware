# Version from file
VERSION := $(shell cat version.txt)

# Folder and file names
ODIR = ./obj
SOURCES_DIR = ./src
BUILD_DIR = ./build
DIST_DIR = ./dist
BIN = BOOT.BIN

# VASM PARAMETERS
# _DEBUG: 1 to enable debug, 0 to disable them
# To disable debug, make target DEBUG_MODE=0
DEBUG_MODE = 1
VASMFLAGS=-Faout -quiet -x -m68000 -spaces -showopt -devpac -D_DEBUG=$(DEBUG_MODE)
VASM = vasm 
VLINK =  vlink

.PHONY: all
all: prepare dist

.PHONY: release
release: prepare dist

.PHONY: prepare
prepare: clean
	mkdir -p $(BUILD_DIR)

clean-compile : clean main.o

main.o: prepare
	$(VASM) $(VASMFLAGS) $(SOURCES_DIR)/main.s -o $(BUILD_DIR)/main.o

.PHONY: build
build: main.o
	$(VLINK) $(BUILD_DIR)/$^ -brawbin1 -o $(BUILD_DIR)/$(BIN)


.PHONY: dist
dist: build
	mkdir -p $(DIST_DIR)
	cp $(BUILD_DIR)/$(BIN) $(DIST_DIR) 	

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)

## Tag this version
.PHONY: tag
tag:
	git tag v$(VERSION) && git push origin v$(VERSION) && \
	echo "Tagged: $(VERSION)"
