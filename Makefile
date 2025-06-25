SRC_DIR = src
UNITTEST_DIR = unittests
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/objs
OBJ_UNITTEST_DIR = $(OBJ_DIR)/unittests
INCLUDE_DIR = include

TARGET = $(BUILD_DIR)/asbtl
UNITTEST_TARGET = $(BUILD_DIR)/run_unittests

CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR) -g

# src/main.c, src/scanner.c, ... -> build/objs/main.o, build/objs/scanner.o ...
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
OBJS_NO_MAIN = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

# unittest/scanner_test.c -> build/objs/scanner_test.o
UNITTEST_SRCS = $(wildcard $(UNITTEST_DIR)/*.c)
UNITTEST_OBJS = $(patsubst $(UNITTEST_DIR)/%.c,$(OBJ_UNITTEST_DIR)/%.o,$(UNITTEST_SRCS))

.PHONY: all run clean unittests tests

all: $(TARGET)

run: $(TARGET)
	$(TARGET)

# Link final binary
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c files to .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_UNITTEST_DIR):
	mkdir -p $(OBJ_UNITTEST_DIR)

# Run the unit tests binary
unittests: $(UNITTEST_TARGET)
	./$(UNITTEST_TARGET)

# Build the unit tests binary with the non-main src and unit test objects
$(UNITTEST_TARGET): $(UNITTEST_OBJS) $(OBJS_NO_MAIN) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -o $@ $^

# Compile unittest .c files to .o files
$(OBJ_UNITTEST_DIR)/%.o: $(UNITTEST_DIR)/%.c | $(OBJ_UNITTEST_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

tests: $(TARGET)
	./tests/bats/bin/bats -r ./tests/suite/

clean:
	rm -rf $(BUILD_DIR)

