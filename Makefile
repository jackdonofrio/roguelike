CC = gcc
CFLAGS = -g -Wall -Wextra
OBJS = map.o items.o player.o

dungeons: $(OBJS)
	$(CC) dungeons.c $^ -o $@ $(CFLAGS) -lcurses

items.o: items.c
map.o: map.c
player.o: player.c

.PHONY: clean test

test:
	make clean
	make
	./dungeons

clean:
	rm dungeons
	rm *.o
