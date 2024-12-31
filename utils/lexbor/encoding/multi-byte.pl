#!/usr/bin/perl

# Last Version:
# https://encoding.spec.whatwg.org/#indexes

# usage:
# multi_byte.pl

use utf8;
use strict;
use FindBin;
use warnings FATAL => 'all';

my ($data, @multi_tables, @multi_maps, @multi_declr, @multi_func);
my (@milti_declr_tables, @milti_declr_maps);
my ($tables_decls, $tables, $map_declr, $map, $declr, $func);

my $output = "$FindBin::RealBin/../../../source/lexbor/encoding";
my $output_test = "$FindBin::RealBin/../../../test/lexbor/unicode";
my $source = "$FindBin::RealBin/multi-byte";
my @files = (
    ["index-euc-kr.txt", "multi_euc_kr"],
    ["index-gb18030.txt", "multi_gb18030"],
    ["index-iso-2022-jp-katakana.txt", "multi_iso_2022_jp_katakana"],
    ["index-jis0212.txt", "multi_jis0212"],
    ["index-jis0208.txt", "multi_jis0208"],
);

my $multi = new MultiByte;

# Special subs
($tables_decls, $tables, 
 $map_declr, $map,
 $declr, $func) = $multi->parse("$source/index-big5.txt", "multi_big5",
sub {
    my ($cp, $cps, $parts) = @_;

    # Let index be index Big5 excluding all entries whose pointer is
    # less than (0xA1 - 0x81) × 157.
    if ($parts->[0] < (0xA1 - 0x81) * 157) {
        return 0;
    }

    # If code point is U+2550, U+255E, U+2561, U+256A, U+5341, or U+5345,
    # return the last pointer corresponding to code point in index.
    if (exists $cps->{$cp} &&
        $cp != 0x2550 && $cp != 0x255E && $cp != 0x2561 &&
        $cp != 0x256A && $cp != 0x5341 && $cp != 0x5345)
    {
        return 0;
    }

    return 1;
});
push @milti_declr_tables, $tables_decls;
push @multi_tables, $tables;
push @milti_declr_maps, $map_declr;
push @multi_maps, $map;
push @multi_func, $func;

# Other
foreach my $entry (@files) {
    ($tables_decls, $tables,
     $map_declr, $map,
     $declr, $func) = $multi->parse("$source/$entry->[0]", $entry->[1],
    sub {
        my ($cp, $cps, $parts) = @_;

        return exists $cps->{$cp} ? 0 : 1;
    });

    push @milti_declr_tables, $tables_decls;
    push @multi_tables, $tables;
    push @milti_declr_maps, $map_declr;
    push @multi_maps, $map;
    push @multi_func, $func;
}

$data = $multi->template_header("ENCODING_MULTI",
            join "\n\n", join("\n", @milti_declr_maps), @milti_declr_tables, @multi_func);
$multi->save_to("$output/multi.h", $data);

$data = $multi->template_source(join "\n\n", @multi_maps, @multi_tables);
$multi->save_to("$output/multi.c", $data);


package MultiByte;

use FindBin;
use File::Spec::Functions;
use warnings FATAL => 'all';

sub new {
    my ($class) = @_;
    my $self;

    $self = {
    };

    bless $self, $class;

    return $self;
}

