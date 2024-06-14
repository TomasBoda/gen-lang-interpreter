CC := gcc
CFLAGS := -Wall -g

SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := build
TEST_DIR := tests

EXEC := $(OBJ_DIR)/main
TEST_EXEC := $(OBJ_DIR)/tests

SOURCES := $(shell find $(SRC_DIR) -name '*.c')
TEST_SOURCES := $(shell find $(TEST_DIR) -name '*.c')

OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SOURCES))

CFLAGS += -I$(INCLUDE_DIR)

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_EXEC): $(TEST_OBJECTS) $(filter-out $(OBJ_DIR)/main.o, $(OBJECTS))
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -O3 -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -O3 -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(EXEC) $(TEST_EXEC)

.PHONY: all clean

# Targets for building and running tests
tests: $(TEST_EXEC)

run_tests: tests
	$(TEST_EXEC)