/*

+JMJ+

TODO - fix procedural generation of paths
    - use bfs perhaps

proc gen adapted from:
http://www.roguebasin.com/index.php?title=A_Simple_Dungeon_Generator_for_Python_2_or_3
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>

// note - hard-coded dims based on input file
#define MAP_WIDTH 64
#define MAP_HEIGHT 28
#define ROOM_COUNT 7

#define PLAYER_SYMBOL '@'
#define OPEN_SPACE '.'
#define HALLWAY '#'
#define STAIR '%'



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
char handle_keypress(player* player_ptr, char key, char* map);
char handle_walk_key(player* player_ptr, char* map, int row_change, int col_change);
char can_step(char* map, int row, int column);
player* player_init(int start_row, int start_col);
room** rooms_gen();
room* room_gen();
bool room_overlaps(room* r, room** rooms, int n);
bool rooms_overlap(room* r1, room* r2);
char* map_gen(room** rooms);
void draw_room(room* r, char* map);
void set_player_spawn(room** rooms, player* p);
void join_rooms(room* r1, room* r2, char* map);
bool within_room(room* r, int row, int col);
void set_stair_spawn(room** rooms, char* map);
void clean_rooms(room** rooms);
static inline int min(int a, int b);
static inline int max(int a, int b);
void dig_vertical_tunnel(int r1, int r2, int c, char* map);
void dig_horizontal_tunnel(int r, int c1, int c2, char* map);
char random_wall();

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
    curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, '?');
    

    char key;
    while ((key = getch()) != 'q') {
        if (handle_keypress(player_ptr, key, map) == STAIR) {
            clean_rooms(rooms);
            free(map);
            srand(time(NULL));
            rooms = rooms_gen();
            map = map_gen(rooms); // map_gen(rooms);
            set_player_spawn(rooms, player_ptr);
        }        
        // update map and player location
        write_map_curse(map);

        // for (int i = 0; i < ROOM_COUNT; i++) {
        //     room* r = rooms[i];
        //     curse_put(r->corner_row, r->corner_col, '*', 3);
        // }
        curse_put(player_ptr->row, player_ptr->column, PLAYER_SYMBOL, '?');
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


void clean_rooms(room** rooms)
{
    for (int i = 0; i < ROOM_COUNT; i++) {
            free(rooms[i]);
        }
    free(rooms);
}

char random_wall()
{
    switch (rand() % 11) {
            case 0:
                return '(';
            case 1:
                return '+';               
            case 2:
                return '$';             
            case 3:
                return '!';               
            case 4:
                return '>';            
            case 5:
                return '&';
            case 6:
                return ';';                
            case 7:
                return '=';          
            case 8:
                return '-';          
            case 9:
                return '}';
            case 10:
                return '[';
        }
}


// given valid rooms, draw map
char* map_gen(room** rooms)
{
    char* map = calloc(MAP_HEIGHT * MAP_WIDTH, sizeof(char));
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        map[i] = random_wall();
        
    }
    for (int r = 0; r < ROOM_COUNT; r++) {
        draw_room(rooms[r], map);
    }

    // now generate pathways between them
    for (int i = 0; i < ROOM_COUNT - 1; i++) {
        join_rooms(rooms[i], rooms[i + 1], map);
    }

    // pick stair location
    set_stair_spawn(rooms, map);
    
    return map;
}

bool within_room(room* r, int row, int col)
{
    return row >= r->corner_row &&
            row <= (r->corner_row + r->height)
            && col >= r->corner_col &&
            col <= (r->corner_col + r->width);
}

void join_rooms(room* room1, room* room2, char* map)
{
    int r1, c1, r2, c2;

    
    srand(time(NULL));
    // pick r1, c1 on wall of room1
    r1 = room1->corner_row + room1->height / 2;
    c1 = room1->corner_col; //+ (rand() % 2) * (room1->width - 1);

    // c1 = room1->corner_col;
    // and r2, c2 on wall of room2
    r2 = room2->corner_row + 2;
    c2 = room2->corner_col;// + (rand() % 2) * room2->width;
    // vertically or horizontally first ?
    if (rand() % 2) {
        dig_vertical_tunnel(r1, r2, c1, map);
        dig_horizontal_tunnel(r2, c1, c2, map);
    } else {
        dig_vertical_tunnel(r1, r2, c2, map);
        dig_horizontal_tunnel(r1, c1, c2, map);
    }

}

static inline int min(int a, int b)
{
    return a < b ? a : b;
}


static inline int max(int a, int b)
{
    return a > b ? a : b;
}

void dig_vertical_tunnel(int r1, int r2, int c, char* map)
{
    for (int row = min(r1, r2); row <= max(r1, r2); row++) {
        set_map_char(row, c, map, OPEN_SPACE);
    }
}

void dig_horizontal_tunnel(int r, int c1, int c2, char* map)
{
    for (int col = min(c1, c2); col <= max(c1, c2); col++) {
        set_map_char(r, col, map, OPEN_SPACE);
    }
}



void draw_room(room* r, char* map)
{
    int right_edge = r->corner_col + r->width;
    int bottom_edge = r->corner_row + r->height;
    
    for (int row = r->corner_row; row < bottom_edge; row++) {
        for (int col = r->corner_col; col < right_edge; col++) {
            set_map_char(row, col, map, OPEN_SPACE);
        }
    }
}

room* room_gen()
{
    // printf("genning room\n");
    const int min_height = 4;
    const int min_width = 4;
    const int max_height = 12;
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

void set_stair_spawn(room** rooms, char* map)
{
    room* r = rooms[rand() % ROOM_COUNT];
    int right = r->corner_col + r->width;
    int bottom = r->corner_row + r->height;
    int row = rand() % (bottom - 1 - r->corner_row) + r->corner_row + 1;
    int column = rand() % (right - 2 - r->corner_col) + r->corner_col + 1;
    set_map_char(row, column, map, STAIR);
}


bool rooms_overlap(room* r1, room* r2)
{
    if (r1 == NULL || r2 == NULL) {
        fprintf(stderr, "error: null room given to room compare func\n");
        exit(1);
        return false;
    }
    return (r1->corner_col <= (r2->corner_col + r2->width)) &&
           (r2->corner_col <= (r1->corner_col + r1->width)) &&
           (r1->corner_row <= (r2->corner_row + r2->height)) &&
           (r2->corner_row <= (r1->corner_row + r1->height));
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

char can_step(char* map, int row, int column)
{
    if (row < 0 || row >= MAP_HEIGHT || column < 0 || column >= MAP_WIDTH) {
        return 0;
    }
    char map_char = get_map_char(row, column, map);
    if (map_char == OPEN_SPACE || map_char == HALLWAY || map_char == STAIR) {
        return map_char;
    }
    return 0;
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
    init_pair(HALLWAY, COLOR_YELLOW, COLOR_BLACK);
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
                case HALLWAY:
                    curse_put(row, column, c, 4);
                    break;
                case STAIR:
                    curse_put(row, column, c, 1);
                    break;
                default:
                    curse_put(row, column, c, 2);
                    break;
            }
        }
    }
}
