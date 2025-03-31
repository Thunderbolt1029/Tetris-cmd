.POSIX:
CC     = gcc
CFLAGS = -W -O
LDLIBS = -lm -lncursesw

tetris: obj/main.o obj/common.o obj/menu.o obj/game.o obj/lost.o
	$(CC) $(LDFLAGS) -o tetris obj/main.o obj/common.o obj/menu.o obj/game.o obj/lost.o $(LDLIBS)

obj/main.o: src/main.c src/menu.h src/game.h src/common.h
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/common.o: src/common.c src/common.h
	$(CC) $(CFLAGS) -c src/common.c -o obj/common.o

obj/menu.o: src/menu.c src/menu.h src/common.h
	$(CC) $(CFLAGS) -c src/menu.c -o obj/menu.o

obj/game.o: src/game.c src/game.h src/common.h
	$(CC) $(CFLAGS) -c src/game.c -o obj/game.o

obj/lost.o: src/lost.c src/lost.h src/common.h
	$(CC) $(CFLAGS) -c src/lost.c -o obj/lost.o
