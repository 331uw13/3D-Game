#include <stdio.h>
#include <string.h>

#include "powerup.h"
#include "state/state.h"

#include "projectile_mod/prjmod_fmj_ability.h"
#include "projectile_mod/prjmod_gravity.h"
#include "projectile_mod/prjmod_cloudburst.h"


#include "util.h"

void set_powerup_defaults(struct state_t* gst, struct powerup_shop_t* shop) {

    shop->powerups[POWERUP_ACCURACY_BOOST] = (struct powerup_t) {
        .type = POWERUP_ACCURACY_BOOST,
        .xp_cost = 50,
        .xp_cost_mult = 1.8,
        .max_level = 3,
        .name = "Accuracy boost\0"
    };
    shop->powerups[POWERUP_FASTER_FIRERATE] = (struct powerup_t) {
        .type = POWERUP_FASTER_FIRERATE,
        .xp_cost = 80,
        .xp_cost_mult = 1.85,
        .max_level = 5,
        .name = "Faster firerate\0"
    };
    shop->powerups[POWERUP_MAX_HEALTH_BOOST] = (struct powerup_t) {
        .type = POWERUP_MAX_HEALTH_BOOST,
        .xp_cost = 60,
        .xp_cost_mult = 1.5,
        .max_level = 3,
        .name = "Max health boost\0"
    };
    shop->powerups[POWERUP_MAX_ARMOR_BOOST] = (struct powerup_t) {
        .type = POWERUP_MAX_ARMOR_BOOST,
        .xp_cost = 75,
        .xp_cost_mult = 1.5,
        .max_level = 3,
        .name = "Max armor boost\0"
    };
    shop->powerups[POWERUP_MOVEMENT_BOOST] = (struct powerup_t) {
        .type = POWERUP_MOVEMENT_BOOST,
        .xp_cost = 100,
        .xp_cost_mult = 1.5,
        .max_level = 3,
        .name = "Movement boost\0"
    };
    shop->powerups[POWERUP_DAMAGE_BOOST] = (struct powerup_t) {
        .type = POWERUP_DAMAGE_BOOST,
        .xp_cost = 80,
        .xp_cost_mult = 2.0,
        .max_level = 3,
        .name = "Damage boost\0"
    };
    shop->powerups[POWERUP_PROJECTILE_SPEED_BOOST] = (struct powerup_t) {
        .type = POWERUP_PROJECTILE_SPEED_BOOST,
        .xp_cost = 70,
        .xp_cost_mult = 1.365,
        .max_level = 6,
        .name = "Projectile speed boost\0"
    };

    shop->powerups[POWERUP_BURST_FIRE] = (struct powerup_t) {
        .type = POWERUP_BURST_FIRE,
        .xp_cost = 350,
        .xp_cost_mult = 1.0,
        .max_level = 2,
        .name = "Burst fire\0"
    };
    shop->powerups[POWERUP_BIGGER_PROJECTILES] = (struct powerup_t) {
        .type = POWERUP_BIGGER_PROJECTILES,
        .xp_cost = 350,
        .xp_cost_mult = 1.2,
        .max_level = 3,
        .name = "Bigger projectiles\0"
    }; 

    shop->powerups[POWERUP_HEALTH_REGEN] = (struct powerup_t) {
        .type = POWERUP_HEALTH_REGEN,
        .xp_cost = 460,
        .xp_cost_mult = 1.0,
        .max_level = 3,
        .name = "Health regen\0"
    };

    shop->powerups[POWERUP_FMJPRJ_ABILITY] = (struct powerup_t) {
        .type = POWERUP_FMJPRJ_ABILITY,
        .xp_cost = 200,
        .xp_cost_mult = 1.2,
        .max_level = 3,
        .name = "FMJ Projectile ability\0"
    };

    shop->powerups[POWERUP_GRAVITY_PROJECTILES] = (struct powerup_t) {
        .type = POWERUP_GRAVITY_PROJECTILES,
        .xp_cost = 5000,
        .xp_cost_mult = 1.0,
        .max_level = 1,
        .name = "Gravity projectiles\0"
    };

    shop->powerups[POWERUP_RECOIL_CONTROL] = (struct powerup_t) {
        .type = POWERUP_RECOIL_CONTROL,
        .xp_cost = 135,
        .xp_cost_mult = 2.25,
        .max_level = 4,
        .name = "Better recoil control\0"
    };

    shop->powerups[POWERUP_PRJ_CLOUDBURST] = (struct powerup_t) {
        .type = POWERUP_PRJ_CLOUDBURST,
        .xp_cost = 1000,
        .xp_cost_mult = 1.0,
        .max_level = 1,
        .name = "Projectile cloudburst\0"
    };
}

