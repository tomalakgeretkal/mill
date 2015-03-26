#!perl -T
use Millc::Lex 'lex';
use Modern::Perl;
use Test::More tests => 1;

my $hello_world = <<EOC;
use std::io;

proc f() { }

MAIN {
    io::writeln("Hello, world!");
}
EOC

is_deeply([lex($hello_world)], [
    { type => 'use' },
    { type => 'identifier', value => 'std' },
    { type => 'colon_colon' },
    { type => 'identifier', value => 'io' },
    { type => 'semicolon' },

    { type => 'proc' },
    { type => 'identifier', value => 'f' },
    { type => 'left_parenthesis' },
    { type => 'right_parenthesis' },
    { type => 'left_brace' },
    { type => 'right_brace' },

    { type => 'main' },
    { type => 'left_brace' },
    { type => 'identifier', value => 'io' },
    { type => 'colon_colon' },
    { type => 'identifier', value => 'writeln' },
    { type => 'left_parenthesis' },
    { type => 'string', value => 'Hello, world!' },
    { type => 'right_parenthesis' },
    { type => 'semicolon' },
    { type => 'right_brace' },
]);
