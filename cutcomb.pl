#!/usr/bin/perl

use strict;
use warnings;

my %E_M_states = ();

my %E_M_E_M_conn = ();

while (my $line = <>)
{
    if ($line =~ /^E_M_PAIR\s+([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
	my $E      = $1;
	my $M      = $2;
	my $states = $3;

	$E_M_states{"${E},${M}"} = $states;
    }

    if ($line =~ /^CONN\s+([-\d]*)\s+([-\d]*)\s*->\s*([-\d]*)\s+([-\d]*)\s*:\s*dE=([-\d]*)\s*dM=\s*([-\d]*)\s*:\s*([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
	my $E1     = $1;
	my $M1     = $2;
	my $E2     = $3;
	my $M2     = $4;
	my $conn   = $9;

	$E_M_E_M_conn{"${E1},${M1},${E2},${M2}"} = $conn;

	# print $line;	
    }
}

foreach my $key (sort keys %E_M_states)
{
    print "$key\n";

}

my $maxE = 14;
my $totM = 2;
my $totP = 0;

# produce all combinations of states up to limits

my $total_mp_states = 0;

my %E_M_E_M_states = ();

for (my $Ep = 0; $Ep <= $maxE; $Ep++)
{
    for (my $Mp = -20; $Mp <= 20; $Mp++)  # todo: fix the 20!
    {
	my $key_p = "${Ep},${Mp}";

	my $states_p = $E_M_states{$key_p};

	if ($states_p)
	{
	    # parity is given by sum l;
	    # since energy goes 2*n+l, parity is given by E&1

	    for (my $En = ($Ep+$totP) & 1; $En <= $maxE - $Ep; $En += 2)
	    {
		my $Mn = $totM - $Mp;

		my $key_n = "${En},${Mn}";

		my $states_n = $E_M_states{$key_n};

		if ($states_n)
		{
		    print "$key_p  $key_n  $states_p  $states_n\n";

		    $total_mp_states += $states_p * $states_n;

		    $E_M_E_M_states{"$key_p,$key_n"} = $states_p * $states_n;
		}
	    }
	}
    }
}

print sprintf ("Total mp states: %d\n", $total_mp_states);

my $total_conn = 0;

foreach my $key1 (sort keys %E_M_E_M_states)
{
    my @EMEM1 = split /,/,$key1;

    my $Ep1 = $EMEM1[0];
    my $Mp1 = $EMEM1[1];
    my $En1 = $EMEM1[2];
    my $Mn1 = $EMEM1[3];

    foreach my $key2 (sort keys %E_M_E_M_states)
    {
	my @EMEM2 = split /,/,$key2;

	my $Ep2 = $EMEM2[0];
	my $Mp2 = $EMEM2[1];
	my $En2 = $EMEM2[2];
	my $Mn2 = $EMEM2[3];

	my $dMp = $Mp2 - $Mp1;
	my $dMn = $Mn2 - $Mn1;

	my $conn_p = $E_M_E_M_conn{"${Ep1},${Mp1},${Ep2},${Mp2}"};
	my $conn_n = $E_M_E_M_conn{"${En1},${Mn1},${En2},${Mn2}"};

	if ($conn_p && $conn_n)
	{
	    # print "$key1  $key2  $dMp  $dMn  $conn_p  $conn_n\n";

	    print sprintf ("%3d %3d  %3d %3d    %3d %3d  %3d %3d   ".
			   "%8d %8d  %12d\n",
			   $Ep1, $Mp1, $En1, $Mn1,
			   $Ep2, $Mp2, $En2, $Mn2,
			   $conn_p, $conn_n,
			   $conn_p * $conn_n);

	    $total_conn += $conn_p * $conn_n;
	}
    }    
}

print sprintf ("Total connections: %d\n", $total_conn);

