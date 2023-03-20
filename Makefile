dungeons:
	gcc dungeons.c -o dungeons -lcurses

.PHONY: clean

clean:
	rm dungeons
