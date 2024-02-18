use utf8;
use strict;

# Please, see https://www.unicode.org/Public/draft/idna/
# https://www.unicode.org/reports/tr46

# use:
# idna_test.pl <IdnaTestV2.txt>

binmode(STDOUT, "encoding(UTF-8)");

my $unicode = new IDNATest $ARGV[0];
my $result = $unicode->build();
$unicode->save($ARGV[1]);


package IDNATest;

sub new {
	my ($class, $filepath) = @_;
	my ($data, $dec_types, $dec_types_raw);

	my $self = {
        filepath   => $filepath,
        data       => [],
		result     => [],
		codepoints => [],
		_prefix    => "lxb_unicode_idna_test"
    };

	bless $self, $class;

	open my $fh, $filepath || die "Failed to open file: $filepath";
	binmode $fh;

	$data = [];

	foreach my $line (<$fh>) {
		$line =~ s/\s*#.*//;
		$line =~ s/\s+$//s;

		if ($line eq "") {
			next;
		}

		my ($source, $toUnicode, $toUnicodeStatus, $toAsciiN, $toAsciiNStatus,
		    $toAsciiT, $toAsciiTStatus) = split /\s*;\s*/, $line;

		my $ref_toUnicode = ($toUnicode eq "") ? $source : $toUnicode;
		my $ref_toAsciiN = ($toAsciiN eq "") ? $ref_toUnicode : $toAsciiN;

		my $status = ($toAsciiNStatus eq "") ? $toUnicodeStatus : $toAsciiNStatus;

		push @$data, [$source, $ref_toAsciiN, $status];
	}

	close($fh);

	$self->{data} = $data;

    return $self;
}

sub build {
	my $self = shift;
	my ($entry, $result, $source, $src_len, $ascii_len, $status, $count);
	my $data = $self->{data};
	my $result = [];

	$count = 0;

	foreach my $idx (0..@$data - 1) {
		$entry = $data->[$idx];
		$status = length $entry->[2];
		$source = $entry->[0];

		push @$result, "    {.source = (const lxb_char_t *) \"$source\", .ascii = (const lxb_char_t *) \"" . $entry->[1] . "\""
		                    .", .status = $status} /* $idx */";
	}

	push @$result, "    {.source = NULL, .ascii = NULL, .status = 0}";

	my $name = $self->lxb_prefix("entries");
	my $entries = "static const lxb_unicode_idna_test_t $name\[\] = {\n" . join(",\n", @$result) . "\n};";

	$self->{result} = $entries;

	return $entries;
}

sub make_entry {
	my ($self, $list, @name) = @_;
	my $count = scalar @$list;

	my $st;
	my $name = $self->lxb_prefix(@name);

	$st = "static const lxb_codepoint_t $name\[$count\] = {";

	if (scalar @$list > 0) {
		$st .= "0x". join(", 0x", @$list) ."};";
	}
	else {
		$st .= "};";
	}

	push @{$self->{codepoints}}, $st;

	return $name;
}

sub save {
	my ($self, $filepath) = @_;
	my $year = 1900 + (localtime)[5];
	my $cps = join("\n", @{$self->{codepoints}});
	my $res = $self->{result};
	my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

#ifndef LEXBOR_UNICODE_IDNA_TEST_RES_H
#define LEXBOR_UNICODE_IDNA_TEST_RES_H

#ifdef __cplusplus
extern "C" {
#endif

$cps

$res


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_UNICODE_IDNA_TEST_RES_H */
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
