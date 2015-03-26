package Millc::Name;
use Exporter 'import';
use Modern::Perl;

our @EXPORT_OK = 'resolve';

sub resolve {
    my ($ast, $symbols) = @_;
    $symbols //= {};
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
            $ast;
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
                ({
                    %$ast,
                    name => {
                        type => 'module_member',
                        module => ['main'],
                        member => $name[0],
                    },
                });
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
