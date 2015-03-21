#!perl -T
use Millc::Lex qw(lex);
use Millc::Parse qw(parse);
use Test::More tests => 1;

my $hello_world = <<EOC;
use std::io;

MAIN {
    io::writeln("Hello, world!");
}
EOC

is_deeply([parse([lex($hello_world)])], [
    { type => 'use_decl', module => ['std', 'io'] },
    {
        type => 'main_decl',
        body => {
            type => 'block_expr',
            stmts => [
                {
                    type => 'expr_stmt',
                    expr => {
                        type => 'call_expr',
                        callee => {
                            type => 'name_expr',
                            name => ['io', 'writeln'],
                        },
                        arguments => [
                            {
                                type => 'string_expr',
                                value => 'Hello, world!',
                            },
                        ],
                    },
                },
            ],
        },
    },
]);
