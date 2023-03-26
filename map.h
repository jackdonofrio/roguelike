/*
Includes all important tools for handling the game map, 
including room generation

+JMJ+

*/

#define MAP_WIDTH 64
#define MAP_HEIGHT 28
#define ROOM_COUNT 10

// map symbols
#define PLAYER_SYMBOL '@'
#define ITEM_SYMBOL '?'
#define WALL '#'
#define OPEN_SPACE '.'
#define STAIR '%'

// useful macros
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))


typedef struct room {
    int width;
    int height;
    int corner_row; // left top corner
    int corner_col; // left top corner
} room;


void load_map(char* filename, char map[]);
char get_map_char(int row, int col, char map[]);
void set_map_char(int row, int col, char map[], char c);

void rooms_gen(room* rooms[]);
room* room_gen();
bool room_overlaps(room* r, room* rooms[], int n);
bool rooms_overlap(room* r1, room* r2);
void join_rooms(room* r1, room* r2, char map[]);
void dig_vertical_tunnel(int r1, int r2, int c, char map[]);
void dig_horizontal_tunnel(int r, int c1, int c2, char map[]);
bool within_room(room* r, int row, int col);
void set_stair_spawn(room* rooms[], char map[]);
void delete_rooms(room* rooms[]);
char random_wall();
void draw_room(room* r, char map[]);
void map_gen(room* rooms[], int level, char map[]);
char can_step(char map[], int row, int column);
// void update_visibility(char* map, bool visible[], int row, int column);

// bool is_visible(char* map, int player_row, int player_col, int row, int column);
// bool visible_helper_straight(char* map, int player_row, int player_col, int row, int column);
// bool can_see_through(char* map, int r, int c);