#!/usr/bin/perl

# Last Version:
# https://www.unicode.org/Public/UCD/latest/ucd/
# https://www.unicode.org/Public/UCD/latest/ucd/UCD.zip
# https://unicode.org/Public/idna/latest/

# usage:
# normalization.pl <path (directory) to UnicodeData.txt, CompositionExclusions.txt, DerivedNormalizationProps.txt, IdnaMappingTable.txt>

use utf8;
use strict;
use FindBin;
use warnings FATAL => 'all';

my $output = "$FindBin::RealBin/../../../source/lexbor/unicode";
my $output_test = "$FindBin::RealBin/../../../test/lexbor/unicode";
my $unicode = new Unicode $ARGV[0];

$unicode->CompositionExclusions();
$unicode->IdnaMappingTable();
$unicode->DerivedNormalizationProps();
$unicode->UnicodeData();

$unicode->make($output);
$unicode->composition_test("$output_test/composition_test.c");


package Unicode;

use FindBin;
use File::Spec::Functions;
use warnings FATAL => 'all';

use constant {
    CP    => 0,  # Code value normative Code value in 4-digit hexadecimal format.
    NAME  => 1,  # Character name normative These names match exactly the names published in Chapter 7 of the Unicode Standard, Version 2.0, except for the two additional characters.
    GC    => 2,  # General category normative / informative
    CCC   => 3,  # Canonical combining classes normative The classes used for the Canonical Ordering Algorithm in the Unicode Standard. These classes are also printed in Chapter 4 of the Unicode Standard.
    BC    => 4,  # Bidirectional category normative See the list below for an explanation of the abbreviations used in this field. These are the categories required by the Bidirectional Behavior Algorithm in the Unicode Standard. These categories are summarized in Chapter 3 of the Unicode Standard.
    DTDM  => 5,  # Character decomposition mapping normative In the Unicode Standard, not all of the mappings are full (maximal) decompositions. Recursive application of look-up for decompositions will, in all cases, lead to a maximal decomposition. The decomposition mappings match exactly the decomposition mappings published with the character names in the Unicode Standard.
    NTNV  => 6,  # Decimal digit value normative This is a numeric field. If the character has the decimal digit property, as specified in Chapter 4 of the Unicode Standard, the value of that digit is represented with an integer value in this field
    DVN   => 7,  # Digit value normative This is a numeric field. If the character represents a digit, not necessarily a decimal digit, the value is here. This covers digits which do not form decimal radix forms, such as the compatibility superscript digits
    NVN   => 8,  # Numeric value normative This is a numeric field. If the character has the numeric property, as specified in Chapter 4 of the Unicode Standard, the value of that character is represented with an integer or rational number in this field. This includes fractions as, e.g., "1/5" for U+2155 VULGAR FRACTION ONE FIFTH Also included are numerical values for compatibility characters such as circled numbers.
    BM    => 8,  # Mirrored normative If the character has been identified as a "mirrored" character in bidirectional text, this field has the value "Y"; otherwise "N". The list of mirrored characters is also printed in Chapter 4 of the Unicode Standard.
    U1N   => 10, # Unicode 1.0 Name informative This is the old name as published in Unicode 1.0. This name is only provided when it is significantly different from the Unicode 3.0 name for the character.
    ISOC  => 11, # 10646 comment field informative This is the ISO 10646 comment field. It is in parantheses in the 10646 names list.
    SUPPM => 12, # Uppercase mapping informative Upper case equivalent mapping. If a character is part of an alphabet with case distinctions, and has an upper case equivalent, then the upper case equivalent is in this field. See the explanation below on case distinctions. These mappings are always one-to-one, not one-to-many or many-to-one. This field is informative.
    SLOWM => 13, # Lowercase mapping informative Similar to Uppercase mapping
    STITM => 14  # Titlecase mapping informative Similar to Uppercase mapping
};

sub new {
    my ($class, $filepath) = @_;
    my ($self, %required);

    $filepath = catfile($FindBin::Bin, "UCD") unless -d $filepath;

    %required = (
        UnicodeData => catfile($filepath, "UnicodeData.txt"),
        CompositionExclusions => catfile($filepath, "CompositionExclusions.txt"),
        IdnaMappingTable => catfile($filepath, "IdnaMappingTable.txt"),
        DerivedNormalizationProps => catfile($filepath, "DerivedNormalizationProps.txt")
    );

    $self = {
        UCD => $filepath,
        unicode => {},
        composition => [],
        decomposition => [],
        decomposition_types => {},
        min_cp => 0,
        max_cp => 0,
        IDNA => {},
        IDNA_type => {},
        normalization_props => {},
        normalization_quick_types => {},
        composition_entries => "lxb_unicode_composition_entries",
        composition_cps => "lxb_unicode_composition_cps",
        decomposition_cps => "lxb_unicode_decomposition_cps",
        idna_cps => "lxb_unicode_idna_cps",
        decomposition_entries => "lxb_unicode_normalization_entries",
        idna_entries => "lxb_unicode_idna_entries",
        entries => "lxb_unicode_entries",
        %required
    };

    foreach (keys %required) {
        die "A file is required to generate the data: " . $required{$_}
            unless -e $required{$_};
    }

    bless $self, $class;

    return $self;
}

