/*

+JMJ+

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include "map.h"
#include "items.h"
#include "enemy.h"
#include "player.h"

#define ESCAPE_ASCII 27
#define INVENTORY_SCREEN_COLOR 5
#define INVENTORY_TEXT_OFFSET 6
#define STATS_TEXT_OFFSET 4
#define HIGHLIGHT_TEXT_COLOR 40
#define STEP_HEALTH_LOSS 1
#define MAP_HEIGHT_OFFSET 1
#define MAP_BOTTOM (MAP_HEIGHT + MAP_HEIGHT_OFFSET)

#define MAP_SCREEN 'M'
#define INVENTORY_SCREEN 'I'
#define EQUIPMENT_SCREEN 'E'
#define STATS_SCREEN 'S'

#define ATTACK_KEY 'Z'
#define BLOCK_KEY  'X'
#define UPPER_TO_LOWER_CONV 32

#define DAMAGE_MSG_COLOR 5
// #define

void setup_ncurses();
void write_map_curse(char map[], int item_grid[], int enemy_grid[]);
void curse_put(int row, int col, char c, int color);
void curse_print(int row, int column, const char* message, int color);
void curse_clear_lines(int start_row, int inclusive_end_row, int column);
char handle_map_keypress(player* player_ptr, char key, char map[], int item_grid[],
    int enemy_grid[]);
char handle_walk_key(player* player_ptr, char map[], int row_change, int col_change, 
    int item_grid[], int enemy_grid[]);
void display_inventory(player* p, int inventory_cursor);
void display_stats(player* p);
void calc_player_stats(player* p);

void display_equipment(player* p);
void display_center_box();

void do_combat_sequence(player* p, int enemy_id, int* enemy_grid);

void set_player_spawn(room* rooms[], player* p);
void set_item_spawns(room* rooms[], int item_grid[], char map[]);
void set_enemy_spawns(room* rooms[], int enemy_grid[], char map[]);
void update_enemy_positions(player* p, int enemy_grid[], char map[]);

void equip_item(player* player_ptr, int* equipment_piece, int inventory_cursor, int item_id);
void display_user_info_line(player* p);
void display_player_char(player* p);

void clear_item_grid(int item_grid[]);
void clear_enemy_grid(int enemy_grid[]);


// status line messages
void print_item_pickup(int item_id);
void print_new_floor(int floor);
void print_item_min_level(int item_id);
void print_enemy_combat(int enemy_id);


int main()
{
    srand(time(NULL));
    player* player_ptr = player_init();
    char map[MAP_WIDTH * MAP_HEIGHT];
    room* rooms[ROOM_COUNT];
    int item_grid[MAP_WIDTH * MAP_HEIGHT];
    int enemy_grid[MAP_WIDTH * MAP_HEIGHT];
    clear_item_grid(item_grid);
    clear_enemy_grid(enemy_grid);

    int floor = 1;
    char current_screen = MAP_SCREEN;
    int inventory_cursor = 0;

    rooms_gen(rooms);
    map_gen(rooms, floor, map); 
    set_item_spawns(rooms, item_grid, map);
    set_enemy_spawns(rooms, enemy_grid, map);
    set_player_spawn(rooms, player_ptr);
        
    setup_ncurses();
    write_map_curse(map, item_grid, enemy_grid);
    display_player_char(player_ptr);
    display_user_info_line(player_ptr);
    curse_print(MAP_BOTTOM + 1, 0, "Inventory (E)", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_BOTTOM + 1, 20, "Equipment (T)", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_BOTTOM + 1, 40, "Stats (R)", HIGHLIGHT_TEXT_COLOR);

    char key;
    while ((key = getch()) != 'q') {
        if (key == 'e' || key == 'E') {
            current_screen = (current_screen == INVENTORY_SCREEN) ? MAP_SCREEN : INVENTORY_SCREEN;
        } else if (key == 't' || key == 'T') {
            current_screen = (current_screen == EQUIPMENT_SCREEN) ? MAP_SCREEN : EQUIPMENT_SCREEN;
        }
        else if (key == 'r' || key == 'R') {
            current_screen = (current_screen == STATS_SCREEN) ? MAP_SCREEN : STATS_SCREEN;
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
                    switch (item_data[item_id].type) {
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
        } else if (current_screen == STATS_SCREEN) {
            display_stats(player_ptr);
        }
        else if (current_screen == MAP_SCREEN) {
            switch (handle_map_keypress(player_ptr, key, map, item_grid, enemy_grid)) {
                case 0:
                    break;
                case STAIR:
                    delete_rooms(rooms);

                    rooms_gen(rooms);
                    map_gen(rooms, ++floor, map); 
                    clear_item_grid(item_grid);
                    clear_enemy_grid(enemy_grid);
                    set_item_spawns(rooms, item_grid, map);
                    set_enemy_spawns(rooms, enemy_grid, map);
                    set_player_spawn(rooms, player_ptr);
                    print_new_floor(floor);
                    break;                    
                default:
                    update_enemy_positions(player_ptr, enemy_grid, map);
                    break;
            }
            // update map, enemy, and player location
            write_map_curse(map, item_grid, enemy_grid);
            

            // check if we've entered combat; if so, do combat
            int stepped_on_enemy_id = enemy_grid[player_ptr->row * MAP_WIDTH + player_ptr->column];
            if (stepped_on_enemy_id != NULL_ENEMY_ID)
            {
                print_enemy_combat(stepped_on_enemy_id);
                do_combat_sequence(player_ptr, stepped_on_enemy_id, enemy_grid);
                write_map_curse(map, item_grid, enemy_grid);
            }

            
            display_user_info_line(player_ptr);
            display_player_char(player_ptr);
        }
    }
    player_delete(player_ptr);
    delete_rooms(rooms);
    
    endwin();
    return 0;
}

void print_item_pickup(int item_id)
{
    move(0, 0);
    clrtoeol();
    attron(COLOR_PAIR(5));
    const int offset = 14;
    mvprintw(0, 0, "You picked up ");
    // later, print color based on item rarity
    attroff(COLOR_PAIR(5));
    attron(COLOR_PAIR(6));
    mvprintw(0, offset, "%s", item_data[item_id].name);
    attroff(COLOR_PAIR(6));
}

void print_new_floor(int floor)
{
    move(0, 0);
    clrtoeol();
    attron(COLOR_PAIR(5));
    mvprintw(0, 0, "You entered Floor %d", floor);
    attroff(COLOR_PAIR(5));
}

void print_item_min_level(int item_id)
{
    move(0, 0);
    clrtoeol();
    attron(COLOR_PAIR(5));
    mvprintw(0, 0, "You must be level %d to equip %s", 
        item_data[item_id].min_level, item_data[item_id].name);
    attroff(COLOR_PAIR(5));
}
void print_enemy_combat(int enemy_id)
{
    move(0, 0);
    clrtoeol();
    attron(COLOR_PAIR(5));
    mvprintw(0, 0, "You are battling a %s!", enemy_combat_data[enemy_id].name);
    attroff(COLOR_PAIR(5));
}

void display_user_info_line(player* p)
{
    attron(COLOR_PAIR(1));
    mvprintw(MAP_HEIGHT + MAP_HEIGHT_OFFSET, 0, "HP: %3d    Gold: %3d", p->health, p->gold);
    attroff(COLOR_PAIR(1));
}


void set_player_spawn(room* rooms[], player* p)
{
    room* r = rooms[rand() % ROOM_COUNT];
    int right = r->corner_col + r->width;
    int bottom = r->corner_row + r->height;
    p->row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
    p->column = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
}

void set_enemy_spawns(room* rooms[], int* enemy_grid, char map[])
{
    for (int i = 0; i < ROOM_COUNT; i++) {
        room* r = rooms[i];
        // do we spawn an enemy here ?
        if (rand() % 1 == 0) { // TODO change value
            int right = r->corner_col + r->width;
            int bottom = r->corner_row + r->height;
            int r_row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
            int r_col = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
            if (get_map_char(r_row, r_col, map) != OPEN_SPACE) {
                continue;
            }
            // TODO choose random enemy instead of just goblin
            enemy_grid[r_row * MAP_WIDTH + r_col] = GOBLIN;
        }
    }
}

void update_enemy_positions(player* p, int enemy_grid[], char map[])
{
    // very naive, but will have to do unless we track enemy locations
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            if (enemy_grid[i * MAP_WIDTH + j] != NULL_ENEMY_ID) 
            {
                int col_change = 0;
                int row_change = 0;

                int row_d = i - p->row;
                int col_d = j - p->column;
                int dist_from_player = sqrt(row_d*row_d + col_d*col_d);

                int rand_factor = 4;
                if (dist_from_player < 12)
                {
                    rand_factor = 8;
                }
                int r = rand() % rand_factor;
                switch (r) {
                    case 0:
                        col_change = -1;
                        break;
                    case 1:
                        col_change = 1;
                        break;
                    case 2:
                        row_change = -1;
                        break;
                    case 3:
                        row_change = 1;
                        break;
                    default: // if rand factor != 4, go here
                        if (r % 2 == 0)
                        {
                            row_change = (i > p->row) ? -1 : 1;
                        }
                        else
                        {
                            col_change = (j > p->column) ? -1 : 1;
                        }
                        break;
                }
                int new_row = i + row_change;
                int new_col = j + col_change;

                if (can_step(map, new_row, new_col) 
                    && enemy_grid[new_row * MAP_WIDTH + new_col] == NULL_ENEMY_ID)
                {
                    enemy_grid[new_row * MAP_WIDTH + new_col] = enemy_grid[i * MAP_WIDTH + j];
                    enemy_grid[i * MAP_WIDTH + j] = NULL_ITEM_ID;
                }
            }
        }
    }
}


void set_item_spawns(room* rooms[], int* item_grid, char map[])
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
            int item_id = rand() % (NUM_ITEMS - 1) + 1;
            item_grid[r_row * MAP_WIDTH + r_col] = item_id;
        }
    }   
}

void equip_item(player* player_ptr, int* equipment_piece, int inventory_cursor, int item_id)
{
    if (player_ptr->level < item_data[item_id].min_level)
    {
        print_item_min_level(item_id);
        return;
    }

    if (*(equipment_piece) != NULL_ITEM_ID) {
        player_ptr->inventory->items[inventory_cursor] = *(equipment_piece);
    } else {
        remove_item(player_ptr->inventory, inventory_cursor);
    }
    *(equipment_piece) = item_id;
}

void display_player_char(player* p)
{
    curse_put(p->row + MAP_HEIGHT_OFFSET, p->column, PLAYER_SYMBOL, PLAYER_SYMBOL);
}

char handle_map_keypress(player* player_ptr, char key, char map[], int* item_grid, int* enemy_grid)
{
    switch (key) {
        case 'q':
            break;
        case 'w':
            return handle_walk_key(player_ptr, map, -1, 0, item_grid, enemy_grid);
        case 'a':
            return handle_walk_key(player_ptr, map, 0, -1, item_grid, enemy_grid);
        case 's':
            return handle_walk_key(player_ptr, map, 1, 0, item_grid, enemy_grid);
        case 'd':
            return handle_walk_key(player_ptr, map, 0, 1, item_grid, enemy_grid);
    }
    return 0;
}

// returns status code based on what was stepped on
char handle_walk_key(player* player_ptr, char map[], int row_change, int col_change, 
    int* item_grid, int* enemy_grid)
{
    int new_row = player_ptr->row + row_change;
    int new_col = player_ptr->column + col_change;
    char c = can_step(map, new_row, new_col);
    if (c) {
        // player_ptr->health -= STEP_HEALTH_LOSS * (rand() % 2);
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
                print_item_pickup(step_item_id);
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

void calc_player_stats(player* p)
{
    p->attack = item_data[p->weapon].attack;
    p->defense = item_data[p->helm].defense + 
        item_data[p->breastplate].defense +
        item_data[p->greaves].defense +
        item_data[p->shield].defense;
}

void do_combat_sequence(player* p, int enemy_id, int* enemy_grid)
{
    calc_player_stats(p);
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;

    char* enemy_name = enemy_combat_data[enemy_id].name;
    int enemy_attack = enemy_combat_data[enemy_id].attack;
    int enemy_defense = enemy_combat_data[enemy_id].defense;
    // calc enemy starting HP
    int enemy_hp = ceil(enemy_defense * 2.5);

    display_center_box();
    curse_print(center_row - q_row + 1, center_column - STATS_TEXT_OFFSET,
        "~Combat~", INVENTORY_SCREEN_COLOR);

    attron(COLOR_PAIR(1));
    mvprintw(center_row - q_row + 2, center_column - q_col + 2,
        "%s", enemy_name);
    mvprintw(center_row - q_row + 4, center_column - q_col + 2,
        "Player");

    attroff(COLOR_PAIR(1));

    char* weapon_name;
    if (p->weapon == NULL_ITEM_ID)
    {
        weapon_name = "Fists";
    }
    else
    {
        weapon_name = item_data[p->weapon].name;
    }

    mvprintw(center_row - q_row + 6, center_column - q_col + 2,
        "Weapon: %s", weapon_name);
    
    int bottom_text_offset = 2;
    mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, 
        "Attack (%c)", ATTACK_KEY);
    bottom_text_offset += 11;
    mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, 
        "Block (%c)", BLOCK_KEY);
    bool battle_over = false;
    bool player_win = false;
    char key = '\0';
    int damage = 0;

    while (!battle_over)
    {

        mvprintw(center_row - q_row + 3, center_column - q_col + 2,
            "HP: %d AT: %d DF: %d", enemy_hp, enemy_attack, enemy_defense);
        
        mvprintw(center_row - q_row + 5, center_column - q_col + 2,
            "HP: %d AT: %d DF: %d", p->health, p->attack, p->defense);

        key = getch();
        switch (key)
        {
            // TODO add speed stat
            case ATTACK_KEY:
            case ATTACK_KEY + UPPER_TO_LOWER_CONV:
                // Roll D(Player attack stat) 
                // damage dealt = [random num between 0 and [attack stat]] + [attack stat / 2]
                // damage = (rand() % ((int) (p->attack / 2))) + (int)(p->attack / 2);
                damage = (rand() % (p->attack + 1)) + (int)((p->attack) / 2);
                enemy_hp = max(enemy_hp - damage, 0);
                attron(COLOR_PAIR(DAMAGE_MSG_COLOR));
                mvprintw(center_row - q_row + 7, center_column - q_col + 2,
                    "You dealt %d damage", damage);
                damage = (rand() % (enemy_attack + 1)) + (int)((enemy_attack) / 2);
                p->health = max(p->health - damage, 0);
                mvprintw(center_row - q_row + 8, center_column - q_col + 2,
                    "%s dealt %d damage", enemy_name, damage);
                attroff(COLOR_PAIR(DAMAGE_MSG_COLOR));


                if (enemy_hp <= 0)
                {
                    mvprintw(center_row - q_row + 3, center_column - q_col + 2,
                        "HP: %d ATK: %d DEF: %d", enemy_hp, enemy_attack, enemy_defense);
                    battle_over = true;
                    player_win = true;
                    mvprintw(center_row - q_row + 9, center_column - q_col + 2,
                        "You defeated %s", enemy_name);

                    // do item drop
                }
                break;
            case BLOCK_KEY:
            case BLOCK_KEY + UPPER_TO_LOWER_CONV:
                // blocking enemy attack 
                break;
        }
    }
    if (player_win)
    {
        enemy_grid[p->row * MAP_WIDTH + p->column] = NULL_ENEMY_ID;
    }

    
}

void display_stats(player* p)
{
    calc_player_stats(p);
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    display_center_box();
    curse_print(center_row - q_row + 1, center_column - STATS_TEXT_OFFSET,
        "~Stats~", INVENTORY_SCREEN_COLOR);
    mvprintw(center_row - q_row + 2, center_column - q_col + 2, 
        "LEVEL:    %d", p->level);
    mvprintw(center_row - q_row + 3, center_column - q_col + 2, 
        "ATTACK:   %d", p->attack);
    mvprintw(center_row - q_row + 4, center_column - q_col + 2, 
        "DEFENSE:  %d", p->defense);
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
        "HELM:   %s", item_data[p->helm].name);
    mvprintw(center_row - q_row + 3, center_column - q_col + 2, 
        "CHEST:  %s", item_data[p->breastplate].name);
    mvprintw(center_row - q_row + 4, center_column - q_col + 2, 
        "LEGS:   %s", item_data[p->greaves].name);
    mvprintw(center_row - q_row + 5, center_column - q_col + 2, 
        "WEAPON: %s", item_data[p->weapon].name);
    mvprintw(center_row - q_row + 6, center_column - q_col + 2, 
        "SHIELD: %s", item_data[p->shield].name);
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
    switch (item_data[item_id].type) {
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
                item_data[p->inventory->items[i]].name, HIGHLIGHT_TEXT_COLOR);
        } else {
            mvprintw(r, c, 
                item_data[p->inventory->items[i]].name);
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
    init_pair(ENEMY_SYMBOL, COLOR_WHITE, COLOR_RED);

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

void write_map_curse(char map[], int* item_grid, int* enemy_grid)
{

    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            char c = get_map_char(row, column, map);
            int display_row = row + MAP_HEIGHT_OFFSET;
            switch (c) {
                case OPEN_SPACE:
                    if (enemy_grid[row * MAP_WIDTH + column] != NULL_ENEMY_ID) {
                        curse_put(display_row, column, ENEMY_SYMBOL, ENEMY_SYMBOL);
                    }
                    else if (item_grid[row * MAP_WIDTH + column] != NULL_ITEM_ID) {
                        curse_put(display_row, column, ITEM_SYMBOL, ITEM_SYMBOL);
                    } else {
                        curse_put(display_row, column, OPEN_SPACE, 0);
                    }
                    break;
                case WALL:
                    curse_put(display_row, column, c, 0);
                    break;
                case STAIR:
                    curse_put(display_row, column, c, STAIR);
                    break;
                default:
                    curse_put(display_row, column, c, 0);
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

void clear_item_grid(int item_grid[])
{
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        item_grid[i] = NULL_ITEM_ID;
    }
}

void clear_enemy_grid(int enemy_grid[])
{
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        enemy_grid[i] = NULL_ENEMY_ID;
    }
}
