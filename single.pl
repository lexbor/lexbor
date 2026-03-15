#!/usr/bin/perl

#############################################################
# Copyright (C) 2025-2026 Alexander Borisov
#
# Author: Alexander Borisov <borisov@lexbor.com>
#
# This file is part of Lexbor.
##############################################################

use utf8;
use strict;
use FindBin;
use File::Spec::Functions;
use Getopt::Long;
use warnings FATAL => 'all';

binmode(STDOUT, ":utf8");
binmode(STDERR, ":utf8");

unless (@ARGV) {
    help_message();
    exit 0;
}

# Global variables
my $ALL = 0;
my $EXPORT = 0;
my $COMPARE;
my $RECURSIVE = 0;
my $PORT = "all";
my $LEXBOR_DIR = $FindBin::RealBin;
my %OPTIONS;
my $OPT_INDEX = 1;

# Parse command line arguments
GetOptions(
    'help' => sub {
        help_message();
        exit 0;
    },
    'all' => \$ALL,
    'port=s' => \$PORT,
    'with-export-symbols' => \$EXPORT,
    'dependencies' => sub {$OPTIONS{"print_modules_dependencies"} = $OPT_INDEX++},
    'modules' => sub {$OPTIONS{"modules"} = $OPT_INDEX++},
    'graph' => sub {$OPTIONS{"print_dependency_graph"} = $OPT_INDEX++},
    'stats' => sub {$OPTIONS{"print_statistics"} = $OPT_INDEX++},
    'reverse-deps' => sub {$OPTIONS{"print_reverse_dependencies"} = $OPT_INDEX++},
    'check-cycles' => sub {$OPTIONS{"check_cyclic_dependencies"} = $OPT_INDEX++},
    'dot-graph' => sub {$OPTIONS{"print_dot_graph"} = $OPT_INDEX++},
    'validate' => sub {$OPTIONS{"validate_dependencies"} = $OPT_INDEX++},
    'compare=s' => sub {$OPTIONS{"compare"} = $OPT_INDEX++; $COMPARE = $_[1] },
    'minimal-deps' => sub {$OPTIONS{"print_minimal_dependencies"} = $OPT_INDEX++},
    'module-deps' => sub {$OPTIONS{"print_module_dependencies"} = $OPT_INDEX++},
    'recursive' => \$RECURSIVE,
    'size-info' => sub {$OPTIONS{"print_size_info"} = $OPT_INDEX++},
    'versions' => sub {$OPTIONS{"print_versions"} = $OPT_INDEX++},
    'version' => sub {$OPTIONS{"print_lexbor_version"} = $OPT_INDEX++},
    'export-json' => sub {$OPTIONS{"export_json"} = $OPT_INDEX++},
    'export-yaml' => sub {$OPTIONS{"export_yaml"} = $OPT_INDEX++},
) or die "Error parsing args";

# If certain options are specified without modules, act like --all
if (!@ARGV) {
    $ALL = 1;
}

# Parse port list (supports comma-separated values and "all")
my @PORTS;
if ($PORT eq "all") {
    my $ports_dir = catfile($LEXBOR_DIR, "source", "lexbor", "ports");
    opendir my $dh, $ports_dir or die "Cannot open ports directory: $ports_dir: $!";
    while (my $sub = readdir $dh) {
        next if $sub =~ /^\./;
        next unless -d catfile($ports_dir, $sub);
        push @PORTS, $sub;
    }
    closedir $dh;
    die "No ports found in $ports_dir" unless @PORTS;
}
else {
    @PORTS = split /,/, $PORT;
}

# Create Single instance
my $single = new Single($LEXBOR_DIR, \@ARGV, $ALL, \@PORTS, $EXPORT, $RECURSIVE);

foreach my $sub (sort { $OPTIONS{$a} <=> $OPTIONS{$b} } keys %OPTIONS) {
    if ($sub eq "compare") {
        my @compare_mods = split /,/, $COMPARE;

        die "Please specify exactly 2 modules to compare (--compare=module1,module2)"
            unless @compare_mods == 2;

        my $single_compare = new Single($LEXBOR_DIR, \@compare_mods, 0, \@PORTS, $EXPORT, $RECURSIVE);
        $single_compare->compare_modules($compare_mods[0], $compare_mods[1]);

        next;
    }

    if ($sub eq "print_reverse_dependencies") {
        my $single_all = new Single($LEXBOR_DIR, [], 1, \@PORTS, $EXPORT, $RECURSIVE);
        $single_all->print_reverse_dependencies(@ARGV ? \@ARGV : $single_all->original_modules());

        next;
    }

    if ($sub eq "modules") {
        my @modules;
        my $dirpath = catfile($LEXBOR_DIR, "source", "lexbor");
        die "I can't find the directory with the lexbor source code: $dirpath."
            unless -d $dirpath;

        opendir my $dh, $dirpath or die "Cannot open $dirpath: $!";

        while (my $sub = readdir $dh) {
            next if $sub =~ /^\./;
            next if $sub eq "ports";

            push @modules, $sub;
        }

        closedir $dh;

        print join(" ", sort { $a cmp $b } @modules), "\n";

        next;
    }

    $single->$sub(@ARGV ? \@ARGV : $single->modules());
}

exit 0 if scalar keys %OPTIONS;

# Fix order of some headers
$single->reorder_headers_by_template(['core/def.h', 'core/types.h',
                                      'core/lexbor.h', 'core/base.h']);
$single->remove_includes_by_template(['sanitizer/asan_interface.h',
                                      'sys/sysctl.h', 'sys/stat.h', 'sys/types.h']);

# Generate single files
my $license = $single->read_license_file();
my $defines = $single->defines();
my $i_generate = $single->i_generate();
my $h_generate = $single->h_generate();
my $r_generate = $single->r_generate();
my $c_generate = $single->c_generate();

my $current_year = (localtime)[5] + 1900;
my $localtime = scalar(localtime);
my $modules = join "\n *", @{$single->print_array_wrapped($single->original_modules(1), 70, 4)};
my $dependencies = join "\n *", @{$single->print_array_wrapped($single->modules(1, 1), 65, 4)};
my $lexbor_version = $single->lexbor_version();

# Print copyright, generated file header
print <<END;
/*
 * Copyright (C) 2018-$current_year Alexander Borisov
 *
 * Author: Alexander Borisov <borisov\@lexbor.com>
 *
 * This file is generated automatically. Do not edit!
 *
 * Generated by: $FindBin::Script
 * Project: Lexbor (https://lexbor.com/)
 * GitHub: https://github.com/lexbor/lexbor
 * Date: $localtime
 * Version: $lexbor_version
 * License: Apache License Version 2.0
 * Modules: $modules
 * Dependencies: $dependencies
 */

END

# Print license
print "/* LICENSE:\n";
print join("\n", @$license);
print "\n */\n\n";

# Print defines
print "/*\n";
print " * Defines:\n";
print " */\n";
print join("\n", @$defines);
print "\n\n";

# Print includes
print "/*\n";
print " * Includes:\n";
print " */\n";
print join("\n", @$i_generate);
print "\n\n";

# Print headers
print "/*\n";
print " * Headers:\n";
print " */\n";
print join("\n", @$h_generate);
print "\n\n";

# Print resources
print "/*\n";
print " * Resources:\n";
print " */\n";
print join("\n", @$r_generate);
print "\n\n";

# Print code
print "/*\n";
print " * Source Code:\n";
print " */\n";
print join("\n", @$c_generate);
print "\n";


sub help_message
{
    print "Usage: $FindBin::Script [options] <module1> <module2> ...\n\n";
    print "Options:\n";
    print "  --help                        Show this help message\n";
    print "  --all                         Include all modules if no modules specified\n";
    print "  --port=<port>                 Specify the port (in source/lexbor/ports) to use (default: posix)\n";
    print "  --with-export-symbols         Export symbols\n";
    print "\nInformation:\n";
    print "  --modules                     Print all available modules\n";
    print "  --dependencies                Print detailed dependencies of specified modules\n";
    print "  --graph                       Print dependency graph as a tree\n";
    print "  --stats                       Print statistics about module dependencies\n";
    print "  --reverse-deps                Print reverse dependencies (which modules depend on specified ones)\n";
    print "  --size-info                   Print size information (lines of code, file counts)\n";
    print "  --minimal-deps                Show minimal set of dependencies\n";
    print "  --module-deps                 Print dependencies for specified modules (space-separated, sorted)\n";
    print "  --recursive                   With --module-deps: include recursive dependencies\n";
    print "  --versions                    Print version information for modules\n";
    print "  --version                     Print Lexbor version\n";
    print "\nValidation:\n";
    print "  --check-cycles                Check for cyclic dependencies\n";
    print "  --validate                    Validate that all dependencies exist\n";
    print "  --compare=mod1,mod2           Compare dependencies between two modules\n";
    print "\nExport:\n";
    print "  --dot-graph                   Export dependency graph in DOT format (Graphviz)\n";
    print "  --export-json                 Export dependency structure to JSON\n";
    print "  --export-yaml                 Export dependency structure to YAML\n";
}

