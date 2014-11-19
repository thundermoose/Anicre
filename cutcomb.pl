#!/usr/bin/perl

use strict;
use warnings;

my %E_M_states = ();

my %E_M_E_M_conn = ();

my $maxE = ();
my $totM = ();
my $totP = ();

while (my $line = <>)
{
    if ($line =~ /^([pn])_E_M_PAIR\s+([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
	my $pn     = $1;
	my $E      = $2;
	my $M      = $3;
	my $states = $4;

	$E_M_states{"${pn}${E},${M}"} = $states;
    }
    elsif ($line =~ /^([pn]*)_CONN\s+([-\d]*)\s+([-\d]*)\s*->\s*([-\d]*)\s+([-\d]*)\s*:\s*dE=\s*([-\d]*)\s*dM=\s*([-\d]*)\s*:\s*([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
        my $pn     = $1;
	my $E1     = $2;
	my $M1     = $3;
	my $E2     = $4;
	my $M2     = $5;
	my $conn   = $10;

	$E_M_E_M_conn{"${pn}${E1},${M1},${E2},${M2}"} = $conn;

	# print $line;
    }
    elsif ($line =~ /^CFG_MAX_E:\s*([-\d]*)$/) { $maxE = $1; }
    elsif ($line =~ /^CFG_M:\s*([-\d]*)$/)     { $totM = $1; }
    elsif ($line =~ /^CFG_P:\s*([-\d]*)$/)     { $totP = $1; }
    else
    {
	# print "BAD: ".$line;
    }
}

foreach my $key (sort keys %E_M_states)
{
    # print "$key\n";

}

# produce all combinations of states up to limits

my $total_mp_states = 0;
my $max_mp_block = 0;

my %E_M_E_M_states = ();

print sprintf ("\n".
	       "*** mp-states ***\n".
	       "\n");
print sprintf ("%3s %3s  %3s %3s   ".
	       "%8s %8s  %8s\n".
	       "\n",
	       "Ep", "Mp", "En", "Mn",
	       "#mp-p", "#mp-n",
	       "#mp");

for (my $Ep = 0; $Ep <= $maxE; $Ep++)
{
    for (my $Mp = -20; $Mp <= 20; $Mp++)  # todo: fix the 20!
    {
	my $key_p = "${Ep},${Mp}";

	my $states_p = $E_M_states{"p".$key_p};

	if ($states_p)
	{
	    # parity is given by sum l;
	    # since energy goes 2*n+l, parity is given by E&1

	    for (my $En = ($Ep+$totP) & 1; $En <= $maxE - $Ep; $En += 2)
	    {
		my $Mn = $totM - $Mp;

		my $key_n = "${En},${Mn}";

		my $states_n = $E_M_states{"n".$key_n};

		if ($states_n)
		{
		    my $mp_states = $states_p * $states_n;

		    print sprintf ("%3d %3d  %3d %3d   ".
				   "%8d %8d  %8d\n",
				   $Ep, $Mp, $En, $Mn,
				   $states_p, $states_n,
				   $mp_states);

		    $total_mp_states += $mp_states;

		    if ($mp_states > $max_mp_block) 
		    {
			$max_mp_block = $mp_states;
		    }

		    $E_M_E_M_states{"$key_p,$key_n"} = $states_p * $states_n;
		}
	    }
	}
    }
}

print sprintf ("\n".
	       "TOTAL-MP-STATES: %d\n".
	       "MAX-MP-BLOCK: %d".
	       "\n",
	       $total_mp_states,
	       $max_mp_block);

my %E_M_E_M_use_1n = ();
my %E_M_E_M_use_2n = ();
my %E_M_E_M_use_3n = ();

sub account_conn_use($$)
{
    my $key = shift;
    my $nforce = shift;

    if ($nforce <= 1) {
	my $old = $E_M_E_M_use_1n{$key};
	if (!$old) { $E_M_E_M_use_1n{$key} = 1; }
	else       { $E_M_E_M_use_1n{$key} = $old + 1; }
    }
    if ($nforce <= 2) {
	my $old = $E_M_E_M_use_2n{$key};
	if (!$old) { $E_M_E_M_use_2n{$key} = 1; }
	else       { $E_M_E_M_use_2n{$key} = $old + 1; }
    }
    if ($nforce <= 3) {
	my $old = $E_M_E_M_use_3n{$key};
	if (!$old) { $E_M_E_M_use_3n{$key} = 1; }
	else       { $E_M_E_M_use_3n{$key} = $old + 1; }
    }
}

my @E_M_E_M_states = sort keys %E_M_E_M_states;

sub pn_conn($$$)
{
    my $ptype = shift;
    my $ntype = shift;
    my $nforce = shift;

    my $total_conn = 0;

    print sprintf ("\n".
		   "*** Connections $ptype-$ntype ***\n".
		   "\n");
    print sprintf ("%3s %3s   %3s %3s   ".
		   "%3s %3s   %3s %3s   ".
		   "%8s %8s  ".
		   "%8s %8s  %12s\n".
		   "\n",
		   "Ep1", "Mp1", "En1", "Mn1",
		   "Ep2", "Mp2", "En2", "Mn2",
		   "#mp1", "#mp2",
		   "#conn-$ptype", "#conn-$ntype",
		   "conn");

    foreach my $key1 (@E_M_E_M_states)
    {
	my @EMEM1 = split /,/,$key1;

	my $Ep1 = $EMEM1[0];
	my $Mp1 = $EMEM1[1];
	my $En1 = $EMEM1[2];
	my $Mn1 = $EMEM1[3];

	my $states1 = $E_M_E_M_states{$key1};

	foreach my $key2 (@E_M_E_M_states)
	{
	    my @EMEM2 = split /,/,$key2;

	    my $Ep2 = $EMEM2[0];
	    my $Mp2 = $EMEM2[1];
	    my $En2 = $EMEM2[2];
	    my $Mn2 = $EMEM2[3];

	    my $states2 = $E_M_E_M_states{$key2};

	    my $dMp = $Mp2 - $Mp1;
	    my $dMn = $Mn2 - $Mn1;

	    my $keyp = "${ptype}${Ep1},${Mp1},${Ep2},${Mp2}";
	    my $keyn = "${ntype}${En1},${Mn1},${En2},${Mn2}";

	    my $conn_p = $E_M_E_M_conn{$keyp};
	    my $conn_n = $E_M_E_M_conn{$keyn};

	    if (!defined($conn_p)) {
		die "Connections $keyp undefined.";
	    }
	    if (!defined($conn_n)) {
		die "Connections $keyp undefined.";
	    }

	    if ($conn_p && $conn_n)
	    {
		# print "$key1  $key2  $dMp  $dMn  $conn_p  $conn_n\n";

		print sprintf ("%3d %3d   %3d %3d   ".
			       "%3d %3d   %3d %3d   ".
			       "%8d %8d  ".
			       "%8d %8d  %12d\n",
			       $Ep1, $Mp1, $En1, $Mn1,
			       $Ep2, $Mp2, $En2, $Mn2,
			       $states1, $states2,
			       $conn_p, $conn_n,
			       $conn_p * $conn_n);

		$total_conn += $conn_p * $conn_n;

		account_conn_use($keyp, $nforce);
		account_conn_use($keyn, $nforce);
	    }
	}    
    }

    print sprintf ("\n".
		   "CONNECTIONS-%s-%s: %d\n".
		   "\n",
		   $ptype, $ntype, $total_conn);
}

pn_conn("p","n",2);

pn_conn("pp","n",3);
pn_conn("p","nn",3);

sub dia_conn($$)
{
    my $xtype = shift;
    my $nforce = shift;

    my $total_conn = 0;

    my $ytype;

    if ($xtype =~ /^p+$/) { $ytype = "n"; } else { $ytype = "p"; }

    print sprintf ("\n".
		   "*** Connections dia-$xtype ***\n".
		   "\n");
    print sprintf ("%3s %3s   %3s %3s   ".
		   "%3s %3s   ".
		   "%8s  ".
		   "%8s %8s  %12s\n".
		   "\n",
		   "Ep1", "Mp1",
		   "Ep2", "Mp2",
		   "En", "Mn",
		   "#mp1",
		   "#conn-$xtype", "#mp-$ytype",
		   "conn");

    foreach my $key1 (@E_M_E_M_states)
    {
	my @EMEM1 = split /,/,$key1;

	my $Ep1 = $EMEM1[0];
	my $Mp1 = $EMEM1[1];
	my $En1 = $EMEM1[2];
	my $Mn1 = $EMEM1[3];

	my $states1 = $E_M_E_M_states{$key1};

      key2iter:
	foreach my $key2 (@E_M_E_M_states)
	{
	    my @EMEM2 = split /,/,$key2;

	    my $Ep2 = $EMEM2[0];
	    my $Mp2 = $EMEM2[1];
	    my $En2 = $EMEM2[2];
	    my $Mn2 = $EMEM2[3];

	    my $states2 = $E_M_E_M_states{$key2};

	    my $conn_x;
	    my $states_y;

	    my $keyx;

	    if ($xtype =~ /^p+$/)
	    {
		# n states must match
		if ($En1 != $En2 || $Mn1 != $Mn2) { next key2iter; }
		#
		$keyx = "${xtype}${Ep1},${Mp1},${Ep2},${Mp2}";
		$states_y = $E_M_states{"n${En1},${Mn1}"};
	    }
	    else
	    {
		# p states must match
		if ($Ep1 != $Ep2 || $Mp1 != $Mp2) { next key2iter; }
		#
		$keyx = "${xtype}${En1},${Mn1},${En2},${Mn2}";
		$states_y = $E_M_states{"p${Ep1},${Mp1}"};
	    }

	    $conn_x = $E_M_E_M_conn{$keyx};

	    if (!defined($conn_x)) {
		die "Connections $keyx undefined.";
	    }

	    if ($conn_x)
	    {
		# print "$key1  $key2  $dMp  $dMn  $conn_p  $conn_n\n";

		print sprintf ("%3d %3d   %3d %3d   ".
			       "%3s %3s   ".
			       "%8d  ".
			       "%8d %8d  %12d\n",
			       $Ep1, $Mp1,
			       $Ep2, $Mp2,
			       $En1, $Mn1,
			       $states1,
			       $conn_x, $states_y,
			       $conn_x);

		$total_conn += $conn_x * $states_y;

		account_conn_use($keyx, $nforce);
	    }
	}
    }

    print sprintf ("\n".
		   "CONNECTIONS-DIA-%s: %d\n".
		   "\n",
		   $xtype, $total_conn);
}

dia_conn("p",1);
dia_conn("n",1);

dia_conn("pp",2);
dia_conn("nn",2);

dia_conn("ppp",3);
dia_conn("nnn",3);

my $max_conn_len_1n = 0;
my $sum_conn_len_1n = 0;

foreach my $key (sort keys %E_M_E_M_use_1n)
{
    my $len = $E_M_E_M_conn{$key};
    $sum_conn_len_1n += $len;
    if ($len > $max_conn_len_1n) { $max_conn_len_1n = $len; }
}

my $max_conn_len_2n = 0;
my $sum_conn_len_2n = 0;

foreach my $key (sort keys %E_M_E_M_use_2n)
{
    my $len = $E_M_E_M_conn{$key};
    $sum_conn_len_2n += $len;
    if ($len > $max_conn_len_2n) { $max_conn_len_2n = $len; }
}

my $max_conn_len_3n = 0;
my $sum_conn_len_3n = 0;

foreach my $key (sort keys %E_M_E_M_use_3n)
{
    my $len = $E_M_E_M_conn{$key};
    $sum_conn_len_3n += $len;
    if ($len > $max_conn_len_3n) { $max_conn_len_3n = $len; }
}

printf sprintf("\n".
	       "SUM-CONN-LEN-1N: %d\n".
	       "MAX-CONN-LEN-1N: %d\n".
	       "SUM-CONN-LEN-2N: %d\n".
	       "MAX-CONN-LEN-2N: %d\n".
	       "SUM-CONN-LEN-3N: %d\n".
	       "MAX-CONN-LEN-3N: %d\n".
	       "\n",
	       $sum_conn_len_1n,
	       $max_conn_len_1n,
	       $sum_conn_len_2n,
	       $max_conn_len_2n,
	       $sum_conn_len_3n,
	       $max_conn_len_3n);

