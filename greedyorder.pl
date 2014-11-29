#!/usr/bin/perl                                                                 
use strict;
use warnings;

my @arraysz = ();
my %cblocks = ();

my @arrayblocks = ();

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

	$cblocks{join(',',@block)} = 1;
    }
}

print sprintf ("NUM-ARRAYS: %d\n", $#arraysz);
print sprintf ("NUM-BLOCKS: %d\n", scalar keys %cblocks);

my $sumraworder = 0;

foreach my $cblock (keys %cblocks)
{
    # print join(',',@{$cblock})."\n";

    my @cblock = split /,/,$cblock;

    for (my $i = 0; $i <= $#cblock; $i++)
    {
	my $array = $cblock[$i];
	# print "$i : $array : $arraysz[$array]\n";

	$sumraworder += $arraysz[$array];

	my $href = $arrayblocks[$array];

	$href->{$cblock} = 1;
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

while (scalar keys %cblocks)
{
    # We are at start, or have no better idea.  Find largest one.
    # Expensive to evaluate

    my $largest = ();
    my $largestsz = 0;

    foreach my $cblock (keys %cblocks)
    {
	my @cblock = split /,/,$cblock;

	my $sz = 0;

	for (my $i = 0; $i <= $#cblock; $i++)
	{
	    my $array = $cblock[$i];
	    $sz += $arraysz[$array];
	}

	if ($sz > $largestsz)
	{
	    $largest = $cblock;
	    $largestsz = $sz;
	}
    }

    print "$largest : $largestsz\n";

    my $current = $largest;

    $sumgreedyload += $largestsz;

    delete $cblocks{$current};

    # Now try to find a block which reuses some information

  find_sibling:
    for ( ; ; )
    {
	my @curblock = split /,/,$current;

	my @siblings = ();

	my $cursz = 0;

	for (my $i = 0; $i <= $#curblock; $i++)
        {
	    my $array = $curblock[$i];

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

	my %curarrays = map { $_, 1 } @curblock;

	my $bestblock = ();
	my $bestunloadsz = 0;
	my $bestloadsz = 0;

      try_sibling:
	foreach my $sibling (@siblings)
	{
	    if (!$cblocks{$sibling}) {
		# already handled
		next try_sibling;
	    }

	    my @sibblock = split /,/,$sibling;

	    my $reusesz = 0;
	    my $loadsz = 0;

	    for (my $i = 0; $i <= $#sibblock; $i++)
	    {
		my $array = $sibblock[$i];

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

	    if (0 && (!$bestblock ||
		      $unloadsz <= $bestunloadsz))
	    {
		if (!$bestblock ||
		    ($unloadsz > $bestunloadsz) ||
		    ($unloadsz == $bestunloadsz &&
		     $loadsz > $bestloadsz))
		{
		    $bestblock = $sibling;
		    $bestunloadsz = $unloadsz;
		    $bestloadsz = $loadsz;
		}
	    }

	    if (1 && (!$bestblock ||
		      $loadsz <$bestloadsz))
	    {
		$bestblock = $sibling;
		$bestunloadsz = $unloadsz;
		$bestloadsz = $loadsz;
	    }
	}

	if (!$bestblock) {
	    last find_sibling;
	}

	# print "$bestblock : $bestunloadsz : $bestloadsz\n";

	$current = $bestblock;

	delete $cblocks{$current};

	$sumgreedyload += $bestloadsz;

	if ((scalar keys %cblocks) % 500 == 0) {
	    print sprintf ("%d\n", scalar keys %cblocks);
	}
    }
}

print sprintf ("GREEDY-LOAD-SIZE: %d\n", $sumgreedyload);

