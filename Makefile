CC = gcc
CFLAGS = -Wall -Wextra
LFLAGS = -L./raylib/lib -lraylib -lm -lX11
IFLAGS = -I./raylib/include

# gcc -I./raylib/include -o main main.c  -L./raylib/lib -lraylib -lm -lX11	

run: compile
	./main

compile: main.c
	$(CC) $(CFLAGS) $(IFLAGS) -o main main.c $(LFLAGS)

christian: main.c
	$(CC) $(CFLAGS) main.c $(LFLAGS) -o main -I/home/christian/Desktop/raylib-4.2.0_linux_amd64/include -L/home/christian/Desktop/raylib-4.2.0_linux_amd64/lib 
	./main

.PHONY: clean
clean:
	rm main

