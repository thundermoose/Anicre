#!/bin/sh

EGVFILE=$1

if [ 1 = 1 ]
then

./mfr /net/data1/saaf/trdens-sample/$EGVFILE.egv --td-dir=$EGVFILE.td
#./mfr ./he4_n2losat_nmax_0_hw_20.egv --td-dir=$EGVFILE.td

make   p-tables-anicr-$EGVFILE.td
make  pp-tables-anicr-$EGVFILE.td
make ppp-tables-anicr-$EGVFILE.td
make   n-tables-anicr-$EGVFILE.td
make  nn-tables-anicr-$EGVFILE.td
make nnn-tables-anicr-$EGVFILE.td

cd $EGVFILE.td

  ./p_tables_anicr >   p_table.txt
 ./pp_tables_anicr >  pp_table.txt
./ppp_tables_anicr > ppp_table.txt # there is an unresolved bug when num_ppp_cons = 0
  ./n_tables_anicr >   n_table.txt
 ./nn_tables_anicr >  nn_table.txt
./nnn_tables_anicr > nnn_table.txt

../cutcomb.pl \
    p_table.txt pp_table.txt ppp_table.txt \
    n_table.txt nn_table.txt nnn_table.txt > comb.txt
else
cd $EGVFILE.td
fi

#cd ..
#exit 0

for nforce in 1 2 3
do
    for blocksz in 1 8 16
    do
	time ../greedyorder $nforce $blocksz < comb.txt > \
	    greedy_${nforce}_${blocksz}.txt
    done
done

cd ..

