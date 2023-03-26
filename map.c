#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "map.h"

/*
Loads map from file
- done for testing purposes
- NOTE: make sure the file has dimensions equal to 
  MAP_WIDTH and MAP_HEIGHT, since they're not calculated here.
*/

// char* load_gospel_of_john(char* filename)
// {
//     FILE* fp = fopen(filename, "r");
//     if (fp == NULL) {
//         fprintf(stderr, "error: unable to read file %s\n", filename);
//         return NULL;
//     }
//     char c;
//     int i = 0;
//     const int charcount = 72200;
//     char* raw = malloc(sizeof(char) * charcount);
//     while ((c = fgetc(fp)) != EOF && i < charcount) {
//         if (c != '\n' && ((int)c) != 13) {
//             raw[i++] = c;
//         }        
//     }
//     fclose(fp);
//     return map;
// }

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
    while ((c = fgetc(fp)) != EOF && i < MAP_HEIGHT * MAP_WIDTH) {
        if (c != '\n' && ((int)c) != 13) {
            map[i++] = c;
        }        
    }
    fclose(fp);
    return map;
}

char get_map_char(int row, int col, char* map)
{
    return map[row * MAP_WIDTH + col];
}

void set_map_char(int row, int col, char* map, char c)
{
    map[row * MAP_WIDTH + col] = c;
}

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

void delete_rooms(room** rooms)
{
    for (int i = 0; i < ROOM_COUNT; i++) {
            free(rooms[i]);
        }
    free(rooms);
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

// given valid rooms, draw map
char* map_gen(room** rooms, int level)
{
    char* map;
    bool use_bible = true; // experimental
    map = calloc(MAP_HEIGHT * MAP_WIDTH, sizeof(char));
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        map[i] = WALL; // random_wall();
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

bool within_room(room* r, int row, int col)
{
    return row >= r->corner_row &&
            row <= (r->corner_row + r->height)
            && col >= r->corner_col &&
            col <= (r->corner_col + r->width);
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

char can_step(char* map, int row, int column)
{
    if (row < 0 || row >= MAP_HEIGHT || column < 0 || column >= MAP_WIDTH) {
        return 0;
    }
    char map_char = get_map_char(row, column, map);
    if (map_char == OPEN_SPACE || map_char == STAIR) {
        return map_char;
    }
    return 0;
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

char random_wall()
{
    int r = rand();
     return r % 26 + (r % 1 ? 65 : 97);
    // switch (rand() % 15) {
    //         case 0: return '(';
    //         case 1: return '+';               
    //         case 2: return '$';             
    //         case 3: return '!';               
    //         case 4: return '>';            
    //         case 5: return '&';
    //         case 6: return ';';                
    //         case 7: return '=';          
    //         case 8: return '-';          
    //         case 9: return '}';
    //         case 10: return '[';
    //         case 11: return '\"';
    //         case 12: return '\'';
    //         case 13: return ':';
    //         case 14: return ',';
    //         default: return WALL;
    //     }
}

// use bounding box
// void update_visibility(char* map, bool visible[], int row, int column)
// {
//     const int radius = 10;
//     for (int r = row - radius; r <= row + radius; r++) {
//         for (int c = column - radius; c <= column + radius; c++) {
//             int dr = r - row;
//             int dc = c - column;
//             if (dr * dr + dc * dc <= radius) {
//                 visible[r * MAP_WIDTH + c] = true;
//             }
//         }
//     }
// }

// // adapted from code I wrote for CS50 final project
// bool is_visible(char* map, int player_row, int player_col, int row, int column)
// {
//     if (map == NULL || row < 0 || row >= MAP_HEIGHT
//         || column < 0 || column >= MAP_WIDTH)
//         return false;
//     if (row == player_row && column == player_col)
//         return true;

//     double delta_row = player_row - row;
//     double delta_col = player_col - column;
//     if (delta_col == 0 || delta_row == 0) {
//         return visible_helper_straight(map, player_row, player_col, row, column);
//     }

//     double current_row = 0;
//     double current_col = 0;

//     double dr_dc = delta_row / delta_col;
//     double dc_dr = delta_col / delta_row;

//     // case 1) point below, right of player
//     if (row > player_row && column > player_col) { // dr_dc, dc_dr > 0
//         current_row = player_row + dr_dc;
//         for (current_col = player_col + 1; current_col < column && current_row >= 0
//             && current_row < MAP_HEIGHT; current_col++) {
//                 bool can_see_thru_intersection = intersection_helper(map, current_row, current_col, true);
//             }
//     }


// }

// bool intersection_helper(char* map, double current_row, double current_col, bool is_row_check)
// {
//     if (is_row_check) {
//         if
//     }
// }



// bool visible_helper_straight(char* map, int player_row, int player_col, int row, int column)
// {
//     double delta_row = player_row - row;
//     double delta_col = player_col - column;
//     if (delta_row == 0) {
//         if (player_col < column) {
//             for (int i = player_col + 1; i < column; i++) {
//                 if (!can_see_through(map, row, i)) {
//                     return false;
//                 }
//             }
//         } else {
//             for (int i = player_col - 1; i > column; i--) {
//                 if (!can_see_through(map, row, i)) {
//                     return false;
//                 }
//             }
//         }
//         return true;
//     }
//     if (delta_col == 0) {
//         if (player_row < row) {
//             for (int i = player_row + 1; i < row; i++) {
//                 if (!can_see_through(map, i, column)) {
//                     return false;
//                 }
//             }
//         } else {
//             for (int i = player_row - 1; i > row; i--) {
//                 if (!can_see_through(map, i, column)) {
//                     return false;
//                 }
//             }
//         }
//         return true;
//     }
//     return true;
// }

// bool can_see_through(char* map, int r, int c)
// {
//     return get_map_char(r, c, map) == OPEN_SPACE;
// }