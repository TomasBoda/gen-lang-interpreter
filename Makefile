# Makefile

# Compiler to use
CC := gcc

# Compiler flags
CFLAGS := -Wall -g

# Source directory
SRC_DIR := src

# Object files directory
OBJ_DIR := build

# Executable name
EXEC := build/main

# Find all .c files in the SRC_DIR and its subdirectories
SOURCES := $(shell find $(SRC_DIR) -name '*.c')

# Object files corresponding to sources
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Default target
all: $(EXEC)

# Rule to create the executable
$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to create object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

.PHONY: all clean
