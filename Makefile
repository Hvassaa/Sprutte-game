CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -lraylib

run: compile
	./main

compile: main.c
	$(CC) $(CFLAGS) $(LFLAGS) -o main main.c
	
christian: main.c
	$(CC) $(CFLAGS) main.c $(LFLAGS) -o main -I/home/christian/Desktop/raylib-4.2.0_linux_amd64/include -L/home/christian/Desktop/raylib-4.2.0_linux_amd64/lib 
	./main

.PHONY: clean
clean:
	rm main

