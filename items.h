#ifndef ITEMS_H
#define ITEMS_H

#define MAX_INVENTORY_SIZE 10

#define DROP_RATE_PER_ROOM 5 // 1 over N odds that an item drops in a given room

// item id's
#define NULL_ITEM_ID       0
#define BRONZE_SWORD       1
#define BRONZE_HELM        2
#define BRONZE_BREASTPLATE 3
#define BRONZE_GREAVES     4
#define BRONZE_SHIELD      5
// food
#define BREAD              6
#define APPLE              7
/////////////////////////////
#define MAX_ITEM_ID        8

// item colors
#define COMMON_ITEM_COLOR  1


static const char* common_item_names[] =
{
    "NULL",               // 0
    "Bronze sword",       // 1
    "Bronze helm",        // 2
    "Bronze breastplate", // 3
    "Bronze greaves",     // 4
    "Bronze shield",      // 5
    "Bread",              // 5
    "Apple"               // 6
};

typedef struct inventory
{
    int current_size;
    int items[MAX_INVENTORY_SIZE]; // list of item IDs
} inventory_t;

bool full_inventory(inventory_t* inventory);
void add_item(inventory_t* inventory, int item_id);

#endif /* ITEMS_H*/