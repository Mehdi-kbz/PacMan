CC = gcc


CFLAGS = -I/opt/homebrew/Cellar/sdl2/2.30.5/include -Wall -Wextra -std=c99
LDFLAGS = -L/opt/homebrew/Cellar/sdl2/2.30.5/lib -lSDL2 -lSDL2_image -lm

EXEC = pacman

all: $(EXEC)

$(EXEC): src/main.c
	$(CC) src/main.c -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(EXEC)
