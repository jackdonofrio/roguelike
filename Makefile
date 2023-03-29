CC = gcc
CFLAGS = -g -Wall -Wextra
OBJS = map.o items.o player.o

dungeons: $(OBJS)
	$(CC) $(CFLAGS) dungeons.c $^ -o $@ -lcurses

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
