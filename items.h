#include <stdbool.h>

#ifndef ITEMS_H
#define ITEMS_H

#define MAX_INVENTORY_SIZE 10

#define DROP_RATE_PER_ROOM 4 // 1 over N odds that an item drops in a given room


// item id's
#define NULL_ITEM_ID       0
/////////////////////////////

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

typedef enum {
    COMMON,
    UNCOMMON,
    RARE
} item_rarity;

typedef struct {
    char* name;
    item_type type;
    int min_level;
    item_rarity rarity;

    // using anonymous union to save space
    union {
        int attack;
        int defense;
        int health_points;
    };
} item;

#define NUM_ITEMS         11
static item item_data[NUM_ITEMS] = {
    {"----",               NONE,        1, COMMON,   .attack        = 1}, // fists have attack = 1
    // COMMON ITEMS
    {"Bronze sword",       WEAPON,      1, COMMON,   .attack        = 3},
    {"Bronze helm",        HELM,        1, COMMON,   .defense       = 2},
    {"Bronze breastplate", BREASTPLATE, 1, COMMON,   .defense       = 4},
    {"Bronze greaves",     GREAVES,     1, COMMON,   .defense       = 3},
    {"Bronze shield",      SHIELD,      1, COMMON,   .defense       = 4},
    {"Bread",              FOOD,        1, COMMON,   .health_points = BASIC_HEALTH_BOOST},
    {"Apple",              FOOD,        1, COMMON,   .health_points = BASIC_HEALTH_BOOST},
    {"Stick",              WEAPON,      1, COMMON,   .attack        = 2},
    // UNCOMMON ITEMS
    {"Iron helm",          HELM,        5, UNCOMMON, .defense       = 3},
    {"Magic staff",        WEAPON,      2, UNCOMMON, .attack        = 3}//,
    // {""}
};


typedef struct inventory
{
    int current_size;
    int items[MAX_INVENTORY_SIZE]; // list of item IDs
} inventory_t;

bool full_inventory(inventory_t* inventory);
void add_item(inventory_t* inventory, int item_id);
int remove_item(inventory_t* inventory, int index);

#endif /* ITEMS_H*/