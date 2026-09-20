CC = clang
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Iinclude
LDFLAGS = -lm

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
TARGET = $(BIN_DIR)/particle_sim

.PHONY: all clean run

all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile each .c translation unit into an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directories if they do not exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run the compiled binary
run: $(TARGET)
	./$(TARGET)

# Remove build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
