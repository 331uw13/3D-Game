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

    gst->player.prjmod_indices[id] = (long int)prjmod_index; 
    gst->player.num_prjmods++;

    printf("%li\n", gst->player.num_prjmods);
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


    if(prjmod_index > 0 && (prjmod_index+1 < gst->player.num_prjmods)) {
        memmove(
                &gst->player.prjmods[prjmod_index-1],
                &gst->player.prjmods[prjmod_index],
                (gst->player.num_prjmods - prjmod_index) * sizeof *gst->player.prjmods
                );
    }
    else
    if(prjmod_index == 0) {
        // Removing first element. Shift all elements from right to begining.
        memmove(
                &gst->player.prjmods[0],
                &gst->player.prjmods[1],
                (gst->player.num_prjmods - 1) * sizeof *gst->player.prjmods

                );
    }

    gst->player.prjmod_indices[prjmod_id] = -1;

    // Indices have been shifted.
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        if(gst->player.prjmod_indices[i] > 0) {
            gst->player.prjmod_indices[i]--;
        }
    }
    gst->player.num_prjmods--;

    printf("Removed prjmod %li from index %li\n", prjmod_id, prjmod_index);

}

void delete_prjmods(struct state_t* gst) {
    if(!gst->player.prjmods) {
        return;
    }

    free(gst->player.prjmods);
    gst->player.prjmods = NULL;

    printf("\033[35m -> Deleted Projectile modifiers\033[0m\n");
}


// TODO: this could be cleaned up a bit.

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

void call_prjmods_enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, struct enemy_t* ent) {
    for(size_t i = 0; i < gst->player.num_prjmods; i++) {
        struct prjmod_t* prjmod = &gst->player.prjmods[i];
        if(prjmod->enemy_hit_callback) {
            prjmod->enemy_hit_callback(gst, psys, part, ent);
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

