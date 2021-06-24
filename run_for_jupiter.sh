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

$MFR_PATH/cutcomb.pl {p,pp,ppp,n,nn,nnn}_table.txt > comb.txt
num_proton_mp_states=$(grep -P -o "(?<=_num_mp_cut_E_M = ).*" p_table.txt)
num_neutron_mp_states=$(grep -P -o "(?<=_num_mp_cut_E_M = ).*" n_table.txt)
: ${num_cpu_cores:=$(grep ^cpu\\scores /proc/cpuinfo | uniq | awk '{print $4}')}
echo " "
echo "$num_proton_mp_states $num_neutron_mp_states"
launch_p_case () {
	/usr/bin/time -v $MFR_PATH/$compile_directory/p_inds_anicr $1 $2 > p_ind_$1-$2.txt 2>1
}

launch_n_case () {
	/usr/bin/time -v $MFR_PATH/$compile_directory/n_inds_anicr $1 $2 > n_ind_$1-$2.txt 2>1
}

launch_pp_case () {
	/usr/bin/time -v $MFR_PATH/$compile_directory/pp_inds_anicr $1 $2 > pp_ind_$1-$2.txt 2>1
}

launch_nn_case () {
	/usr/bin/time -v $MFR_PATH/$compile_directory/nn_inds_anicr $1 $2 > nn_ind_$1-$2.txt 2>1
}
launch_ppp_case () {
	/usr/bin/time -v $MFR_PATH/$compile_directory/ppp_inds_anicr $1 $2 > ppp_ind_$1-$2.txt 2>1
}

launch_nnn_case () {
	/usr/bin/tim -v $MFR_PATH/$compile_directory/nnn_inds_anicr $1 $2 > nnn_ind_$1-$2.txt 2>1
}
num_running_threads=0
for m in $(seq 0 $((num_proton_mp_states - 1)))
do
	for n in $(seq 0 $((num_proton_mp_states -1)))
	do
		launch_p_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
for m in $(seq 0 $((num_neutron_mp_states - 1)))
do
	for n in $(seq 0 $((num_neutron_mp_states -1)))
	do
		launch_n_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
for m in $(seq 0 $((num_proton_mp_states - 1)))
do
	for n in $(seq 0 $((num_proton_mp_states -1)))
	do
		launch_pp_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
for m in $(seq 0 $((num_neutron_mp_states - 1)))
do
	for n in $(seq 0 $((num_neutron_mp_states -1)))
	do
		launch_nn_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
for m in $(seq 0 $((num_proton_mp_states - 1)))
do
	for n in $(seq 0 $((num_proton_mp_states -1)))
	do
		launch_ppp_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
for m in $(seq 0 $((num_neutron_mp_states - 1)))
do
	for n in $(seq 0 $((num_neutron_mp_states -1)))
	do
		launch_nnn_case $m $n &
		if [ $num_running_threads -eq $num_cpu_cores ]
		then
			num_running_threads=0
			wait $(jobs -p)
		else
			num_running_threads=$((num_running_threads+1))
		fi
	done
done
wait $(jobs -p)


for nforce in 1 2 3
do
	for blocksz in 1 8 16
	do
		time $MFR_PATH/greedyorder $nforce $blocksz < comb.txt >\
			greedy_${nforce}_${blocksz}.txt 
	done
done