sub CompositionExclusions {
    my $self = shift;
    my $exclude = {};

    open my $fh, $self->{CompositionExclusions}
        || die "Failed to open file: " . $self->{CompositionExclusions};

    foreach my $line (<$fh>) {
        if ($line =~ /^\s*#/ || $line =~ /^\s+$/ || $line eq "") {
            if ($line =~ /^# \(\d+\) (.*)$/) {
                if ($1 ne "Script Specifics" 
                    && $1 ne "Post Composition Version precomposed characters")
                {
                    last;
                }
            }

            next;
        }

        $line =~ s/\s*#.*//s;

        if ($line !~ /^[0-9a-fA-F]+$/) {
            die "Failed to get Composition Exclusions entry: $line";
        }

        $exclude->{hex($line)} = 1;
    }

    close($fh);

    if (keys %$exclude == 0) {
        die "The CompositionExclusions.txt file was submitted, ",
            "but the data could not be extracted from it.";
    }

    $self->{composition_exclusions} = $exclude;

    return $exclude;
}

sub IdnaMappingTable {
    my $self = shift;
    my ($idna, $key, $type, $types, @entries);

    $idna = {};
    $types = {};

    open my $fh, $self->{IdnaMappingTable}
        || die "Failed to open file: " . $self->{IdnaMappingTable};

    foreach my $line (<$fh>) {
        my @mapped;

        if ($line =~ /^\s*#/ || $line =~ /^\s+$/ || $line eq "") {
            next;
        }

        $line =~ s/\s*#.*//s;
        @entries = split /\s*;\s*/, $line;

        if (exists $entries[2] && $entries[2] ne "") {
            @mapped = map {hex($_)} split /\s+/, $entries[2];
        }

        if (!exists $entries[1] || $entries[1] eq "") {
            die "Failed to get IDNA Type for entry: $line";
        }

        $type = lxb_prefix("IDNA", uc($entries[1]));
        $types->{$type} = [] unless exists $types->{$type};

        if ($entries[0] =~ /^([0-9a-fA-F]+)$/) {
            $idna->{hex($1)} = [$type, \@mapped];
            push @{$types->{$type}}, hex($1);
        }
        elsif ($entries[0] =~ /^([0-9a-fA-F]+)\.\.([0-9a-fA-F]+)$/) {
            for my $cp (hex($1)..hex($2)) {
                $idna->{$cp} = [$type, \@mapped];
                push @{$types->{$type}}, $cp;
            }
        }
        else {
            die "Failed to get IDNA Mapping Table for entry: $line";
        }
    }

    close($fh);

    if (keys %$idna == 0) {
        die "The IdnaMappingTable.txt file was submitted, ",
            "but the data could not be extracted from it.";
    }

    $self->{IDNA} = $idna;
    $self->{IDNA_type} = $types;

    return ($idna, $types);
}

sub DerivedNormalizationProps {
    my $self = shift;
    my ($quick, $type, $types);

    $quick = {};
    $types = {};

    open my $fh, $self->{DerivedNormalizationProps}
        || die "Failed to open file: " . $self->{DerivedNormalizationProps};

    foreach my $line (<$fh>) {
        if ($line =~ /^\s+$/ || $line eq "") {
            next;
        }

        if ($line =~ /^\s*#/) {
            $type = undef;

            if ($line =~ /^# (\S+)_Quick_Check=(\S+)$/) {
                $type = lxb_quick(uc($1), uc($2));
                $types->{$type} = 1;
            }

            next;
        }

        if (!defined $type) {
            next;
        }

        $line =~ s/\s*;.*//s;

        if ($line =~ /^[0-9a-fA-F]+$/) {
            $line = hex($line);
            $quick->{$line} = {} unless exists $quick->{$line};
            $quick->{$line}->{$type} = 1;
        }
        elsif ($line =~ /^([0-9a-fA-F]+)\.\.([0-9a-fA-F]+)$/) {
            for my $cp (hex($1)..hex($2)) {
                $quick->{$cp} = {} unless exists $quick->{$cp};
                $quick->{$cp}->{$type} = 1;
            }
        }
        else {
            die "Failed to get Derived Normalization Props for entry: $line";
        }
    }

    close($fh);

    if (keys %$quick == 0) {
        die "The DerivedNormalizationProps.txt file was submitted, ",
            "but the data could not be extracted from it.";
    }

    $self->{normalization_props} = $quick;
    $self->{normalization_quick_types} = $types;

    return $quick;
}

