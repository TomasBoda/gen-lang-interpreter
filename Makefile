
CC := gcc
CFLAGS := -Wall -g

SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := build

EXEC := build/main

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

CFLAGS += -I$(INCLUDE_DIR)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(EXEC)

.PHONY: all clean
