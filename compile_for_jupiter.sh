#!/bin/bash

egvfile=$1

compile_directory=${egvfile}.jtd

./mfr $WAVEFUNCTION_PATH/$egvfile.egv --td-dir=$compile_directory

interaction_types="n nn nnn p pp ppp"

for interaction_type in $interaction_types
do
	make $interaction_type-tables-anicr-$compile_directory
	make $interaction_type-inds-anicr-$compile_directory
done


