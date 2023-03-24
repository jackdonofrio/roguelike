#include "items.h"

#define DEFAULT_MAX_HEALTH 100

typedef struct player
{
    int row;
    int column;
    int health; // integer from 0 to 100
    inventory_t* inventory;

    // equipable items
    int helm;        // helm item id
    int breastplate; // breastplate item id
    int greaves;     // greaves item id
    int shield;
    int weapon;

} player;


void player_delete(player* p);
player* player_init();
