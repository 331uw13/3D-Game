#!/bin/bash



echo " --- Create empty particle system C files ---"
echo "Do not use special characters (,/([=* etc..) or spaces."

read -p "Particle system name: " name_input

include_file=${name_input}_psys.h
source_file=${name_input}_psys.c

echo -en "\033[90m$include_file, $source_file\033[0m\n"

if [ -f "./src/particle_systems/$include_file" ]; then
    echo -e "\033[31m$include_file Already exists.\033[0m"
    exit
fi

if [ -f "./src/particle_systems/$source_file" ]; then
    echo -e "\033[31m$source_file Already exists.\033[0m"
    exit
fi


default_init_name="${name_input}_psys_init"
default_update_name="${name_input}_psys_update"
read -p "Particle Init function name ($default_init_name): " init_func_name
read -p "Particle Update function name ($default_update_name): " update_func_name

init_func_name=${init_func_name:-$default_init_name}
update_func_name=${update_func_name:-$default_update_name}

echo ""
echo -e "\033[90mInit function: \033[34m$init_func_name"
echo -e "\033[90mUpdate function: \033[34m$update_func_name\033[0m"
read -p "Everything is correct? (y/n): " yesorno
echo ""

if [ ${yesorno^^} != "Y" ]; then
    echo "exit."
    exit
fi



name=${name_input^^}
def=${name}_H

echo "------------------------------"
tee ./src/particle_systems/$include_file << EOF
#ifndef $def
#define $def

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;

// PARTICLE UPDATE
void $update_func_name(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);

// PARTICLE INITIALIZATION
void $init_func_name(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
);


#endif
EOF

echo "------------------------------"

tee ./src/particle_systems/$source_file << EOF
#include "$include_file"

#include <raymath.h>


// PARTICLE UPDATE
void $update_func_name(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    // ...
}

// PARTICLE INITIALIZATION
void $init_func_name(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    // ...
}

EOF
echo "------------------------------"





