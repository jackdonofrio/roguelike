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

#define     WHITE_TEXT_COLOR  0
#define     CYAN_TEXT_COLOR   1
#define     GREEN_TEXT_COLOR  2
#define     RED_TEXT_COLOR    3
#define     BLUE_TEXT_COLOR   4
#define    YELLOW_TEXT_COLOR  5
#define   MAGENTA_TEXT_COLOR  6
#define HIGHLIGHT_TEXT_COLOR 40


#define ESCAPE_ASCII 27
#define INVENTORY_SCREEN_COLOR 5
#define INVENTORY_TEXT_OFFSET 6
#define STATS_TEXT_OFFSET 4
#define STEP_HEALTH_LOSS 1
#define MAP_HEIGHT_OFFSET 1
#define MAP_BOTTOM (MAP_HEIGHT + MAP_HEIGHT_OFFSET)

#define MAP_SCREEN 'M'
#define INVENTORY_SCREEN 'I'
#define EQUIPMENT_SCREEN 'E'
#define STATS_SCREEN 'S'

#define ATTACK_KEY 'Z'
#define BLOCK_KEY  'X'

#define ATTACK_MOVE 0
#define BLOCK_MOVE  1

#define UPPER_TO_LOWER_CONV 32

#define DAMAGE_MSG_COLOR 5
// #define

void setup_ncurses();
void curse_put(int row, int col, char c, int color);
void curse_print(int row, int column, const char* message, int color);
void curse_clear_lines(int start_row, int inclusive_end_row, int column);
char handle_map_keypress(player* player_ptr, char key, char map[], int item_grid[],
    int enemy_grid[]);
char handle_walk_key(player* player_ptr, char map[], int row_change, int col_change, 
    int item_grid[], int enemy_grid[]);

void display_inventory(player* p, int inventory_cursor);
void do_inventory_event(player* p, int* inventory_cursor_ptr, char key, int* item_grid);

void display_stats(player* p);
void calc_player_stats(player* p);

void display_equipment(player* p);
void display_center_box();

void write_map_curse(char map[], int item_grid[], int enemy_grid[], bool visibility_grid[]);
// void do_map_screen_event(char map[], )

void do_combat_sequence(player* p, int enemy_id, int* enemy_grid);
int calc_damage_done(int attacker_attack, int defender_defense);


void set_player_spawn(room* rooms[], player* p);
void set_item_spawns(room* rooms[], int item_grid[], char map[]);
void set_enemy_spawns(room* rooms[], int enemy_grid[], char map[], int floor, int item_grid[]);
int pick_enemy(int floor);
void update_enemy_positions(player* p, int enemy_grid[], char map[]);
void update_visiblity_grid(bool visibility_grid[], char* map, player* p);

void equip_item(player* player_ptr, int* equipment_piece, int inventory_cursor, int item_id);
void display_user_info_line(player* p);
void display_player_char(player* p);

void clear_item_grid(int item_grid[]);
void clear_enemy_grid(int enemy_grid[]);
void clear_visibility_grid(bool visibility_grid[]);

// status line messages
void clear_status_line();
void print_item_pickup(int item_id);
void print_new_floor(int floor);
void print_item_min_level(int item_id);
void print_enemy_combat(int enemy_id);


void save_game_state(char* map, player* p, int item_grid[], int enemy_grid[], bool visibility_grid[],
    int* floor_level);
void load_game_state(char* map, player* p, int item_grid[], int enemy_grid[], bool visibility_grid[],
    int* floor_level);
bool save_state_exists();

bool choose_save_menu();

bool save_state_exists()
{
    FILE* file = fopen("savegame.dat", "rb");
    bool exists = file != NULL;
    fclose(file);
    return exists;
}

