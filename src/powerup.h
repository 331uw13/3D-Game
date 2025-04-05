#ifndef POWERUP_H
#define POWERUP_H

#include <stddef.h>

// Powerup Type





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

// Player can shoot multiple projectiles at once.
#define POWERUP_BURST_FIRE 7

// Projectiles size is increased.
#define POWERUP_BIGGER_PROJECTILES 8

// Player's health regenrates by itself overtime.
#define POWERUP_HEALTH_REGEN 9

// Projectiles go through enemies.
#define POWERUP_FMJPRJ_ABILITY 10

// Projectiles gravitate towards enemies.
#define POWERUP_GRAVITY_PROJECTILES 11

#define POWERUP_RECOIL_CONTROL 12

// Projectiles fall from the sky sometimes.
#define POWERUP_PRJ_CLOUDBURST 13


#define MAX_POWERUP_TYPES 14
#define NUM_POWERUP_OFFERS 5 

struct powerup_t {
    int type;
    int xp_cost;
    int max_level;
    float xp_cost_mult;    // xp_cost is multiplied with this number everytime player buys this powerup.
    char* name;
};



struct powerup_shop_t {
    int open;
    int selected_index; // If set to negative value, nothing is selected.
    struct powerup_t      offers[NUM_POWERUP_OFFERS];
    struct powerup_t prev_offers[NUM_POWERUP_OFFERS]; // Save previous offers so they cant be the same.
    
    struct powerup_t powerups[MAX_POWERUP_TYPES];
};

struct state_t;
struct player_t;

void set_powerup_defaults(struct state_t* gst, struct powerup_shop_t* shop);
void apply_powerup(struct state_t* gst, struct player_t* player, int powerup_type);
void update_powerup_shop_offers(struct state_t* gst);


#endif
