#include <stdio.h>

#include "powerup.h"
#include "state.h"



const static struct powerup_t g_powerups[NUM_POWERUPS] = {
    
    // ----- Common powerups -----
    
    (struct powerup_t) {
        .type = POWERUP_ACCURACY_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 50.0,
        .can_be_stacked = 1,
        .name = "Accuracy boost\0"
    },
    (struct powerup_t) {
        .type = POWERUP_FASTER_FIRERATE,
        .rarity = POWERUP_COMMON,
        .xp_cost = 80.0,
        .can_be_stacked = 1,
        .name = "Faster firerate\0"
    },
    (struct powerup_t) {
        .type = POWERUP_MAX_HEALTH_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 65.0,
        .can_be_stacked = 1,
        .name = "Max health boost\0"
    },
    (struct powerup_t) {
        .type = POWERUP_MAX_ARMOR_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 75.0,
        .can_be_stacked = 1,
        .name = "Max armor boost\0"
    },
    (struct powerup_t) {
        .type = POWERUP_MOVEMENT_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 100.0,
        .can_be_stacked = 1,
        .name = "Movement boost\0"
    },
    (struct powerup_t) {
        .type = POWERUP_DAMAGE_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 80.0,
        .can_be_stacked = 1,
        .name = "Damage boost\0"
    },
    (struct powerup_t) {
        .type = POWERUP_PROJECTILE_SPEED_BOOST,
        .rarity = POWERUP_COMMON,
        .xp_cost = 120.0,
        .can_be_stacked = 1,
        .name = "Projectile speed boost\0"
    },


    // ----- Rare powerups -----
    
    (struct powerup_t) {
        .type = POWERUP_BURST_FIRE,
        .rarity = POWERUP_RARE,
        .xp_cost = 350.0,
        .can_be_stacked = 1,
        .name = "Burst fire\0"
    },
    (struct powerup_t) {
        .type = POWERUP_BIGGER_PROJECTILES,
        .rarity = POWERUP_RARE,
        .xp_cost = 350.0,
        .can_be_stacked = 1,
        .name = "Bigger projectiles\0"
    }, 
    (struct powerup_t) {
        .type = POWERUP_IMMUNITY_TO_BLINDNESS,
        .rarity = POWERUP_RARE,
        .xp_cost = 385.0,
        .can_be_stacked = 1,
        .name = "Immunity to blindness\0"
    },
    (struct powerup_t) {
        .type = POWERUP_APPLE_MAGNET,
        .rarity = POWERUP_RARE,
        .xp_cost = 385.0,
        .can_be_stacked = 1,
        .name = "Apple magnet\0"
    },



    // ----- Special powerups -----

    (struct powerup_t) {
        .type = POWERUP_HEALTH_REGEN,
        .rarity = POWERUP_SPECIAL,
        .xp_cost = 460.0,
        .can_be_stacked = 1,
        .name = "Health regen\0"
    }

    // ...
};
#include "util.h"

int apply_powerup(struct state_t* gst, struct player_t* player, int powerup_type) {
    int result = 0;


    if(gst->has_audio) {
        PlaySound(gst->sounds[POWERUP_SOUND]);
    }

    switch(powerup_type) {

        // Common powerups

        case POWERUP_ACCURACY_BOOST:
            {
                player->weapon.accuracy += 0.25;
                player->weapon.accuracy = CLAMP(player->weapon.accuracy, 
                        WEAPON_ACCURACY_MIN, WEAPON_ACCURACY_MAX);
            }
            break;
        
        case POWERUP_FASTER_FIRERATE:
            {
                player->firerate -= 0.0185;
                player->firerate = CLAMP(player->firerate, 0.001, 1.0);
            }
            break;
    
        case POWERUP_MAX_HEALTH_BOOST:
            {
                player->max_health += 30;
                player->max_health = CLAMP(player->max_health, 0, ABS_MAX_HEALTH);
                player_heal(gst, &gst->player, 65.0);
            }
            break;

        case POWERUP_MAX_ARMOR_BOOST:
            {
                player->max_armor += 3;
                player->max_armor = CLAMP(player->max_armor, 0, ABS_MAX_ARMOR);
            
                player->armor_damage_dampen -= 0.01;
                player->armor_damage_dampen = CLAMP(player->armor_damage_dampen, 0, ABS_MAX_ARMOR);
            }
            break;

        case POWERUP_MOVEMENT_BOOST:
            {
                player->walkspeed += 0.5;
                player->run_speed_mult += 0.5;
                player->dash_timer_max -= 1.0;
            }
            break;

        case POWERUP_DAMAGE_BOOST:
            {
                player->weapon.damage += 10;
            }
            break;

        case POWERUP_PROJECTILE_SPEED_BOOST:
            {
                player->weapon.prj_speed += 150;
            }
            break;

            // Rare powerups
        case POWERUP_BURST_FIRE:
            {
            }
            break;

        case POWERUP_BIGGER_PROJECTILES:
            {
            }
            break;

        case POWERUP_IMMUNITY_TO_BLINDNESS:
            {
            }
            break;

        case POWERUP_APPLE_MAGNET:
            {
            }
            break;


        // Special powerups
        
        case POWERUP_HEALTH_REGEN:
            {
            }
            break;

    }



    return result;
}

struct powerup_t get_powerup(int powerup_type) {
    if(powerup_type >= NUM_POWERUPS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid powerup type\033[0m\n",
                __func__);
        powerup_type = 0;
    }

    return g_powerups[powerup_type];
}

void update_powerup_shop_offers(struct state_t* gst) {
    struct powerup_shop_t* shop = &gst->player.powerup_shop;
    printf("--- New Powerup offers:\n");

    for(int i = 0; i < NUM_POWERUP_OFFERS; i++) {
        struct powerup_t* powerup = &shop->offers[i];
        
        int powerup_type = GetRandomValue(0, NUM_POWERUPS-1);

        *powerup = get_powerup(powerup_type);

        printf(" \"%s\"\n", powerup->name);
    }

    printf("---------\n");

    gst->player.powerup_shop.selected_index = -1;
}


