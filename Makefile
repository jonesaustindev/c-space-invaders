OS := $(shell uname -s)

ifeq ($(OS),Darwin) # macOS
    CFLAGS = -Iinclude -Wall $(shell sdl2-config --cflags) -MMD -MP
    LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf
else # Assuming Linux
    CFLAGS = -Iinclude -Wall $(shell sdl2-config --cflags) -MMD -MP
    LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf
endif

CC = gcc
SRC = $(wildcard src/*.c)
OBJDIR = obj
DEP = $(OBJDIR)/*.d
OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
TARGET = c_invaders

# Ensure the object file directory exists
$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*.d $(TARGET)

run: $(TARGET)
	./$(TARGET)

