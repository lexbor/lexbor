#!/usr/bin/perl

# usage:
# make.pl <UnicodeData.txt> <CompositionExclusions.txt> <DerivedNormalizationProps.txt> <save_to_file_path>

use utf8;
use strict;

my $unicode = new Unicode $ARGV[0], $ARGV[1], $ARGV[2];

$unicode->make_quick_check();
$unicode->build();

if ($ARGV[3]) {
    $unicode->save($ARGV[3]);
}

package Unicode;

sub new {
    my ($class, $filepath, $compose_exc_filepath, $norm_props_filepath) = @_;
    my ($data, $dec_types, $dec_types_raw, $comp, $decp, $comp_excl);
    my ($quick, $quick_name, $quick_type);

    my $self = {
        decomposition_types     => [],
        decomposition_types_raw => [],
        decomposition           => {},
        composition             => {},
        composition_excl        => {},
        ccc                     => {},
        quick                   => {},
        data                    => [],
        result                  => [],
        result_compose          => [],
        _prefix                 => "lxb_unicode"
    };

    bless $self, $class;

    $comp_excl = {};

    open my $fh, $compose_exc_filepath || die "Failed to open file: $compose_exc_filepath";

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

        $comp_excl->{$line} = 1;
    }

    close($fh);

    if (keys %$comp_excl == 0) {
        die "Failed parse Composition Exclusions file: $compose_exc_filepath";
    }

    $quick = {};

    open my $fh, $norm_props_filepath || die "Failed to open file: $norm_props_filepath";

    foreach my $line (<$fh>) {
        if ($line =~ /^\s+$/ || $line eq "") {
            next;
        }

        if ($line =~ /^\s*#/) {
            $quick_name = undef;
            $quick_type = undef;

            if ($line =~ /^# (\S+)_Quick_Check=(\S+)$/) {
                $quick_name = $1;
                $quick_type = uc($2);
            }

            next;
        }

        if (!defined $quick_name) {
            next;
        }

        $line =~ s/\s*;.*//s;

        if ($line =~ /^[0-9a-fA-F]+$/) {
            $quick->{$line} = {} unless exists $quick->{$line};
            $quick->{$line}->{"$quick_name\_$quick_type"} = 1;
        }
        elsif ($line =~ /^([0-9a-fA-F]+)\.\.([0-9a-fA-F]+)$/) {
            for my $cp (hex($1)..hex($2)) {
                $cp = sprintf("%04X", $cp);

                $quick->{$cp} = {} unless exists $quick->{$cp};
                $quick->{$cp}->{"$quick_name\_$quick_type"} = 1;
            }
        }
        else {
            die "Failed to get Derived Normalization Props entry: $line";
        }
    }

    close($fh);

    if (keys %$quick == 0) {
        die "Failed parse Derived Normalization Props file: $norm_props_filepath";
    }

    open my $fh, $filepath || die "Failed to open file: $filepath";

    # GC = General_Category
    # CCC = Canonical_Combining_Class
    # BC = Bidi_Class
    # DTDM = Decomposition_Type and Decomposition_Mapping
    # NTNV = Numeric_Type and Numeric_Value
    # BM = Bidi_Mirrored
    # U1N = Unicode_1_Name
    # ISOC = (Obsolete as of 5.2.0; Deprecated and Stabilized as of 6.0.0)
    # SUPPM = Simple_Uppercase_Mapping
    # SLOWM = Simple_Lowercase_Mapping
    # STITM = Simple_Titlecase_Mapping

    $comp = {};
    $decp = {};
    $data = [];
    $dec_types = {};
    $dec_types_raw = {};

    foreach my $line (<$fh>) {
        my ($cp, $name, $GC, $CCC, $BC, $DTDM, $NTNV,
            $BM, $U1N, $ISOC, $SUPPM, $SLOWM, $STITM) = split /;/, $line;
        my $id;
        my @map = split /\s+/, $DTDM;

        my $dec = {};
        my $dec_type = "_undef";

        if (scalar @map > 0) {
            if ($map[0] =~ /<(.*)>/) {
                $dec_type = $1;
                shift @map;
            }
        }

        $dec->{type} = uc $self->lxb_prefix("DECOMPOSITION_TYPE", $dec_type);
        $dec->{map} = \@map;

        if ($dec_type ne "_undef") {
            $dec_types->{$dec->{type}} = 1;
            $dec_types_raw->{$dec_type} = 1;
        }
        elsif (int($CCC) == 0 && scalar @map > 1) {
            $id = (hex($map[0]) % 65535) << 16 | (hex($map[1]) % 65535);

            if (exists $comp->{$id}) {
                my $er = $comp->{$id}->[1];

                die "Found dupl: " . join(", ", @map)
                    . " (" . join(", ", map {hex($_)} @map) . ") == "
                    . join(", ", @$er)
                    . " (" . join(", ", map {hex($_)} @$er) . ")";
            }

            $comp->{$id} = [$cp, \@map, exists $comp_excl->{$cp} ? 1 : 0];
        }

        push @$data, [$cp, $name, $GC, $CCC, $BC, $dec, $NTNV,
                      $BM, $U1N, $ISOC, $SUPPM, $SLOWM, $STITM, $quick->{$cp}];

        if (scalar @map > 0) {
            $decp->{$cp} = $data->[-1];
        }
    }

    close($fh);

    $self->{data} = $data;
    $self->{quick} = $quick;
    $self->{composition} = $comp;
    $self->{composition_excl} = $comp_excl;
    $self->{decomposition} = $decp;

    @{$self->{decomposition_types}} = sort keys %$dec_types;
    @{$self->{decomposition_types_raw}} = sort keys %$dec_types_raw;

    unshift @{$self->{decomposition_types}}, uc $self->lxb_prefix("DECOMPOSITION_TYPE__UNDEF");
    push @{$self->{decomposition_types}}, uc $self->lxb_prefix("DECOMPOSITION_TYPE__LAST_ENTRY");

    $self->_full_decomposition($decp);

    return $self;
}

