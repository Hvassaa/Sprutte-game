CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -L./raylib/lib -lraylib -lm -lX11
IFLAGS = -I./raylib/include

run: compile
	./main

compile: main.c
	$(CC) $(CFLAGS) $(IFLAGS) -o main main.c $(LFLAGS)

.PHONY: clean
clean:
	rm main

