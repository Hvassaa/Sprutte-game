CC = gcc
CFLAGS = -Wall -Wextra -Wunused-result

run: compile
	./main

compile: main.c
	$(CC) -o main main.c

.PHONY: clean
clean:
	rm main

