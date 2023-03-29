CC = gcc
CFLAGS = -g -Wall -Wextra
OBJS = map.o items.o player.o

dungeons: $(OBJS)
	$(CC) dungeons.c $^ -o $@ $(CFLAGS) -lcurses

items.o: items.h
map.o: map.h
player.o: player.h

.PHONY: clean test

test:
	make clean
	make
	./dungeons

clean:
	rm dungeons
	rm *.o