sub parse {
    my ($self, $filepath, $name, $sub) = @_;
    my ($cp, $cps, $index, $desc, $range, @parts, @cps);
    my ($declr, $func, $map, $map_declr, $tables, $tables_decls, $callback);

    $cps = {};
    $index = {};

    open my $fh, $filepath || die "Failed to open file: $filepath";

    foreach my $line (<$fh>) {
        $line =~ s/^\s+//;
        $line =~ s/\s+$//;
        next if $line =~ /^\s*\#/ || $line eq "";

        @parts = split /\s+/, $line, 4;

        $cp = $parts[1];
        $cp =~ s/^0x//;
        $cp = hex($cp);

        if ($sub->($cp, $cps, \@parts)) {
            die "Failed to parse $filepath. Bad index." unless $parts[0] =~ /^\d+$/;

            $cps->{$cp} = int $parts[0];
        }

        $desc = $parts[3];
        $desc =~ s/^\(|\)$//g;

        $index->{int $parts[0]} = [$cp, $desc];
    }

    close($fh);

    @cps = sort {$a <=> $b} keys %$cps;
    $range = to_range(\@cps, 5000, $cps[0]);

    $callback = sub {
        my ($cp, $uint, $from, $to) = @_;

        return (exists $cps->{$cp}) ? $cps->{$cp} : 'UINT16_MAX';
    };

    ($tables_decls, $tables) = $self->range_tables($range, $callback, $name);
    ($map_declr, $map) = $self->data_map($index, "$name\_map");
    ($declr, $func) = $self->function($range, $name);

    $map_declr = "LXB_EXTERN $map_declr;";
    $tables_decls = "LXB_EXTERN ". join(";\nLXB_EXTERN ", @$tables_decls) .";";

    # print $map, "\n";
    # print $tables, "\n";
    # print $declr, "\n";
    # print $func, "\n";

    return ($tables_decls, $tables, $map_declr, $map, $declr, $func);
}