package Single;

use utf8;
use FindBin;
use File::Spec::Functions;
use warnings FATAL => 'all';

sub new
{
    my ($class, $lexbor_dir, $modules, $all, $ports, $export_symbols, $recursive) = @_;
    my ($self, $dirpath, $cc, $original_modules);

    $dirpath = catfile($lexbor_dir, "source", "lexbor");
    die "I can't find the directory with the lexbor source code: $dirpath."
        unless -d $dirpath;

    # If no modules specified, get all modules
    if ($all && !@$modules) {
        opendir my $dh, $dirpath or die "Cannot open $dirpath: $!";

        while (my $sub = readdir $dh) {
            next if $sub =~ /^\./;
            next if $sub eq "ports";

            push @$modules, $sub;
        }

        closedir $dh;
    }

    # Handle ports: accept arrayref or string (backward compat)
    if (ref $ports eq 'ARRAY') {
        $ports = [@$ports];
    }
    else {
        $ports = [$ports // "posix"];
    }

    $dirpath = catfile($lexbor_dir, "source", "lexbor");
    $original_modules = [@$modules];

    # Validate all ports exist and read their configs
    my %port_configs;
    foreach my $port (@$ports) {
        my $port_path = catfile($dirpath, "ports", $port);
        die "I can't find the directory with the port source code: $port"
            unless -d $port_path;

        $port_configs{$port} = read_port_config($port_path, $port);
    }

    my $multi_port = (scalar @$ports > 1);

    # Sort ports: ports with condition first, fallback last
    my @sorted_ports = sort {
        my $a_fb = $port_configs{$a}->{fallback} ? 1 : 0;
        my $b_fb = $port_configs{$b}->{fallback} ? 1 : 0;
        $a_fb <=> $b_fb || $a cmp $b;
    } @$ports;
    $ports = \@sorted_ports;

    # Remove duplicate modules
    my %seen;
    $modules = [grep { !$seen{$_}++ } @$modules];

    if ($dirpath =~ /\\$|\/$/) {
        chop $dirpath;
    }

    die "I can't find the directory with the lexbor source code."
        unless -d $dirpath;

    $self = {
        lexbor => $lexbor_dir,
        source => $dirpath,
        ports => $ports,
        port => $ports->[0],       # backward compat: first port
        port_configs => \%port_configs,
        multi_port => $multi_port,
        port_files => {},          # { port_name => { module => { headers => [], code => [] } } }
        modules => $modules,
        modules_h => {},
        modules_c => {},
        original_modules => $original_modules,
        dependencies => {},
        res => [],
        h_index => {},
        headers => {},
        sources => {},
        includes => {},
        resources => {},
        versions => {},
        export_symbols => $export_symbols,
        recursive => $recursive
    };

    bless $self, $class;

    $self->dependencies();
    $self->c_dependencies();
    $self->sort_dependencies();
    $self->resolve_dependencies();

    my ($headers, $sources, $includes, $resources) = $self->compile_sources();

    $self->{headers} = $headers;
    $self->{sources} = $sources;
    $self->{includes} = $includes;
    $self->{resources} = $resources;
    $self->{versions} = $self->parse_all_versions();

    return $self;
}

sub read_port_config
{
    my ($port_path, $port_name) = @_;
    my %config;

    # Keys that support multiple values
    my %multi_keys = (define => 1, ifndef => 1);

    my $conf_file = catfile($port_path, "port.conf");
    if (-f $conf_file) {
        open my $fh, "<:encoding(UTF-8)", $conf_file
            or die "Cannot open port config ($conf_file): $!";

        while (my $line = <$fh>) {
            $line =~ s/^\s+|\s+$//g;
            next if $line eq "" || $line =~ /^#/;

            if ($line =~ /^(\w+)\s*=\s*(.+)$/) {
                my ($key, $val) = ($1, $2);

                if ($multi_keys{$key}) {
                    $config{$key} //= [];
                    push @{$config{$key}}, $val;
                }
                else {
                    $config{$key} = $val;
                }
            }
        }

        close $fh;
    }
    else {
        # No config file — treat as fallback
        $config{fallback} = "true";
    }

    # Normalize fallback to boolean
    if (exists $config{fallback}) {
        $config{fallback} = ($config{fallback} =~ /^(true|1|yes)$/i) ? 1 : 0;
    }

    return \%config;
}

sub resolve_dependencies
{
    my $self = shift;
    my $dependencies = $self->{dependencies};

    foreach my $module (@{$self->{modules}}) {
        my ($internal, $other, $modules_all) = $self->resolve_recursive_dependencies($module);
        $dependencies->{$module}->{internal_all} = $internal;
        $dependencies->{$module}->{other_all} = $other;
        $dependencies->{$module}->{modules_all} = $modules_all;
    }
}

sub resolve_recursive_dependencies
{
    my ($self, $module, $res_internal, $res_other, $res_modules) = @_;

    my $dependencies = $self->{dependencies};
    my $modules = $dependencies->{$module}->{modules};

    $res_internal //= [];
    $res_other //= [];
    $res_modules //= [];

    foreach my $h (@{$dependencies->{$module}->{internal}}) {
        unless (grep { $_ eq $h } @$res_internal) {
            push @$res_internal, $h;
        }
    }

    foreach my $h (@{$dependencies->{$module}->{other}}) {
        unless (grep { $_ eq $h } @$res_other) {
            push @$res_other, $h;
        }
    }

    foreach my $dep_module (@$modules) {
        unless (grep { $_ eq $dep_module } @$res_modules) {
            push @$res_modules, $dep_module;

            $self->resolve_recursive_dependencies($dep_module, $res_internal, $res_other, $res_modules);
        }
    }

    return ($res_internal, $res_other, $res_modules);
}

sub c_dependencies
{
    my $self = shift;

    foreach my $module (@{$self->{modules}}) {
        my $deps = $self->{dependencies}->{$module};
        $deps->{source_all} = $self->resolve_c_recursive_dependencies($module);
    }
}

sub resolve_c_recursive_dependencies
{
    my ($self, $module, $res, $index, $index_c) = @_;

    my $dependencies = $self->{dependencies};
    my $modules = $dependencies->{$module}->{modules};

    $res //= [];
    $index //= {};
    $index_c //= {};
    $index->{$module} = 1;

    foreach my $c (@{$self->{modules_c}->{$module}}) {
        # Port files are already stored as paths relative to $self->{source}
        my $sub_path = ($c =~ /^ports\//) ? $c : catfile($module, $c);

        next if exists $index_c->{$sub_path};
        $index_c->{$sub_path} = 1;

        push @$res, $sub_path;
    }

    foreach my $dep_module (@$modules) {
        next if exists $index->{$dep_module};

        $self->resolve_c_recursive_dependencies($dep_module, $res, $index, $index_c);
    }

    return $res;
}

sub dependencies
{
    my $self = shift;

    my $need_deps = 0;
    my $dependencies = $self->{dependencies};
    my $modules_c = $self->{modules_c};

    foreach my $module (@{$self->{modules}}) {
        next if exists $dependencies->{$module};

        $self->index_source($module);

        my ($internal, $other, $dep_modules) = $self->module_dependencies($module);

        $dependencies->{$module} = {
            # Internal include (.h) dependencies, only start with "lexbor/"
            internal => $internal,
            # Other include (.h) dependencies
            other => $other,
            headers => $self->subpath_headers($module),
            # Source files (.c) dependencies
            source => $self->subpath_sources($module),
            # Module dependencies
            modules => $dep_modules,
            # Recursive internal include (.h) dependencies, only start with "lexbor/"
            internal_all => [],
            # Recursive other include (.h) dependencies
            other_all => [],
            # Recursive source files (.c) dependencies
            source_all => [],
            # Recursive module dependencies
            modules_all => []
        };

        foreach my $dep_module (@$dep_modules) {
            unless (grep { $_ eq $dep_module } @{$self->{modules}}) {
                push @{$self->{modules}}, $dep_module;
                $need_deps = 1;
            }
        }
    }

    if ($need_deps) {
        $self->dependencies();
    }

    return $dependencies;
}

sub subpath_sources
{
    my ($self, $module) = @_;

    my $sources = [];
    my $modules_c = $self->{modules_c};

    foreach my $c (@{$modules_c->{$module}}) {
        # Port files are already stored as paths relative to $self->{source}
        push @$sources, ($c =~ /^ports\//) ? $c : catfile($module, $c);
    }

    return $sources;
}

sub subpath_headers
{
    my ($self, $module) = @_;

    my $headers = [];
    my $modules_h = $self->{modules_h};

    foreach my $c (@{$modules_h->{$module}}) {
        # Port files are already stored as paths relative to $self->{source}
        push @$headers, ($c =~ /^ports\//) ? $c : catfile($module, $c);
    }

    return $headers;
}

sub module_dependencies
{
    my ($self, $module) = @_;

    my $internal = {};
    my $other = {};
    my $dep_modules = {};
    my $index = $self->{h_index};
    my $modules_h = $self->{modules_h};
    my $modules_c = $self->{modules_c};

    # Scan main (non-port) files
    foreach my $h (@{$modules_h->{$module}}, @{$modules_c->{$module}}) {
        # Port files are stored with paths relative to $self->{source}
        my $full_path = ($h =~ /^ports\//)
            ? catfile($self->{source}, $h)
            : catfile($self->{source}, $module, $h);
        my $deps = $self->source_dependencies($full_path);

        foreach my $dep (@$deps) {
            if ($dep =~ /^lexbor\//) {
                $dep =~ s/^lexbor\///;

                $internal->{$dep} = 1;

                my $h_full = ($h =~ /^ports\//)
                    ? $h
                    : catfile($module, $h);
                $index->{$h_full}->{$dep} = 1 if $h =~ /\.h$/;

                my @entries = split /\//, $dep;
                my $dep_module = defined $entries[0] ? $entries[0] : "";
                $dep_modules->{$dep_module} = 1
                    if $dep_module ne $module;
            }
            else {
                $other->{$dep} = 1;
            }
        }
    }

    # In multi-port mode, also scan port-specific files for dependencies (union)
    # but do NOT add their external includes to $other — those stay inside
    # the port .c files which are wrapped in #if/#else guards.
    if ($self->{multi_port}) {
        foreach my $port (@{$self->{ports}}) {
            my $pf = $self->{port_files}->{$port};
            next unless $pf && $pf->{$module};

            my @port_all = (@{$pf->{$module}->{headers}}, @{$pf->{$module}->{code}});

            foreach my $h (@port_all) {
                my $full_path = catfile($self->{source}, $h);
                my $deps = $self->source_dependencies($full_path);

                foreach my $dep (@$deps) {
                    if ($dep =~ /^lexbor\//) {
                        $dep =~ s/^lexbor\///;

                        $internal->{$dep} = 1;

                        $index->{$h}->{$dep} = 1 if $h =~ /\.h$/;

                        my @entries = split /\//, $dep;
                        my $dep_module = defined $entries[0] ? $entries[0] : "";
                        $dep_modules->{$dep_module} = 1
                            if $dep_module ne $module;
                    }
                    # Skip external includes from port files — they remain
                    # in the port source code under #if/#else guards.
                }
            }
        }
    }

    return ([sort { $a cmp $b } keys %$internal],
            [sort { $a cmp $b } keys %$other],
            [sort { $a cmp $b } keys %$dep_modules]);
}

sub source_dependencies
{
    my ($self, $path) = @_;
    my ($depends);

    $depends = [];

    open my $fh, "<:encoding(UTF-8)", $path
        or die "Cannot open the file ($path): $!";

    while (my $line = <$fh>) {
        $line =~ s/^\s+|\s+$//g;

        if ($line =~ /\#include\s*<([^>]+)>/) {
           push @$depends, $1 unless grep { $_ eq $1 } @$depends;
        }
        elsif ($line =~ /\#include\s*"([^"]+)"/) {
            push @$depends, $1 unless grep { $_ eq $1 } @$depends;
        }
        elsif ($line =~ /\#include/) {
            die "Cannot parse include line: $line in file $path";
        }
    }

    return $depends;
}

sub sort_dependencies
{
    my $self = shift;
    my $dependencies = $self->{dependencies};

    foreach my $module (keys %$dependencies) {
        my $dep = $dependencies->{$module};

        @{$dep->{internal}} = sort { $a cmp $b } @{$dep->{internal}};
        @{$dep->{other}} = sort { $a cmp $b } @{$dep->{other}};
        @{$dep->{source}} = sort { $a cmp $b } @{$dep->{source}};
        @{$dep->{modules}} = sort { $a cmp $b } @{$dep->{modules}};
        @{$dep->{internal_all}} = sort { $a cmp $b } @{$dep->{internal_all}};
        @{$dep->{other_all}} = sort { $a cmp $b } @{$dep->{other_all}};
        @{$dep->{source_all}} = sort { $a cmp $b } @{$dep->{source_all}};
        @{$dep->{modules_all}} = sort { $a cmp $b } @{$dep->{modules_all}};
    }
}

sub index_source_files
{
    my $self = shift;
    my $modules = $self->{modules};

    foreach my $module (@$modules) {
        $self->index_source($module);
    }
}

sub index_source
{
    my ($self, $module) = @_;

    my $modules_h = $self->{modules_h};
    my $modules_c = $self->{modules_c};

    # Find module path
    my $path = catfile($self->{source}, $module);
    if (!-d $path) {
        die "Module '$module' not found.";
    }

    my ($headers, $code) = $self->walk($path);

    # Add port-specific files for this module.
    # Port files live in: ports/<port>/lexbor/<module>/
    foreach my $port (@{$self->{ports}}) {
        my $port_module_path = catfile($self->{source}, "ports", $port,
                                       "lexbor", $module);
        next unless -d $port_module_path;

        my ($port_headers, $port_code) = $self->walk($port_module_path);
        my $port_prefix = catfile("ports", $port, "lexbor", $module);

        if ($self->{multi_port}) {
            # Multi-port: store port files separately
            $self->{port_files}->{$port} //= {};
            $self->{port_files}->{$port}->{$module} //= { headers => [], code => [] };

            foreach my $h (@$port_headers) {
                push @{$self->{port_files}->{$port}->{$module}->{headers}},
                     catfile($port_prefix, $h);
            }

            foreach my $c (@$port_code) {
                push @{$self->{port_files}->{$port}->{$module}->{code}},
                     catfile($port_prefix, $c);
            }
        }
        else {
            # Single port: mix into main arrays (backward compat)
            foreach my $h (@$port_headers) {
                push @$headers, catfile($port_prefix, $h);
            }

            foreach my $c (@$port_code) {
                push @$code, catfile($port_prefix, $c);
            }
        }
    }

    # Sort found files
    @$headers = sort { $a cmp $b } @$headers;
    @$code = sort { $a cmp $b } @$code;

    $modules_h->{$module} //= [];
    $modules_c->{$module} //= [];

    # Store found files
    push @{$modules_h->{$module}}, @$headers;
    push @{$modules_c->{$module}}, @$code;
}

sub walk
{
    my ($self, $source, $sub_path, $headers, $code) = @_;
    my ($full_path, $sub_path_local);

    $headers //= [];
    $code //= [];
    $sub_path //= "";

    opendir my $dh, $source or die "Cannot open $source: $!";

    while (my $sub = readdir $dh) {
        next if $sub =~ /^\./;

        $full_path = catfile($source, $sub);
        $sub_path_local = catfile($sub_path, $sub);

        $sub_path_local =~ s/^\\+|^\/+//;
        next if $sub_path_local =~ /^ports/;

        if (!-d $full_path) {
            if ($sub =~ /\.h$/) {
                push @$headers, $sub_path_local;
            }
            elsif ($sub =~ /\.c$/) {
                push @$code, $sub_path_local;
            }
        }

        $self->walk($full_path, $sub_path_local, $headers, $code)
            if -d $full_path;
    }

    closedir $dh;

    return ($headers, $code);
}

sub compile_sources
{
    my $self = shift;
    my ($headers, $sources, $dependencies, $internal, $source, $sub_paths);
    my ($includes, @resources, $is_wout_deps);

    $is_wout_deps = $self->{without_dependencies};
    $internal = $is_wout_deps ? "internal" : "internal_all";
    $source = $is_wout_deps ? "source" : "source_all";
    $dependencies = $self->{dependencies};

    $headers = {};
    $sources = {};
    $includes = {};

    foreach my $module (@{$self->{modules}}) {
        $sub_paths = $dependencies->{$module}->{$internal};
        $headers->{$_} = 1 foreach (@$sub_paths);

        $sub_paths = $dependencies->{$module}->{$source};
        $sources->{$_} = 1 foreach (@$sub_paths);

        $includes->{$_} = 1 foreach (@{$dependencies->{$module}->{other_all}});
    }

    $headers = [keys %$headers];
    $sources = [sort { $a cmp $b } keys %$sources];
    $includes = [sort { $a cmp $b } keys %$includes];

    $headers = $self->sort_headers_by_dependencies($self->{h_index}, $headers);

    # Extract resources from headers
    foreach my $idx (0..$#$headers) {
        my $sub_path = $headers->[$idx];

        if ($sub_path =~ /res\.h$/) {
            push @resources, delete $headers->[$idx];
        }
    }

    $headers = [grep { defined $_ } @$headers];

    # Extract resources from sources
    foreach my $idx (0..$#$sources) {
        my $sub_path = $sources->[$idx];

        if ($sub_path =~ /res\.c$/) {
            push @resources, delete $sources->[$idx];
        }
    }

    $sources = [grep { defined $_ } @$sources];

    # In multi-port mode, compile port-specific sources separately
    if ($self->{multi_port}) {
        my %port_compiled;

        foreach my $port (@{$self->{ports}}) {
            my $pf = $self->{port_files}->{$port};
            next unless $pf;

            my (@port_headers, @port_sources);

            foreach my $module (@{$self->{modules}}) {
                next unless $pf->{$module};

                push @port_headers, @{$pf->{$module}->{headers}};
                push @port_sources, @{$pf->{$module}->{code}};
            }

            @port_sources = sort { $a cmp $b } @port_sources;
            @port_headers = sort { $a cmp $b } @port_headers;

            $port_compiled{$port} = {
                headers => \@port_headers,
                sources => \@port_sources
            };
        }

        $self->{port_compiled} = \%port_compiled;
    }

    return ($headers, $sources, $includes, \@resources);
}

sub sort_headers_by_dependencies {
    my ($self, $index, $headers) = @_;

    my %header_set = map { $_ => 1 } @$headers;

    my %visited;
    my %in_stack;
    my @result;

    my $visit;
    $visit = sub {
        my ($header) = @_;

        return if $visited{$header};
        return if $in_stack{$header};

        $in_stack{$header} = 1;

        if (exists $index->{$header}) {
            for my $dep (sort keys %{$index->{$header}}) {
                if ($header_set{$dep}) {
                    $visit->($dep);
                }
            }
        }

        delete $in_stack{$header};
        $visited{$header} = 1;

        push @result, $header;
    };

    for my $header (sort @$headers) {
        $visit->($header);
    }

    return \@result;
}

sub r_generate
{
    my $self = shift;
    my (@res, $resources);

    $resources = $self->{resources};

    foreach my $idx (0..$#$resources) {
        my $file_path = $resources->[$idx];

        push @res, "/* Resource: $file_path */\n";
        push @res, @{$self->process_h_file($file_path, 2)};

        if ($idx != $#$resources) {
            push @res, "";
        }
    }

    return \@res;
}

sub i_generate
{
    my $self = shift;
    my @res;
    my $includes = $self->{includes};

    foreach my $idx (0..$#$includes) {
        my $file_path = $includes->[$idx];

        push @res, "#include <$file_path>";
    }

    return \@res;
}

sub h_generate
{
    my $self = shift;
    my @res;

    my $headers_files = $self->{headers};

    foreach my $idx (0..$#$headers_files) {
        my $file_path = $headers_files->[$idx];
        my $path = catfile("source", "lexbor", $file_path);

        push @res, "/* Header: $path */\n";
        push @res, @{$self->process_h_file($file_path, 2)};

        if ($idx != $#$headers_files) {
            push @res, "";
        }
    }

    # In multi-port mode, append port-specific headers with #if/#else guards
    if ($self->{multi_port} && $self->{port_compiled}) {
        my @port_res = $self->generate_port_section("headers", "Header");
        push @res, "", @port_res if @port_res;
    }

    return \@res;
}

sub process_h_file
{
    my ($self, $file_path, $ncount) = @_;
    my ($full_path, $res);

    $full_path = catfile($self->{source}, $file_path);

    open my $fh, "<:encoding(UTF-8)", $full_path
        or die "Cannot open the file ($full_path): $!";

    my $include_guards = 0;
    my @cplusplus;
    my $have_extern_c = 0;
    $ncount //= 0;
    $res = [];

    while (my $line = <$fh>) {
        my $tmp = $line;

        $tmp =~ s/\s+$//g;

        # Remove copyright comments in beginning of file
        if (@$res == 0 &&
            ($tmp =~ /^\s*$/
            || $tmp =~ /^\s*\/\*/
            || $tmp =~ /^\s*\*/))
        {
            next;
        }

        # Remove include guards
        if ($include_guards == 0 && @$res == 0 && $tmp =~ /^\s*#ifndef\s+(LEXBOR[A-Z0-9_]+)$/) {
            my $guard_name = $1;
            $tmp = <$fh>;
            $tmp =~ /^\s*#define\s+$guard_name\s*$/
                or die "Cannot parse include guard in file $full_path";

            $include_guards = 1;
            next;
        }

        # Remove cplusplus guards
        if ($tmp =~ /^\s*#ifdef\s+__cplusplus/) {
            push @cplusplus, $tmp;
            next;
        }
        if (@cplusplus) {
            push @cplusplus, $tmp;

            if ($tmp =~ /^\s*extern\s*"C"/) {
                $have_extern_c = 1;
            }
            elsif ($tmp =~ /^\s*#endif/) {
                unless ($have_extern_c) {
                    push @$res, @cplusplus;
                }

                @cplusplus = ();
                $have_extern_c = 0;
            }

            next;
        }

        # Remove lexbor includes
        # Other includes are retained because we don't know what might be
        # there; they are external or systemic.
        if ($tmp =~ /\#include\s*<([^>]+)>/
            || $tmp =~ /\#include\s*"([^"]+)"/)
        {
            next if $1 =~ /^lexbor\//;
        }
        elsif ($tmp =~ /\#include/) {
            die "Cannot parse include line: $tmp in file $full_path";
        }

        # Remove multiple empty lines (only more than 2)
        if ($tmp eq "") {
            $ncount += 1;
        }
        else {
            $ncount = 0;
        }
        next if $ncount > 2;

        push @$res, $tmp;
    }

    close $fh;

    # Remove lexbor include guard from end of file
    for (my $i = $#$res; $i >= 0; $i--) {
        next if $res->[$i] =~ /^\s*$/;

        if ($res->[$i] =~ /^\s*#endif\s*\/\*\s*LEXBOR/) {
            splice(@$res, $i);
        }

        last;
    }

    # Remove cplusplus guards from end of file
    my $found_start = -1;
    my $stage = 0;

    for (my $i = $#$res; $i >= 0; $i--) {
        my $line = $res->[$i];

        next if $line =~ /^\s*$/;

        if ($stage == 0) {
            if ($line =~ /^\s*#endif\s*$/) {
                $stage = 1;
            } else {
                last;
            }
        }
        elsif ($stage == 1) {
            if ($line =~ /^\s*\}/) {
                $stage = 2;
            } else {
                last;
            }
        }
        elsif ($stage == 2) {
            if ($line =~ /^\s*#ifdef\s*__cplusplus\s*$/) {
                $found_start = $i;
                last;
            } else {
                last;
            }
        }
    }

    if ($found_start != -1) {
        splice(@$res, $found_start);
    }

    # Remove trailing empty lines
    pop @$res while (@$res && $res->[-1] =~ /^\s*$/);

    return $res;
}

sub c_generate
{
    my $self = shift;
    my @res;

    my $source_files = $self->{sources};

    foreach my $idx (0..$#$source_files) {
        my $file_path = $source_files->[$idx];
        my $path = catfile("source", "lexbor", $file_path);

        push @res, "/* Source: $path */\n";
        push @res, @{$self->process_c_file($file_path, 2)};

        if ($idx != $#$source_files) {
            push @res, "";
        }
    }

    # In multi-port mode, append port-specific sources with #if/#else guards
    if ($self->{multi_port} && $self->{port_compiled}) {
        my @port_res = $self->generate_port_section("sources", "Source");
        push @res, "", @port_res if @port_res;
    }

    return \@res;
}

sub generate_port_section
{
    my ($self, $file_type, $comment_prefix) = @_;
    my @res;

    # file_type: "headers" or "sources"
    # comment_prefix: "Header" or "Source"
    my $is_header = ($file_type eq "headers");
    my $ports = $self->{ports};
    my $port_configs = $self->{port_configs};
    my $port_compiled = $self->{port_compiled};

    # Check if there are any port files at all
    my $has_files = 0;
    foreach my $port (@$ports) {
        if ($port_compiled->{$port} && @{$port_compiled->{$port}->{$file_type}}) {
            $has_files = 1;
            last;
        }
    }
    return () unless $has_files;

    my $guard_idx = 0;

    foreach my $port (@$ports) {
        my $pc = $port_configs->{$port};
        my $files = $port_compiled->{$port} ? $port_compiled->{$port}->{$file_type} : [];
        next unless @$files;

        # Emit preprocessor guard
        if ($pc->{fallback}) {
            if ($guard_idx > 0) {
                push @res, "";
                push @res, "#else /* Port: $port (fallback) */";
            }
            # If fallback is the only port (shouldn't happen in multi_port), no guard
        }
        else {
            my $condition = $pc->{condition};
            if ($guard_idx == 0) {
                push @res, "#if $condition /* Port: $port */";
            }
            else {
                push @res, "";
                push @res, "#elif $condition /* Port: $port */";
            }
        }

        $guard_idx++;

        # Emit files for this port
        foreach my $idx (0..$#$files) {
            my $file_path = $files->[$idx];
            my $path = catfile("source", "lexbor", $file_path);

            push @res, "";
            push @res, "/* $comment_prefix: $path */\n";

            if ($is_header) {
                push @res, @{$self->process_h_file($file_path, 2)};
            }
            else {
                push @res, @{$self->process_c_file($file_path, 2)};
            }
        }
    }

    if ($guard_idx > 0) {
        push @res, "";
        push @res, "#endif";
    }

    return @res;
}

sub process_c_file
{
    my ($self, $file_path, $ncount) = @_;
    my ($full_path, $res);

    $full_path = catfile($self->{source}, $file_path);

    open my $fh, "<:encoding(UTF-8)", $full_path
        or die "Cannot open the file ($full_path): $!";

    $ncount //= 0;
    $res = [];

    while (my $line = <$fh>) {
        my $tmp = $line;

        $tmp =~ s/\s+$//g;

        # Remove copyright comments in beginning of file
        if (($tmp =~ /^\s*$/
            || $tmp =~ /^\s*\/\*/
            || $tmp =~ /^\s*\*/) && @$res == 0)
        {
            next;
        }

        # Remove lexbor includes
        # Other includes are retained because we don't know what might be
        # there; they are external or systemic.
        if ($tmp =~ /\#include\s*<([^>]+)>/
            || $tmp =~ /\#include\s*"([^"]+)"/)
        {
            next if $1 =~ /^lexbor\//;
        }
        elsif ($tmp =~ /\#include/) {
            die "Cannot parse include line: $tmp in file $full_path";
        }

        # Remove multiple empty lines (only more than 2)
        if ($tmp eq "") {
            $ncount += 1;
        }
        else {
            $ncount = 0;
        }
        next if $ncount > 2;

        push @$res, $tmp;
    }

    close $fh;

    # Remove trailing empty lines
    pop @$res while (@$res && $res->[-1] =~ /^\s*$/);

    return $res;
}

sub reorder_headers_by_template
{
    my ($self, $template) = @_;

    $self->{headers} = reorder_by_template($self->{headers}, $template);
}

sub reorder_by_template
{
    my ($source, $template) = @_;

    my %reserved;
    for my $val (@$template) {
        $reserved{$val}++ if defined $val;
    }

    my @free = grep {
        not (defined $_ && $reserved{$_} && $reserved{$_}--)
    } @$source;

    my @result;
    my $free_idx = 0;

    for my $i (0 .. $#$template) {
        if (defined $template->[$i]) {
            $result[$i] = $template->[$i];
        } else {
            $result[$i] = $free[$free_idx++] if $free_idx < @free;
        }
    }

    push @result, @free[$free_idx .. $#free] if $free_idx <= $#free;

    return \@result;
}

sub remove_includes_by_template
{
    my ($self, $template) = @_;

    my $includes = $self->{includes};
    my %exclude_map = map { $_ => 1 } @$template;

    $self->{includes} = [grep { !exists $exclude_map{$_} } @$includes];
}

sub read_license_file
{
    my $self = shift;
    my ($res, $full_path);

    $full_path = catfile($self->{lexbor}, "LICENSE");

    open my $fh, "<:encoding(UTF-8)", $full_path
        or die "Cannot open the file ($full_path): $!";

    $res = [];

    while (my $line = <$fh>) {
        $line =~ s/\s+$//g;
        push @$res, $line;
    }

    close $fh;

    return $res;
}

sub original_modules
{
    my ($self, $with_version) = @_;

    if ($with_version) {
        my $versions = $self->{versions};
        my @modules = @{$self->{original_modules}};

        for my $i (0 .. $#modules) {
            my $mod = $modules[$i];
            if (exists $versions->{$mod}) {
                my $ver = $versions->{$mod};
                $modules[$i] = sprintf("%s-%d.%d.%d", $mod, $ver->[0],
                                                      $ver->[1], $ver->[2]);
            }
        }

        return [sort {$a cmp $b} @modules];
    }

    return [sort {$a cmp $b} @{$_[0]->{original_modules}}];
}

sub modules
{
    my ($self, $without_originals, $with_version) = @_;

    my @modules = @{$self->{modules}};

    if ($without_originals) {
        my %originals = map { $_ => 1 } @{$self->{original_modules}};
        @modules = grep { not $originals{$_} } @modules;
    }

    if ($with_version) {
        my $versions = $self->{versions};

        for my $i (0 .. $#modules) {
            my $mod = $modules[$i];
            if (exists $versions->{$mod}) {
                my $ver = $versions->{$mod};
                $modules[$i] = sprintf("%s-%d.%d.%d", $mod, $ver->[0],
                                                      $ver->[1], $ver->[2]);
            }
        }
    }

    return [sort {$a cmp $b} @modules];
}

sub print_dependency_graph
{
    my ($self, $modules) = @_;

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];

        my $dependencies = $self->{dependencies};
        my $dep = $dependencies->{$module};
        my $modules_deps = $dep ? $dep->{modules} : [];

        print "\e[1;36m$module\e[0m";
        if (@$modules_deps) {
            print " \e[90m-> [" . join(", ", @$modules_deps) . "]\e[0m";
        }
        print "\n";

        for my $i (0..$#$modules_deps) {
            my $dep_module = $modules_deps->[$i];
            my $is_last_child = ($i == $#$modules_deps);

            # Each direct dependency gets a fresh visited set with only root module marked
            my $visited = {};
            $visited->{$module} = 1;

            $self->print_dependency_tree($dep_module, "", $visited, $is_last_child);
        }

        print "\n" if ($idx != $#$modules);
    }
}

sub print_dependency_tree
{
    my ($self, $module, $prefix, $visited, $is_last) = @_;

    $prefix //= "";
    $visited //= {};
    $is_last //= 1;

    my $dependencies = $self->{dependencies};
    my $dep = $dependencies->{$module};

    # Return early if module doesn't exist
    return unless $dep;

    my $modules_deps = $dep->{modules};

    # Check if already visited (for cyclic dependencies)
    my $already_visited = $visited->{$module};

    my $branch = $is_last ? "`-- " : "|-- ";
    my $extension = $is_last ? "    " : "|   ";

    # Always print the node
    print "$prefix$branch\e[1;33m$module\e[0m";

    if ($already_visited) {
        print " \e[90m(already shown)\e[0m\n";
        return;
    }

    if (@$modules_deps) {
        print " \e[90m-> [" . join(", ", @$modules_deps) . "]\e[0m";
    }
    print "\n";

    # Mark as visited
    $visited->{$module} = 1;

    my $new_prefix = $prefix . $extension;

    for my $i (0..$#$modules_deps) {
        my $dep_module = $modules_deps->[$i];
        my $is_last_child = ($i == $#$modules_deps);

        $self->print_dependency_tree($dep_module, $new_prefix, $visited, $is_last_child);
    }
}

sub print_statistics
{
    my ($self, $modules) = @_;
    my $dependencies = $self->{dependencies};
    my %usage_count;
    my $total_direct = 0;
    my $total_recursive = 0;

    # Count how many times each module is used
    foreach my $module (@{$self->{modules}}) {
        my $dep = $dependencies->{$module};
        next unless $dep;

        foreach my $dep_mod (@{$dep->{modules}}) {
            $usage_count{$dep_mod}++;
        }
    }

    # Calculate statistics for requested modules
    foreach my $module (@$modules) {
        my $dep = $dependencies->{$module};
        next unless $dep;

        my $direct_count = scalar @{$dep->{modules}};
        my $recursive_count = scalar @{$dep->{modules_all}};

        $total_direct += $direct_count;
        $total_recursive += $recursive_count;

        print "\e[1;36m$module\e[0m:\n";
        print "  Direct dependencies:    $direct_count\n";
        print "  Recursive dependencies: $recursive_count\n";
        print "  Used by modules:        " . ($usage_count{$module} // 0) . "\n";
        my $h_count = scalar(@{$dep->{headers}});
        my $s_count = scalar(@{$dep->{source}});

        # Add port file counts in multi-port mode
        if ($self->{multi_port} && $self->{port_files}) {
            foreach my $port (@{$self->{ports}}) {
                my $pf = $self->{port_files}->{$port};
                next unless $pf && $pf->{$module};
                $h_count += scalar(@{$pf->{$module}->{headers}});
                $s_count += scalar(@{$pf->{$module}->{code}});
            }
        }

        print "  Header files:           $h_count\n";
        print "  Source files:           $s_count\n";
        print "\n";
    }

    # Most used modules
    print "\e[1;32mMost Used Modules:\e[0m\n";
    my @sorted_usage = sort { $usage_count{$b} <=> $usage_count{$a} } keys %usage_count;
    foreach my $i (0..4) {
        last if $i > $#sorted_usage;
        my $mod = $sorted_usage[$i];
        print "  $i. \e[1;33m$mod\e[0m - used by $usage_count{$mod} module(s)\n";
    }
    print "\n";

    # Modules without dependencies
    my @no_deps;
    foreach my $module (@{$self->{modules}}) {
        my $dep = $dependencies->{$module};
        push @no_deps, $module if $dep && scalar(@{$dep->{modules}}) == 0;
    }

    print "\e[1;32mModules without dependencies:\e[0m " . scalar(@no_deps) . "\n";
    print "  " . join(", ", @no_deps) . "\n" if @no_deps;
}

sub print_reverse_dependencies
{
    my ($self, $modules) = @_;
    my $dependencies = $self->{dependencies};
    my %reverse_deps;

    # Build reverse dependency map
    foreach my $module (@{$self->{modules}}) {
        my $dep = $dependencies->{$module};
        next unless $dep;

        foreach my $dep_mod (@{$dep->{modules}}) {
            $reverse_deps{$dep_mod} //= [];
            push @{$reverse_deps{$dep_mod}}, $module;
        }
    }

    # Print reverse dependencies for requested modules
    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];

        print "\e[1;36m$module\e[0m is used by:\n";

        if ($reverse_deps{$module}) {
            my @users = sort @{$reverse_deps{$module}};
            foreach my $i (0..$#users) {
                my $user = $users[$i];
                my $branch = ($i == $#users) ? "`-- " : "|-- ";
                print "$branch\e[1;33m$user\e[0m\n";
            }
        } else {
            print "  \e[90m(no modules depend on this)\e[0m\n";
        }

        print "\n" if ($idx != $#$modules);
    }
}

sub check_cyclic_dependencies
{
    my ($self, $modules) = @_;
    my $dependencies = $self->{dependencies};
    my $found_cycles = 0;
    my %reported_cycles;

    foreach my $module (@$modules) {
        my @path;
        my %visited;

        if ($self->find_cycle($module, \@path, \%visited)) {
            # Create a normalized cycle key to detect duplicates
            # Sort the cycle to create a canonical representation
            my @sorted_cycle = sort @path;
            my $cycle_key = join("|", @sorted_cycle);

            unless ($reported_cycles{$cycle_key}) {
                $reported_cycles{$cycle_key} = 1;
                $found_cycles = 1;
                print "\e[1;31mCycle found:\e[0m ";
                print join(" -> ", map { "\e[1;33m$_\e[0m" } @path);
                print " -> \e[1;33m$path[0]\e[0m\n";
            }
        }
    }

    if (!$found_cycles) {
        print "\e[1;32mNo cyclic dependencies found!\e[0m\n";
    }
}

sub find_cycle
{
    my ($self, $module, $path, $visited) = @_;

    # Check if we've seen this module in current path (cycle detected)
    foreach my $m (@$path) {
        if ($m eq $module) {
            # Found cycle - trim path to cycle only
            my $idx = 0;
            $idx++ while $idx <= $#$path && $path->[$idx] ne $module;
            @$path = @$path[$idx..$#$path];
            return 1;
        }
    }

    # Already fully explored this module
    return 0 if $visited->{$module};

    push @$path, $module;

    my $dependencies = $self->{dependencies};
    my $dep = $dependencies->{$module};

    if ($dep) {
        foreach my $dep_mod (@{$dep->{modules}}) {
            return 1 if $self->find_cycle($dep_mod, $path, $visited);
        }
    }

    pop @$path;
    $visited->{$module} = 1;
    return 0;
}

sub print_dot_graph
{
    my ($self, $modules) = @_;

    print "digraph dependencies {\n";
    print "  rankdir=LR;\n";
    print "  node [shape=box, style=rounded];\n\n";

    my $dependencies = $self->{dependencies};
    my %printed;

    foreach my $module (@$modules) {
        $self->print_dot_node($module, \%printed);
    }

    print "}\n";
}

sub print_dot_node
{
    my ($self, $module, $printed) = @_;

    return if $printed->{$module};
    $printed->{$module} = 1;

    my $dependencies = $self->{dependencies};
    my $dep = $dependencies->{$module};
    return unless $dep;

    my $label = $module;
    $label =~ s/\//_/g;

    foreach my $dep_mod (@{$dep->{modules}}) {
        my $dep_label = $dep_mod;
        $dep_label =~ s/\//_/g;

        print "  $label -> $dep_label;\n";

        $self->print_dot_node($dep_mod, $printed);
    }
}

sub validate_dependencies
{
    my ($self, $modules) = @_;
    my $dependencies = $self->{dependencies};
    my $errors = 0;

    foreach my $module (@$modules) {
        my $dep = $dependencies->{$module};

        unless ($dep) {
            print "\e[1;31mERROR:\e[0m Module \e[1;33m$module\e[0m not found\n";
            $errors++;
            next;
        }

        # Check if all dependency modules exist
        foreach my $dep_mod (@{$dep->{modules}}) {
            unless ($dependencies->{$dep_mod}) {
                print "\e[1;31mERROR:\e[0m Module \e[1;33m$module\e[0m depends on non-existent module \e[1;33m$dep_mod\e[0m\n";
                $errors++;
            }
        }

        # Check if header files exist
        foreach my $header (@{$dep->{internal}}) {
            my $path = catfile($self->{source}, $header);
            unless (-f $path) {
                print "\e[1;33mWARNING:\e[0m Header file not found: $path\n";
            }
        }
    }

    if ($errors == 0) {
        print "\e[1;32mAll dependencies are valid!\e[0m\n";
    } else {
        print "\n\e[1;31mFound $errors error(s)\e[0m\n";
    }
}

sub compare_modules
{
    my ($self, $module1, $module2) = @_;
    my $dependencies = $self->{dependencies};
    my $dep1 = $dependencies->{$module1};
    my $dep2 = $dependencies->{$module2};

    unless ($dep1 && $dep2) {
        print "\e[1;31mERROR:\e[0m One or both modules not found\n";
        return;
    }

    my %deps1 = map { $_ => 1 } @{$dep1->{modules}};
    my %deps2 = map { $_ => 1 } @{$dep2->{modules}};

    # Common dependencies
    my @common = grep { $deps2{$_} } keys %deps1;

    # Unique to module1
    my @unique1 = grep { !$deps2{$_} } keys %deps1;

    # Unique to module2
    my @unique2 = grep { !$deps1{$_} } keys %deps2;

    print "\e[1;32mCommon dependencies:\e[0m " . scalar(@common) . "\n";
    if (@common) {
        print "  " . join(", ", sort @common) . "\n";
    }
    print "\n";

    print "\e[1;36mUnique to $module1:\e[0m " . scalar(@unique1) . "\n";
    if (@unique1) {
        print "  " . join(", ", sort @unique1) . "\n";
    }
    print "\n";

    print "\e[1;36mUnique to $module2:\e[0m " . scalar(@unique2) . "\n";
    if (@unique2) {
        print "  " . join(", ", sort @unique2) . "\n";
    }
}

sub print_minimal_dependencies
{
    my ($self, $modules) = @_;
    my $dependencies = $self->{dependencies};

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];
        my $dep = $dependencies->{$module};
        next unless $dep;

        print "\e[1;36m$module\e[0m requires:\n";

        my @direct = @{$dep->{modules}};
        if (@direct) {
            foreach my $d (sort @direct) {
                print "  - \e[1;33m$d\e[0m\n";
            }
        } else {
            print "  \e[90m(no dependencies)\e[0m\n";
        }

        print "\n" if ($idx != $#$modules);
    }
}

sub print_size_info
{
    my ($self, $modules) = @_;

    my $total_headers = 0;
    my $total_sources = 0;
    my $total_headers_lines = 0;
    my $total_sources_lines = 0;
    my $dependencies = $self->{dependencies};

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];
        my $dep = $dependencies->{$module};
        next unless $dep;

        my $header_count = scalar(@{$dep->{headers}});
        my $source_count = scalar(@{$dep->{source}});

        # Count lines of code1
        my $header_lines = 0;
        my $source_lines = 0;

        foreach my $h (@{$dep->{headers}}) {
            my $path = catfile($self->{source}, $h);
            $header_lines += $self->count_lines($path) if -f $path;
        }

        foreach my $s (@{$dep->{source}}) {
            my $path = catfile($self->{source}, $s);
            $source_lines += $self->count_lines($path) if -f $path;
        }

        $total_headers += $header_count;
        $total_sources += $source_count;
        $total_headers_lines += $header_lines;
        $total_sources_lines += $source_lines;

        print "\e[1;36m$module\e[0m:\n";
        print "  Header files: $header_count (" . $self->format_number($header_lines) . " lines)\n";
        print "  Source files: $source_count (" . $self->format_number($source_lines) . " lines)\n";
        print "  Total lines:  " . $self->format_number($header_lines + $source_lines) . "\n";
        print "\n" if ($idx != $#$modules);
    }

    print "\n\e[1;32mTotal modules:\e[0m " . scalar(@$modules) . "\n";
    print "\e[1;32mTotal header files:\e[0m " . $self->format_number($total_headers) . "\n";
    print "\e[1;32mTotal source files:\e[0m " . $self->format_number($total_sources) . "\n";
    print "\e[1;32mTotal header lines:\e[0m " . $self->format_number($total_headers_lines) . "\n";
    print "\e[1;32mTotal source lines:\e[0m " . $self->format_number($total_sources_lines) . "\n";
    print "\e[1;32mTotal files:\e[0m " . $self->format_number($total_headers + $total_sources) . "\n";
    print "\e[1;32mTotal lines:\e[0m " . $self->format_number($total_headers_lines + $total_sources_lines) . "\n";

    # Show port-specific file info
    # Include port files from requested modules and all their recursive dependencies
    my %all_modules;
    foreach my $module (@$modules) {
        $all_modules{$module} = 1;
        my $dep = $dependencies->{$module};
        next unless $dep;
        $all_modules{$_} = 1 foreach @{$dep->{modules_all}};
    }

    if ($self->{multi_port} && $self->{port_files}) {
        # Multi-port: port files stored separately
        print "\n";
        foreach my $port (@{$self->{ports}}) {
            my $pf = $self->{port_files}->{$port};
            next unless $pf;

            my ($ph_count, $ps_count, $ph_lines, $ps_lines) = (0, 0, 0, 0);

            foreach my $module (keys %all_modules) {
                next unless $pf->{$module};

                foreach my $h (@{$pf->{$module}->{headers}}) {
                    $ph_count++;
                    my $path = catfile($self->{source}, $h);
                    $ph_lines += $self->count_lines($path) if -f $path;
                }

                foreach my $c (@{$pf->{$module}->{code}}) {
                    $ps_count++;
                    my $path = catfile($self->{source}, $c);
                    $ps_lines += $self->count_lines($path) if -f $path;
                }
            }

            my $pc = $self->{port_configs}->{$port};
            my $cond = $pc->{fallback} ? "fallback" : $pc->{condition};

            print "\e[1;32mPort $port ($cond):\e[0m\n";
            print "  Header files: " . $self->format_number($ph_count) . " (" . $self->format_number($ph_lines) . " lines)\n";
            print "  Source files: " . $self->format_number($ps_count) . " (" . $self->format_number($ps_lines) . " lines)\n";
        }
    }
    else {
        # Single port: port files are mixed in, count them from source_all
        my $port = $self->{ports}->[0];
        my ($ph_count, $ps_count, $ph_lines, $ps_lines) = (0, 0, 0, 0);

        foreach my $module (keys %all_modules) {
            my $dep = $dependencies->{$module};
            next unless $dep;

            foreach my $s (@{$dep->{source}}) {
                if ($s =~ /^ports\//) {
                    if ($s =~ /\.h$/) { $ph_count++; }
                    else { $ps_count++; }
                    my $path = catfile($self->{source}, $s);
                    if ($s =~ /\.h$/) { $ph_lines += $self->count_lines($path) if -f $path; }
                    else { $ps_lines += $self->count_lines($path) if -f $path; }
                }
            }

            foreach my $h (@{$dep->{headers}}) {
                if ($h =~ /^ports\//) {
                    $ph_count++;
                    my $path = catfile($self->{source}, $h);
                    $ph_lines += $self->count_lines($path) if -f $path;
                }
            }
        }

        if ($ph_count > 0 || $ps_count > 0) {
            my $pc = $self->{port_configs}->{$port};
            my $cond = $pc->{fallback} ? "fallback" : $pc->{condition};

            print "\n\e[1;32mPort $port ($cond):\e[0m\n";
            print "  Header files: " . $self->format_number($ph_count) . " (" . $self->format_number($ph_lines) . " lines)\n";
            print "  Source files: " . $self->format_number($ps_count) . " (" . $self->format_number($ps_lines) . " lines)\n";
        }
    }
}

sub count_lines
{
    my ($self, $path) = @_;

    return 0 unless -f $path;

    open my $fh, '<', $path or return 0;
    my $count = 0;
    $count++ while <$fh>;
    close $fh;

    return $count;
}

sub format_number
{
    my ($self, $num) = @_;

    $num = reverse $num;
    $num =~ s/(\d{3})(?=\d)/$1 /g;
    return scalar reverse $num;
}

sub print_versions
{
    my ($self, $modules) = @_;
    my $versions = $self->{versions};

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];

        if (exists $versions->{$module}) {
            my $ver = $versions->{$module};
            printf "\e[1;36m%-20s\e[0m %d.%d.%d\n", $module, $ver->[0], $ver->[1], $ver->[2];
        } else {
            printf "\e[1;36m%-20s\e[0m \e[90m(no version info)\e[0m\n", $module;
        }
    }
}

sub export_json
{
    my ($self, $modules) = @_;

    my $recursive = $self->{recursive};
    my $dependencies = $self->{dependencies};
    my $mod_attr = $recursive ? "modules_all" : "modules";
    my $src_attr = $recursive ? "source_all" : "source";

    print "{\n";
    print "  \"modules\": [\n";

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];
        my $dep = $dependencies->{$module};
        next unless $dep;

        my $headers = $recursive
            ? $self->collect_recursive_headers($module)
            : $dep->{headers};

        print "    {\n";
        print "      \"name\": \"$module\",\n";
        print "      \"dependencies\": [" . join(", ", map { '"'. $_ .'"' } @{$dep->{$mod_attr}}) . "],\n";
        print "      \"headers\": [" . join(", ", map { '"'. catfile("lexbor", $_) .'"' } @$headers) . "],\n";
        print "      \"sources\": [" . join(", ", map { '"'. catfile("lexbor", $_) .'"' } @{$dep->{$src_attr}}) . "]\n";
        print "    }";
        print "," if $idx != $#$modules;
        print "\n";
    }

    # Add port-specific sources in multi-port mode
    # Include port files from requested modules and all their recursive dependencies
    if ($self->{multi_port} && $self->{port_files}) {
        my %all_modules;
        foreach my $module (@$modules) {
            $all_modules{$module} = 1;
            my $dep = $dependencies->{$module};
            next unless $dep;
            $all_modules{$_} = 1 foreach @{$dep->{$mod_attr}};
        }

        print "  ],\n";
        print "  \"port_sources\": {\n";

        my $ports = $self->{ports};
        foreach my $pidx (0..$#$ports) {
            my $port = $ports->[$pidx];
            my $pf = $self->{port_files}->{$port};
            my $pc = $self->{port_configs}->{$port};
            my $cond = $pc->{fallback} ? "fallback" : $pc->{condition};

            print "    \"$port\": {\n";
            print "      \"condition\": \"$cond\",\n";
            print "      \"files\": [\n";

            my @all_port_files;
            if ($pf) {
                foreach my $module (sort keys %all_modules) {
                    next unless $pf->{$module};
                    push @all_port_files, @{$pf->{$module}->{headers}};
                    push @all_port_files, @{$pf->{$module}->{code}};
                }
            }

            foreach my $fidx (0..$#all_port_files) {
                print "        \"" . catfile("lexbor", $all_port_files[$fidx]) . "\"";
                print "," if $fidx != $#all_port_files;
                print "\n";
            }

            print "      ]\n";
            print "    }";
            print "," if $pidx != $#$ports;
            print "\n";
        }

        print "  }\n";
    }
    else {
        print "  ]\n";
    }

    print "}\n";
}

sub export_yaml
{
    my ($self, $modules) = @_;

    my $recursive = $self->{recursive};
    my $dependencies = $self->{dependencies};
    my $mod_attr = $recursive ? "modules_all" : "modules";
    my $src_attr = $recursive ? "source_all" : "source";

    print "modules:\n";

    foreach my $module (@$modules) {
        my $dep = $dependencies->{$module};
        next unless $dep;

        my $headers = $recursive
            ? $self->collect_recursive_headers($module)
            : $dep->{headers};

        print "  - name: $module\n";
        print "    dependencies:\n";
        foreach my $d (@{$dep->{$mod_attr}}) {
            print "      - $d\n";
        }
        print "    headers:\n";
        foreach my $h (@$headers) {
            print "      - ", catfile("lexbor", $h), "\n";
        }
        print "    sources:\n";
        foreach my $s (@{$dep->{$src_attr}}) {
            print "      - ", catfile("lexbor", $s), "\n";
        }
    }

    # Add port-specific sources in multi-port mode
    # Include port files from requested modules and all their recursive dependencies
    if ($self->{multi_port} && $self->{port_files}) {
        my %all_modules;
        foreach my $module (@$modules) {
            $all_modules{$module} = 1;
            my $dep = $dependencies->{$module};
            next unless $dep;
            $all_modules{$_} = 1 foreach @{$dep->{$mod_attr}};
        }

        print "port_sources:\n";

        foreach my $port (@{$self->{ports}}) {
            my $pf = $self->{port_files}->{$port};
            my $pc = $self->{port_configs}->{$port};
            my $cond = $pc->{fallback} ? "fallback" : $pc->{condition};

            print "  $port:\n";
            print "    condition: \"$cond\"\n";
            print "    files:\n";

            if ($pf) {
                foreach my $module (sort keys %all_modules) {
                    next unless $pf->{$module};
                    foreach my $f (@{$pf->{$module}->{headers}}, @{$pf->{$module}->{code}}) {
                        print "      - ", catfile("lexbor", $f), "\n";
                    }
                }
            }
        }
    }
}

sub collect_recursive_headers
{
    my ($self, $module) = @_;

    my $dependencies = $self->{dependencies};
    my $dep = $dependencies->{$module};
    my %seen;
    my @headers;

    foreach my $h (@{$dep->{headers}}) {
        unless ($seen{$h}++) {
            push @headers, $h;
        }
    }

    foreach my $dep_module (@{$dep->{modules_all}}) {
        my $dep_dep = $dependencies->{$dep_module};
        next unless $dep_dep;

        foreach my $h (@{$dep_dep->{headers}}) {
            unless ($seen{$h}++) {
                push @headers, $h;
            }
        }
    }

    return [sort @headers];
}

sub print_module_dependencies
{
    my ($self, $modules) = @_;
    my (%names);

    my $dependencies = $self->{dependencies};
    my $attr = ($self->{recursive}) ? "modules_all" : "modules";

    foreach my $module (@$modules) {
        my $dep = $dependencies->{$module};

        for my $m (@{$dep->{$attr}}) {
            $names{$m} = 1;
        }
    }

    print join(" ", sort keys %names) . "\n";
}

sub print_modules_dependencies
{
    my ($self, $modules) = @_;

    foreach my $idx (0..$#$modules) {
        my $module = $modules->[$idx];

        print "Module: $module\n";

        $self->print_dependencies($module);

        print "\n" if ($idx != $#$modules);
    }
}

sub print_dependencies
{
    my ($self, $module, $ident) = @_;

    $ident //= 4;

    my $dependencies = $self->{dependencies};
    my $dep = $dependencies->{$module};
    my $witspace = " " x $ident;

    print "$witspace"."Internal Headers (from #include <...>):\n";
    foreach my $h (@{$dep->{internal}}) {
        print "$witspace    $h\n";
    }

    print "$witspace"."Other Headers (from #include <...>):\n";
    foreach my $h (@{$dep->{other}}) {
        print "$witspace    $h\n";
    }

    print "$witspace"."Header files:\n";
    foreach my $h (@{$dep->{headers}}) {
        print "$witspace    $h\n";
    }

    print "$witspace"."Source files:\n";
    foreach my $h (@{$dep->{source}}) {
        print "$witspace    $h\n";
    }

    print "$witspace"."Modules:\n";
    foreach my $m (@{$dep->{modules}}) {
        print "$witspace    $m\n";
    }

    print "$witspace"."Internal Headers with recursive dependencies (from #include <...>):\n";
    foreach my $m (@{$dep->{internal_all}}) {
        print "$witspace    $m\n";
    }

    print "$witspace"."Other Headers with recursive dependencies (from #include <...>):\n";
    foreach my $m (@{$dep->{other_all}}) {
        print "$witspace    $m\n";
    }

    print "$witspace"."Source files with recursive dependencies:\n";
    foreach my $h (@{$dep->{source_all}}) {
        print "$witspace    $h\n";
    }

    print "$witspace"."Modules with recursive dependencies:\n";
    foreach my $m (@{$dep->{modules_all}}) {
        print "$witspace    $m\n";
    }

    # Show port-specific files in multi-port mode
    # Include port files from this module and all its recursive dependencies
    if ($self->{multi_port} && $self->{port_files}) {
        my @all_modules = ($module, @{$dep->{modules_all}});

        foreach my $port (@{$self->{ports}}) {
            my $pf = $self->{port_files}->{$port};
            next unless $pf;

            my @port_files;
            foreach my $m (@all_modules) {
                next unless $pf->{$m};
                push @port_files, @{$pf->{$m}->{headers}};
                push @port_files, @{$pf->{$m}->{code}};
            }

            next unless @port_files;

            my $pc = $self->{port_configs}->{$port};
            my $cond = $pc->{fallback} ? "fallback" : $pc->{condition};

            print "$witspace"."Port files ($port, $cond):\n";

            foreach my $f (@port_files) {
                print "$witspace    $f\n";
            }
        }
    }
}

sub defines
{
    my $self = shift;
    my @defines;

    push @defines, "#define LEXBOR_BUILDING_STATIC";
    push @defines, "#define LEXBOR_STATIC" unless $self->{export_symbols};

    if ($self->{multi_port}) {
        # Multi-port: wrap port-specific defines in #if/#else guards
        # First, check which ports actually have defines
        my @ports_with_defines;
        foreach my $port (@{$self->{ports}}) {
            my $pc = $self->{port_configs}->{$port};
            my @pd = $self->port_defines($pc);
            push @ports_with_defines, $port if @pd;
        }

        if (@ports_with_defines > 1) {
            # Multiple ports with defines — use #if/#else guards
            my $guard_idx = 0;

            foreach my $port (@{$self->{ports}}) {
                my $pc = $self->{port_configs}->{$port};
                my @port_defines = $self->port_defines($pc);
                next unless @port_defines;

                if ($pc->{fallback}) {
                    push @defines, "";
                    push @defines, "#else /* Port: $port */";
                }
                elsif ($guard_idx == 0) {
                    push @defines, "";
                    push @defines, "#if $pc->{condition} /* Port: $port */";
                }
                else {
                    push @defines, "";
                    push @defines, "#elif $pc->{condition} /* Port: $port */";
                }

                $guard_idx++;
                push @defines, @port_defines;
            }

            push @defines, "";
            push @defines, "#endif";
        }
        elsif (@ports_with_defines == 1) {
            # Only one port has defines — emit without guards
            my $port = $ports_with_defines[0];
            my $pc = $self->{port_configs}->{$port};
            push @defines, $self->port_defines($pc);
        }
    }
    else {
        # Single port
        my $pc = $self->{port_configs}->{$self->{port}};
        push @defines, $self->port_defines($pc);
    }

    return \@defines;
}

sub port_defines
{
    my ($self, $pc) = @_;
    my @res;

    # "define = NAME VALUE" -> #define NAME VALUE
    if ($pc->{define}) {
        foreach my $d (@{$pc->{define}}) {
            push @res, "#define $d";
        }
    }

    # "ifndef = NAME" -> #ifndef NAME / #define NAME / #endif
    if ($pc->{ifndef}) {
        foreach my $d (@{$pc->{ifndef}}) {
            push @res, "#ifndef $d";
            push @res, "#define $d";
            push @res, "#endif";
        }
    }

    return @res;
}

sub print_lexbor_version
{
    my $self = shift;
    my $version = $self->lexbor_version();

    print "Lexbor Version: $version\n";
}

sub lexbor_version
{
    my $self = shift;

    open my $fh, "<:encoding(UTF-8)", catfile($self->{lexbor}, "version")
        or die "Cannot open VERSION file: $!";
    my $version = <$fh>;
    close $fh;

    $version =~ s/^LEXBOR_VERSION=//;
    $version =~ s/^\s+|\s+$//g;

    return $version;
}

sub versions
{
    return $_[0]->{versions};
}

sub parse_all_versions
{
    my ($self) = @_;
    my %versions;

    foreach my $module (@{$self->{modules}}) {
        $versions{$module} = $self->parse_version($module);
    }

    return \%versions;
}

sub parse_version
{
    my ($self, $module) = @_;

    my $path = catfile($self->{source}, $module, "base.h");

    open my $fh, '<', $path or die "Can't open '$path': $!";

    my %ver;

    while (my $line = <$fh>) {
        # #define LXB_ENCODING_VERSION_MAJOR 2
        if ($line =~ /^\s*#define\s+LXB_\w+_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)\b/) {
            my ($part, $num) = (lc($1), 0 + $2);
            $ver{$part} = $num;
        }
        elsif ($line =~ /^\s*#define\s+LEXBOR_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)\b/) {
            my ($part, $num) = (lc($1), 0 + $2);
            $ver{$part} = $num;
        }
    }

    close $fh;

    die "Version information not found in '$path'" unless
        exists $ver{major} && exists $ver{minor} && exists $ver{patch};

    return [$ver{major}, $ver{minor}, $ver{patch}];
}

sub print_array_wrapped {
    my ($self, $vals, $max_width, $ident) = @_;
    my $line = '';
    my @res;

    $ident ||= 0;
    $max_width ||= 80;

    $ident = ' ' x $ident if defined $ident;

    $line = $vals->[0] if scalar @$vals;

    for (my $idx = 1; $idx <scalar @$vals; $idx++) {
        my $v = $vals->[$idx];
        my $chunk = length($line) ? ", $v" : "$ident$v";

        if (length($line) + length($chunk) > $max_width) {
            push @res, "$line" if length $line;
            $line = "";
            $idx -= 1; # Re-process this value
        }
        else {
            $line .= $chunk;
        }
    }

    push @res, "$line" if length $line;

    return \@res;
}
