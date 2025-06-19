SRC_DIR = src
UNITTEST_DIR = unittest
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/objs
UNITTEST_BIN_DIR = $(BUILD_DIR)/$(UNITTEST_DIR)
INCLUDE_DIR = include

TARGET = $(BUILD_DIR)/asbtl

CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR) -g

# src/main.c, src/scanner.c, ... -> build/objs/main.o, build/objs/scanner.o ...
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
OBJS_NO_MAIN = $(filter-out $(OBJ_DIR)/main.o,$(OBJS))

# unittest/scanner_test.c -> build/unittest/scanner_test
UNITTEST_SRCS = $(wildcard $(UNITTEST_DIR)/*_test.c)
UNITTEST_BINS = $(patsubst $(UNITTEST_DIR)/%.c,$(UNITTEST_BIN_DIR)/%,$(UNITTEST_SRCS))

.PHONY: all clean unittest

all: $(TARGET)

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

$(UNITTEST_BIN_DIR):
	mkdir -p $(UNITTEST_BIN_DIR)

# Build and run each unit test binary
unittest: $(UNITTEST_BINS) | $(BUILD_DIR)
	@for bin in $(UNITTEST_BINS); do \
		echo "Running $$bin..."; \
		./$$bin; \
	done

# Build each unittest executable from it's objects and non-main src objects
$(UNITTEST_BIN_DIR)/%: $(UNITTEST_BIN_DIR)/%.o $(OBJS_NO_MAIN)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each unittest/*.c file to build/unittest/*.o files
$(UNITTEST_BIN_DIR)/%.o: $(UNITTEST_DIR)/%.c | $(UNITTEST_BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

