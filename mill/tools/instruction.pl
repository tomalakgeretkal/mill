#!/usr/bin/env perl
use strict;
use warnings;

open(my $in, '<', $ARGV[0]) or die $!;
open(my $out_types, '>', $ARGV[1]) or die $!;
open(my $out_decode, '>', $ARGV[2]) or die $!;

my @instructions;
while (<$in>) {
    next if /^\s*(#.*)?$/;
    my ($opcode, $mnemonic, @operands) = split;
    push(@instructions, {
        opcode => $opcode,
        mnemonic => lc($mnemonic =~ s/(?!^)([A-Z])/_$1/gr),
        operands => [@operands],
    });
}

for my $instruction (@instructions) {
    print $out_types "struct $instruction->{mnemonic}_instruction {\n";
    for my $index (0..$#{$instruction->{operands}}) {
        print $out_types "std::$instruction->{operands}[$index]_t op$index;\n";
    }
    print $out_types "};\n";
}
my $type_names = join(', ', map("$_->{mnemonic}_instruction", @instructions));
print $out_types "using instruction = boost::variant<$type_names>;\n";

for my $instruction (@instructions) {
    print $out_decode "case $instruction->{opcode}: {\n";
    print $out_decode "$instruction->{mnemonic}_instruction result;\n";
    for my $index (0..$#{$instruction->{operands}}) {
        print $out_decode "read_int(result.op$index);\n";
    }
    print $out_decode "return result;\n";
    print $out_decode "}\n";
}