sub _full_decomposition {
    my ($self, $decp)= @_;
    my ($ret, $kret, $entry);

    foreach my $key (keys %$decp) {
        $entry = $decp->{$key};

        $ret = $self->_full_decomposition_map($decp, $entry->[5]->{map});
        $kret = $self->_full_decomposition_compatibility_map($decp, $entry->[5]->{map});
        $entry->[5]->{cmap} = $ret;
        $entry->[5]->{kmap} = $kret;
    }
}

sub _full_decomposition_map {
    my ($self, $decp, $map)= @_;
    my $res = [];
    my $ret;

    foreach my $cp (@$map) {
        if (exists $decp->{$cp} && $decp->{$cp}->[5]->{type} eq "LXB_UNICODE_DECOMPOSITION_TYPE__UNDEF") {
            $ret = $self->_full_decomposition_map($decp, $decp->{$cp}->[5]->{map});
            push @$res, @$ret;
        }
        else {
            push @$res, $cp;
        }
    }

    return $res;
}

sub _full_decomposition_compatibility_map {
    my ($self, $decp, $map)= @_;
    my $res = [];
    my $ret;

    foreach my $cp (@$map) {
        if (exists $decp->{$cp}) {
            $ret = $self->_full_decomposition_compatibility_map($decp, $decp->{$cp}->[5]->{map});
            push @$res, @$ret;
        }
        else {
            push @$res, $cp;
        }
    }

    return $res;
}

sub composition {
    my $self = shift;
    my $comp = $self->{composition};
    my ($data, $dupl, $seq, $seq_str, $cps, $entry);

    $dupl = {};
    $data = [];

    foreach my $key (sort {$a <=> $b} keys %$comp) {
        print "$key:\n";

        $cps = $comp->{$key};

        $seq = $cps->[1];
        $seq_str = join(", ", @$seq);

        print "    $seq_str -> ", $cps->[0], "\n";

        if (exists $dupl->{$seq_str}) {
            die "Found dupl: $seq_str\n";
        }

        if (scalar @$seq != 2) {
            die "Bad seq: $seq_str\n";
        }

        $dupl->{$seq_str} = 1;

        push @$data, [sprintf("%x", $key), $cps];
    }

    my ($hash, $size) = $self->make_from_array($data, 54);
    $self->composition_hash($hash, $size, "composition");
}

sub composition_hash {
    my ($self, $data, $table_size, $table_name) = @_;
    my ($ref, $size, $node_name);
    my (@res, @nodes);

    foreach my $key (0..$table_size - 1) {
        if (!exists $data->{$key}) {
            push @nodes, "{.entry = NULL, .table = NULL}";
            next;
        }

        $ref = $data->{$key};

        push @nodes, $self->composition_hash_node($ref);
    }

    $node_name = $self->lxb_prefix("nodes", $table_name);
    $table_name = $self->lxb_prefix($table_name);
    $size = scalar @nodes;

    $ref = join "\n", "static const lxb_unicode_compose_node_t $node_name\[$size\] = {",
                      "    " . join(",\n    ", @nodes), "};";

    push @{$self->{result_compose}}, $ref;

    $ref = join "", "static const lxb_unicode_compose_table_t $table_name = ",
                    "{.length = $table_size, .nodes = $node_name};";

    push @{$self->{result_compose}}, $ref;

    return $table_name;
}

