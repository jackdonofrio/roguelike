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
#include "player.h"

#define ESCAPE_ASCII 27
#define INVENTORY_SCREEN_COLOR 5
#define INVENTORY_TEXT_OFFSET 6
#define HIGHLIGHT_TEXT_COLOR 40

#define MAP_SCREEN 'M'
#define INVENTORY_SCREEN 'I'
#define EQUIPMENT_SCREEN 'E'
#define STATS_SCREEN 'S'

void setup_ncurses();
void write_map_curse(char* map, int* item_grid);
void curse_put(int row, int col, char c, int color);
void curse_print(int row, int column, const char* message, int color);
void curse_clear_lines(int start_row, int inclusive_end_row, int column);
char handle_map_keypress(player* player_ptr, char key, char* map, int* item_grid);
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change, int* item_grid);
void display_inventory(player* p, int inventory_cursor);
void display_equipment(player* p);
void display_center_box();
void set_player_spawn(room** rooms, player* p);
void set_item_spawns(room** rooms, int* item_grid, char* map);
void equip_item(player* player_ptr, int* equipment_piece, int inventory_cursor, int item_id);


int main()
{
    // full_inventory(NULL);
    srand(time(NULL));
    player* player_ptr = player_init();
    char* map;
    room** rooms;
    int* item_grid;
    int floor = 0;
    char current_screen = MAP_SCREEN;
    int inventory_cursor = 0;

    rooms = rooms_gen();
    map = map_gen(rooms, floor); 
    item_grid = create_item_grid();
    set_item_spawns(rooms, item_grid, map);
    set_player_spawn(rooms, player_ptr);
        
    setup_ncurses();
    write_map_curse(map, item_grid);
    curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
    mvprintw(MAP_HEIGHT, 0, "HP: %d", DEFAULT_MAX_HEALTH);
    curse_print(MAP_HEIGHT + 1, 0, "Inventory (E)", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_HEIGHT + 1, 20, "Equipment (T)", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_HEIGHT + 1, 40, "Stats (R)", HIGHLIGHT_TEXT_COLOR);

    char key;
    while ((key = getch()) != 'q') {
        if (key == 'e' || key == 'E') {
            current_screen = (current_screen == INVENTORY_SCREEN) ? MAP_SCREEN : INVENTORY_SCREEN;
        } else if (key == 't' || key == 'T') {
            current_screen = (current_screen == EQUIPMENT_SCREEN) ? MAP_SCREEN : EQUIPMENT_SCREEN;
        }
        if (current_screen == INVENTORY_SCREEN) {
            int item_id = player_ptr->inventory->items[inventory_cursor];
            switch (key) {
                case 'w': case 'W':
                    inventory_cursor = max(0, inventory_cursor - 1);
                    break;
                case 's': case 'S':
                    inventory_cursor = min(player_ptr->inventory->current_size - 1, inventory_cursor + 1);
                    break;
                case 'z': case 'Z':
                    if (item_grid[player_ptr->row * MAP_WIDTH + player_ptr->column] == NULL_ITEM_ID) {
                        remove_item(player_ptr->inventory, inventory_cursor);
                        item_grid[player_ptr->row * MAP_WIDTH + player_ptr->column] = item_id;
                        inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                    }
                    break;
                case 'f': case 'F':
                    switch (item_use_table[item_id]) {
                        case FOOD:
                            remove_item(player_ptr->inventory, inventory_cursor);
                            int health_boost = calc_food_hp_boost(item_id);
                            player_ptr->health = min(DEFAULT_MAX_HEALTH, player_ptr->health + health_boost);
                            // so the cursor doesn't go out of bounds, we do the following
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                        case HELM:
                            equip_item(player_ptr, &(player_ptr->helm), inventory_cursor, item_id);
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                        case BREASTPLATE:
                            equip_item(player_ptr, &(player_ptr->breastplate), inventory_cursor, item_id);
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                        case GREAVES:
                            equip_item(player_ptr, &(player_ptr->greaves), inventory_cursor, item_id);
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                        case WEAPON:
                            equip_item(player_ptr, &(player_ptr->weapon), inventory_cursor, item_id);
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                        case SHIELD:
                            equip_item(player_ptr, &(player_ptr->shield), inventory_cursor, item_id);
                            inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                            break;
                    }
                    break;
            }
            display_inventory(player_ptr, inventory_cursor);
        } else if (current_screen == EQUIPMENT_SCREEN) {
            display_equipment(player_ptr);
        }
        else if (current_screen == MAP_SCREEN) {
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
        // curse_print(MAP_HEIGHT, 0, "Inventory (E)", HIGHLIGHT_TEXT_COLOR);
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
        }
    }   
}

void equip_item(player* player_ptr, int* equipment_piece, int inventory_cursor, int item_id)
{
    if (*(equipment_piece) != NULL_ITEM_ID) {
        player_ptr->inventory->items[inventory_cursor] = *(equipment_piece);
    } else {
        remove_item(player_ptr->inventory, inventory_cursor);
    }
    *(equipment_piece) = item_id;
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
        default:
            return 0;
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

void display_center_box()
{
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    int r, c;
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
}


void display_equipment(player* p)
{
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    display_center_box();
    curse_print(center_row - q_row + 1, center_column - INVENTORY_TEXT_OFFSET,
        "~Equipment~", INVENTORY_SCREEN_COLOR);
    // display equipment    
    mvprintw(center_row - q_row + 2, center_column - q_col + 2, 
        "HELM:   %s", common_item_names[p->helm]);
    mvprintw(center_row - q_row + 3, center_column - q_col + 2, 
        "CHEST:  %s", common_item_names[p->breastplate]);
    mvprintw(center_row - q_row + 4, center_column - q_col + 2, 
        "LEGS:   %s", common_item_names[p->greaves]);
    mvprintw(center_row - q_row + 5, center_column - q_col + 2, 
        "WEAPON: %s", common_item_names[p->weapon]);
    mvprintw(center_row - q_row + 6, center_column - q_col + 2, 
        "SHIELD: %s", common_item_names[p->shield]);
}

void display_inventory(player* p, int inventory_cursor)
{
    // go to center
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    int r, c;
    
    display_center_box();
    curse_print(center_row - q_row + 1, center_column - INVENTORY_TEXT_OFFSET,
        "~Inventory~", INVENTORY_SCREEN_COLOR);

    int bottom_text_offset = 2;
    int item_id = p->inventory->items[inventory_cursor];
    switch (item_use_table[item_id]) {
        case FOOD:
            mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Eat (F)");
            bottom_text_offset += 8;
            break;
        case HELM:
        case BREASTPLATE:
        case GREAVES:
        case WEAPON:
        case SHIELD:
            mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Equip (F)");
            bottom_text_offset += 10;
            break;
    }
    if (p->inventory->current_size > 0) {
        mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Drop (Z)");
    }


    // display items
    c = center_column - q_col + 2;
    for (int i = 0; i < p->inventory->current_size; i++) {
        r = center_row - q_row + i + 2;
        if (i == inventory_cursor) {
            curse_print(r, c, 
                common_item_names[p->inventory->items[i]], HIGHLIGHT_TEXT_COLOR);
        } else {
            mvprintw(r, c, 
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