
OBJS = map.o items.o


dungeons: $(OBJS)
	gcc dungeons.c $^ -o $@ -lcurses

.PHONY: clean test

test:
	make clean
	make
	./dungeons

clean:
	rm dungeons
	rm *.o