#include "items.h"

#define DEFAULT_MAX_HEALTH 20

typedef struct player
{
    // location / map-relevant info
    int row;
    int column;


    // data
    int health; // integer from 0 to 100
    int gold;

    // stats
    int level;
    int exp;
    int attack;
    int defense;


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
