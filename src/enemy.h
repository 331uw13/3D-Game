#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>


struct state_t;


#define ENEMY_RANDOM_SEARCH_RADIUS 10.0

// enemy state
#define ENEMY_SEARCH 1


struct enemy_t {    
    long int id;
    int health;
    Vector3 position;
    Vector3 hitbox;

    Model model;
    int model_loaded;


    int dest_reached;
    Vector3 travel_dest;
    Vector3 travel_start; 
    float travelled;

    int state;
};

#define ENEMY_0_MAX_HEALTH 100


void free_enemyarray(struct state_t* gst);

struct enemy_t* create_enemy(
        struct state_t* gst, 
        const char* model_filepath,
        int texture_id,
        int max_health,
        Vector3 hitbox,
        Vector3 position
        );

void draw_enemy_hitbox(struct enemy_t* enemy);
void move_enemy(struct enemy_t* enemy, Vector3 position);
void update_enemy(struct state_t* gst, struct enemy_t* enemy);




#endif
