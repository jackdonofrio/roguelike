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
#include "items.h"

#define ESCAPE_ASCII 27
#define INVENTORY_SCREEN_COLOR 5
#define INVENTORY_TEXT_OFFSET 5
#define HIGHLIGHT_TEXT_COLOR 40

typedef struct player
{
    int row;
    int column;
    uint health; // integer from 0 to 100
    inventory_t* inventory;
} player;

void setup_ncurses();
void write_map_curse(char* map, int* item_grid);
void curse_put(int row, int col, char c, int color);
void curse_print(int row, int column, const char* message, int color);
void curse_clear_lines(int start_row, int inclusive_end_row, int column);
char handle_map_keypress(player* player_ptr, char key, char* map, int* item_grid);
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change, int* item_grid);
player* player_init(int start_row, int start_col);
void player_delete(player* p);
void display_inventory(player* p, int inventory_cursor);
void set_player_spawn(room** rooms, player* p);
void set_item_spawns(room** rooms, int* item_grid, char* map);
int* create_item_grid();
void delete_item_grid(int* item_grid);


int main()
{
    // full_inventory(NULL);
    srand(time(NULL));
    player* player_ptr = player_init(2, 2);
    char* map;
    room** rooms;
    int* item_grid;
    int floor = 0;
    bool in_inventory_screen = false;
    int inventory_cursor = 0;

    rooms = rooms_gen();
    map = map_gen(rooms, floor); 
    item_grid = create_item_grid();
    set_item_spawns(rooms, item_grid, map);
    set_player_spawn(rooms, player_ptr);
        
    setup_ncurses();
    write_map_curse(map, item_grid);
    curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
    
   

    char key;
    while ((key = getch()) != 'q') {
        if (key == 'e') {
            in_inventory_screen = !in_inventory_screen;
        } 
        if (in_inventory_screen) {
            switch (key) {
                case 'w': case 'W':
                    inventory_cursor = max(0, inventory_cursor - 1);
                    break;
                case 's': case 'S':
                    inventory_cursor = min(player_ptr->inventory->current_size - 1, inventory_cursor + 1);
                    break;
            }
            display_inventory(player_ptr, inventory_cursor);
        }
        else {

            switch (handle_map_keypress(player_ptr, key, map, item_grid)) {
                case STAIR:
                    delete_rooms(rooms);
                    delete_item_grid(item_grid);
                    free(map);

                    rooms = rooms_gen();
                    map = map_gen(rooms, ++floor); 
                    item_grid = create_item_grid();
                    set_item_spawns(rooms, item_grid, map);
                    set_player_spawn(rooms, player_ptr);
                    break;
            }
            // update map and player location
            write_map_curse(map, item_grid);
            curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
        }
    }
    free(map);
    player_delete(player_ptr);
    delete_item_grid(item_grid);
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
    // if (get_map_char(p->row, p->column, map) != OPEN_SPACE) {
        
    // }

}

void delete_item_grid(int* item_grid)
{
    free(item_grid);
}

int* create_item_grid()
{
    int* item_grid = malloc(sizeof(int) * MAP_HEIGHT * MAP_WIDTH);
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        item_grid[i] = NULL_ITEM_ID;
    }
    return item_grid;
}

void set_item_spawns(room** rooms, int* item_grid, char* map)
{
    for (int i = 0; i < ROOM_COUNT; i++) {
        room* r = rooms[i];
        // do we drop an item here ?
        if (rand() % DROP_RATE_PER_ROOM == 0) {
            // pick position
            int right = r->corner_col + r->width;
            int bottom = r->corner_row + r->height;
            int r_row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
            int r_col = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
            if (get_map_char(r_row, r_col, map) != OPEN_SPACE) {
                continue;
            }
            // pick item
            int item_id = rand() % (MAX_ITEM_ID - 1) + 1;
            item_grid[r_row * MAP_WIDTH + r_col] = item_id;
            // set_map_char(r_row, r_col, map, ITEM_SYMBOL);
        }
    }   
}

char handle_map_keypress(player* player_ptr, char key, char* map, int* item_grid)
{
    char map_char;
    switch (key) {
        case 'q':
            break;
        case 'w':
            return handle_walk_key(player_ptr, map, -1, 0, item_grid);
        case 'a':
            return handle_walk_key(player_ptr, map, 0, -1, item_grid);
        case 's':
            return handle_walk_key(player_ptr, map, 1, 0, item_grid);
        case 'd':
            return handle_walk_key(player_ptr, map, 0, 1, item_grid);
        }
}

