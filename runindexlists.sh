#!/bin/bash

EGVFILE=$1

./mfr /net/data1/saaf/trdens-sample/$EGVFILE.egv --td-dir=$EGVFILE.jupiter
./mfr /net/data1/saaf/trdens-sample/$EGVFILE.egv --dump=full > $EGVFILE.jupiter/$EGVFILE-basis.jupiter

for part in "n" "nn" "nnn" "p" "pp" "ppp"
do
    make $part-inds-anicr-$EGVFILE.jupiter &>$EGVFILE.jupiter/$part-compilation-log
    if [ $? -ne 0 ]
    then
	echo "compilation of anicr failed at " $part
	exit 1
    fi
done


cd $EGVFILE.jupiter
for part in "n" "nn" "nnn" "p" "pp" "ppp"
do
    ./$part"_inds_anicr" &> $part-execution-log
    if [ $? -ne 0 ]
    then
	echo "execution of anicr failed at " $part
	exit 1
    fi
done
cd ..

