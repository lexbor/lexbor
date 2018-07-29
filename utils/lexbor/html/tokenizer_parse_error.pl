#!/usr/bin/perl -w

use utf8;
use strict;
use Encode;

use HTML::MyHTML;

my $body = q~
~;

# init
my $myhtml = HTML::MyHTML->new(MyHTML_OPTIONS_DEFAULT, 1);
my $tree = $myhtml->new_tree();

# parse
$myhtml->parse($tree, 0, $body);

my @enum;
my $count = 0;

my $list = $tree->document->get_nodes_by_tag_id(MyHTML_TAG_DFN);

foreach my $node (@$list) {
	my $res = [];
	
	join_text($node, $res);
	
	my $text = join("\n", @$res);
	my $mne = join "", map {substr $_, 0, 2} split /-/, $text;
	
	push @enum, [undef, "/* $text */"];
	push @enum, ["LXB_HTML_TOKENIZER_ERROR_". uc($mne), sprintf("0x%04X", $count++) . ","];
}

push @enum, ["LXB_HTML_TOKENIZER_ERROR_LAST_ENTRY", sprintf("0x%04X", $count++) . ","];

my $enum = format_list_text(\@enum, "= ");

print join("\n\t", @$enum);

$tree->destroy();
$myhtml->destroy();

sub join_text {
	my ($node, $res) = @_;
	
	while ($node) {
		my $info = $node->info();
		
		if ($info->{tag_id} == MyHTML_TAG__COMMENT ||
			$info->{tag_id} == MyHTML_TAG_STYLE ||
			$info->{tag_id} == MyHTML_TAG_SCRIPT ||
			$info->{tag_id} == MyHTML_TAG_TEXTAREA)
		{
			$node = $node->next;
			next;
		}
		
		if($info->{tag_id} == MyHTML_TAG__TEXT) {
			my $text = $node->text();
			Encode::_utf8_on($text) unless utf8::is_utf8($text);
			
			push @$res, $text unless $text =~ /^\s+$/;
		}
		
		join_text($node->child, $res) if $node->child;
		
		$node = $node->next;
	}
}

sub format_list_text {
    my ($list, $join_val) = @_;

    my ($max, $len) = (0, 0);
    foreach my $struct (@$list) {
		next unless defined $struct->[0];

        $len = length($struct->[0]);
        $max = $len if $len > $max;
    }

    my @res;
    foreach my $struct (@$list) {
		unless (defined $struct->[0]) {
			push @res, $struct->[1];
			next;
		}

        $len = $max - length($struct->[0]);
        push @res, sprintf("%s%$len"."s %s%s", $struct->[0], ($len ? " " : ""), $join_val, $struct->[1]);
    }

    \@res;
}

