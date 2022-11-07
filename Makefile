CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib

run: compile
	./main

compile: main.c
	$(CC) $(CFLAGS) $(LFLAGS) -o main main.c

.PHONY: clean
clean:
	rm main