// TODO create game_state struct which stores all this info
void save_game_state(char* map, player* p, int item_grid[], int enemy_grid[], bool visibility_grid[],
    int* floor_level)
{
    if (p == NULL)
    {
        printf("err: save_game_state null player ptr\n");
        return;
    }
    if (p->inventory == NULL)
    {
        printf("err: save_game_state null player inventory ptr\n");
        return;
    }
    printf("SAVING GAME HERE\n");


    FILE* file = fopen("savegame.dat", "wb");
    if (file != NULL)
    {
        fwrite(map, sizeof(char), MAP_WIDTH * MAP_HEIGHT, file);
        fwrite(p, sizeof(player), 1, file);
        fwrite(p->inventory, sizeof(inventory_t), 1, file);
        fwrite(item_grid, sizeof(int), MAP_WIDTH * MAP_HEIGHT, file);
        fwrite(enemy_grid, sizeof(int), MAP_WIDTH * MAP_HEIGHT, file);
        fwrite(visibility_grid, sizeof(bool), MAP_WIDTH * MAP_HEIGHT, file);
        fwrite(floor_level, sizeof(int), 1, file);
        fclose(file);
    }
    printf("DONE SAVING GME\n");
}

void load_game_state(char* map, player* p, int item_grid[], int enemy_grid[], bool visibility_grid[],
    int* floor_level)
{
    FILE* file = fopen("savegame.dat", "rb");
    if (file != NULL)
    {
        fread(map, sizeof(char), MAP_WIDTH * MAP_HEIGHT, file);
        fread(p, sizeof(player), 1, file);
        fread(p->inventory, sizeof(inventory_t), 1, file);
        fread(item_grid, sizeof(int), MAP_WIDTH * MAP_HEIGHT, file);
        fread(enemy_grid, sizeof(int), MAP_WIDTH * MAP_HEIGHT, file);
        fread(visibility_grid, sizeof(bool), MAP_WIDTH * MAP_HEIGHT, file);
        fread(floor_level, sizeof(int), 1, file);
        fclose(file);
    }
}


bool choose_save_menu()
{
    // create larger white box around main menu
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        int display_row = i + MAP_HEIGHT_OFFSET;
        curse_put(display_row, 0, WALL, WHITE_TEXT_COLOR);
        curse_put(display_row, MAP_WIDTH - 1, WALL, WHITE_TEXT_COLOR);
    }

    for (int j = 0; j < MAP_WIDTH; j++)
    {
        int display_col = j;
        curse_put(MAP_HEIGHT_OFFSET, display_col, WALL, WHITE_TEXT_COLOR);
        curse_put(MAP_HEIGHT_OFFSET + MAP_HEIGHT - 1, display_col, WALL, WHITE_TEXT_COLOR);
    }

    const int bottom_text_offset = 2;

    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;

    display_center_box(WHITE_TEXT_COLOR);
    curse_print(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Select [X]\t Quit [Q]",
        WHITE_TEXT_COLOR);

    char key;
    bool made_choice = false;
    bool new_save_selected = false;
    bool chose_load_from_save;
    while (!made_choice)
    {
        curse_print(center_row - q_row + 6, center_column - q_col + 7, "Load save game", 
            new_save_selected ? WHITE_TEXT_COLOR : HIGHLIGHT_TEXT_COLOR);
        curse_print(center_row - q_row + 7, center_column - q_col + 7, "New save game", 
            new_save_selected ? HIGHLIGHT_TEXT_COLOR : WHITE_TEXT_COLOR);

        key = getch();
        switch (key)
        {
            case 'w':
            case 'W':
                new_save_selected = false;
                break;
            case 's':
            case 'S':
                new_save_selected = true;
                break;
            case 'x':
            case 'X':
                made_choice = true;
                chose_load_from_save = !new_save_selected;
                return chose_load_from_save;
                break;
            case 'q':
            case 'Q':
                endwin();
                exit(1);
                break;
        }

    }
    return false;
}


