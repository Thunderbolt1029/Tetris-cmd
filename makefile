.POSIX:
CC     = gcc
CFLAGS = -W -O
LDLIBS = -lm -lncurses

game: obj/main.o
	$(CC) $(LDFLAGS) -o tetris obj/main.o $(LDLIBS)
obj/main.o: src/main.c
	$(CC) -c $(CFLAGS) src/main.c -o obj/main.o
