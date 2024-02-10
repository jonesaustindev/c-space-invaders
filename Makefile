OS := $(shell uname -s)

ifeq ($(OS),Darwin) # macOS
    CFLAGS = -I/usr/local/include -Wall $(shell sdl2-config --cflags)
    LDFLAGS = $(shell sdl2-config --libs)
else # Assuming Linux
    CFLAGS = -Iinclude -Wall $(shell sdl2-config --cflags)
    LDFLAGS = $(shell sdl2-config --libs)
endif

CC = gcc
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = c_invaders

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

