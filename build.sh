#!/bin/bash



files=$(find ./src/ -type f -name *.c)

name="out"
compiler_flag="-ggdb"

if [[ $1 == "o" || $2 == "o" ]]; then
    compiler_flag="-O3"
fi

echo $files | sed 's/ /\n/g'

if cc -o $name $files $compiler_flag \
    -Wall \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 ; then
 
    echo -en "\033[32m"
    ls -lh $name
    echo -en "\033[0m"

    if [[ $1 == "r" || $2 == "r" ]]; then
        ./$name
    fi

fi