int main()
{
    srand(time(NULL));
    player* player_ptr = player_init();
    char map[MAP_WIDTH * MAP_HEIGHT];
    room* rooms[ROOM_COUNT];
    int item_grid[MAP_WIDTH * MAP_HEIGHT];
    int enemy_grid[MAP_WIDTH * MAP_HEIGHT];
    bool visibility_grid[MAP_WIDTH * MAP_HEIGHT];
    clear_item_grid(item_grid);
    clear_enemy_grid(enemy_grid);
    clear_visibility_grid(visibility_grid);

    setup_ncurses();

    int floor = 1;
    char current_screen = MAP_SCREEN;
    int inventory_cursor = 0;
    char key;
    
    bool chose_load_from_save = choose_save_menu();
    bool loaded_from_save_file = chose_load_from_save && save_state_exists();
    if (loaded_from_save_file)
    {
        load_game_state(map, player_ptr, item_grid, enemy_grid, visibility_grid, &floor);
        print_new_floor(floor);
    }
    else 
    {
        rooms_gen(rooms);
        map_gen(rooms, floor, map); 
        set_item_spawns(rooms, item_grid, map);
        set_enemy_spawns(rooms, enemy_grid, map, floor, item_grid);
        set_player_spawn(rooms, player_ptr);
        update_visiblity_grid(visibility_grid, map, player_ptr);
    }
    int spawned_floor = floor;
    
    write_map_curse(map, item_grid, enemy_grid, visibility_grid);
    display_player_char(player_ptr);
    display_user_info_line(player_ptr);
    curse_print(MAP_BOTTOM + 1, 0, "Inventory [E]", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_BOTTOM + 1, 20, "Equipment [T]", HIGHLIGHT_TEXT_COLOR);
    curse_print(MAP_BOTTOM + 1, 40, "Stats [R]", HIGHLIGHT_TEXT_COLOR);

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
            do_inventory_event(player_ptr, &inventory_cursor, key, item_grid);
        } else if (current_screen == EQUIPMENT_SCREEN) {
            display_equipment(player_ptr);
        } else if (current_screen == STATS_SCREEN) {
            display_stats(player_ptr);
        }
        else if (current_screen == MAP_SCREEN) {
            // TODO modularize
            switch (handle_map_keypress(player_ptr, key, map, item_grid, enemy_grid)) {
                case 0:
                    break;
                case STAIR:

                    if (! (floor == spawned_floor && loaded_from_save_file) )
                    {
                        delete_rooms(rooms);
                    }

                    rooms_gen(rooms);
                    map_gen(rooms, ++floor, map); 
                    clear_item_grid(item_grid);
                    clear_enemy_grid(enemy_grid);
                    clear_visibility_grid(visibility_grid);

                    // if new floor is divisible by 5, spawn a merchant
                    // TODO

                    set_item_spawns(rooms, item_grid, map);
                    set_enemy_spawns(rooms, enemy_grid, map, floor, item_grid);
                    set_player_spawn(rooms, player_ptr);
                    update_visiblity_grid(visibility_grid, map, player_ptr);
                    print_new_floor(floor);
                    break;                    
                default:
                    update_enemy_positions(player_ptr, enemy_grid, map);
                    update_visiblity_grid(visibility_grid, map, player_ptr);
                    break;
            }
            // update map, enemy, and player location
            write_map_curse(map, item_grid, enemy_grid, visibility_grid);
            

            // check if we've entered combat; if so, do combat
            int stepped_on_enemy_id = enemy_grid[player_ptr->row * MAP_WIDTH + player_ptr->column];
            if (stepped_on_enemy_id != NULL_ENEMY_ID)
            {
                print_enemy_combat(stepped_on_enemy_id);
                do_combat_sequence(player_ptr, stepped_on_enemy_id, enemy_grid);
                write_map_curse(map, item_grid, enemy_grid, visibility_grid);
            }

            
            display_user_info_line(player_ptr);
            display_player_char(player_ptr);
        }
    }
    save_game_state(map, player_ptr, item_grid, enemy_grid, visibility_grid, &floor);

    if (!loaded_from_save_file)
    {
        player_delete(player_ptr);
        delete_rooms(rooms);
    }
    
    
    endwin();
    return 0;
}

void clear_status_line()
{
    move(0, 0);
    clrtoeol();   
}

