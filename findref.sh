#!/bin/bash


files=$(find ./src/ -type f)

for sf in ${files[@]}; do
    if cat $sf | grep $1 > /dev/null; then
        echo -en "\033[32m\"$sf\"\033[0m <- contains \033[34m\"$1\"\033[0m\n"
    fi
done


