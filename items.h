#include <stdbool.h>

#ifndef ITEMS_H
#define ITEMS_H

#define MAX_INVENTORY_SIZE 10

#define DROP_RATE_PER_ROOM 1 // 1 over N odds that an item drops in a given room

// item use constants
#define FOOD        'F'
#define HELM        'H'
#define BREASTPLATE 'B'
#define GREAVES     'G'
#define WEAPON      'W'
#define SHIELD      'S'

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
/////////////////////////////
#define MAX_ITEM_ID        9

// item colors
#define COMMON_ITEM_COLOR  1


// food HP boost tiers
#define BASIC_HEALTH_BOOST 5


static const char* common_item_names[] =
{
    "None",               // 0
    "Bronze sword",       // 1
    "Bronze helm",        // 2
    "Bronze breastplate", // 3
    "Bronze greaves",     // 4
    "Bronze shield",      // 5
    "Iron helm",          // 6
    "Bread",              // 7
    "Apple"               // 8
};

// TODO - compress info into bytes to create one lookup table for
// compressed item data, and one lookup table for names

static const int item_use_table[] =
{
    NULL_ITEM_ID,   // NULL 0
    WEAPON,         // B sword
    HELM,           // B Helm
    BREASTPLATE,    // B breastplate
    GREAVES,        // B greaves
    SHIELD,         // B shield
    HELM,           // I helm
    FOOD,           // bread
    FOOD            // apple
};

typedef struct inventory
{
    int current_size;
    int items[MAX_INVENTORY_SIZE]; // list of item IDs
} inventory_t;

void clear_item_grid(int item_grid[]);
bool full_inventory(inventory_t* inventory);
void add_item(inventory_t* inventory, int item_id);
int remove_item(inventory_t* inventory, int index);
int calc_food_hp_boost(int item_id);

#endif /* ITEMS_H*/