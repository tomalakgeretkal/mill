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
        cc_mnemonic => $mnemonic,
        mnemonic => lc($mnemonic =~ s/(?!^)([A-Z])/_$1/gr),
        operands => [@operands],
    });
}

for my $instruction (@instructions) {
    my $type = "$instruction->{mnemonic}_instruction";

    print $out_types "struct $type {\n";

    my $ops = join(', ', map("std::$instruction->{operands}[$_]_t op$_", 0..$#{$instruction->{operands}}));
    print $out_types "$type($ops) {\n";
    for my $index (0..$#{$instruction->{operands}}) {
        print $out_types "this->op$index = op$index;\n";
    }
    print $out_types "}\n";

    for my $index (0..$#{$instruction->{operands}}) {
        print $out_types "std::$instruction->{operands}[$index]_t op$index;\n";
    }

    print $out_types "};\n";

    print $out_types "inline\n";
    print $out_types "bool operator==($type const& a, $type const& b) {\n";
    print $out_types "(void)a; (void)b;\n";
    for my $index (0..$#{$instruction->{operands}}) {
        print $out_types "if (a.op$index != b.op$index) {\n";
        print $out_types "return false;\n";
        print $out_types "}\n";
    }
    print $out_types "return true;\n";
    print $out_types "}\n";

    print $out_types "inline\n";
    print $out_types "bool operator!=($type const& a, $type const& b) {\n";
    print $out_types "return !(a == b);";
    print $out_types "}\n";

    print $out_types "inline\n";
    print $out_types "std::ostream& operator<<(std::ostream& stream, $type const& instruction) {\n";
    print $out_types "(void)instruction;\n";
    print $out_types "stream << \"$instruction->{cc_mnemonic}\";\n";
    for my $index (0..$#{$instruction->{operands}}) {
        print $out_types "stream << ' ' << instruction.op$index;\n";
    }
    print $out_types "return stream;\n";
    print $out_types "}\n";
}
my $type_names = join(', ', map("$_->{mnemonic}_instruction", @instructions));
print $out_types "using instruction = boost::variant<$type_names>;\n";

for my $instruction (@instructions) {
    print $out_decode "case $instruction->{opcode}: {\n";

    for my $index (0..$#{$instruction->{operands}}) {
        print $out_decode "std::$instruction->{operands}[$index]_t op$index;\n";
        print $out_decode "read_int(op$index);\n";
    }

    my $ops = join(', ', map("op$_", 0..$#{$instruction->{operands}}));
    print $out_decode "return $instruction->{mnemonic}_instruction($ops);\n";

    print $out_decode "}\n";
}
