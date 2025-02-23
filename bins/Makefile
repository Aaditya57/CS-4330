CC=mipsel-linux-gnu-gcc 
ASFLAGS=-mips32 -nostartfiles -Ttext=0

# Find all .s files in current directory
SOURCES = $(wildcard *.s)
# Convert .s filenames to .o
OBJECTS = $(SOURCES:.s=.o)

# Default target
all: $(OBJECTS)

%.o: %.s
	$(CC) $(ASFLAGS) $< -o $@

clean:
	rm *.o