void print_item_pickup(int item_id)
{
    clear_status_line();
    attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
    const int offset = 14;
    mvprintw(0, 0, "You picked up ");
    // later, print color based on item rarity
    attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
    attron(COLOR_PAIR(MAGENTA_TEXT_COLOR));
    mvprintw(0, offset, "%s", item_data[item_id].name);
    attroff(COLOR_PAIR(MAGENTA_TEXT_COLOR));
}

void print_new_floor(int floor)
{
    clear_status_line();
    attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
    mvprintw(0, 0, "You entered Floor %d.", floor);
    attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
}

void print_item_min_level(int item_id)
{
    clear_status_line();
    attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
    mvprintw(0, 0, "You must be level %d to equip %s", 
        item_data[item_id].min_level, item_data[item_id].name);
    attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
}
void print_enemy_combat(int enemy_id)
{
    clear_status_line();
    attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
    mvprintw(0, 0, "You are battling a %s!", enemy_combat_data[enemy_id].name);
    attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
}

void display_user_info_line(player* p)
{
    attron(COLOR_PAIR(CYAN_TEXT_COLOR));
    mvprintw(MAP_HEIGHT + MAP_HEIGHT_OFFSET, 0, "HP: %3d    Gold: %3d    Level: %d (%d / %d exp)", p->health, p->gold,
        p->level, p->exp, (int) pow(2, p->level + 1) );
    attroff(COLOR_PAIR(CYAN_TEXT_COLOR));
}


void set_player_spawn(room* rooms[], player* p)
{
    room* r = rooms[rand() % ROOM_COUNT];
    int right = r->corner_col + r->width;
    int bottom = r->corner_row + r->height;
    p->row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
    p->column = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
}

int pick_enemy(int floor)
{
    // TODO - choose based on floor difficulty
    if (floor < 5)
    {
        return GOBLIN;
    }
    return 1 + (rand() % (NUM_ENEMIES - 1));
}