void apply_powerup(struct state_t* gst, struct player_t* player, int powerup_type) {


    if(gst->has_audio) {
        PlaySound(gst->sounds[POWERUP_SOUND]);
    }

    struct powerup_shop_t* shop = &player->powerup_shop;

    if(player->powerup_levels[powerup_type] >= shop->powerups[powerup_type].max_level) {
        fprintf(stderr, "\33[31m(ERROR) '%s': Powerup is already at max level\033[0m\n",
                __func__);
        return;
    }

    player->powerup_levels[powerup_type]++;

    switch(powerup_type) {

        case POWERUP_ACCURACY_BOOST:
            {
                player->weapon.accuracy += 0.25;
            }
            break;
        
        case POWERUP_FASTER_FIRERATE:
            {
                player->firerate -= 0.0135;
            }
            break;
    
        case POWERUP_MAX_HEALTH_BOOST:
            {
                player->max_health += 30;
                player_heal(gst, &gst->player, 100.0);
            }
            break;

        case POWERUP_MAX_ARMOR_BOOST:
            {
                player->max_armor += 3;
                player->armor_damage_dampen -= 0.01;
            }
            break;

        case POWERUP_MOVEMENT_BOOST:
            {
                player->walkspeed += 0.25;
                player->run_speed_mult += 0.5;
                player->dash_timer_max -= 1.0;
            }
            break;

        case POWERUP_DAMAGE_BOOST:
            {
                player->weapon.damage += 5.5;
            }
            break;

        case POWERUP_PROJECTILE_SPEED_BOOST:
            {
                player->weapon.prj_speed += 200;
            }
            break;

        case POWERUP_BURST_FIRE:
            {
            }
            break;

        case POWERUP_BIGGER_PROJECTILES:
            {
                const float factor = 1.15;
                player->weapon.prj_scale += factor;
                player->weapon.prj_hitbox_size.x += factor;
                player->weapon.prj_hitbox_size.y += factor;
                player->weapon.prj_hitbox_size.z += factor;
            }
            break;


        case POWERUP_RECOIL_CONTROL:
            {
                player->recoil_control += 0.175;
            }
            break;
        
        case POWERUP_HEALTH_REGEN:
            {
            }
            break;

        case POWERUP_FMJPRJ_ABILITY:
            if(!prjmod_exists(gst, PRJMOD_FMJ_ABILITY_ID)) {
                struct prjmod_t mod = (struct prjmod_t) {
                    .init_callback = prjmod_fmj__init,
                    .update_callback = prjmod_fmj__update,
                    .enemy_hit_callback = prjmod_fmj__enemy_hit,
                    .env_hit_callback = prjmod_fmj__env_hit
                };
                add_prjmod(gst, &mod, PRJMOD_FMJ_ABILITY_ID);
            }
            break;

        case POWERUP_GRAVITY_PROJECTILES:
            if(!prjmod_exists(gst, PRJMOD_GRAVITY_PRJ_ID)) {
                struct prjmod_t mod = (struct prjmod_t) {
                    .init_callback      = prjmod_gravity__init,
                    .update_callback    = prjmod_gravity__update,
                    .enemy_hit_callback = prjmod_gravity__enemy_hit,
                    .env_hit_callback   = prjmod_gravity__env_hit
                };
                add_prjmod(gst, &mod, PRJMOD_GRAVITY_PRJ_ID);
            }
            break;

        case POWERUP_PRJ_CLOUDBURST:
            if(!prjmod_exists(gst, PRJMOD_CLOUDBURST_ID)) {
                struct prjmod_t mod = (struct prjmod_t) {
                    .init_callback      = prjmod_cloudburst__init,
                    .update_callback    = prjmod_cloudburst__update,
                    .enemy_hit_callback = prjmod_cloudburst__enemy_hit,
                    .env_hit_callback   = prjmod_cloudburst__env_hit
                };
                add_prjmod(gst, &mod, PRJMOD_CLOUDBURST_ID);
            }
            break;
    }

}

static int get_new_offer_type(struct state_t* gst, struct powerup_shop_t* shop) {
    int offer_type = 0;

    int attemps = 0;
    const int max_attemps = 1000;
    while(attemps < max_attemps) {
        offer_type = GetRandomValue(0, MAX_POWERUP_TYPES-1);
        int offer_is_new = 1;

        for(int i = 0; i < NUM_POWERUP_OFFERS; i++) {
            if(shop->prev_offers[i].type == offer_type) {
                offer_is_new = 0;
                break;
            }
        }

        if(offer_is_new) {
            break;
        }

        printf("'%s': offer was not new, retrying\n", __func__);
    }

    return offer_type;
}

void update_powerup_shop_offers(struct state_t* gst) {
    struct powerup_shop_t* shop = &gst->player.powerup_shop;


    for(int i = 0; i < NUM_POWERUP_OFFERS; i++) {
        shop->offers[i] = shop->powerups[get_new_offer_type(gst, shop)];
    }
    
    memmove(shop->prev_offers, shop->offers, NUM_POWERUP_OFFERS * sizeof *shop->offers);
    gst->player.powerup_shop.selected_index = -1;
}




