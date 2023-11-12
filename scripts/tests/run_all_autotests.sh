#!/bin/bash

cases_list=$(./bin/AutoTests --list)

while IFS= read -r line; do
    ./bin/AutoTests --case $line "$@"
done <<< "$cases_list"

