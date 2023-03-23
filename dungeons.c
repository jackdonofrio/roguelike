/*

+JMJ+

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include "map.h"

typedef struct player
{
    int row;
    int column;
    uint health; // integer from 0 to 100
} player;

void setup_ncurses();
void write_map_curse(char* map);
void curse_put(int row, int col, char c, int color);
char handle_keypress(player* player_ptr, char key, char* map);
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change);
player* player_init(int start_row, int start_col);
void set_player_spawn(room** rooms, player* p);


int main()
{
    srand(time(NULL));
    player* player_ptr = player_init(2, 2);
    char* map;
    room** rooms;
    int floor = 0;

    rooms = rooms_gen();
    map = map_gen(rooms, floor); 
    set_player_spawn(rooms, player_ptr);
        
    setup_ncurses();
    write_map_curse(map);
    curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
    

    char key;
    while ((key = getch()) != 'q') {
        if (handle_keypress(player_ptr, key, map) == STAIR) {
            delete_rooms(rooms);
            free(map);

            rooms = rooms_gen();
            map = map_gen(rooms, ++floor); 
            set_player_spawn(rooms, player_ptr);
        }        
        // update map and player location
        write_map_curse(map);
        curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
    }
    free(map);
    free(player_ptr);
    for (int i = 0; i < ROOM_COUNT; i++) {
        free(rooms[i]);
    }
    free(rooms);
    
    endwin();
    return 0;
}


void set_player_spawn(room** rooms, player* p)
{
    room* r = rooms[rand() % ROOM_COUNT];
    int right = r->corner_col + r->width;
    int bottom = r->corner_row + r->height;
    p->row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
    p->column = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;

}

char handle_keypress(player* player_ptr, char key, char* map)
{
    char map_char;
    switch (key) {
        case 'q':
            break;
        case 'w':
            return handle_walk_key(player_ptr, map, -1, 0);
        case 'a':
            return handle_walk_key(player_ptr, map, 0, -1);
        case 's':
            return handle_walk_key(player_ptr, map, 1, 0);
        case 'd':
            return handle_walk_key(player_ptr, map, 0, 1);
        }
}

// returns code based on what was stepped on
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change)
{
    int new_row = player_ptr->row + row_change;
    int new_col = player_ptr->column + col_change;
    char c = can_step(map, new_row, new_col);
    if (c) {
        player_ptr->row = new_row;
        player_ptr->column = new_col;
        return c;
    }
    return 0;
}

player* player_init(int start_row, int start_column)
{
    player* p = malloc(sizeof(player));
    if (p == NULL) {
        fprintf(stderr, "error: unable to allocate player data\n");
        exit(1);
        return NULL;
    }

    p->row = start_row;
    p->column = start_column;
    p->health = 100; // starting health - make macro later later

    // move to start location
    curse_put(start_row, start_column, PLAYER_SYMBOL, 2);
    return p;
}


void setup_ncurses()
{
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(0);
    // change white to gray
    init_color(COLOR_WHITE, 700, 700, 700);
    init_pair(PLAYER_SYMBOL, COLOR_BLUE, COLOR_GREEN);
    init_pair(STAIR, COLOR_CYAN, COLOR_BLACK);

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair('!', COLOR_BLACK, COLOR_RED);
    init_pair('?', COLOR_BLUE, COLOR_GREEN);
}

void curse_put(int row, int col, char c, int color)
{
    attron(COLOR_PAIR(color));
    mvaddch(row, col, c);
    attroff(COLOR_PAIR(color));
}

void write_map_curse(char* map)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            char c = get_map_char(row, column, map);
            switch (c) {
                case OPEN_SPACE:
                    curse_put(row, column, c, 0);
                    break;
                case '#':
                    curse_put(row, column, c, 4);
                    break;
                case STAIR:
                    curse_put(row, column, c, STAIR);
                    break;
                default:
                    curse_put(row, column, c, 0);
                    break;
            }
        }
    }
}
