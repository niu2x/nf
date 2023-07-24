#!/bin/bash

run_test() {
	result=$(build/nf/nf < $1)
	expect=$(cat ${1/.nf/.expect})

	if [[ "$result" !=  "$expect" ]]; then
		echo "$1" error;
		exit 1;
	else
		echo "$1 success";
	fi;
}


ls tests/test-*.nf|while read item; do
	run_test $item;
done;