sub UnicodeData {
    my $self = shift;
    my ($cp, $type, $unicode, $dtypes, $exclude, @entry, @cps);
    my ($ccc, $comps, $decomp, @decomps, $resolved);

    $unicode = {};
    $comps = {};
    $dtypes = {};
    $exclude = $self->{composition_exclusions};

    open my $fh, $self->{UnicodeData}
        or die "Failed to open file: " . $self->{UnicodeData};

    foreach my $line (<$fh>) {
        @entry = split /;/, $line;
        $cp = hex($entry[CP]);

        # Composition and Decomposition entries.

        $type = "_undef";
        undef $decomp;

        my @map = split /\s+/, $entry[DTDM];

        if ($#map > 0 && $map[0] =~ /<(.*)>/) {
            $type = $1;
            shift @map;
        }

        @map = map {hex($_)} @map;

        $dtypes->{$type} = 0 unless exists $dtypes->{$type};
        $dtypes->{$type} += 1;

        if ($type eq "_undef" && int($entry[CCC]) == 0 && @map > 1) {
            # Composition.

            if (@map != 2) {
                die "Composition. Only two codepoints expecting. We have: "
                    . join(", ", @map);
            }

            $comps->{$cp} = [@map, exists $exclude->{$cp}];

            # print sprintf("%04X", $cp), ": ", join(", ", @map), "\n";
        }

        $decomp = [$cp, $type, \@map] if @map > 0;
        push @decomps, $decomp if $decomp;

        $ccc->{$cp} = int($entry[CCC]) if int($entry[CCC]);

        $unicode->{$cp} = \@entry;
    }

    close($fh);

    @cps = sort {$a <=> $b} keys %$unicode;

    $self->{unicode} = $unicode;
    $self->{decomposition} = \@decomps;
    $self->{decomposition_types} = $dtypes;
    $self->{composition} = $comps;
    $self->{ccc} = $ccc;
    $self->{min_cp} = $cps[0];
    $self->{max_cp} = $cps[-1];
}

sub make {
    my ($self, $output) = @_;
    my ($decomps, $decomps_index, $decomps_entry);
    my ($idna_res, $idna_map, $idna_index, $idna_entries);
    my ($map_decomp, $map_idna, $map_range, $map_general, $data_funct);
    my ($header, $source, $general_index, $general, $codepoints_map, $keys);
    my ($comps, $comps_map, $res_map);
    my ($comps_index, $comps_entry);

    # Composition
    ($comps, $comps_map) = $self->composition();
    $res_map = $self->data_map($comps_map, "lxb_unicode_composition_cp_t",
                               $self->{composition_cps});
    ($comps_index, $comps_entry) = $self->composition_entries($comps,
                                                $self->{composition_entries});

    # Decomposition
    ($decomps, $codepoints_map) = $self->decomposition();
    $map_decomp = $self->data_codepoints($codepoints_map, $self->{decomposition_cps});
    $self->print_decomposition_types();
    $self->print_decomposition_quick_types();
    $self->print_idna_types();

    # IDNA
    ($idna_res, $idna_map) = $self->idna();
    $map_idna = $self->data_codepoints($idna_map, $self->{idna_cps});

    # General Data
    $general = [];
    $general_index = {};

    # Reserved NULL entry
    push @$general, [0, 0];
    $general_index->{join "%", @{$general->[0]}} = 0;

    ($decomps_index, $decomps_entry) = $self->decomposition_entries($decomps,
                                       $comps_index, $self->{decomposition_entries});
    ($idna_index, $idna_entries) = $self->idna_entries($idna_res,
                                   $self->{idna_entries});

    my $callback = sub {
        my ($cp, $uint, $from, $to) = @_;
        my ($decomp, $idna);

        $decomp = exists $decomps_index->{$cp} ? $decomps_index->{$cp} : 0;
        $idna = exists $idna_index->{$cp} ? $idna_index->{$cp} : 0;

        return general_entry($general, $general_index, $decomp, $idna);
    };

    $keys = $self->used_codepoints();

    my $range = to_range($keys, 5000);
    $map_range = $self->range_data_codepoints($range, $callback, "map");
    $data_funct = $self->function($range);

    $map_general = $self->general_entries($general, $self->{entries});

    $header = $unicode->template_header("UNICODE_RES", join "\n\n",
              $map_range, $map_general, $map_decomp, $map_idna,
              $decomps_entry, $idna_entries, $comps_entry, $res_map);

    $unicode->save_to("$output/res.h", $header);

    print "\nImportant!\nPlease, update function lxb_unicode_entry():\n";
    print $data_funct->[1], "\n";
}

sub general_entry {
    my ($res, $index, $decom, $idna) = @_;
    my ($key, $pos);

    $key = join "%", $decom, $idna;

    return $index->{$key} if exists $index->{$key};

    $pos = scalar @$res;
    $index->{$key} = $pos;

    push @$res, [$decom, $idna];

    return $pos;
}

sub general_entries {
    my ($self, $map, $name) = @_;
    my @data;

    push @data, "static const lxb_unicode_entry_t $name\[". scalar(@$map) ."] =";
    push @data, "{";
    push @data, "    {". join("},\n    {", map {join(", ", @$_)} @$map) ."}";
    push @data, "};";

    return join("\n", @data);
}

