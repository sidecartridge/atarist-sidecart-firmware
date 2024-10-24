# Version from file
VERSION := $(shell cat version.txt)
VERSION_MAJOR := $(shell echo $(VERSION) | awk -F'.' '{print $$1}')
VERSION_MINOR := $(shell echo $(VERSION) | awk -F'.' '{print $$2}')
VERSION_PATCH := $(shell echo $(VERSION) | awk -F'.' '{print $$3}')


# GEMDOS Date: YYYYMMDD format
YEAR = $(shell date +'%Y')
MONTH = $(shell date +'%-m')   # %-m gives the month without leading zeros
DAY = $(shell date +'%-d')     # %-d gives the day without leading zeros


# Create the GEMDOS date format
GEMDOS_DATE_FORMATTED := $(shell echo "$$(($(YEAR) - 1980 << 9 | $(MONTH) << 5 | $(DAY)))")


# GEMDOS Time: HHMMSS format
HOUR = $(shell date +'%-H')    # %-H gives the hour without leading zeros
MINUTE = $(shell date +'%-M')  # %-M gives the minute without leading zeros
SECOND = $(shell date +'%-S')  # %-S gives the second without leading zeros

# Halve the seconds for GEMDOS format
HALVED_SECOND = $(shell echo "$$(( $(SECOND) / 2 ))")


# Create the GEMDOS time format
GEMDOS_TIME_FORMATTED :=  $(shell echo "$$(( $(HOUR) << 11 | $(MINUTE) << 5 | $(HALVED_SECOND) ))")

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
VASMFLAGS=-Faout -quiet -x -m68000 -spaces -showopt -devpac -D_DEBUG=$(DEBUG_MODE) -DGEMDOS_DATE=$(GEMDOS_DATE_FORMATTED) -DGEMDOS_TIME=$(GEMDOS_TIME_FORMATTED) -DVERSION_MAJOR=\"$(VERSION_MAJOR)\" -DVERSION_MINOR=\"$(VERSION_MINOR)\" -DVERSION_PATCH=\"$(VERSION_PATCH)\"
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
