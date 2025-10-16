.POSIX:
CC     = gcc
CFLAGS = -W -O
LDLIBS = -lm -lncursesw

SRC_FILES := $(wildcard src/*.c)
OBJ_FILES := $(patsubst src/%.c,obj/%.o,$(SRC_FILES))

tetris: $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^ 

obj/%.o: src/%.c | obj
	$(CC) -c -o $@ $^

obj:
	mkdir -p obj

clean:
	rm -r obj/ tetris
