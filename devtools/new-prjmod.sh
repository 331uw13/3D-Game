#!/bin/bash


if [[ $1 == "" || $1 == "-help" || $1 == "--help" ]]; then
    echo "first argument is empty it should be the name of new projectile mod"
    exit
fi


source_file="$1.c"
header_file="$1.h"


header_fileto="../src/projectile_mod/$header_file"
source_fileto="../src/projectile_mod/$source_file"
if [ -f $header_fileto ]; then
    echo -en "\033[31m'$header_fileto' already exists abort!!\033[0m\n"
    exit
fi
if [ -f $source_fileto ]; then
    echo -en "\033[31m'$source_fileto' already exists abort!!\033[0m\n"
    exit
fi


echo -en "\033[90m ------- Source file -----------\033[0m\n"

tee $source_file << EOF
#include "$header_file"

void $1__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {

}

void $1__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {

}


void $1__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {

}

int $1__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {

    return 0;
}
EOF

echo -en "\033[90m ------- Header file -----------\033[0m\n"

tee $header_file << EOF
#ifndef PRJMOD_${1^^}_H
#define PRJMOD_${1^^}_H

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;
struct enemy_t;
struct hitbox_t;



void $1__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);
void $1__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part);

void $1__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal);
int $1__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage);



#endif
EOF

echo -en "\033[90m------------------------------\033[0m\n"
format="\033[35m"
echo -en "$format Accept changes and move to \033[4m'src/projectile_mod/'\033[0m$format directory? \033[1m(y/n): \033[0m"
read -t 30 accept_move


if [[ ${accept_move^^} == "Y" ]]; then
    mv $header_file $header_fileto
    mv $source_file $source_fileto
    echo -en "\033[32mDone.\033[0m\n"
else
    if [[ $header_file != "" ]]; then
        rm $header_file
    fi
    if [[ $source_file != "" ]]; then
        rm $source_file
    fi
fi

