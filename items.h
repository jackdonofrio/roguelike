#include <stdbool.h>

#ifndef ITEMS_H
#define ITEMS_H

#define MAX_INVENTORY_SIZE 10

#define DROP_RATE_PER_ROOM 1 // 1 over N odds that an item drops in a given room


// item id's
#define NULL_ITEM_ID       0
#define BRONZE_SWORD       1
#define BRONZE_HELM        2
#define BRONZE_BREASTPLATE 3
#define BRONZE_GREAVES     4
#define BRONZE_SHIELD      5
#define IRON_HELM          6
// food
#define BREAD              7
#define APPLE              8
// misc weapons
#define STICK              9
/////////////////////////////
#define NUM_ITEMS         10

// item colors
#define COMMON_ITEM_COLOR  1


// food HP boost tiers
#define BASIC_HEALTH_BOOST 5


// TODO - revamp item system
typedef enum {
    NONE,
    WEAPON,
    HELM,
    BREASTPLATE,
    GREAVES,
    SHIELD,
    FOOD
} item_type;

typedef struct {
    char* name;
    item_type type;
    int min_level;
    // using anonymous union
    union {
        int attack;
        int defense;
        int health_points;
    };
} item;

static item item_data[NUM_ITEMS] = {
    {"None",               NONE,        1, .attack        = 1}, // fists have attack = 1
    {"Bronze sword",       WEAPON,      1, .attack        = 3},
    {"Bronze helm",        HELM,        1, .defense       = 2},
    {"Bronze breastplate", BREASTPLATE, 1, .defense       = 4},
    {"Bronze greaves",     GREAVES,     1, .defense       = 3},
    {"Bronze shield",      SHIELD,      1, .defense       = 4},
    {"Iron helm",          HELM,        5, .defense       = 3},
    {"Bread",              FOOD,        1, .health_points = BASIC_HEALTH_BOOST},
    {"Apple",              FOOD,        1, .health_points = BASIC_HEALTH_BOOST},
    {"Stick",              WEAPON,      1, .attack        = 2}
};


typedef struct inventory
{
    int current_size;
    int items[MAX_INVENTORY_SIZE]; // list of item IDs
} inventory_t;

bool full_inventory(inventory_t* inventory);
void add_item(inventory_t* inventory, int item_id);
int remove_item(inventory_t* inventory, int index);
int calc_food_hp_boost(int item_id);

#endif /* ITEMS_H*/