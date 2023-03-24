#include <stdbool.h>
#include <stdio.h>
#include "items.h"

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
    if (item_id == NULL_ITEM_ID || inventory->current_size >= MAX_INVENTORY_SIZE) {
        return;
    }
    inventory->items[inventory->current_size++] = item_id;
}