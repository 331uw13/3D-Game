#ifndef POWERUP_H
#define POWERUP_H


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





// ------------ Special powerups ---------------

#define POWERUP_SPECIAL 2

// Player can handle more powerups. (can be stacked)
#define POWERUP_MORE_POWERUPS_SLOTS 10

// ...




// ------------ Legendary powerups ---------------
// ...


// ------------ Mythical powerups ---------------
// ...




struct powerup_t {
    
    int type;
    int rarity;
    int xp_cost;
 

    float level;  // This multiplies the powerup stats if the powerup can be stacked.
    int can_be_stacked;
};


struct player_t;

int apply_powerup(struct player_t* player, int powerup_type);




#endif
