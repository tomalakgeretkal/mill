#!perl -T
use Millc::Lex qw(lex);
use Millc::Parse qw(parse);
use Test::More tests => 1;

my $hello_world = <<EOC;
use std::io;

proc f(x: String) { }
proc g() { }

MAIN {
    io::writeln("Hello, world!");
}
EOC

is_deeply(parse([lex($hello_world)]), {
    type => 'module',
    decls => [
        { type => 'use_decl', module => ['std', 'io'] },
        {
            type => 'proc_decl',
            name => 'f',
            params => [
                {
                    name => 'x',
                    type => {
                        type => 'name_expr',
                        name => ['String'],
                    },
                },
            ],
            body => { type => 'block_expr', stmts => [] },
        },
        {
            type => 'proc_decl',
            name => 'g',
            params => [],
            body => { type => 'block_expr', stmts => [] },
        },
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
    ],
});
