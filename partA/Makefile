# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -std=c11

# Targets
TARGET = cat

# Source files
SRCS = cat.c lib.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile main.c to main.o
cat.o: cat.c lib.h
	$(CC) $(CFLAGS) -c cat.c

# Compile mylib.c to mylib.o
mylib.o: lib.c lib.h
	$(CC) $(CFLAGS) -c lib.c

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean