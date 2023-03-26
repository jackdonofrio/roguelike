#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "items.h"
#include "map.h"

void clear_item_grid(int item_grid[])
{
    for (int i = 0; i < MAP_HEIGHT * MAP_WIDTH; i++) {
        item_grid[i] = NULL_ITEM_ID;
    }
}


bool full_inventory(inventory_t* inventory)
{
    if (inventory == NULL) {
        fprintf(stderr, "error: null inventory passed to full_inventory\n");
        return true;
    }
    return inventory->current_size == MAX_INVENTORY_SIZE;
}

void add_item(inventory_t* inventory, int item_id)
{
    if (inventory == NULL || item_id == NULL_ITEM_ID || inventory->current_size >= MAX_INVENTORY_SIZE) {
        return;
    }
    inventory->items[inventory->current_size++] = item_id;
}


// return: removed item ID
int remove_item(inventory_t* inventory, int index)
{
    if (inventory == NULL || index < 0 || index >= inventory->current_size) {
        return NULL_ITEM_ID;
    }
    int item_id = inventory->items[index];
    // overwrite with other inventory items
    for (int i = index; i < inventory->current_size - 1; i++) {
        inventory->items[i] = inventory->items[i + 1];
    }
    inventory->items[inventory->current_size - 1] = NULL_ITEM_ID;
    inventory->current_size--;
    return item_id;
}

// food HP LUT
int calc_food_hp_boost(int item_id)
{
    switch (item_id) {
        
        // lowest healing tier: +5 HP
        case APPLE:
        case BREAD:
            return BASIC_HEALTH_BOOST;
        default:
            return 0;
    }
}