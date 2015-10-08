#!/bin/bash

SRCBASE=`basename $1`

MFRSTART=$(date +%s.%N)
time ./mfr $1 --td-dir=$SRCBASE
MFREND=$(date +%s.%N)

COMPILESTART=$(date +%s.%N)
make -C $SRCBASE dumpnlj -j 12
make -j 12 nn-anicr-$SRCBASE pp-anicr-$SRCBASE np-anicr-$SRCBASE
COMPILEEND=$(date +%s.%N)

cd $SRCBASE
ANICRSTART=$(date +%s.%N)
./nn_anicr
./pp_anicr
./np_anicr
ANICRSEND=$(date +%s.%N)
cd ..

WORK=$(echo "$MFREND - $MFRSTART + $ANICRSEND - $ANICRSTART" | bc)
COMPILE=$(echo "$COMPILEEND - $COMPILESTART" | bc)

echo "SRCBASE  $WORK  $COMPILE"