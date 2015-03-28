package Millc::Name;
use Exporter 'import';
use Modern::Perl;

our @EXPORT_OK = 'resolve';

sub resolve {
    my ($ast, $symbols) = @_;
    $symbols //= {
        'String' => {
            type => 'module_member_symbol',
            module => ['std', 'always'],
            member => 'String',
        },
    };
    my %visitors = (
        module => sub {
            ({ %$ast, decls => [map { resolve($_, $symbols) } @{$ast->{decls}}] });
        },

        use_decl => sub {
            $symbols->{$ast->{module}->[-1]} = {
                type => 'module_symbol',
                module => $ast->{module},
            };
            $ast;
        },

        proc_decl => sub {
            $symbols->{$ast->{name}} = {
                type => 'module_member_symbol',
                module => ['main'],
                member => $ast->{name},
            };
            my %body_symbols = %$symbols;
            for (0..$#{$ast->{params}}) {
                $body_symbols{$ast->{params}->[$_]->{name}} = {
                    type => 'parameter_symbol',
                    index => $_,
                };
            }
            return {
                %$ast,
                params => [map {
                    {
                        name => $_->{name},
                        type => resolve($_->{type}, { %$symbols }),
                    };
                } @{$ast->{params}}],
                body => resolve($ast->{body}, { %body_symbols }),
            };
        },

        main_decl => sub {
            ({ %$ast, body => resolve($ast->{body}, { %$symbols }) });
        },

        call_expr => sub {
            ({
                %$ast,
                callee => resolve($ast->{callee}, { %$symbols }),
                arguments => [map { resolve($_, { %$symbols }) } @{$ast->{arguments}}],
            });
        },

        name_expr => sub {
            my @name = @{$ast->{name}};
            if (@name == 1) {
                my $symbol = $symbols->{$name[0]};
                die "name $name[0] not in scope" if !$symbol;
                if ($symbol->{type} eq 'module_member_symbol') {
                    return {
                        %$ast,
                        name => {
                            type => 'module_member',
                            module => $symbol->{module},
                            member => $symbol->{member},
                        },
                    };
                } elsif ($symbol->{type} eq 'parameter_symbol') {
                    return {
                        %$ast,
                        name => {
                            type => 'parameter',
                            index => $symbol->{index},
                        },
                    };
                } else {
                    ...
                }
            } elsif (@name == 2) {
                my ($base, $member) = @{$ast->{name}};
                my $symbol = $symbols->{$base};
                die "name $base not in scope" if !$symbol;
                ({
                    %$ast,
                    name => {
                        type => 'module_member',
                        module => $symbol->{module},
                        member => $member,
                    },
                });
            } else {
                ...
            }
        },

        string_expr => sub {
            $ast;
        },

        block_expr => sub {
            my $block_symbols = { %$symbols };
            ({ %$ast, stmts => [map { resolve($_, $block_symbols) } @{$ast->{stmts}}] });
        },

        expr_stmt => sub {
            ({ %$ast, expr => resolve($ast->{expr}, $symbols) });
        },
    );
    if ($visitors{$ast->{type}}) {
        $visitors{$ast->{type}}->();
    } else {
        die "invalid AST node type $ast->{type}";
    }
}

1;
