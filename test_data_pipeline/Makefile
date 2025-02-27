# Makefile for assembling test.s files in all subdirectories

# Variables
ASM = mipsel-linux-gnu-gcc    # Assembler command (change if needed)
ASMFLAGS = -mips32 -nostartfiles -Ttext=0 
BINS_DIR = bins

# Find all directories containing test.s files
DIRS := $(dir $(shell find . -name "test.s" -type f))

# Generate target names for the object files
TARGETS := $(patsubst ./%, $(BINS_DIR)/%.o, $(DIRS:%/=%))

# Default target
all: $(BINS_DIR) $(TARGETS)

# Create bins directory if it doesn't exist
$(BINS_DIR):
	mkdir -p $(BINS_DIR)

# Rule to assemble test.s files
$(BINS_DIR)/%.o: %/test.s
	@echo "Assembling $< into $@"
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

# Clean up
clean:
	rm -rf $(BINS_DIR)

# Show which directories will be processed
list:
	@echo "Directories containing test.s files:"
	@for dir in $(DIRS); do echo "  $$dir"; done
	@echo "Target object files:"
	@for target in $(TARGETS); do echo "  $$target"; done

# Help message
help:
	@echo "Available targets:"
	@echo "  all    - Assemble all test.s files (default)"
	@echo "  clean  - Remove bins directory and all output files"
	@echo "  list   - List directories containing test.s files and target object files"
	@echo "  help   - Show this help message"

.PHONY: all clean list help
