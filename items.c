#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

item_rarity pick_item_rarity(item_rarity max_rarity)
{
    if (max_rarity == COMMON)
    {
        return COMMON;
    }

    const int max_distr_value = 100;
    int common_likelihood = 100;
    int uncommon_likelihood = 0;
    int rare_likelihood = 0;

    if (max_rarity == UNCOMMON)
    {
        common_likelihood = 70;
        uncommon_likelihood = 30;
        rare_likelihood = 0;
    }
    else if (max_rarity == RARE)
    {
        common_likelihood = 50;
        uncommon_likelihood = 35;
        rare_likelihood = 15;
    }
    int r_value = rand() % max_distr_value;
    if (r_value < common_likelihood)
    {
        return COMMON;
    }

    // implictly >= common_likelihood, or would have returned
    if (r_value < (common_likelihood + uncommon_likelihood))
    {
        return UNCOMMON;
    }
    if (r_value < (common_likelihood + uncommon_likelihood + rare_likelihood))
    {
        return RARE;
    }

    // undefined
    return 0;
}

int pick_random_item(item_rarity max_rarity)
{
    item_rarity rarity = pick_item_rarity(max_rarity);

    return 0;
}