sub decomposition_entries {
    my ($self, $res, $comps, $name) = @_;
    my ($pos, $idx, $entry, $type, $result, $index, @data, @map) = @_;
    my ($quick, $qtype, $count, $num, $CCC, $ccc, $comp);

    $result = {};
    $index = {};
    $CCC = $self->{ccc};
    $quick = $self->{normalization_props};

    push @map, [0, lxb_quick("_UNDEF"), 0, 0, 0, 0];

    foreach my $key (keys %$res) {
        $entry = $res->{$key};
        $pos = scalar @map;
        $qtype = lxb_quick("_UNDEF");
        $ccc = exists $CCC->{$key} ? $CCC->{$key} : 0;
        $comp = exists $comps->{$key} ? $comps->{$key} : 0;

        if (exists $quick->{$key}) {
            $qtype = join "|", sort {$a cmp $b} keys %{$quick->{$key}};
        }

        $idx = join "%", $entry->[0], $qtype, $ccc, $entry->[1], $entry->[2],
                         $comp;

        if (exists $index->{$idx}) {
            $pos = $index->{$idx};
        }
        else {
            $index->{$idx} = $pos;
            push @map, [$entry->[0], $qtype, $ccc, $entry->[1], $entry->[2],
                        $comp];
        }

        $result->{$key} = $pos;
    }

    # Normalization Props Quick
    $count = 0;

    foreach my $cp (keys %$quick) {
        next if exists $result->{$cp};

        $type = $quick->{$cp};
        $pos = scalar @map;
        $qtype = join "|", sort {$a cmp $b} keys %{$quick->{$cp}};
        $ccc = exists $CCC->{$cp} ? $CCC->{$cp} : 0;
        $comp = exists $comps->{$cp} ? $comps->{$cp} : 0;
        $idx = join "%", "0", $qtype, $ccc, "0", "0", $comp;

        if (exists $index->{$idx}) {
            $pos = $index->{$idx};
        }
        else {
            $index->{$idx} = $pos;
            push @map, [0, $qtype, $ccc, 0, 0, $comp];
        }

        $result->{$cp} = $pos;
        $count += 1;
    }

    print "Fix Normalization Props (append if not exists): $count\n";

    # CCC
    $count = 0;

    foreach my $cp (keys %$CCC) {
        next if exists $result->{$cp};

        $pos = scalar @map;
        $num = $CCC->{$cp};
        $comp = exists $comps->{$cp} ? $comps->{$cp} : 0;
        $idx = join "%", "0", lxb_quick("_UNDEF"), $num, "0", "0", $comp;

        if (exists $index->{$idx}) {
            $pos = $index->{$idx};
        }
        else {
            $index->{$idx} = $pos;
            push @map, [0, lxb_quick("_UNDEF"), $num, 0, 0, $comp];
        }

        $result->{$cp} = $pos;
        $count += 1;
    }

    print "Fix CCC (append if not exists): $count\n";

    # Composition
    $count = 0;

    foreach my $cp (keys %$comps) {
        next if exists $result->{$cp};

        $pos = scalar @map;
        $ccc = exists $CCC->{$cp} ? $CCC->{$cp} : 0;
        $comp = exists $comps->{$cp} ? $comps->{$cp} : 0;
        $idx = join "%", "0", lxb_quick("_UNDEF"), $ccc, "0", "0", $comp;

        if (exists $index->{$idx}) {
            $pos = $index->{$idx};
        }
        else {
            $index->{$idx} = $pos;
            push @map, [0, lxb_quick("_UNDEF"), $ccc, 0, 0, $comp];
        }

        $result->{$cp} = $pos;
        $count += 1;
    }

    push @data, "static const lxb_unicode_normalization_entry_t $name\[". scalar(@map) ."] =";
    push @data, "{";
    push @data, "    {". join("},\n    {", map {join(", ", @$_)} @map) ."}";
    push @data, "};";

    return ($result, join("\n", @data));
}

sub idna_entries {
    my ($self, $res, $name) = @_;
    my ($pos, $idx, $entry, $type, $result, $index, @data, @map) = @_;

    $result = {};
    $index = {};

    push @map, [0, 0, 0];

    foreach my $key (keys %$res) {
        $entry = $res->{$key};
        $type = $entry->[0];
        $pos = scalar @map;

        if ($type !~ /_MAPPED$/) {
            if (exists $index->{$type}) {
                $pos = $index->{$type};
            } else {
                $index->{$type} = $pos;
                push @map, [$type, 0, 0];
            }

            $result->{$key} = $pos;
            next;
        }

        $idx = join "%", $type, $entry->[1], $entry->[2];

        if (exists $index->{$idx}) {
            $pos = $index->{$idx};
        }
        else {
            $index->{$idx} = $pos;
            push @map, [$type, $entry->[1], $entry->[2]];
        }

        $result->{$key} = $pos;
    }

    push @data, "static const lxb_unicode_idna_entry_t $name\[". scalar(@map) ."] =";
    push @data, "{";
    push @data, "    {". join("},\n    {", map {join(", ", @$_)} @map) ."}";
    push @data, "};";

    return ($result, join("\n", @data));
}

