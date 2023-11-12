#!/bin/bash

cases_list=$(./bin/AutoTests --list)

while IFS= read -r line; do
	echo Run test case $line
	./bin/AutoTests --case $line "$@"
done <<< "$cases_list"

