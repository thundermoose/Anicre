#!/usr/bin/perl                                                                 
use strict;
use warnings;

my @arraysz = ();
my %cblockids = ();

my @arrayblocks = ();
my @cblockarrays = ();

my $numcblock = 0;

while (my $line = <>)
{
    if ($line =~ /#\s*ARRAY\s*:\s*([\d]+)\s*=\s*([\d]+)\s*$/)
    {
	$arraysz[$1] = $2;
	$arrayblocks[$1] = { };
    }
    elsif ($line =~ /#\s*CALCBLOCK\s*:\s*([\d\s,]+?)\s*$/)
    {
	my $block = $1;

	my @block = split /\s*,\s*/,$block;

	$numcblock++;

	$cblockarrays[$numcblock] = \@block;
	$cblockids{$numcblock} = 1;
    }
}

print sprintf ("NUM-ARRAYS: %d\n", $#arraysz);
print sprintf ("NUM-BLOCKS: %d\n", scalar keys %cblockids);

my $sumraworder = 0;

foreach my $cblockid (keys %cblockids)
{
    # print join(',',@{$cblock})."\n";

    my $cblockref = $cblockarrays[$cblockid];

    for (my $i = 0; $i <= $#{$cblockref}; $i++)
    {
	my $array = $cblockref->[$i];
	# print "$i : $array : $arraysz[$array]\n";

	$sumraworder += $arraysz[$array];

	my $href = $arrayblocks[$array];

	$href->{$cblockid} = 1;
    }
}

print sprintf ("RAW-LOAD-SIZE: %d\n", $sumraworder);

# We employ a greedy approach to choosing blocks to calculate.  When
# nothing else applies, (and at startup) we begin with the totally
# largest block (i.e. largest sum array size) When selecting a block
# to move to, we choose the one that reuses the most information, and
# of those possible, the one that loads the most.  I.e., we overall
# begin with large arrays.

my $sumgreedyload = 0;

while (scalar keys %cblockids)
{
    # We are at start, or have no better idea.  Find largest one.
    # Expensive to evaluate

    my $largestid = ();
    my $largestsz = 0;

    foreach my $cblockid (keys %cblockids)
    {
	my $cblockref = $cblockarrays[$cblockid];

	my $sz = 0;

	for (my $i = 0; $i <= $#{$cblockref}; $i++)
	{
	    my $array = $cblockref->[$i];
	    $sz += $arraysz[$array];
	}

	if ($sz > $largestsz)
	{
	    $largestid = $cblockid;
	    $largestsz = $sz;
	}
    }

    print "$largestid : $largestsz\n";

    my $currentid = $largestid;

    $sumgreedyload += $largestsz;

    delete $cblockids{$currentid};

    # Now try to find a block which reuses some information

  find_sibling:
    for ( ; ; )
    {
	my $curblockref = $cblockarrays[$currentid];

	my @siblings = ();

	my $cursz = 0;

	for (my $i = 0; $i <= $#{$curblockref}; $i++)
        {
	    my $array = $curblockref->[$i];

	    $cursz += $arraysz[$array];

	    my $href = $arrayblocks[$array];

	    # print "$array: \n";

	    my @arraysiblings = keys %{$href};

	    # print join(':',@arraysiblings)."\n";

	    push @siblings, @arraysiblings;
        }

	my %siblings = map { $_, 1 } @siblings;
	@siblings = keys %siblings;

	# print join(':',@siblings)."\n";

	# Now, for each sibling, find which arrays are actually common,
	# and sum up the size of those.

	my %curarrays = map { $_, 1 } @{$curblockref};

	my $bestblockid = ();
	my $bestunloadsz = 0;
	my $bestloadsz = 0;

      try_sibling:
	foreach my $siblingid (@siblings)
	{
	    if (!$cblockids{$siblingid}) {
		# already handled
		next try_sibling;
	    }

	    my $sibblockref = $cblockarrays[$siblingid];

	    my $reusesz = 0;
	    my $loadsz = 0;

	    for (my $i = 0; $i <= $#{$sibblockref}; $i++)
	    {
		my $array = $sibblockref->[$i];

		if ($curarrays{$array})
		{
		    # common
		    $reusesz += $arraysz[$array];
		}
		else
		{
		    $loadsz += $arraysz[$array];
		}
	    }

	    my $unloadsz = $cursz - $reusesz;

#	    if (!$bestblockid ||
#		$unloadsz <= $bestunloadsz)
#	    {
#		if (!$bestblockid ||
#		    ($unloadsz > $bestunloadsz) ||
#		    ($unloadsz == $bestunloadsz &&
#		     $loadsz > $bestloadsz))
#		{
#		    $bestblockid = $siblingid;
#		    $bestunloadsz = $unloadsz;
#		    $bestloadsz = $loadsz;
#		}
#	    }

	    if (!$bestblockid ||
		$loadsz <$bestloadsz)
	    {
		$bestblockid = $siblingid;
		$bestunloadsz = $unloadsz;
		$bestloadsz = $loadsz;
	    }
	}

	if (!$bestblockid) {
	    last find_sibling;
	}

	# print "$bestblockid : $bestunloadsz : $bestloadsz\n";

	$currentid = $bestblockid;

	delete $cblockids{$currentid};

	$sumgreedyload += $bestloadsz;

	if ((scalar keys %cblockids) % 500 == 0) {
	    print sprintf ("%d\n", scalar keys %cblockids);
	}
    }
}

print sprintf ("GREEDY-LOAD-SIZE: %d\n", $sumgreedyload);