sub idna {
    my $self = shift;
    my ($idna, $idna_types, $dupl, $dupls, $dupl_count, $result);
    my ($pos, $mapping, @map);

    $idna = $self->{IDNA};
    $idna_types = $self->{IDNA_type};

    $dupls = {};
    $result = {};
    $dupl_count = 0;

    print "IDNA:\n";

    foreach my $type (sort {$a cmp $b} keys %$idna_types) {
        print "    $type: ", scalar(@{$idna_types->{$type}}), "\n";

        foreach my $key (@{$idna_types->{$type}}) {
            $mapping = $idna->{$key}->[1];

            unless (@$mapping) {
                $result->{$key} = [$type, 0, 0];
                next;
            }

            $pos = scalar @map;
            $dupl = join(",", @$mapping);

            if (exists $dupls->{$dupl}) {
                $dupl_count += 1;
                $pos = $dupls->{$dupl};
            } else {
                push @map, @$mapping;
            }

            foreach my $i (0..$#$mapping) {
                $dupl = join(",", @$mapping[0..$i]);

                unless (exists $dupls->{$dupl}) {
                    $dupls->{$dupl} = $pos;
                }
            }

            $result->{$key} = [$type, scalar(@$mapping), $pos];
        }
    }

    print "IDNA mapping: ", scalar(@map), "\n";
    print "IDNA mapping duplicates: ", $dupl_count, " of ", 
        scalar @{$idna_types->{"LXB_UNICODE_IDNA_MAPPED"}}, "\n";

    return ($result, \@map);
}

sub used_codepoints {
    my $self = shift;
    my ($idna_types, $result, $comps, $ccc, $quick, $exclude, @keys);

    $result = {};
    $idna_types = $self->{IDNA_type};
    $ccc = $self->{ccc};
    $quick = $self->{normalization_props};
    $exclude = $self->{composition_exclusions};
    $comps = $self->{composition};

    $result->{$_} = 1 foreach keys %{$self->{unicode}};
    $result->{$_} = 1 foreach keys %$ccc;
    $result->{$_} = 1 foreach keys %$quick;
    $result->{$_} = 1 foreach keys %$exclude;
    $result->{$_} = 1 foreach map {$comps->{$_}->[0]} keys %$comps;

    foreach my $type (grep {$_ !~ /_DISALLOWED$/} keys %$idna_types) {
       $result->{$_} = 1 foreach @{$idna_types->{$type}};
    }

    @keys = sort {$a <=> $b} keys %$result;

    return \@keys;
}

sub composition_entries {
    my ($self, $res, $name) = @_;
    my ($pos, $dupl, $idx, $entry, $length, $cp, $index, @data, @map) = @_;
    my $result;

    $result = {};
    $index = {};

    push @map, [0, 0, 0];

    foreach my $key (sort {$a <=> $b} keys %$res) {
        $entry = $res->{$key};
        $length = $entry->[0];
        $idx = $entry->[1];
        $cp = $entry->[2];
        $pos = scalar @map;

        $dupl = join "%", $length, $idx, $cp;

        if (exists $index->{$dupl}) {
            $pos = $index->{$dupl};
        }
        else {
            $index->{$dupl} = $pos;
            push @map, [$length, $idx, $cp];
        }

        $result->{$key} = $pos;
    }

    push @data, "static const lxb_unicode_composition_entry_t $name\[". scalar(@map) ."] =";
    push @data, "{";
    push @data, "    {". join("},\n    {", map {join(", ", @$_)} @map) ."}";
    push @data, "};";

    return ($result, join("\n", @data));
}

sub composition {
    my $self = shift;
    my ($comps, $exclude, $firsts, $entries, $entry, $is, $index, @seconds);
    my ($result, @map, @cps);

    $comps = $self->{composition};
    $exclude = $self->{composition_exclusions};
    $firsts = {};
    $result = {};

    foreach my $cp (sort {$comps->{$a}->[0] <=> $comps->{$b}->[0]} keys %$comps) {
        $entry = $comps->{$cp};
        $is = (exists $exclude->{$cp}) ? "true" : "false";

        $firsts->{$entry->[0]} = [] unless $firsts->{$entry->[0]};
        push @{$firsts->{$entry->[0]}}, [$entry->[1], $cp, $is];
    }

    foreach my $first (sort {$a <=> $b} keys %$firsts) {
        $entries = $firsts->{$first};
        $index = {};
        @seconds = sort {$a->[0] <=> $b->[0]} @$entries;
        @cps = $seconds[0]->[0]..$seconds[-1]->[0];

        $index->{$_->[0]} = $_ foreach @seconds;

        $result->{$first} = [scalar(@cps), scalar @map, $cps[0]];

        foreach my $second (@cps) {
            if (exists $index->{$second}) {
                $entry = $index->{$second};

                push @map, sprintf("{0x%04X, %s}", $entry->[1], $entry->[2]);
            }
            else {
                push @map, "{0x0000, false}";
            }
        }
    }

    print "Composition:\n";
    print "    Count: ", scalar keys %$comps, "\n";
    print "    Entries in table: ", scalar(@map), "\n";

    return ($result, \@map);
}

