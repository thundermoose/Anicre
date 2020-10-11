#!/bin/bash

egvfile=$1

compile_directory=${egvfile}.jtd

run_directory=$SCRATCHDIR/anicr_runs/$compile_directory

echo "run_directory = $run_directory"

mkdir -p $run_directory
cd $run_directory
cp $MFR_PATH/$compile_directory/*.bin ./

$MFR_PATH/$compile_directory/p_tables_anicr > p_table.txt
$MFR_PATH/$compile_directory/pp_tables_anicr > pp_table.txt
$MFR_PATH/$compile_directory/ppp_tables_anicr > ppp_table.txt
$MFR_PATH/$compile_directory/n_tables_anicr > n_table.txt
$MFR_PATH/$compile_directory/nn_tables_anicr > nn_table.txt
$MFR_PATH/$compile_directory/nnn_tables_anicr > nnn_table.txt

$MFR_PATH/$compile_directory/p_inds_anicr > p_ind.txt
$MFR_PATH/$compile_directory/pp_inds_anicr > pp_ind.txt
$MFR_PATH/$compile_directory/ppp_inds_anicr > ppp_ind.txt
$MFR_PATH/$compile_directory/n_inds_anicr > n_ind.txt
$MFR_PATH/$compile_directory/nn_inds_anicr > nn_ind.txt
$MFR_PATH/$compile_directory/nnn_inds_anicr > nnn_ind.txt

$MFR_PATH/cutcomb.pl {p,pp,ppp,n,nn,nnn}_table.txt > comb.txt

for nforce in 1 2 3
do
	for blocksz in 1 8 16
	do
		time $MFR_PATH/greedyorder $nforce $blocksz < comb.txt >\
			greedy_${nforce}_${blocksz}.txt 
	done
done
