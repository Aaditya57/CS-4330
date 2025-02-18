CC=mips-linux-gnu-as
ASFLAGS=-EL -mips32

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
