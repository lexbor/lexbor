#!/usr/bin/perl

# use:
# make.pl <UnicodeData.txt> <CompositionExclusions.txt> <DerivedNormalizationProps.txt> <idnaMappingTable.txt> <save_to_file_path>

use utf8;
use strict;

my $unicode = new Unicode $ARGV[0], $ARGV[1], $ARGV[2], $ARGV[3];

$unicode->make_quick_check();
$unicode->composition();
$unicode->build();

if ($ARGV[4]) {
    $unicode->save($ARGV[4]);
}

package Unicode;

sub new {
    my ($class, $filepath, $compose_exc_filepath, $norm_props_filepath, $idna_filepath) = @_;
    my ($data, $dec_types, $dec_types_raw, $comp, $decp, $comp_excl);
    my ($quick, $quick_name, $quick_type, $idna, $idna_map_max);

    my $self = {
        decomposition_types     => [],
        decomposition_types_raw => [],
        decomposition           => {},
        composition             => {},
        composition_excl        => {},
        ccc                     => {},
        quick                   => {},
        idna                    => {},
        idna_types              => "",
        idna_map_max            => 0,
        data                    => {},
        result                  => {},
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

    $idna = {};
    $idna_map_max = 0;

    open my $fh, $idna_filepath || die "Failed to open file: $idna_filepath";

    foreach my $line (<$fh>) {
        if ($line =~ /^\s*#/ || $line =~ /^\s+$/ || $line eq "") {
            next;
        }

        $line =~ s/\s*#.*//s;

        my @res;
        my @items = split /\s*;\s*/, $line;

        push @res, uc $self->lxb_prefix("IDNA", $items[1]);

        if (exists $items[2] && $items[2] ne "") {
            my @map = split /\s+/, $items[2];
            push @res, \@map;
        }
        else {
            push @res, [];
        }

        if (scalar @{$res[-1]} > $idna_map_max) {
            $idna_map_max = scalar @{$res[-1]};
        }

        if ($items[0] =~ /^([0-9a-fA-F]+)$/) {
            $idna->{$1} = \@res;
        }
        elsif ($items[0] =~ /^([0-9a-fA-F]+)\.\.([0-9a-fA-F]+)$/) {
            for my $cp (hex($1)..hex($2)) {
                $cp = sprintf("%04X", $cp);

                $idna->{$cp} = \@res;
            }
        }
        else {
            die "Failed to get IDNA Mapping Table entry: $line";
        }
    }

    close($fh);

    if (keys %$quick == 0) {
        die "Failed parse IDNA Mapping Table file: $norm_props_filepath";
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
    $data = {};
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

        
        $data->{$cp} = [$cp, $name, $GC, $CCC, $BC, $dec, $NTNV,
                        $BM, $U1N, $ISOC, $SUPPM, $SLOWM, $STITM, $quick->{$cp}];

        if (scalar @map > 0) {
            $decp->{$cp} = $data->{$cp};
        }
    }

    close($fh);

    $self->{data} = $data;
    $self->{quick} = $quick;
    $self->{idna} = $idna;
    $self->{idna_map_max} = $idna_map_max;
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
        $cps = $comp->{$key};

        $seq = $cps->[1];
        $seq_str = join(", ", @$seq);

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
    my ($data, $cp, $type, $uniq, $map, $len, $idna, $idna_name, $str, $entry);
    my ($ucode_name, $copy, $name, $pos, $prev, $res, $dec, $idx, $delta);
    my (@table, @idna_map, @entries, @udata, @idna_types, @types);

    $data = $self->{data};
    $idna = $self->{idna};
    $uniq = {};
    $res = {};
    $idx = 1;
    $copy = {"start" => 0, "str" => ""};

    foreach my $id (0x0000..0x10FFFF) {
        $cp = sprintf("%04X", $id);

        if (exists $idna->{$cp}) {
            $type = $idna->{$cp}->[0];
            $map = $idna->{$cp}->[1];
        }
        else {
            die "IDNA code point not exists: $cp\n";
        }

        if (!exists $uniq->{$type}) {
            $uniq->{$type} = $idx;
            $idx += 1;
        }

        push @idna_types, $uniq->{$type};

        $ucode_name = "NULL";
        $idna_name = "NULL";
        $str = undef;
        $len = scalar @$map;

        if ($len > 0) {
            $idna_name = $self->lxb_prefix("idna_map", $cp);
            $str = "0x" . join ", 0x", @$map;

            $str = "static const lxb_unicode_idna_map_t $idna_name = "
                   . "{.cps = (lxb_codepoint_t[]) {$str}, .length = $len};";

            $idna_name = "&$idna_name";
        }

        push @idna_map, $str;
        $str = undef;

        if (exists $data->{$cp}) {
            $dec = $data->{$cp}->[5];

            if (scalar @{$dec->{map}} > 0 || $dec->{type} ne "LXB_UNICODE_DECOMPOSITION_TYPE__UNDEF"
               || $data->{$cp}->[3] > 0 || scalar keys %{$data->{$cp}->[13]} > 0)
            {
                $ucode_name = $self->lxb_prefix("entry", $cp);
                ($entry, $str) = $self->make_hash_entry($data->{$cp});

                $str = "$str\n" . "static const lxb_unicode_entry_t $ucode_name = $entry;";

                $ucode_name = "&$ucode_name";
            }
        }

        push @entries, $str;
        $str = undef;

        $name = "NULL";

        if ($ucode_name ne "NULL" || $idna_name ne "NULL") {
            $name = $self->lxb_prefix("data", $cp);
            $str = "static const lxb_unicode_data_t $name = {$ucode_name, $idna_name};";
            $name = "&$name";
        }

        push @udata, $str;
        push @table, $name;

        if ($copy->{str} ne $name) {
            $copy->{start} = $id;
            $copy->{str} = $name;
        }
    }

    $len = 0x10FFFF - $copy->{start};

    if ($len > 500000) {
        splice @table, $copy->{start};
        splice @udata, $copy->{start};
        splice @entries, $copy->{start};
        splice @idna_map, $copy->{start};

        print "Remove duples: ", $copy->{str}, "\n";
        print "From ", sprintf("%04X", $copy->{start}), " to $cp; Count: $len\n";
    }

    my $limit = 5000;

    for (my $id = 0x0000; $id < scalar @table; $id += $limit) {
        my @ret;

        $pos = int($id / $limit);

        $len = ($id + $limit > scalar @table) ? scalar @table - $id: $limit;
        $name = $self->lxb_prefix("table", $pos);

        $delta = ($id + $len) - 1;

        $str = "    " . join ",\n    ", @table[$id...$delta];
        $str = "static const lxb_unicode_data_t *$name\[$len\] = {\n$str\n};";

        if ($pos == 111) {
            print join("\n", @udata[$id...$delta]), "\n";
        }

        push @ret, grep {$_ ne undef} @idna_map[$id...$delta];
        push @ret, grep {$_ ne undef} @entries[$id...$delta];
        push @ret, grep {$_ ne undef} @udata[$id...$delta];
        push @ret, $str;

        $self->{result}->{$pos} = \@ret;
    }

    $prev = 0;
    $idx = 0;

    for (my $id = scalar @idna_types - 1; $id != 0; $id--) {
        if ($idna_types[$id] != 0x04) {
            print "IDNA removed last duplicates (", scalar @idna_types - $id,"). ", 
                  "Now last entry is ", sprintf("%04X", $id), "\n";

            splice @idna_types, $id + 1;
            last;
        }
    }

    foreach my $id (0x0000..scalar @idna_types - 1) {
        if ($prev != $idna_types[$id]) {
            $len = $id - $idx;

            if ($len > 500000) {
                splice @types, $idx;

                print "IDNA remove duplicates: ", sprintf("0x%02X", $prev), "\n";

                print "From ", sprintf("%04X", $idx), " to ",
                      sprintf("%04X", $id), "; Count: $len\n";
            }

            $idx = $id;
            $prev = $idna_types[$id];
        }

        push @types, $idna_types[$id];
    }

    $len = scalar @types;
    $str = join ", ", @types;
    $str = "static const lxb_unicode_idna_type_t lxb_unicode_idna_types[$len] = {$str};";

    $self->{idna_types} = $str;

    print "Includes:\n";

    foreach my $id (0...$pos) {
        print "#include \"lexbor/unicode/table_$id.h\"\n";
    }

    print "\nFor table:\n";

    foreach my $id (0...$pos) {
        print "    lxb_unicode_table_$id,\n";
    }

    print "\ntypedef enum {\n", "    ",
          join(",\n    ",
               map {"$_ = " . sprintf("0x%02x", $uniq->{$_})}
                   sort {$uniq->{$a} <=> $uniq->{$b}}
                        keys %$uniq), "\n}\n",
           "lxb_unicode_idna_type_t;\n";

    return $self->{result};
}

sub save {
    my ($self, $filepath) = @_;
    my $compose = $unicode->result_compose;
    my $result = $unicode->result;
    my $types = $self->{idna_types};

    foreach my $n (sort {$a <=> $b} keys %$result) {
        my $structs = join "\n", @{$result->{$n}};

        $self->save_to($filepath, $n, $structs);
    }

    $self->save_to($filepath, "compose", $compose);
    $self->save_to($filepath, "idna_types", $types);
}

sub template {
    my ($self, $name, $data) = @_;
    my $year = 1900 + (localtime)[5];
    
    $name = uc $name;

    my $temp = <<EOM;
/*
 * Copyright (C) $year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 */

#ifndef LEXBOR_UNICODE_TABLES_$name\_H
#define LEXBOR_UNICODE_TABLES_$name\_H

#ifdef __cplusplus
extern "C" {
#endif


$data


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LEXBOR_UNICODE_TABLES_$name\_H */
EOM

    return $temp;
}

sub save_to {
    my ($self, $filepath, $name, $data) = @_;

    my $res = $self->template($name, $data);
    my $path = "$filepath/table_$name.h";

    open(my $fh, '>', $path) || die "Failed to save to file: $path";
    binmode $fh;

    print $fh $res;

    close($fh);
}

sub result {
    return $_[0]->{result};
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

sub make_hash_entry {
    my ($self, $entry) = @_;
    my ($cp, $name, $GC, $CCC, $BC, $DTDM, $NTNV,
        $BM, $U1N, $ISOC, $SUPPM, $SLOWM, $STITM) = @$entry;
    my ($dec, $str, $name, $dcount, $types, $prefix, @names);
    my ($quick, $res);

    $res = [];
    $types = {"map" => "", "cmap" => "c", "kmap" => "k"};

    if (scalar @{$DTDM->{map}} != 0) {
        foreach my $key (sort {$a cmp $b} keys %$types) {
            $prefix = $types->{$key};
            $dcount = scalar @{$DTDM->{$key}};

            $name = $self->lxb_prefix("decomposition", "cp", $prefix, $cp);
            $str = "{" . join(", ", map {"0x$_"} @{$DTDM->{$key}}) . "}";

            $str = join "", "static const lxb_codepoint_t $name\[$dcount\] = $str;";
            push @$res, $str;

            $dec = join "", "{.type = ", $DTDM->{type}, ",",
                            " .mapping = $name,",
                            " .length = ", $dcount,"}";

            $name = $self->lxb_prefix("decomposition", $prefix, $cp);

            $str = join "", "static const lxb_unicode_decomposition_t $name = $dec;";
            push @$res, $str;

            push @names, "&$name";
        }
    }

    $quick = $self->make_quick_by_entry($entry);

    return (join("", "{.cp = 0x", uc($cp), ", .ccc = ", $CCC, ", .quick = $quick",
                    ", .de = ", exists $names[2] ? $names[2] : "NULL" ,
                    ", .cde = ", exists $names[0] ? $names[0] : "NULL" ,
                    ", .kde = ", exists $names[1] ? $names[1] : "NULL", "}"),
            join("\n", @$res));
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

    $data = $self->{data};
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
