# Find all directories containing test.s files
DIRS := $(dir $(shell find . -name "test.s"))

# Derive output file names based on directory names
# This removes the trailing slash and prepends "build/"
TARGETS := $(patsubst %/,build/%.o,$(DIRS))

# MIPS assembler command
AS=mipsel-linux-gnu-gcc 
ASFLAGS=-mips32 -nostartfiles -Ttext=0

.PHONY: all clean

all: $(TARGETS)

# Rule to create the build directory
build:
	mkdir -p build

# Rule to assemble test.s files
# $(@F) extracts the filename from the target path
# $(patsubst build/%.o,%,$@) extracts the directory name from the target
build/%.o: build
	$(AS) $(ASFLAGS) $(patsubst build/%.o,%,$@)/test.s -o $@

# Clean rule
clean:
	rm -rf build
