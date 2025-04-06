#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "prjmod.h"

#include "../state.h"
#include "../memory.h"




size_t add_prjmod(struct state_t* gst, struct prjmod_t* prjmod, size_t id) {
    size_t prjmod_index = 0;
    if(id >= MAX_PRJMOD_INDICES) {
        fprintf(stderr, 
                "\033[31m(ERROR) '%s': id is out of bounds,"
                " increase the 'prjmod_indices' array size in player.h\033[0m\n",
                __func__);
        goto error;
    }

    long int new_size = gst->player.num_prjmods;
    gst->player.prjmods = m_resize_array(
            gst->player.prjmods,
            sizeof *gst->player.prjmods,
            gst->player.num_prjmods,
            gst->player.num_prjmods+1,
            &new_size
            );

    prjmod_index = gst->player.num_prjmods;
    struct prjmod_t* ptr = &gst->player.prjmods[prjmod_index];
    memmove(ptr, prjmod, sizeof *ptr);

    ptr->id = id;
    gst->player.prjmod_indices[id] = (long int)prjmod_index; 
    gst->player.num_prjmods++;

    printf("'%s': prjmod_index=%li id=%li arraysize=%li\n",
            __func__, 
            prjmod_index, id, gst->player.num_prjmods);

error:
    return prjmod_index;
}

void rem_prjmod(struct state_t* gst, size_t prjmod_id) {
    if(gst->player.num_prjmods == 0) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Trying to remove prjmod but there is nothing in the array\033[0m\n",
                __func__);
        return;
    }
    if(prjmod_id >= MAX_PRJMOD_INDICES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': prjmod id is out of bounds\033[0m\n",
                __func__);
        return;
    }

    long int prjmod_index = gst->player.prjmod_indices[prjmod_id];
    if(prjmod_index >= (long int)gst->player.num_prjmods) {
        fprintf(stderr, "\033[31m(ERROR) '%s': prjmod_index is invalid at '%li' got %li\n"
                "prjmods array size = %li\033[0m\n",
                __func__, prjmod_id, prjmod_index, gst->player.num_prjmods);
        return;
    }

    if(prjmod_index < 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': prjmod %li doesnt exist\033[0m\n",
                __func__, prjmod_id);
        return;
    }


    for(size_t i = prjmod_index; i < gst->player.num_prjmods-1; i++) {
        gst->player.prjmods[i] = gst->player.prjmods[i+1];
        gst->player.prjmod_indices[gst->player.prjmods[i].id]--;
    }
    gst->player.prjmod_indices[prjmod_id] = -1;
    gst->player.num_prjmods--;

    printf("Removed prjmod %li from index %li\n", prjmod_id, prjmod_index);

}

int prjmod_exists(struct state_t* gst, size_t prjmod_id) {
    int result = 0;

    if(prjmod_id >= MAX_PRJMOD_INDICES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid id\033[0m\n",
                __func__);
        goto error;
    }

    long int prjmod_index = gst->player.prjmod_indices[prjmod_id];

    result = (prjmod_index >= 0);
error:
    return result;
}


void delete_prjmods(struct state_t* gst) {
    if(!gst->player.prjmods) {
        return;
    }

    for(size_t i = 0; i < MAX_PRJMOD_INDICES; i++) {
        gst->player.prjmod_indices[i] = -1;
    }

    free(gst->player.prjmods);
    gst->player.prjmods = NULL;
    gst->player.num_prjmods = 0;
    printf("\033[35m -> Deleted Projectile modifiers\033[0m\n");
}



void call_prjmods_update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        struct prjmod_t* prjmod = &gst->player.prjmods[i];
        if(prjmod->update_callback) {
            prjmod->update_callback(gst, psys, part);
        }
    }
}

void call_prjmods_init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        struct prjmod_t* prjmod = &gst->player.prjmods[i];
        if(prjmod->init_callback) {
            prjmod->init_callback(gst, psys, part);
        }
    }
}

void call_prjmods_env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        struct prjmod_t* prjmod = &gst->player.prjmods[i];
        if(prjmod->env_hit_callback) {
            prjmod->env_hit_callback(gst, psys, part, normal);
        }
    }
}

int call_prjmods_enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, 
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {
    int disable_prj = 1;
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        struct prjmod_t* prjmod = &gst->player.prjmods[i];
        if(prjmod->enemy_hit_callback) {
            disable_prj = prjmod->enemy_hit_callback(gst, psys, part, ent, hitbox, cancel_defdamage);
        }
    }
    return disable_prj;
}