sub composition_test {
    my ($self, $save_to) = @_;
    my ($comps, $output, $entry, @result);

    $comps = $self->{composition};

    foreach my $cp (sort {$comps->{$a}->[0] <=> $comps->{$b}->[0]} keys %$comps) {
        $entry = $comps->{$cp};

        push @result, sprintf("    {0x%04X, 0x%04X, 0x%04X},", $entry->[0], $entry->[1], $cp);
    }

    $output = $self->template_test_composition(join "\n", @result);
    $self->save_to($save_to, $output);
}

sub decomposition {
    my $self = shift;
    my ($canonical, $compatibility, $can, $com);
    my ($is, $index, $pos, $key, $dupl, $dupl_count, $dupls, @map, $result);

    # Full Canonical Decomposition.
    $canonical = $self->resolve_decomposition($self->{decomposition}, "_undef");
    # Full Compatibility Decomposition.
    $compatibility = $self->resolve_decomposition($self->{decomposition});

    $self->decomposition_check($canonical, $compatibility);

    $index = {};
    $result = {};
    $dupls = {};

    foreach my $i (0..$#$canonical) {
        $can = $canonical->[$i];
        $com = $compatibility->[$i];
        $key = $com->[0];

        $is = array_compare($can->[2], $com->[2]);
        $pos = scalar @map;

        $result->{$key} = [
            lxb_prefix("DECOMPOSITION_TYPE", uc($com->[1])), # unit8_t
            scalar @{$com->[2]},                             # unit8_t
            $pos                                             # unit16_t
        ];

        unless ($is) {
            $result->{$key}->[0] = "LXB_UNICODE_CANONICAL_SEPARATELY|" 
                                   . $result->{$key}->[0];
            push @map, @{$com->[2]}, scalar(@{$can->[2]}), @{$can->[2]};
            next;
        }

        # Remove duplicates

        $dupl = join(",", @{$com->[2]});

        if (exists $dupls->{$dupl}) {
            $dupl_count += 1;
            $pos = $dupls->{$dupl};
            $result->{$key}->[-1] = $pos;
        } else {
            push @map, @{$com->[2]};
        }

        foreach my $i (0..$#{$com->[2]}) {
            $dupl = join(",", @{$com->[2]}[0..$i]);

            unless (exists $dupls->{$dupl}) {
                $dupls->{$dupl} = $pos;
            }
        }
    }

    return ($result, \@map);
}

sub decomposition_check {
    my ($self, $canonical, $compatibility) = @_;
    my ($is, $total, @can, @com);

    die "Elements count in Canonical and Compatibility Decomposition not equal"
        if $#$canonical != $#$compatibility;

    @can = map {$_->[0]} @$canonical;
    @com = map {$_->[0]} @$compatibility;

    die "Codepoints in Canonical and Compatibility Decomposition not equal"
        unless array_compare(\@can, \@com);

    print "Decomposition count: ", scalar(@can), "\n";
    $self->decomposition_difference($canonical, "Canonical",
                                    $compatibility, "Compatibility");
    $self->decomposition_difference($canonical, "Canonical",
                                    $self->{decomposition}, "Original Decomposition");
    $self->decomposition_difference($compatibility, "Compatibility",
                                    $self->{decomposition}, "Original Decomposition");

    $self->decomposition_max_map($self->{decomposition}, "Original Decomposition");
    $self->decomposition_max_map($canonical, "Full Canonical");
    $self->decomposition_max_map($compatibility, "Full Compatibility");
}

sub decomposition_difference {
    my ($self, $can, $name, $decomps, $dname) = @_;
    my ($is, $total, $max, $ncan, $ncom);

    $total = 0;
    $max = 0;

    foreach my $i (0..$#$decomps) {
        $is = array_compare($decomps->[$i]->[2], $can->[$i]->[2]);

        unless ($is) {
            $total += 1;
            $ncan = scalar @{$decomps->[$i]->[2]};
            $ncom = scalar @{$can->[$i]->[2]};

            if ($ncan >= $ncom) {
                $max = $ncan - $ncom if $ncan - $ncom > $max;
            } else {
                $max = $ncom - $ncan if $ncom - $ncan > $max;
            }
        }
    }

    print "Full $name and $dname difference: $total (max difference length: $max)\n";
}

sub decomposition_max_map {
    my ($self, $decomps, $name) = @_;
    my $max;

    $max = 0;

    foreach my $entry (@$decomps) {
        $max = scalar @{$entry->[2]} if $max < scalar @{$entry->[2]};
    }

    print "Maximum map entries for $name: $max\n";
}

sub resolve_decomposition {
    my ($self, $decomps, $type) = @_;
    my ($index, @map);

    $index = {};
    $index->{$_->[0]} = $_ foreach @$decomps;

    foreach my $entry (@$decomps) {
        push @map, [$entry->[0], $entry->[1],
                    $self->resolve_decomposition_entry($entry, $index, $type)];

        die "Failed to resolve decomposition for ". sprintf("%04X", $entry->[0])
            unless @{$map[-1]->[2]};
    }

    return \@map;
}

