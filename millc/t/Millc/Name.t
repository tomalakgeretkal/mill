#!perl -T
use Millc::Lex 'lex';
use Millc::Name 'resolve';
use Millc::Parse 'parse';
use Modern::Perl;
use Test::Differences;
use Test::More tests => 1;

my $hello_world = <<EOC;
use std::io;

proc f(x: String) {
    io::writeln(x);
}

MAIN {
    f();
    io::writeln("Hello, world!");
}
EOC

eq_or_diff(resolve(parse([lex($hello_world)])), {
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
                        name => {
                            type => 'module_member',
                            module => ['std', 'always'],
                            member => 'String',
                        },
                    },
                },
            ],
            body => {
                type => 'block_expr',
                stmts => [
                    {
                        type => 'expr_stmt',
                        expr => {
                            type => 'call_expr',
                            callee => {
                                type => 'name_expr',
                                name => {
                                   type => 'module_member',
                                   module => ['std', 'io'],
                                   member => 'writeln',
                               },
                            },
                            arguments => [
                                {
                                    type => 'name_expr',
                                    name => {
                                        type => 'parameter',
                                        index => 0,
                                    },
                                },
                            ],
                        },
                    },
                ],
            },
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
                                name => {
                                    type => 'module_member',
                                    module => ['main'],
                                    member => 'f',
                                },
                            },
                            arguments => [],
                        },
                    },
                    {
                        type => 'expr_stmt',
                        expr => {
                            type => 'call_expr',
                            callee => {
                                type => 'name_expr',
                                name => {
                                   type => 'module_member',
                                   module => ['std', 'io'],
                                   member => 'writeln',
                               },
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
