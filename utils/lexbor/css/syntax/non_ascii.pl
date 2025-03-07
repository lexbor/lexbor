 #!/usr/bin/perl

 use strict;
 use warnings FATAL => 'all';


my $range = [
    [0x00F8, 0x037D],
    [0x037F, 0x1FFF],
    [0x2070, 0x218F],
    [0x2C00, 0x2FEF],
    [0x3001, 0xDFFF],
    [0xF900, 0xFDCF],
    [0xFDF0, 0xFFFD],
    [0x10000, 0x10FFFF]
];

my $func = branch_function($range);

print $func, "\n";

sub branch_function {
	my ($range) = @_;
	my ($first, @result);

	push @result, join("\n", @{branch($range, 0, $#$range, 1)});
	push @result, "\n\treturn false;";

	return join "\n", @result;
}

sub branch {
	my ($range, $from, $to, $ind) = @_;
	my ($idx, $space, $entry, @result);

	$idx = ($from + int(($to - $from) / 2));
	return \@result unless exists $range->[$idx];

	$space = join "", map {"\t"} 1..$ind;

	$entry = $range->[$idx];

	# IF state
	if ($idx <= $from) {
		if ($idx == 0 && $entry->[0] != 0) {
			push @result, sprintf("%sif (cp >= 0x%04X && cp < 0x%04X) {",
								  $space, $entry->[0], $entry->[1]);
		}
        else {
			push @result, sprintf("%sif (cp <= 0x%04X) {", $space, $entry->[1]);
		}

		push @result, sprintf("%s\treturn true;", $space);
	}
    else {
		push @result, sprintf("%sif (cp <= 0x%04X) {", $space, $entry->[1]);
		push @result, @{branch($range, $from, $idx - 1, $ind + 1)};
	}

	push @result, $space. "}";

	return \@result if $entry->[1] >= $range->[-1]->[1];

	# ELSE state
	push @result, sprintf("%selse if (cp >= 0x%04X) {", $space, $range->[$idx + 1]->[0]);

	if ($idx == $to) {
		push @result, sprintf("%s\treturn true;", $space);
	}
    else {
		push @result, @{branch($range, $idx + 1, $to, $ind + 1)};
	}

	push @result, $space. "}";

	return \@result;
}
