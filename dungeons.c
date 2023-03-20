/*

+JMJ+

*/

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

// note - hard-coded dims based on input file
#define MAP_WIDTH 32
#define MAP_HEIGHT 16

char get_map_char(int row, int col, char* map);
char* load_map(char* filename);
void setup_ncurses();
void write_map_curse(char* map);
void curse_put(int row, int col, char c, int color);


int main()
{
    char* map = load_map("test.txt");
    setup_ncurses();
    write_map_curse(map);

    int player_row = 2;
    int player_col = 2;
    char map_char;

    curse_put(player_row, player_col, '@', 1);

    char c = '\0';
    while (c != 'q') {
        c = getch();
        switch (c) {
            case 'q':
                break;
            case 'w':
                map_char = get_map_char(player_row - 1, player_col, map);
                if (map_char == '.') {
                    player_row--;
                }
                break;
            case 'a':
                map_char = get_map_char(player_row, player_col - 1, map);
                if (map_char == '.') {
                    player_col--;
                }
                break;
            case 's':
                map_char = get_map_char(player_row + 1, player_col, map);
                if (map_char == '.') {
                    player_row++;
                }
                break;
            case 'd':
                map_char = get_map_char(player_row, player_col + 1, map);
                if (map_char == '.') {
                    player_col++;
                }
                break;
        }
        
        // update map 
        write_map_curse(map);    
        curse_put(player_row, player_col, '@', 1);
        // refresh();
    }
    free(map);
    endwin();
    return 0;
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