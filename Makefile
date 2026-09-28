CC = clang
CFLAGS = -std=c89 -pedantic -Wall -Wextra -Iinclude $(shell pkg-config --cflags raylib 2>/dev/null || echo "-I/opt/homebrew/include")
LDFLAGS = $(shell pkg-config --libs raylib 2>/dev/null || echo "-L/opt/homebrew/lib -lraylib") -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm

SRC = src/simulator.c src/main.c
OBJ = $(SRC:src/%.c=build/%.o)
TARGET = bin/particle_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build bin

.PHONY: all run clean
