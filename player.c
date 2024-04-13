#include <stdio.h>
#include <stdlib.h>
#include "player.h"



void player_delete(player* p)
{
    if (p == NULL) {
        return;
    }
    free(p->inventory);
    free(p);
}


player* player_init()
{
    player* p = malloc(sizeof(player));
    if (p == NULL) {
        fprintf(stderr, "error: unable to allocate player data\n");
        exit(1);
        return NULL;
    }

    p->row = 0; // placeholder - they get changed inevitably
    p->column = 0;
    p->helm = NULL_ITEM_ID;
    p->breastplate = NULL_ITEM_ID;
    p->greaves = NULL_ITEM_ID;
    p->weapon = NULL_ITEM_ID;
    p->shield = NULL_ITEM_ID;

    p->health = DEFAULT_MAX_HEALTH; 
    p->gold = 0;
    
    p->level = 1;
    // p->attack = 1;
    // p->defense = 1;

    p->inventory = malloc(sizeof(inventory_t));
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        p->inventory->items[i] = NULL_ITEM_ID;
    }
    p->inventory->current_size = 0;

    return p;
}

