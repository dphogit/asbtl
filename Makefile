SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/objs
INCLUDE_DIR = include

TARGET = $(BUILD_DIR)/asbtl

CC = gcc
CFLAGS = -Wall -Wextra -I$(INCLUDE_DIR) -g

# src/main.c, src/scanner.c, ... -> build/objs/main.o, build/objs/scanner.o ...
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean

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

clean:
	rm -rf $(BUILD_DIR)