sub composition_hash_node {
    my ($self, $ref) = @_;
    my ($origin, $entry_name, $entry_str, $hash, $size, $name, $table);

    $origin = shift @$ref;

    $entry_str = join "", ".idx = 0x", $origin->[0] ,
                          ", .cp = 0x", $origin->[1][0],
                          ", .exclusion = ", ($origin->[1][2]) ? "true" : "false";

    if (scalar @$ref > 0) {
        ($hash, $size) = $self->make_from_array($ref);
        $name = "&" . $self->composition_hash($hash, $size,
                                              join("_", "comp_map", uc $origin->[0]));
    }
    else {
        $name = "NULL";
    }

    $entry_name = $self->lxb_prefix("codepoints", $origin->[0]);

    $ref = join "", "static const lxb_unicode_compose_entry_t $entry_name = {$entry_str};";

    push @{$self->{result_compose}}, $ref;

    return "{.entry = &$entry_name, .table = $name}";
}

sub build {
    my $self = shift;

    my ($hash, $size) = $self->make_from_array($self->{data}, 1000);
    $self->make_hash($hash, $size, "map");

    $unicode->composition();
}

sub save {
    my ($self, $filepath) = @_;
    my $year = 1900 + (localtime)[5];
    my $res = $unicode->result;
    my $compose = $unicode->result_compose;
    my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

#ifndef LEXBOR_UNICODE_TABLES_H
#define LEXBOR_UNICODE_TABLES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Composition map. */

$compose

/* Unicode data. */

$res


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_UNICODE_TABLES_H */
EOM

    open(my $fh, '>', $filepath) || die "Failed to save to file: $filepath";
    binmode $fh;

    print $fh $temp, "\n";

    close($fh);
}

sub result {
    return join "\n", @{$_[0]->{result}};
}

sub result_compose {
    return join "\n", @{$_[0]->{result_compose}};
}

sub decomposition_types {
    return $_[0]->{decomposition_types};
}

sub decomposition_enum {
    my $self = shift;
    return $self->make_enum($self->decomposition_types, "decomposition_type_t");
}

sub make_enum {
    my ($self, $dt, $name) = @_;
    my @res;

    push @res, (defined $name) ? "typedef enum {" : "enum {";
    push @res, join "", "    ", $dt->[0], " = 0x00,";

    foreach my $type (@$dt[1..scalar @$dt - 1]) {
        push @res, "    $type,";
    }

    chop($res[-1]);

    if (defined $name) {
        push @res, "}";
        push @res, join "", $self->lxb_prefix($name), ";";
    }
    else {
        push @res, "};";
    }

    return \@res;
}

sub lxb_prefix {
    my $self = shift;

    return join "_", $self->{_prefix}, @_;
}

sub make_hash {
    my ($self, $data, $table_size, $table_name) = @_;
    my ($ref, $size, $node_name);
    my (@res, @nodes);

    foreach my $key (0..$table_size - 1) {
        if (!exists $data->{$key}) {
            push @nodes, "{.entry = NULL, .table = NULL}";
            next;
        }

        $ref = $data->{$key};

        push @nodes, $self->make_hash_node($ref);
    }

    $node_name = $self->lxb_prefix("nodes", $table_name);
    $table_name = $self->lxb_prefix($table_name);
    $size = scalar @nodes;

    $ref = join "\n", "static const lxb_unicode_node_t $node_name\[$size\] = {",
                      "    " . join(",\n    ", @nodes), "};";

    push @{$self->{result}}, $ref;

    $ref = join "", "static const lxb_unicode_table_t $table_name = ",
                    "{.length = $table_size, .nodes = $node_name};";

    push @{$self->{result}}, $ref;

    return $table_name;
}

sub make_hash_node {
    my ($self, $ref) = @_;
    my ($origin, $entry_name, $entry_str, $hash, $size, $name, $table);

    $origin = shift @$ref;
    $entry_str = $self->make_hash_entry($origin);

    if (scalar @$ref > 0) {
        ($hash, $size) = $self->make_from_array($ref);
        $name = "&" . $self->make_hash($hash, $size,
                                        join("_", "map", uc $origin->[0]));
    }
    else {
        $name = "NULL";
    }

    $entry_name = $self->lxb_prefix("entry", $origin->[0]);

    $ref = join "", "static const lxb_unicode_entry_t $entry_name = $entry_str;";

    push @{$self->{result}}, $ref;

    return "{.entry = &$entry_name, .table = $name}";
}

