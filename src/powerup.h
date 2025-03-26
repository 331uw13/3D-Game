#ifndef POWERUP_H
#define POWERUP_H

#include <stddef.h>

// Powerup Type



// ------------ Common powerups ---------------

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

// Player can shoot multiple projectiles at once.
#define POWERUP_BURST_FIRE 7

// Projectiles size is increased.
#define POWERUP_BIGGER_PROJECTILES 8


// ------------ Special powerups ---------------

#define POWERUP_SPECIAL 2

// Player's health regenrates by itself overtime.
#define POWERUP_HEALTH_REGEN 9

// ...




// ------------ Legendary powerups ---------------
#define POWERUP_LEGENDARY 3
// ...

// ------------ Mythical powerups ---------------
#define POWERUP_MYTHICAL 4
// ...


#define NUM_POWERUPS 10
#define NUM_POWERUP_OFFERS 4

struct powerup_t {
    int type;
    int rarity;
    int xp_cost;
    int max_level;
    float xp_cost_mult;    // xp_cost is multiplied with this number everytime player buys this powerup.
    char* name;
};



struct powerup_shop_t {
    int open;
    int selected_index; // If set to negative value, nothing is selected.
    struct powerup_t offers[NUM_POWERUP_OFFERS];

    struct powerup_t powerups[NUM_POWERUPS];
};

struct state_t;
struct player_t;

void set_powerup_defaults(struct state_t* gst, struct powerup_shop_t* shop);
void apply_powerup(struct state_t* gst, struct player_t* player, int powerup_type);
void update_powerup_shop_offers(struct state_t* gst);


#endif
