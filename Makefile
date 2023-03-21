dungeons:
	gcc dungeons.c -o dungeons -lcurses

.PHONY: clean test

test:
	make clean
	make
	./dungeons

clean:
	rm dungeons
