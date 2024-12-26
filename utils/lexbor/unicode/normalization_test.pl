#!/usr/bin/perl

# Last Version:
# https://www.unicode.org/Public/UCD/latest/ucd/
# https://www.unicode.org/Public/UCD/latest/ucd/UCD.zip

# usage:
# test.pl <NormalizationTest.txt>

use utf8;
use strict;
use FindBin;
use warnings FATAL => 'all';

my $unicode = new UnicodeTest $ARGV[0];
my $result = $unicode->build();
$unicode->save("$FindBin::RealBin/../../../test/lexbor/unicode/unicode_normalization_test_res.h");

print join("\n", @{$unicode->{codepoints}}), "\n\n";
print $result, "\n";

package UnicodeTest;

sub new {
	my ($class, $filepath) = @_;
	my ($data, $dec_types, $dec_types_raw);

	my $self = {
        filepath   => $filepath,
        data       => [],
		result     => [],
		codepoints => [],
		_prefix    => "lxb_unicode_test"
    };

	bless $self, $class;

	open my $fh, $filepath || die "Failed to open file: $filepath";

	# source; NFC; NFD; NFKC; NFKD; # comment

	$data = [];

	foreach my $line (<$fh>) {
		if ($line =~ /^\s*#/ || $line =~ /^\s*\@/) {
			next;
		}

		my ($source, $NFC, $NFD, $NFKC, $NFKD) = split /;/, $line;
		my $hash = {};

		foreach my $en (@{[["source", $source], ["NFC", $NFC], ["NFD", $NFD], ["NFKC", $NFKC], ["NFKD", $NFKD]]}) {
			if ($en->[1] !~ /^[0-9a-fA-F ]+$/) {
				die "Error: ". $en->[0] ."\nLine: $line\n";
			}

			my @arr = map {"0x$_"} split(/\s+/, $en->[1]);

			$hash->{$en->[0]} = \@arr;
		}

		push @$data, $hash;
	}

	close($fh);

	$self->{data} = $data;

    return $self;
}

sub build {
	my $self = shift;
	my ($entry, $result, $source, $nfc, $nfd, $nfkc, $nfkd);
	my $data = $self->{data};
	
	$result = [];

	foreach my $idx (0..@$data - 1) {
		$entry = $data->[$idx];

		$source = $self->make_entry($entry->{source}, "source", $idx + 1);
		$nfc = $self->make_entry($entry->{NFC}, "nfc", $idx + 1);
		$nfd = $self->make_entry($entry->{NFD}, "nfd", $idx + 1);
		$nfkc = $self->make_entry($entry->{NFKC}, "nfkc", $idx + 1);
		$nfkd = $self->make_entry($entry->{NFKD}, "nfkd", $idx + 1);

		push @$result, "    {.source = $source, .nfc = $nfc, .nfd = $nfd, .nfkc = $nfkc, .nfkd = $nfkd}";
	}

	push @$result, "    {.source = NULL, .nfc = NULL, .nfd = NULL, .nfkc = NULL, .nfkd = NULL}";

	my $name = $self->lxb_prefix("entries");
	my $entries = "static const lxb_unicode_test_t $name\[\] = {\n" . join(",\n", @$result) . "\n};";

	$self->{result} = $entries;

	return $entries;
}

sub make_entry {
	my ($self, $list, @name) = @_;
	my $count = scalar @$list + 1;

	my $name = $self->lxb_prefix(@name);

	my $st = "static const lxb_codepoint_t $name\[$count\] = {". join(", ", @$list) .", 0x10FFFF};";

	push @{$self->{codepoints}}, $st;

	return $name;
}

sub save {
	my ($self, $filepath) = @_;
	my $year = 1900 + (localtime)[5];
	my $cps = join("\n", @{$unicode->{codepoints}});
	my $res = $unicode->{result};
	my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

#ifndef LEXBOR_UNICODE_TEST_RES_H
#define LEXBOR_UNICODE_TEST_RES_H

#ifdef __cplusplus
extern "C" {
#endif

$cps

$res


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_UNICODE_TEST_RES_H */
EOM

	open(my $fh, '>', $filepath) || die "Failed to save to file: $filepath";
	binmode $fh;

	print $fh $temp, "\n";

	close($fh);
}

sub lxb_prefix {
	my $self = shift;

	return join "_", $self->{_prefix}, @_;
}
