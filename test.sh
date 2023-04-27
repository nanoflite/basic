#!/usr/bin/env bash

# command="build/output/basic"
command="valgrind --leak-check=full --show-leak-kinds=all ./build/output/basic"

cat examples/autolist | while read bas; do echo $bas; $command $bas; done
