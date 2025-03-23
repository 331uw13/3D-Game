#ifndef POWERUP_H
#define POWERUP_H

#include <stddef.h>

// Powerup Type



// ------------ Common powerups ---------------
// (all common powerups can be stacked)

#define POWERUP_COMMON 0

// Player can shoot more accurately
// and "recoil" doesnt affect accuracy that much.
#define POWERUP_ACCURACY_BOOST   0

// Player can shoot faster with fullauto mode
#define POWERUP_FASTER_FIRERATE  1

// Max health is boosted.
#define POWERUP_MAX_HEALTH_BOOST 2

// Max armor is increased and damage dampening(if player has armor) is boosted.
#define POWERUP_MAX_ARMOR_BOOST  3

// Dash ability cooldown is reduced and movement speed is increased.
#define POWERUP_MOVEMENT_BOOST   4

// Damage from projectiles to enemies is boosted.
#define POWERUP_DAMAGE_BOOST 5

// Projectile speed is increased.
#define POWERUP_PROJECTILE_SPEED_BOOST 6



// ------------ Rare powerups ---------------
// (only some rare powerups can be stacked)

#define POWERUP_RARE 1

// Player can shoot multiple projectiles at once. (can be stacked)
#define POWERUP_BURST_FIRE 7

// Projectiles size is increased. (can be stacked)
#define POWERUP_BIGGER_PROJECTILES 8

// Some enemies have small chance to make player temporarily "blind". (can not be stacked.)
#define POWERUP_IMMUNITY_TO_BLINDNESS 9 

// Player picksup and eats apples automatically when nearby. (can be stacked, increases the range)
#define POWERUP_APPLE_MAGNET 10


// ------------ Special powerups ---------------

#define POWERUP_SPECIAL 2

// Player's health regenrates by itself overtime. (can be stacked)
#define POWERUP_HEALTH_REGEN 11

// ...




// ------------ Legendary powerups ---------------
#define POWERUP_LEGENDARY 3
// ...

// ------------ Mythical powerups ---------------
#define POWERUP_MYTHICAL 4
// ...


#define NUM_POWERUPS 12
#define NUM_POWERUP_OFFERS 3


struct powerup_t {
    int type;
    int rarity;
    int xp_cost;
    int can_be_stacked;
    char* name;
};

#define POWERUP_SHOP_TIMEOUT 6


struct powerup_shop_t {
    int open;
    int selected_index; // If set to negative value, nothing is selected.
    struct powerup_t offers[NUM_POWERUP_OFFERS];

    int available;
    float timeout_time;
};

struct state_t;
struct player_t;

struct powerup_t get_powerup(int powerup_type);
int apply_powerup(struct state_t* gst, struct player_t* player, int powerup_type);
void update_powerup_shop_offers(struct state_t* gst);


#endif
