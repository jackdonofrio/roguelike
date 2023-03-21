/*

+JMJ+


proc gen adapted from:
http://www.roguebasin.com/index.php?title=A_Simple_Dungeon_Generator_for_Python_2_or_3
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ncurses.h>

// note - hard-coded dims based on input file
#define MAP_WIDTH 64
#define MAP_HEIGHT 21
#define ROOM_COUNT 3

#define PLAYER_SYMBOL '@'
#define OPEN_SPACE '.'
#define HALLWAY '#'

typedef struct player
{
    int row;
    int column;
    uint health; // integer from 0 to 100
} player;

typedef struct room {
    int width;
    int height;
    int corner_row; // left top corner
    int corner_col; // left top corner
} room;

char get_map_char(int row, int col, char* map);
void set_map_char(int row, int col, char* map, char c);
char* load_map(char* filename);
void setup_ncurses();
void write_map_curse(char* map);
void curse_put(int row, int col, char c, int color);
void handle_keypress(player* player_ptr, char key, char* map);
void handle_walk_key(player* player_ptr, char* map, int row_change, int col_change);
bool can_step(char* map, int row, int column);
player* player_init(int start_row, int start_col);
room** rooms_gen();
room* room_gen();
bool room_overlaps(room* r, room** rooms, int n);
bool rooms_overlap(room* r1, room* r2);
char* map_gen(room** rooms);
void draw_room(room* r, char* map);
void set_player_spawn(room** rooms, player* p);
void join_rooms(room* r1, room* r2);

int main()
{
    player* player_ptr = player_init(2, 2);
    char* map;
    room** rooms;
    bool use_proc_gen = true;
    if (use_proc_gen) {
        srand(time(NULL));
        rooms = rooms_gen();
        map = map_gen(rooms); // map_gen(rooms);
        set_player_spawn(rooms, player_ptr);
    }
    else {
        map = load_map("test.txt");  
    }
    setup_ncurses();
    write_map_curse(map);
    curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, 2);
    

    char key;
    while ((key = getch()) != 'q') {
        handle_keypress(player_ptr, key, map);        
        // update map and player location
        write_map_curse(map);

        // for (int i = 0; i < ROOM_COUNT; i++) {
        //     room* r = rooms[i];
        //     curse_put(r->corner_row, r->corner_col, '*', 3);
        // }
        curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, 2);
    }
    free(map);
    free(player_ptr);
    if (use_proc_gen) {
        for (int i = 0; i < ROOM_COUNT; i++) {
            free(rooms[i]);
        }
        free(rooms);
    }
    endwin();
    return 0;
}


// given valid rooms, draw map
char* map_gen(room** rooms)
{
    char* map = calloc(MAP_HEIGHT * MAP_WIDTH, sizeof(char));
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        map[i] = ' ';
    }
    for (int r = 0; r < ROOM_COUNT; r++) {
        draw_room(rooms[r], map);
    }

    // now generate pathways between them
    for (int i = 0; i < ROOM_COUNT - 1; i++) {
        join_rooms(rooms[i], rooms[i + 1]);
    }
    


    return map;
}



void join_rooms(room* r1, room* r2)
{
    

}





void draw_room(room* r, char* map)
{
    // draw top border
    int right_edge = r->corner_col + r->width;
    int bottom_edge = r->corner_row + r->height;

    // top left corner
    set_map_char(r->corner_row, r->corner_col, map, '+');
    // top border
    for (int c = r->corner_col + 1; c < right_edge - 1; c++) {
        set_map_char(r->corner_row, c, map, '-');
    }
    // top right corner
    set_map_char(r->corner_row, right_edge - 1, map, '+');
    
    for (int row = r->corner_row + 1; row < bottom_edge; row++) {
        // draw left bar
        set_map_char(row, r->corner_col, map, '|');

        // fill interior
        for (int col = r->corner_col + 1; col < right_edge - 1; col++) {
            set_map_char(row, col, map, '.');
        }
        // draw right bar
        set_map_char(row, right_edge - 1, map, '|');
    }

    // bottom left corner
    set_map_char(bottom_edge, r->corner_col, map, '+');
    // draw bottom border
    for (int c = r->corner_col + 1; c < right_edge - 1; c++) {
        set_map_char(bottom_edge, c, map, '-');
    }
    // bottom right corner
    set_map_char(bottom_edge, right_edge - 1, map, '+');
}

room* room_gen()
{
    printf("genning room\n");
    const int min_height = 5;
    const int min_width = 7;
    const int max_height = 18;
    const int max_width = 15;

    room* r = malloc(sizeof(room));
    if (r == NULL) {
        fprintf(stderr, "error: unable to alloc room\n");
        exit(1);
        return NULL;
    }

    r->width = rand() % (max_width - min_width + 1) + min_width;
    r->height = rand() % (max_height - min_height + 1) + min_height;
    r->corner_row = rand() % (MAP_HEIGHT - r->height - 1) + 1;
    r->corner_col = rand() % (MAP_WIDTH - r->width - 1) + 1;
    return r;

}

void set_player_spawn(room** rooms, player* p)
{
    room* r = rooms[rand() % ROOM_COUNT];
    int right = r->corner_col + r->width;
    int bottom = r->corner_row + r->height;
    p->row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
    p->column = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;

}

bool rooms_overlap(room* r1, room* r2)
{
    if (r1 == NULL || r2 == NULL) {
        fprintf(stderr, "error: null room given to room compare func\n");
        exit(1);
        return false;
    }
    return (r1->corner_col < (r2->corner_col + r2->width)) &&
           (r2->corner_col < (r1->corner_col + r1->width)) &&
           (r1->corner_row < (r2->corner_row + r2->height)) &&
           (r2->corner_row < (r1->corner_row + r1->height));
}

// return whether a given room overlaps with any other room
bool room_overlaps(room* r, room** rooms, int n)
{
    if (r == NULL || rooms == NULL) {
        return NULL;
    }
    for (int i = 0; i < n; i++) {
        if (rooms[i] != NULL && rooms_overlap(r, rooms[i])) {
            return true;
        }
    }
    return false;
}

// generates room locations, data
room** rooms_gen()
{
    room** rooms = malloc(sizeof(room) * ROOM_COUNT);
    for (int i = 0; i < ROOM_COUNT; i++) {
        room* r = room_gen();
        while (room_overlaps(r, rooms, i)) {
            free(r);
            r = room_gen();
        }
        rooms[i] = r;
    }
    return rooms;
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

void set_map_char(int row, int col, char* map, char c)
{
    map[row * MAP_WIDTH + col] = c;
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