sub resolve_decomposition_entry {
    my ($self, $entry, $index, $type) = @_;
    my ($res, @map);

    foreach my $cp (@{$entry->[2]}) {
        if (exists $index->{$cp}
            && (!defined $type || $type eq $index->{$cp}->[1]))
        {
            $res = $self->resolve_decomposition_entry($index->{$cp}, $index,
                                                      $type);
        } else {
            $res = [$cp];
        }

        push @map, @$res;
    }

    return \@map;
}

sub function {
    my ($self, $range) = @_;
    my ($max, @result);

    my $entry = $self->{entries};
    my $declr = "const lxb_unicode_entry_t *\n".
                "lxb_unicode_entry(lxb_codepoint_t cp)";

    $max = $range->[-1]->[1];

    push @result, $declr;
    push @result, "{";
    push @result, sprintf("    if (cp > 0x%04X) {", $max);
    push @result, sprintf("        return &$entry\[0];");
    push @result, sprintf("    }\n");
    push @result, $self->branching($range);
    push @result, "\n    return &$entry\[0];";
    push @result, "}";

    return ["LXB_API $declr;", join("\n", @result)];
}

sub branching {
    my ($self, $range) = @_;
    return join "\n", @{$self->branch($range, 0, $#$range, 1)};
}

sub branch {
    my ($self, $range, $from, $to, $ind) = @_;
    my ($idx, $space, $entry, $table, $entries, @result);

    $idx = ($from + int(($to - $from) / 2));
    return \@result unless exists $range->[$idx];

    $space .= "    " foreach 1..$ind;
    $entries = $self->{entries};

    $entry = $range->[$idx];
    $table = table_name($entry->[0], $entry->[1]);

    # IF state
    push @result, sprintf("%s"."if (cp < $entry->[1]) {", $space);

    if ($idx <= $from) {
        push @result, sprintf("$space    return &$entries\[$table\[cp - $entry->[0]]];");
    } else {
        push @result, @{$self->branch($range, $from, $idx - 1, $ind + 1)};
    }

    push @result, $space. "}";

    if ($entry->[1] >= $range->[-1]->[1]) {
        return \@result;
    }

    # ELSE state
    $table = table_name($entry->[2], $range->[$idx + 1]->[1]);

    push @result, sprintf("%s"."else if (cp >= $entry->[2]) {", $space);

    if ($idx == $to) {
        push @result, sprintf("$space    return &$entries\[$table\[cp - $entry->[2]]];");
    } else {
        push @result, @{$self->branch($range, $idx + 1, $to, $ind + 1)};
    }

    push @result, $space. "}";

    return \@result;
}

sub range_data_codepoints {
    my ($self, $range, $sub, $name) = @_;
    my ($num, $from, $to, $dif, $uint, $orig, $sum, $size, @result, @lines);

    $sum = 0;
    $size = 0;

    print "Generating basic tables:\n";

    foreach my $entry (@$range) {
        $dif = $entry->[1] - $entry->[0];
        $from = $entry->[0];
        $to = $entry->[1] - 1;
        $uint = ($dif < (1 << 16) - 1) ? "uint16_t" : "uint32_t";
        $orig = table_name($entry->[0], $entry->[1]);

        print sprintf("    %8u -> %8u: %8u ($uint) ($orig)", $from, $to, $dif), "\n";

        foreach my $cp ($from..$to) {
            push @lines, sprintf("    %s, /* %04X (%u) */",
                                 $sub->($cp, $uint, $entry->[0], $entry->[1]),
                                 $cp, $cp);
        }

        $num = scalar @lines;
        $sum += $num;

        $size += ($uint eq "uint16_t") ? $num * 2 : $num * 4;

        push @result, sprintf("/* From: %04X; To: %04X */", $from, $to);
        push @result, "static const $uint $orig\[". scalar @lines ."] =";
        push @result, "{";
        push @result, @lines;
        push @result, "};\n";

        @lines = ();
    }

    print "Total entries: $sum\n";
    print "Generated tables: ", scalar @$range, "\n";
    print "Tables size: ", $size, " Byte; ", int($size / 1000), " Kilobyte \n";

    return join "\n", @result;
}

sub data_codepoints {
    my ($self, $map, $name) = @_;
    my @result;

    push @result, "static const lxb_codepoint_t $name\[". scalar @$map ."] =";
    push @result, "{";

    for (my $i = 0; $i < scalar @$map; $i += 8) {
        if ($i + 8 >= $#$map) {
            push @result, "    ". join ", ", map { sprintf("0x%04X", $_) } @{$map}[$i..$#$map];
        } else {
            push @result, "    ". join(", ", map { sprintf("0x%04X", $_) } @{$map}[$i..$i + 7]) .",";
        }
    }
 
    push @result, "};";

    return join "\n", @result;
}

sub data_map {
    my ($self, $map, $type, $name) = @_;
    my @result;

    push @result, "static const $type $name\[". scalar @$map ."] =";
    push @result, "{";

    for (my $i = 0; $i < scalar @$map; $i += 8) {
        if ($i + 8 >= $#$map) {
            push @result, "    ". join(", ", @{$map}[$i..$#$map]);
        } else {
            push @result, "    ". join(", ", @{$map}[$i..$i + 7]) .",";
        }
    }
 
    push @result, "};";

    return join "\n", @result;
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

sub array_compare {
    my ($f, $t) = @_;

    return 0 if $#$f != $#$t;

    foreach (0..$#$f) {
        if ($f->[$_] != $t->[$_]) {
            return 0;
        }
    }

    return 1;
}

sub lxb_prefix {
    return join "_", "LXB_UNICODE", @_;
}

sub table_name {
    return join "_", "lxb_unicode_table_map", @_;
}

sub lxb_quick {
    return lxb_prefix("QUICK", @_);
}

sub lxb_idna {
    return lxb_prefix("IDNA", @_);
}

sub save_to {
    my ($self, $filepath, $data) = @_;

    open(my $fh, '>', $filepath) || die "Failed to save to file: $filepath";
    binmode $fh;

    print $fh $data;

    close($fh);
}

sub print_decomposition_types {
    my $self = shift;
    my $dtypes = $self->{decomposition_types};

    print("\nDecomposition Enum (copy this to lexbor/unicode/base.h):\n");
    print("/* Begin */\n");
    print("enum {\n");
    print("    ", lxb_prefix("DECOMPOSITION_TYPE__UNDEF"), " = 0x00,\n");

    foreach my $key (grep {$_ ne "_undef"} sort {$a cmp $b} keys %$dtypes) {
        print("    ", lxb_prefix("DECOMPOSITION_TYPE", uc($key)), ",\n");
    }

    print("    ", lxb_prefix("DECOMPOSITION_TYPE__LAST_ENTRY"), "\n");
    print("};\n");
    print("#define LXB_UNICODE_CANONICAL_SEPARATELY        (1 << 7)\n");
    print("#define LXB_UNICODE_IS_CANONICAL_SEPARATELY(a)  ((a) >> 7)\n");
    print("#define LXB_UNICODE_DECOMPOSITION_TYPE(a)       ((a) & ~(1 << 7))\n");
    print("typedef uint8_t lxb_unicode_decomposition_type_t;\n");
    print("/* End */\n\n");
}

sub print_decomposition_quick_types {
    my $self = shift;
    my ($types, $count);

    $count = 0;
    $types = $self->{normalization_quick_types};

    print("\nDecomposition Quick Enum (copy this to lexbor/unicode/base.h):\n");
    print("/* Begin */\n");
    print("enum {\n");
    print("    ", lxb_quick("_UNDEF"), " = 0x00,\n");

    foreach my $key (sort {$a cmp $b} keys %$types) {
        print("    $key = 1 << $count,\n");
        $count += 1;
    }

    print("};\n");
    print("typedef uint8_t lxb_unicode_quick_type_t;\n");
    print("/* End */\n\n");
}

sub print_idna_types {
    my $self = shift;
    my $types;

    $types = $self->{IDNA_type};;

    print("\nIDNA Types Enum (copy this to lexbor/unicode/base.h):\n");
    print("/* Begin */\n");
    print("enum {\n");
    print("    ", lxb_idna("_UNDEF"), " = 0x00,\n");

    foreach my $key (sort {$a cmp $b} keys %$types) {
        print("    $key,\n");
    }

    print("};\n");
    print("typedef uint8_t lxb_unicode_idna_type_t;\n");
    print("/* End */\n\n");
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
 * This file generated by the script "utils/lexbor/unicode/build.pl"!
 * Do not change this file!
 */

#ifndef LEXBOR_$name\_H
#define LEXBOR_$name\_H

#ifdef __cplusplus
extern "C" {
#endif


$data


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_$name\_H */
EOM

    return $temp;
}

sub template_test_composition {
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
 * This file generated by the script "utils/lexbor/unicode/build.pl"!
 * Do not change this file!
 */

#include <unit/test.h>
#include <lexbor/unicode/unicode.h>


struct {
    lxb_codepoint_t first;
    lxb_codepoint_t second;
    lxb_codepoint_t cp;
}
typedef test_unicode_entry_t;


static const test_unicode_entry_t entries[] =
{
$data
};

TEST_BEGIN(composition)
{
    const lxb_unicode_composition_cp_t *entry;
    const size_t length = sizeof(entries) / sizeof(test_unicode_entry_t);

    for (size_t i = 0; i < length; i++) {
        entry = lxb_unicode_composition_cp(entries[i].first, entries[i].second);
        test_eq(entry->cp, entries[i].cp);
    }
}
TEST_END

int
main(int argc, const char *argv[])
{
    TEST_INIT();

    TEST_ADD(composition);

    TEST_RUN("lexbor/unicode/composition");
    TEST_RELEASE();
}
EOM

    return $temp;
}
