#!/bin/bash

# create folder for results
DIR="../../data/results/$2-$3-$4-$5-$6-$7-$8"
if [ ! -d "$DIR" ]; then
	mkdir $DIR
fi

# run experiment
../../main "$@"
