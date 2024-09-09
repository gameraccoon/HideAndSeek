#!/bin/bash

cases_list=$(./bin/AutoTests --list)

failed_cases=()

while IFS= read -r case_name; do
	echo Run test case $case_name
	./bin/AutoTests --case $case_name "$@"
	if [ $? -ne 0 ]; then
		failed_cases+=($case_name)
	fi
done <<< "$cases_list"

if [ ${#failed_cases[@]} -ne 0 ]; then
	echo ""

	for case_name in ${failed_cases[@]}; do
		echo "Test case '$case_name' failed"
	done

	exit 1
else
	echo "All test cases successfully completed"
fi
