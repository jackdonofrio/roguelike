#include "items.h"

// note - any individual enemy's stats only
// matter once you enter combat

#define ENEMY_SPAWN_RATE 3 // 1 / N = likelihood of enemy spawning in room
#define ENEMY_SYMBOL '^'
#define NULL_ENEMY_ID 0
#define GOBLIN		  1
#define WRAITH		  2

typedef struct
{
	int row;
	int column;
	int enemy_combat_id;
	int health_points;
} enemy;

typedef struct
{
	char* name;
	int attack;
	int defense;
	int exp;
} enemy_combat;

#define NUM_ENEMIES   3
static enemy_combat enemy_combat_data[NUM_ENEMIES] = {
	{"None", 	0, 0, 0},
	{"Goblin",	1, 3, 5},
	{"Wraith",	3, 5, 8}
};

