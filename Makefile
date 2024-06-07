# Makefile

# Compiler to use
CC := gcc

# Compiler flags
CFLAGS := -Wall -g

# Source directory
SRC_DIR := ./

# Object files directory
OBJ_DIR := build

# Executable name
EXEC := build/main

# Find all .c files in the SRC_DIR
SOURCES := $(wildcard $(SRC_DIR)/*.c)

# Object files corresponding to sources
OBJECTS := $(SOURCES:%.c=$(OBJ_DIR)/%.o)

# Default target
all: $(EXEC)

# Rule to create the executable
$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to create object files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

.PHONY: all clean
