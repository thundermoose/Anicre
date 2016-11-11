#!/bin/sh

EGVFILE=$1

./mfr /net/data1/saaf/trdens-sample/$EGVFILE.egv --td-dir=$EGVFILE.td

make nn-inds-anicr-$EGVFILE.td
make np-inds-anicr-$EGVFILE.td
make pp-inds-anicr-$EGVFILE.td

cd $EGVFILE.td
./nn_inds_anicr
./np_inds_anicr
./pp_inds_anicr
mkdir -p $HOME/rundirectories/$EGVFILE.run/
mkdir -p $HOME/rundirectories/$EGVFILE.run/indexlists_2bf/
mv nn_inds_index_list $HOME/rundirectories/$EGVFILE.run/indexlists_2bf/
mv np_inds_index_list $HOME/rundirectories/$EGVFILE.run/indexlists_2bf/
mv pp_inds_index_list $HOME/rundirectories/$EGVFILE.run/indexlists_2bf/
