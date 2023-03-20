/*

+JMJ+

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>

// note - hard-coded dims based on input file
#define MAP_WIDTH 32
#define MAP_HEIGHT 16
#define PLAYER_SYMBOL '@'
#define OPEN_SPACE '.'
#define HALLWAY '#'

typedef struct player
{
    int row;
    int column;
    uint health; // integer from 0 to 100
} player;

char get_map_char(int row, int col, char* map);
char* load_map(char* filename);
void setup_ncurses();
void write_map_curse(char* map);
void curse_put(int row, int col, char c, int color);
void handle_keypress(player* player_ptr, char key, char* map);
void handle_walk_key(player* player_ptr, char* map, int row_change, int col_change);
bool can_step(char* map, int row, int column);
player* player_init(int start_row, int start_col);


int main()
{
    char* map = load_map("test.txt");
    setup_ncurses();
    write_map_curse(map);

    int start_row = 2;
    int start_col = 2;
    player* player_ptr = player_init(start_row, start_col);

    char key = '\0';
    while ((key = getch()) != 'q') {
        handle_keypress(player_ptr, key, map);        
        // update map and player location
        write_map_curse(map);    
        curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, 2);
        // refresh();
    }
    free(map);
    free(player_ptr);
    endwin();
    return 0;
}

void handle_keypress(player* player_ptr, char key, char* map)
{
    char map_char;
    switch (key) {
        case 'q':
            break;
        case 'w':
            handle_walk_key(player_ptr, map, -1, 0);
            break;
        case 'a':
            handle_walk_key(player_ptr, map, 0, -1);
            break;
        case 's':
            handle_walk_key(player_ptr, map, 1, 0);
            break;
        case 'd':
            handle_walk_key(player_ptr, map, 0, 1);
            break;
        }
}

void handle_walk_key(player* player_ptr, char* map, int row_change, int col_change)
{
    int new_row = player_ptr->row + row_change;
    int new_col = player_ptr->column + col_change;
    if (can_step(map, new_row, new_col)) {
        player_ptr->row = new_row;
        player_ptr->column = new_col;
    }
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

bool can_step(char* map, int row, int column)
{
    char map_char = get_map_char(row, column, map);
    return map_char == OPEN_SPACE || map_char == HALLWAY;
}

char get_map_char(int row, int col, char* map)
{
    return map[row * MAP_WIDTH + col];
}

char* load_map(char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "error: unable to read file %s\n", filename);
        return NULL;
    }
    char c;
    int i = 0;
    char* map = malloc(sizeof(char) * MAP_HEIGHT * MAP_WIDTH);
    while ((c = fgetc(fp)) != EOF) {
        if (c != '\n' && ((int)c) != 13) {
            map[i++] = c;
        }        
    }
    fclose(fp);
    return map;
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
    init_pair('#', COLOR_YELLOW, COLOR_BLACK);
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
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
                case '.':
                    curse_put(row, column, c, 0);
                    break;
                default:
                    curse_put(row, column, c, 1);
                    break;
            }
        }
    }
}