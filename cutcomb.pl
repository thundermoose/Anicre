#!/usr/bin/perl

use strict;
use warnings;

my %E_M_states = ();

my %V_E_M_combs = ();

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
    elsif ($line =~ /^([pn]*)_V_E_M_PAIR\s+([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
	my $pn     = $1;
	my $E      = $2;
	my $M      = $3;
	my $combs  = $4;

	$V_E_M_combs{"${pn}${E},${M}"} = $combs;

	# printf "xx ${pn}${E},${M}\n";
    }
    elsif ($line =~ /^([pn]*)_CONN\s+([-\d]*)\s+([-\d]*)\s*->\s*([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*:\s*dE=\s*([-\d]*)\s*dM=\s*([-\d]*)\s*:\s*([-\d]*)\s+([-\d]*)\s*:\s*([-\d]*)\s*$/)
    {
        my $pn     = $1;
	my $E1     = $2;
	my $M1     = $3;
	my $E2     = $4;
	my $M2     = $5;
	my $D1     = $6;
	my $conn   = $11;

	$E_M_E_M_conn{"${pn}${E1},${M1},${E2},${M2},${D1}"} = $conn;

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

my %V_E_M_E_M_use = ();
my %V_E_M_use = ();

sub pn_conn($$$)
{
    my $ptype = shift;
    my $ntype = shift;
    my $nforce = shift;

    my $total_conn = 0;

    print sprintf ("\n".
		   "*** Connections $ptype-$ntype ***\n".
		   "\n");
    print sprintf ("%3s %3s   %3s %3s   %3s   ".
		   "%3s %3s   %3s %3s   %3s   ".
		   "%8s %8s  ".
		   "%8s %8s  %12s\n".
		   "\n",
		   "Ep1", "Mp1", "En1", "Mn1", "Dp1",
		   "Ep2", "Mp2", "En2", "Mn2", "Dn1",
		   "#mp1", "#mp2",
		   "#conn-$ptype", "#conn-$ntype",
		   "conn");

    for (my $i1 = 0; $i1 <= $#E_M_E_M_states; $i1++)
    {
	my $key1 = $E_M_E_M_states[$i1];
	my @EMEM1 = split /,/,$key1;

	my $Ep1 = $EMEM1[0];
	my $Mp1 = $EMEM1[1];
	my $En1 = $EMEM1[2];
	my $Mn1 = $EMEM1[3];

	my $states1 = $E_M_E_M_states{$key1};

	for (my $i2 = $i1; $i2 <= $#E_M_E_M_states; $i2++)
	{
	    my $key2 = $E_M_E_M_states[$i2];
	    my @EMEM2 = split /,/,$key2;

	    my $Ep2 = $EMEM2[0];
	    my $Mp2 = $EMEM2[1];
	    my $En2 = $EMEM2[2];
	    my $Mn2 = $EMEM2[3];

	    my $states2 = $E_M_E_M_states{$key2};

	    my $dEp = $Ep2 - $Ep1;
	    my $dEn = $En2 - $En1;

	    my $dMp = $Mp2 - $Mp1;
	    my $dMn = $Mn2 - $Mn1;

	    for (my $Dp1 = 0; $Dp1 <= $Ep1; $Dp1++)
	    {
		for (my $Dn1 = 0; $Dn1 <= $En1; $Dn1++)
		{
		    my $keyp = "${ptype}${Ep1},${Mp1},${Ep2},${Mp2},${Dp1}";
		    my $keyn = "${ntype}${En1},${Mn1},${En2},${Mn2},${Dn1}";

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
			my $Vkeyp = "${ptype}${dEp},${dMp},${Dp1}";
			my $Vkeyn = "${ntype}${dEn},${dMn},${Dn1}";

			# print "$key1  $key2  $dMp  $dMn  $conn_p  $conn_n\n";

			my $conn = $conn_p * $conn_n;

			print sprintf ("%3d %3d   %3d %3d   %3d   ".
				       "%3d %3d   %3d %3d   %3d   ".
				       "%8d %8d  ".
				       "%8d %8d  %12d\n",
				       $Ep1, $Mp1, $En1, $Mn1, $Dp1,
				       $Ep2, $Mp2, $En2, $Mn2, $Dn1,
				       $states1, $states2,
				       $conn_p, $conn_n,
				       $conn);

			$total_conn += $conn * ($i1 == $i2 ? 1 : 2);

			account_conn_use($keyp, $nforce);
			account_conn_use($keyn, $nforce);

			$V_E_M_E_M_use{$Vkeyp."_".$Vkeyn} = 1;
		    }
		}
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
		   "%3s %3s   %3s   ".
		   "%8s  ".
		   "%8s %8s  %12s\n".
		   "\n",
		   "Ep1", "Mp1",
		   "Ep2", "Mp2",
		   "En", "Mn",
		   "D-$xtype",
		   "#mp1",
		   "#conn-$xtype", "#mp-$ytype",
		   "conn");

    for (my $i1 = 0; $i1 <= $#E_M_E_M_states; $i1++)
    {
	my $key1 = $E_M_E_M_states[$i1];
	my @EMEM1 = split /,/,$key1;

	my $Ep1 = $EMEM1[0];
	my $Mp1 = $EMEM1[1];
	my $En1 = $EMEM1[2];
	my $Mn1 = $EMEM1[3];

	my $states1 = $E_M_E_M_states{$key1};

      key2iter:
	for (my $i2 = $i1; $i2 <= $#E_M_E_M_states; $i2++)
	{
	    my $key2 = $E_M_E_M_states[$i2];
	    my @EMEM2 = split /,/,$key2;

	    my $Ep2 = $EMEM2[0];
	    my $Mp2 = $EMEM2[1];
	    my $En2 = $EMEM2[2];
	    my $Mn2 = $EMEM2[3];

	    my $states2 = $E_M_E_M_states{$key2};

	    my $dEp = $Ep2 - $Ep1;
	    my $dEn = $En2 - $En1;

	    my $dMp = $Mp2 - $Mp1;
	    my $dMn = $Mn2 - $Mn1;

	    my $conn_x;
	    my $states_y;
	    my $Dxmax;

	    my $keyx_1;
	    my $Vkeyx_1;

	    if ($xtype =~ /^p+$/)
	    {
		# n states must match
		if ($En1 != $En2 || $Mn1 != $Mn2) { next key2iter; }
		#
		$keyx_1 = "${xtype}${Ep1},${Mp1},${Ep2},${Mp2}";
		$states_y = $E_M_states{"n${En1},${Mn1}"};
		$Dxmax = ${Ep1};
		$Vkeyx_1 = "${xtype}${dEp},${dMp}";
	    }
	    else
	    {
		# p states must match
		if ($Ep1 != $Ep2 || $Mp1 != $Mp2) { next key2iter; }
		#
		$keyx_1 = "${xtype}${En1},${Mn1},${En2},${Mn2}";
		$states_y = $E_M_states{"p${Ep1},${Mp1}"};
		$Dxmax = ${En1};
		$Vkeyx_1 = "${xtype}${dEn},${dMn}";
	    }

	    for (my $Dx1 = 0; $Dx1 <= $Dxmax; $Dx1++)
	    {
		my $keyx = $keyx_1.",${Dx1}";

		$conn_x = $E_M_E_M_conn{$keyx};

		if (!defined($conn_x)) {
		    die "Connections $keyx undefined.";
		}

		if ($conn_x)
		{
		    my $Vkeyx = $Vkeyx_1.",${Dx1}";

		    # print "$key1  $key2  $dMp  $dMn  $conn_p  $conn_n\n";

		    my $conn = $conn_x * $states_y;

		    print sprintf ("%3d %3d   %3d %3d   ".
				   "%3d %3d   %3d".
				   "%8d  ".
				   "%8d %8d  %12d\n",
				   $Ep1, $Mp1,
				   $Ep2, $Mp2,
				   $En1, $Mn1,
				   $Dx1,
				   $states1,
				   $conn_x, $states_y,
				   $conn_x * $states_y);

		    $total_conn += $conn * ($i1 == $i2 ? 1 : 2);

		    account_conn_use($keyx, $nforce);

		    $V_E_M_use{$Vkeyx} = 1;
		}
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

sub num_V_ani_cre($)
{
    my $Vkey = shift;

    if (!($Vkey =~ /([pn]*)([-\d]*),([-\d]*),([-\d]*)/)) {
	die "Bad Vkey $Vkey";
    }

    my $xtype = $1;
    my $dEx = $2;
    my $dMx = $3;
    my $Dx  = $4;

    printf sprintf ("%-3s %3d %3d %3d\n",
		    $xtype, $dEx, $dMx, $Dx);

    # so we want do do dEx, dMx in total, by going Dx down

    my $sumcomb = 0;

    for (my $Mani = -20; $Mani <= 20; $Mani++)
    {
	my $Eani = $Dx;

	my $Ecre = $Dx   + $dEx;
	my $Mcre = $Mani + $dMx;

	my $keyani = "${xtype}${Eani},${Mani}";
	my $keycre = "${xtype}${Ecre},${Mcre}";

	my $comb_ani = $V_E_M_combs{$keyani};
	my $comb_cre = $V_E_M_combs{$keycre};

	# printf "  $keyani  $keycre\n";

	if (defined($comb_ani) && defined($comb_cre))
	{
	    my $comb = $comb_ani * $comb_cre;

	    printf sprintf ("%3d %3d  %3d %3d : %10d %10d : %10d\n",
			    $Eani, $Mani, $Ecre, $Mcre,
			    $comb_ani, $comb_cre, $comb);

	    $sumcomb += $comb;
	}
    }

    return (length($xtype),$sumcomb);
}

my @sum_Vc_size = (0, 0, 0);
my @max_Vc_size = (0, 0, 0);

foreach my $Vkey (sort keys %V_E_M_E_M_use)
{
    my @EMEM = split /_/,$Vkey;

    my ($nump,$numVp) = num_V_ani_cre($EMEM[0]);
    my ($numn,$numVn) = num_V_ani_cre($EMEM[1]);

    my $sizeV = $numVp * $numVn;
    my $order = $nump+$numn;

    printf sprintf("%-20s  %d  %10d %10d  %10d\n",
		   $Vkey, $order, $numVp, $numVn, $sizeV);

    $sum_Vc_size[$order-1] += $sizeV;
    if ($sizeV > $max_Vc_size[$order-1]) {
	$max_Vc_size[$order-1] = $sizeV;
    }
}

my @sum_Vx_size = (0, 0, 0);
my @max_Vx_size = (0, 0, 0);

foreach my $Vkey (sort keys %V_E_M_use)
{
    my ($numx,$numVx) = num_V_ani_cre($Vkey);

    my $sizeV = $numVx;
    my $order = $numx;

    printf sprintf("%-20s  %d  %10d\n",
		   $Vkey, $order, $sizeV);

    $sum_Vx_size[$order-1] += $sizeV;
    if ($sizeV > $max_Vx_size[$order-1]) {
	$max_Vx_size[$order-1] = $sizeV;
    }
}

print "\n";
for (my $order = 1; $order <= 3; $order++)
{
    print sprintf ("SUM-Vc-SIZE-%dN: %d\n".
		   "MAX-Vc-SIZE-%dN: %d\n",
		   $order, $sum_Vc_size[$order-1],
		   $order, $max_Vc_size[$order-1]);
    print sprintf ("SUM-Vx-SIZE-%dN: %d\n".
		   "MAX-Vx-SIZE-%dN: %d\n",
		   $order, $sum_Vx_size[$order-1],
		   $order, $max_Vx_size[$order-1]);
}
print "\n";