sub make_hash_entry {
    my ($self, $entry) = @_;
    my ($cp, $name, $GC, $CCC, $BC, $DTDM, $NTNV,
        $BM, $U1N, $ISOC, $SUPPM, $SLOWM, $STITM) = @$entry;
    my ($dec, $str, $name, $dcount, $types, $prefix, @names);
    my ($quick);

    $types = {"map" => "", "cmap" => "c", "kmap" => "k"};

    if (scalar @{$DTDM->{map}} != 0) {
        foreach my $key (sort {$a cmp $b} keys %$types) {
            $prefix = $types->{$key};
            $dcount = scalar @{$DTDM->{$key}};

            $name = $self->lxb_prefix("decomposition", "cp", $prefix, $cp);
            $str = "{" . join(", ", map {"0x$_"} @{$DTDM->{$key}}) . "}";

            $str = join "", "static const lxb_codepoint_t $name\[$dcount\] = $str;";
            push @{$self->{result}}, $str;

            $dec = join "", "{.type = ", $DTDM->{type}, ",",
                            " .mapping = $name,",
                            " .length = ", $dcount,"}";

            $name = $self->lxb_prefix("decomposition", $prefix, $cp);

            $str = join "", "static const lxb_unicode_decomposition_t $name = $dec;";
            push @{$self->{result}}, $str;

            push @names, "&$name";
        }
    }

    $quick = $self->make_quick_by_entry($entry);

    return join "", "{.cp = 0x", uc($cp), ", .ccc = ", $CCC, ", .quick = $quick",
                    ", .de = ", exists $names[2] ? $names[2] : "NULL" ,
                    ", .cde = ", exists $names[0] ? $names[0] : "NULL" ,
                    ", .kde = ", exists $names[1] ? $names[1] : "NULL", "}";
}

sub make_quick_by_entry {
    my ($self, $entry) = @_;
    my ($quick, $name, $type, @enum);

    $quick = $entry->[13];

    if (!defined $quick ) {
        return uc $self->lxb_prefix("quick", "undef");
    }

    foreach my $name (sort {$a cmp $b} keys %$quick) {
        ($name, $type) = split /_/, $name;

        push @enum, uc $self->lxb_prefix($name, "quick", $type);
    }

    return join "|", @enum;
}

sub make_quick_check {
    my ($self) = @_;
    my ($data, $quick);

    %$data = map {$_->[0] => $_} @{$self->{data}};
    $quick = $self->{quick};

    foreach my $cp (sort {hex($a) <=> hex($b)} keys %$quick) {
        if (!exists $data->{$cp}) {
            my $sp = join(", ", sort {$a cmp $b} keys %{$quick->{$cp}});

            if (hex($cp) >= 0xAC00 && hex($cp) <= 0xD7A3 && $sp ne "NFD_NO, NFKD_NO") {
                print "Quick codepoint not in Unicode Data: ", $cp, " == ", $sp, "\n";
            }
        }
    }
}

sub make_from_array {
    my ($self, $data, $size) = @_;
    my ($id, $hash);

    if (scalar(@$data) == 0) {
        return {};
    }

    if (!$size) {
        $size = $self->test_from_array($data, 1);
        if ($size == -1) {
            print "Failed to find size for ", scalar(@$data), "\n";
            exit(1);
        }
    }

    $hash = {};

    foreach my $entry (@$data) {
        $id = make_hash_id(hex($entry->[0]), $size);

        if (!exists $hash->{$id}) {
            $hash->{$id} = [];
        }

        push @{$hash->{$id}}, $entry;
    }

    return ($hash, $size);
}

sub test_from_array {
    my ($self, $data, $max_deep) = @_;
    my ($id, $hash, $len, $overflow);

    $len = scalar(@$data);

    for my $n ($len..($len * 55)) {
        $overflow = 0;
        $hash = {};

        foreach my $entry (@$data) {
            $id = make_hash_id(hex($entry->[0]), $n);

            if (!exists $hash->{$id}) {
                $hash->{$id} = 0;
            }

            $hash->{$id} += 1;

            if ($hash->{$id} > $max_deep) {
                $overflow = 1;
                last;
            }
        }

        if ($overflow == 0) {
            return $n;
        }
    }

    return -1;
}

sub make_hash_id {
    return $_[0] % $_[1];
}