sub function {
    my ($self, $range, $name) = @_;
    my ($declr, @result);

    $declr = "lxb_inline uint16_t\nlxb_encoding_$name\_index(lxb_codepoint_t cp)";

    push @result, $declr;
    push @result, "{";

    if (@$range > 2) {
        push @result, sprintf("    if (cp > 0x%04X) {", $range->[-1]->[1]);
        push @result, sprintf("        return UINT16_MAX;");
        push @result, sprintf("    }\n");
    }

    push @result, join("\n", @{$self->branch($range, 0, $#$range, 1, $name)});
    push @result, "\n    return UINT16_MAX;";
    push @result, "}";

    return ("$declr;", join("\n", @result));
}

sub branch {
    my ($self, $range, $from, $to, $ind, $name) = @_;
    my ($idx, $space, $entry, $table, @result);

    $idx = ($from + int(($to - $from) / 2));
    return \@result unless exists $range->[$idx];

    $space .= "    " foreach 1..$ind;

    $entry = $range->[$idx];
    $table = table_name($name, $entry->[0], $entry->[1]);

    # IF state
    if ($idx <= $from) {
        if ($idx == 0 && $entry->[0] != 0) {
            push @result, sprintf("%s"."if (cp >= $entry->[0] && cp < $entry->[1]) {", $space);
        }
        else {
            push @result, sprintf("%s"."if (cp < $entry->[1]) {", $space);
        }

        push @result, sprintf("$space    return $table\[cp - $entry->[0]];");
    } else {
        push @result, sprintf("%s"."if (cp < $entry->[1]) {", $space);
        push @result, @{$self->branch($range, $from, $idx - 1, $ind + 1, $name)};
    }

    push @result, $space. "}";

    if ($entry->[1] >= $range->[-1]->[1]) {
        return \@result;
    }

    # ELSE state
    $table = table_name($name, $entry->[2], $range->[$idx + 1]->[1]);

    push @result, sprintf("%s"."else if (cp >= $entry->[2]) {", $space);

    if ($idx == $to) {
        push @result, sprintf("$space    return $table\[cp - $entry->[2]];");
    } else {
        push @result, @{$self->branch($range, $idx + 1, $to, $ind + 1, $name)};
    }

    push @result, $space. "}";

    return \@result;
}

sub data_map {
    my ($self, $index, $name) = @_;
    my ($entry, $size, $declr, @result, @lines, @ids);

    @ids = sort {$a <=> $b} keys %$index;

    foreach my $id (0..$ids[-1]) {
        if (exists $index->{$id}) {
            $entry = $index->{$id};
            push @lines, sprintf("    0x%04X, /* %s */", @$entry);
        }
        else {
            push @lines, sprintf("    LXB_ENCODING_ERROR_CODEPOINT, /* Not defined */");
        }
    }

    $declr = "lxb_codepoint_t lxb_encoding_$name\[". scalar(@lines) ."]";

    push @result, "LXB_API $declr =";
    push @result, "{";
    push @result, @lines;
    push @result, "};\n";

    $size = scalar(@lines) * 4;

    print "Tables size: ", $size, " Byte; ", int($size / 1000), " Kilobyte \n";

    return ($declr, join "\n", @result);
}

sub range_tables {
    my ($self, $range, $sub, $name) = @_;
    my ($num, $from, $to, $dif, $uint, $orig, $sum, $size, @result, @lines);
    my (@declrs);

    $sum = 0;
    $size = 0;

    print "Generating basic tables:\n";

    foreach my $entry (@$range) {
        $dif = $entry->[1] - $entry->[0];
        $from = $entry->[0];
        $to = $entry->[1] - 1;
        $uint = ($dif < (1 << 16) - 1) ? "uint16_t" : "uint32_t";
        $orig = table_name($name, $entry->[0], $entry->[1]);

        print sprintf("    %8u -> %8u: %8u ($uint) ($orig)", $from, $to, $dif), "\n";

        foreach my $cp ($from..$to) {
            push @lines, sprintf("    %s, /* %04X (%u) */",
                                 $sub->($cp, $uint, $entry->[0], $entry->[1]),
                                 $cp, $cp);
        }

        $num = scalar @lines;
        $sum += $num;

        $size += ($uint eq "uint16_t") ? $num * 2 : $num * 4;

        push @declrs, "$uint $orig\[". scalar @lines ."]";

        push @result, sprintf("/* From: %04X; To: %04X */", $from, $to);
        push @result, "LXB_API $declrs[-1] =";
        push @result, "{";
        push @result, @lines;
        push @result, "};\n";

        @lines = ();
    }

    print "Total entries: $sum\n";
    print "Generated tables: ", scalar @$range, "\n";
    print "Tables size: ", $size, " Byte; ", int($size / 1000), " Kilobyte \n";

    return (\@declrs, join "\n", @result);
}

sub to_range {
    my ($decomps, $limit, $min) = @_;
    my ( $prev, $first, $last, $dif, @range);

    $prev = defined $min ? $min : 0;
    $first = $prev;
    $last = $decomps->[-1] + 1;

    foreach my $val (@$decomps, $last) {
        $dif = $val - $prev;

        if ($dif > $limit || $val == $last) {
            push @range, [$first, $prev, $val];

            print "Added range: \n",
                  sprintf("    Exists: %8u (%6X)", $first, $first),
                  sprintf(" -> %8u (%6X);", $prev, $prev),
                  " Count: ", $prev - $first, "\n",
                  sprintf("    Empty:  %8u (%6X)", $prev, $prev),
                  sprintf(" -> %8u (%6X);", $val, $val),
                  " Count: ", $val - $prev, "\n";

            $first = $val;
        }

        $prev = $val + 1;
    }

    return \@range;
}

sub table_name {
    return join "_", "lxb_encoding", @_, "map";
}

sub save_to {
    my ($self, $filepath, $data) = @_;

    open(my $fh, '>', $filepath) || die "Failed to save to file: $filepath";
    binmode $fh;

    print $fh $data;

    close($fh);
}

sub template_header {
    my ($self, $name, $data) = @_;
    my $year = 1900 + (localtime)[5];
    my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

/*
 * Caution!
 * This file generated by the script "utils/lexbor/encoding/multi-byte.pl"!
 * Do not change this file!
 */

#ifndef LEXBOR_$name\_H
#define LEXBOR_$name\_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lexbor/encoding/base.h"


$data


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_$name\_H */
EOM

    return $temp;
}

sub template_source {
    my ($self, $data) = @_;
    my $year = 1900 + (localtime)[5];
    my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

/*
 * Caution!
 * This file generated by the script "utils/lexbor/encoding/multi-byte.pl"!
 * Do not change this file!
 */

#include "lexbor/encoding/multi.h"


$data
EOM

    return $temp;
}