void set_enemy_spawns(room* rooms[], int* enemy_grid, char map[], int floor, int* item_grid)
{
    for (int i = 0; i < ROOM_COUNT; i++) {
        room* r = rooms[i];
        // do we spawn an enemy here ?
        if (rand() % ENEMY_SPAWN_RATE == 0) {
            int right = r->corner_col + r->width;
            int bottom = r->corner_row + r->height;
            int r_row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
            int r_col = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
            if (get_map_char(r_row, r_col, map) != OPEN_SPACE
                || item_grid[r_row * MAP_WIDTH + r_col] != NULL_ITEM_ID) {
                continue;
            }
            enemy_grid[r_row * MAP_WIDTH + r_col] = pick_enemy(floor);
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
                if (dist_from_player < 7)
                {
                    rand_factor = 16;
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
                    enemy_grid[i * MAP_WIDTH + j] = NULL_ENEMY_ID;

                    // clear_status_line();
                    // attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
                    // mvprintw(0, 0, "r delta: %d -> %d || c delta: %d -> %d ", i, new_row, j, new_col );
                    // attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));

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

void display_center_box(int box_color)
{
    int center_row = MAP_HEIGHT / 2;
    int q_row = center_row / 2;
    int center_column = MAP_WIDTH / 2;
    int q_col = center_column / 2;
    int r, c;
    curse_put(center_row - q_row, center_column - q_col, '+', box_color);
    for (c = center_column - q_col + 1; c < center_column + q_col - 1; c++) {
        curse_put(center_row - q_row, c, '-', box_color);
    }
    curse_put(center_row - q_row, center_column + q_col - 1, '+', box_color);
    for (r = center_row - q_row + 1; r < center_row + q_row; r++) {
        curse_put(r, center_column - q_col, '|', box_color);
        for (c = center_column - q_col + 1; c < center_column + q_col; c++) {
            mvaddch(r, c, ' ');
        }
        curse_put(r, c - 1, '|', box_color);
    }
    for (c = center_column - q_col + 1; c < center_column + q_col - 1; c++) {
        curse_put(center_row + q_row, c, '-', box_color);
    }
    curse_put(center_row + q_row, center_column - q_col, '+', box_color);
    curse_put(center_row + q_row, center_column + q_col - 1, '+', box_color);
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

    display_center_box(INVENTORY_SCREEN_COLOR);
    curse_print(center_row - q_row + 1, center_column - STATS_TEXT_OFFSET,
        "~Combat~", INVENTORY_SCREEN_COLOR);

    attron(COLOR_PAIR(CYAN_TEXT_COLOR));
    mvprintw(center_row - q_row + 2, center_column - q_col + 2,
        "%s", enemy_name);
    mvprintw(center_row - q_row + 4, center_column - q_col + 2,
        "Player");
    attroff(COLOR_PAIR(CYAN_TEXT_COLOR));

    char* weapon_name = (p->weapon == NULL_ITEM_ID) ?  "Fists"
        : item_data[p->weapon].name;

    mvprintw(center_row - q_row + 6, center_column - q_col + 2,
        "Weapon: %s", weapon_name);
    
    int bottom_text_offset = 2;
    mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, 
        "Attack [%c]", ATTACK_KEY);
    bottom_text_offset += 11;
    mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, 
        "Block [%c]", BLOCK_KEY);
    bool battle_over = false;
    bool player_win = false;
    char player_move_key = '\0';
    int player_damage = 0;
    int enemy_damage  = 0;

    // amounts to reduce damage
    int player_block_amount  = 0; 
    int enemy_block_amount   = 0;

    int enemy_move;
    int player_move;

    while (!battle_over)
    {

        mvprintw(center_row - q_row + 3, center_column - q_col + 2,
            "HP: %d AT: %d DF: %d", enemy_hp, enemy_attack, enemy_defense);
        
        mvprintw(center_row - q_row + 5, center_column - q_col + 2,
            "HP: %d AT: %d DF: %d", p->health, p->attack, p->defense);




        enemy_move = rand() % 2;
        player_move_key = getch();

        // TODO - separate out into func
        switch (player_move_key)
        {
            case ATTACK_KEY: 
            case ATTACK_KEY + UPPER_TO_LOWER_CONV:
                player_move = ATTACK_MOVE;
                break;
            case BLOCK_KEY: 
            case BLOCK_KEY + UPPER_TO_LOWER_CONV:
                player_move = BLOCK_MOVE;
                break;
            default:
                continue;
                break;
        }


        // TODO - separate into modular funcs
        // Damage dealt = [rnd num btwn 0 and [atk stat]] + [atk stat / 2]
        //                - [rnd num between 0 and [opp. def stat / 3] ]

        // i max defense / 3 with 1 to avoid div by 0 errors
        // i max the whole thing with 0 to avoid 'negative' dmg

        // Effect of blocking - subtract anywhere from 0 
        // to 1/4 of (damage dealt + blocker defense)
        // from total dmg dealt

        player_damage = calc_damage_done(p->attack, enemy_defense);
        enemy_damage  = calc_damage_done(enemy_attack, p->defense);


        player_block_amount = rand() % max(( (enemy_damage + p->defense)     / 4), 1);
        enemy_block_amount  = rand() % max(( (player_damage + enemy_defense) / 4), 1);

        // set damages dealt based on each's moves

        player_damage = 
            (player_move == BLOCK_MOVE) ? 0 :
                ((enemy_move == BLOCK_MOVE) ? max(player_damage - enemy_block_amount, 0)
                                            : player_damage);

        enemy_damage = 
            (enemy_move == BLOCK_MOVE) ? 0 :
                ((player_move == BLOCK_MOVE) ? max(enemy_damage - player_block_amount, 0)
                                            : enemy_damage);

        
        // TODO - add speed stat to decide who goes first. for now,
        // player always goes first
        attron(COLOR_PAIR(DAMAGE_MSG_COLOR));
        switch (player_move)
        {
            case ATTACK_MOVE:
                enemy_hp = max(enemy_hp - player_damage, 0);

                mvprintw(center_row - q_row + 7, center_column - q_col + 2,
                    "You dealt %d damage", player_damage);
                
                break;
            case BLOCK_MOVE:
                mvprintw(center_row - q_row + 7, center_column - q_col + 2,
                    "You blocked         ");
                break;
            default:
                break;
        }

        switch(enemy_move)
        {
            case ATTACK_MOVE:
                p->health = max(p->health - enemy_damage, 0);
                mvprintw(center_row - q_row + 8, center_column - q_col + 2,
                    "%s dealt %d damage", enemy_name, enemy_damage);
                break;
            case BLOCK_MOVE:
                mvprintw(center_row - q_row + 8, center_column - q_col + 2,
                    "%s blocked        ", enemy_name);
                break;
            default:
                break;
        }
        attroff(COLOR_PAIR(DAMAGE_MSG_COLOR));


        // TODO add player defeat


        if (enemy_hp <= 0)
        {
            mvprintw(center_row - q_row + 3, center_column - q_col + 2,
                "HP: %d ATK: %d DEF: %d", enemy_hp, enemy_attack, enemy_defense);
            battle_over = true;
            player_win = true;
            mvprintw(center_row - q_row + 9, center_column - q_col + 2,
                "You defeated %s", enemy_name);
            int exp_gained = rand() % max(1, (enemy_attack + enemy_defense) / 2) 
                + (enemy_attack + enemy_defense) / 2;
            p->exp += exp_gained;
            if (p->exp >= (int)(pow(2, p->level + 1)))
            {
                p->exp -= (int)(pow(2, p->level + 1));
                p->level += 1;
                p->health += 1;
                p->max_health += 1;
            }
            int gold_dropped = rand() % max(1, (enemy_attack + enemy_defense) / 2) 
                + (enemy_attack + enemy_defense) / 3;
            p->gold += gold_dropped;
            clear_status_line();
            attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
            mvprintw(0, 0, "You defeated the %s! Gained %d exp and %d gold.", enemy_combat_data[enemy_id].name,
                exp_gained, gold_dropped);
            attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
            // do item drop
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
    display_center_box(INVENTORY_SCREEN_COLOR);
    curse_print(center_row - q_row + 1, center_column - STATS_TEXT_OFFSET,
        "~Stats~", INVENTORY_SCREEN_COLOR);
    mvprintw(center_row - q_row + 2, center_column - q_col + 2, 
        "LEVEL:    %d (%d / %d exp) ", p->level, p->exp, (int) pow(2, p->level + 1));
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
    display_center_box(INVENTORY_SCREEN_COLOR);
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
    
    display_center_box(INVENTORY_SCREEN_COLOR);
    curse_print(center_row - q_row + 1, center_column - INVENTORY_TEXT_OFFSET,
        "~Inventory~", INVENTORY_SCREEN_COLOR);

    int bottom_text_offset = 2;
    int item_id = p->inventory->items[inventory_cursor];
    switch (item_data[item_id].type) {
        case FOOD:
            mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Eat [F]");
            bottom_text_offset += 8;
            break;
        case HELM:
        case BREASTPLATE:
        case GREAVES:
        case WEAPON:
        case SHIELD:
            mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Equip [F]");
            bottom_text_offset += 10;
            break;
    }
    if (p->inventory->current_size > 0) {
        mvprintw(center_row + q_row - 1, center_column - q_col + bottom_text_offset, "Drop [Z]");
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

void do_inventory_event(player* player_ptr, int* inventory_cursor_ptr, char key, int* item_grid)
{
    // TODO - make key presses happen only in here
    int inventory_cursor = (*inventory_cursor_ptr);
    int item_id = player_ptr->inventory->items[inventory_cursor];
    switch (key) {
        case 'w': case 'W':
            inventory_cursor = max(0, inventory_cursor - 1);
            break;
        case 's': case 'S':
            inventory_cursor = min(player_ptr->inventory->current_size - 1, inventory_cursor + 1);
            break;
        case 'z': case 'Z':
            {
                int floor_item = item_grid[player_ptr->row * MAP_WIDTH + player_ptr->column];
                if (floor_item == NULL_ITEM_ID) {
                    remove_item(player_ptr->inventory, inventory_cursor);
                    item_grid[player_ptr->row * MAP_WIDTH + player_ptr->column] = item_id;
                    inventory_cursor = max(min(inventory_cursor, player_ptr->inventory->current_size - 1), 0);
                } 
                else
                {
                    clear_status_line();
                    attron(COLOR_PAIR(YELLOW_TEXT_COLOR));
                    const int offset = 25;
                    mvprintw(0, 0, "Cannot drop! Standing on");
                    // later, print color based on item rarity
                    attroff(COLOR_PAIR(YELLOW_TEXT_COLOR));
                    attron(COLOR_PAIR(MAGENTA_TEXT_COLOR));
                    mvprintw(0, offset, "%s", item_data[floor_item].name);
                    attroff(COLOR_PAIR(MAGENTA_TEXT_COLOR));
                }
                break;
            }
        case 'f': case 'F':
            switch (item_data[item_id].type) {
                case FOOD:
                    remove_item(player_ptr->inventory, inventory_cursor);
                    int health_boost = item_data[item_id].health_points;
                    player_ptr->health = min(player_ptr->max_health, player_ptr->health + health_boost);
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
    (*inventory_cursor_ptr) = inventory_cursor;
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

    init_pair(WHITE_TEXT_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(CYAN_TEXT_COLOR, COLOR_CYAN, COLOR_BLACK);
    init_pair(GREEN_TEXT_COLOR, COLOR_GREEN, COLOR_BLACK);
    init_pair(RED_TEXT_COLOR, COLOR_RED, COLOR_BLACK);
    init_pair(BLUE_TEXT_COLOR, COLOR_BLUE, COLOR_BLACK);
    init_pair(YELLOW_TEXT_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(MAGENTA_TEXT_COLOR, COLOR_MAGENTA, COLOR_BLACK);
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



/*

returns whether a coordinate on the map
is next to an open space; either vertically,
horizontally, or diagonally
*/
bool next_to_open_space(char map[], int row, int column)
{
    
    for (int r = -1; r <= 1; r++)
    {
        for (int c = -1; c <= 1; c++)
        {
            int check_row = row + r;
            int check_col = column + c;
            if (check_row >= 0 && check_row < MAP_HEIGHT 
                && check_col >= 0 && check_col < MAP_WIDTH)
            {
                if (get_map_char(check_row, check_col, map) == OPEN_SPACE)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void write_map_curse(char map[], int* item_grid, int* enemy_grid, bool visibility_grid[])
{

    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int column = 0; column < MAP_WIDTH; column++) {
            char c = get_map_char(row, column, map);
            int display_row = row + MAP_HEIGHT_OFFSET;
            switch (c) {
                case OPEN_SPACE:
                    // bool is_visible(char* map, int player_row, int player_col, int row, int column)

                    if (!visibility_grid[row * MAP_WIDTH + column])
                    {
                        curse_put(display_row, column, EMPTY_VOID, 0);
                        break;
                    }

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
                    if (next_to_open_space(map, row, column))
                    {
                        curse_put(display_row, column, c, 0);
                    }
                    else
                    {
                        curse_put(display_row, column, EMPTY_VOID, 0);
                    }
                    
                    break;
                case STAIR:
                    if (!visibility_grid[row * MAP_WIDTH + column])
                    {
                        curse_put(display_row, column, EMPTY_VOID, 0);
                        break;
                    }
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

void clear_visibility_grid(bool visibility_grid[])
{
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        visibility_grid[i] = false;
    }
}

void update_visiblity_grid(bool visibility_grid[], char* map, player* p)
{
    for (int row = 0; row < MAP_HEIGHT; row++)
    {
        for (int col = 0; col < MAP_WIDTH; col++)
        {
            if (is_visible(map, p->row, p->column, row, col))
            {
                visibility_grid[row * MAP_WIDTH + col] = true;
            }
        }
    }
}


// Damage dealt = [rnd num btwn 0 and [atk stat]] + [atk stat / 2]
//                - [rnd num between 0 and [opp. def stat / 3] ]
int calc_damage_done(int attacker_attack, int defender_defense)
{
    return  max((rand() % (attacker_attack + 1)) + (int)((attacker_attack) / 2)
            - (rand() % max(defender_defense / 3, 1)), 0);   
}