// returns status code based on what was stepped on
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change, int* item_grid)
{
    int new_row = player_ptr->row + row_change;
    int new_col = player_ptr->column + col_change;
    char c = can_step(map, new_row, new_col);
    if (c) {
        player_ptr->row = new_row;
        player_ptr->column = new_col;

        // did we step on an item?
        int step_item_id = item_grid[new_row * MAP_WIDTH + new_col];
        if (step_item_id != NULL_ITEM_ID) {
            // manage it
            // is the player's inventory full ?
            if (!full_inventory(player_ptr->inventory)) {
                // if not, pick up item
                // ...
                add_item(player_ptr->inventory, step_item_id);
                // ...
                item_grid[new_row * MAP_WIDTH + new_col] = NULL_ITEM_ID;
            }
        }

        return c;
    }
    return 0;
}

void display_inventory(player* p, int inventory_cursor)
{
    // go to center
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    int r, c;

    // display inventory menu frame
    curse_put(center_row - q_row, center_column - q_col, '+', INVENTORY_SCREEN_COLOR);
    for (c = center_column - q_col + 1; c < center_column + q_col - 1; c++) {
        curse_put(center_row - q_row, c, '-', INVENTORY_SCREEN_COLOR);
    }
    curse_put(center_row - q_row, center_column + q_col - 1, '+', INVENTORY_SCREEN_COLOR);
    for (r = center_row - q_row + 1; r < center_row + q_row; r++) {
        curse_put(r, center_column - q_col, '|', INVENTORY_SCREEN_COLOR);
        for (c = center_column - q_col + 1; c < center_column + q_col; c++) {
            mvaddch(r, c, ' ');
        }
        curse_put(r, c - 1, '|', INVENTORY_SCREEN_COLOR);
    }
    for (c = center_column - q_col + 1; c < center_column + q_col - 1; c++) {
        curse_put(center_row + q_row, c, '-', INVENTORY_SCREEN_COLOR);
    }
    curse_put(center_row + q_row, center_column - q_col, '+', INVENTORY_SCREEN_COLOR);
    curse_put(center_row + q_row, center_column + q_col - 1, '+', INVENTORY_SCREEN_COLOR);

    mvprintw(center_row - q_row + 1, center_column - INVENTORY_TEXT_OFFSET, "Inventory");


    // display items
    for (int i = 0; i < p->inventory->current_size; i++) {
        if (i == inventory_cursor) {
            curse_print(center_row - q_row + i + 2, center_column - q_col + 1, 
                common_item_names[p->inventory->items[i]], HIGHLIGHT_TEXT_COLOR);
        } else {
            mvprintw(center_row - q_row + i + 2, center_column - q_col + 1, 
                common_item_names[p->inventory->items[i]]);
        }
    }
}

void curse_print(int row, int column, const char* message, int color)
{
    attron(COLOR_PAIR(color));
    mvprintw(row, column, message);
    attroff(COLOR_PAIR(color));
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
    p->inventory = malloc(sizeof(inventory_t));
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        p->inventory->items[i] = NULL_ITEM_ID;
    }
    p->inventory->current_size = 0;

    // move to start location
    curse_put(start_row, start_column, PLAYER_SYMBOL, 2);
    return p;
}

void player_delete(player* p)
{
    if (p == NULL) {
        return;
    }
    free(p->inventory);
    free(p);
}


void setup_ncurses()
{
    initscr();
    cbreak();
    noecho();
    start_color();
    curs_set(0);
    // change white to gray
    // init_color(COLOR_WHITE, 700, 700, 700);
    init_pair(PLAYER_SYMBOL, COLOR_BLUE, COLOR_GREEN);
    init_pair(STAIR, COLOR_BLACK, COLOR_CYAN);
    init_pair(ITEM_SYMBOL, COLOR_GREEN, COLOR_BLACK);

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair('!', COLOR_BLACK, COLOR_RED);
    init_pair(HIGHLIGHT_TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);
    // init_pair('?', COLOR_BLUE, COLOR_GREEN);
}

void curse_put(int row, int col, char c, int color)
{
    attron(COLOR_PAIR(color));
    mvaddch(row, col, c);
    attroff(COLOR_PAIR(color));
}

void write_map_curse(char* map, int* item_grid)
{
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            char c = get_map_char(row, column, map);
            switch (c) {
                case OPEN_SPACE:
                    if (item_grid[row * MAP_WIDTH + column] != NULL_ITEM_ID) {
                        curse_put(row, column, ITEM_SYMBOL, ITEM_SYMBOL);
                    } else {
                        curse_put(row, column, OPEN_SPACE, 0);
                    }
                    break;
                case WALL:
                    curse_put(row, column, c, 0);
                    break;
                case STAIR:
                    curse_put(row, column, c, STAIR);
                    break;
                default:
                    curse_put(row, column, WALL, 0);
                    break;
            }
        }
    }
}

void curse_clear_lines(int start_row, int inclusive_end_row, int column)
{
    for (int line = start_row; line <= inclusive_end_row; line++) {
        move(line, column);
        clrtoeol();
    }
}