package Millc::Parse;
use Exporter 'import';
use List::Util 'reduce';
use Modern::Perl;

our @EXPORT_OK = qw(parse);

our $tokens;

sub with_tokens {
    local $tokens = shift;
    shift->();
}

sub expect {
    my $type = shift;
    my $token = shift @$tokens;
    if ($token->{type} eq $type) {
        $token;
    } else {
        die "expected $type but got $token->{type}";
    }
}

sub try {
    my $parser = shift;
    my $tokens_backup = [@$tokens];
    my $result = eval { $parser->() };
    if ($@) {
        $tokens = $tokens_backup;
        die $@;
    } else {
        $result;
    }
}

sub many {
    my $parser = shift;
    my @result;
    for (;;) {
        my $result = eval { try $parser };
        if ($@) {
            last;
        } else {
            push @result, $result;
        }
    }
    \@result;
}

sub one_of {
    for (@_) {
        my $result = eval { try $_ };
        if (!$@) {
            return $result;
        }
    }
    die "oops";
}

sub decl {
    one_of(\&use_decl, \&proc_decl, \&main_decl);
}

sub use_decl {
    expect('use');
    my $a = expect('identifier')->{value};
    expect('colon_colon');
    my $b = expect('identifier')->{value};
    expect('semicolon');
    { type => 'use_decl', module => [$a, $b] };
}

sub proc_decl {
    expect('proc');
    my $name = expect('identifier')->{value};
    expect('left_parenthesis');
    my @params = eval { try sub {
        my $name = expect('identifier')->{value};
        expect('colon');
        my $type = expr();
        { name => $name, type => $type };
    } };
    expect('right_parenthesis');
    my $body = block_expr();
    {
        type => 'proc_decl',
        name => $name,
        params => \@params,
        body => $body,
    };
}

sub main_decl {
    expect('main');
    my $body = block_expr();
    { type => 'main_decl', body => $body };
}

sub expr {
    add_expr();
}

sub add_expr {
    my %operators = (
        '~' => 'tilde',
        '+' => 'plus',
        '-' => 'minus',
    );
    my @operands = (call_like_expr(), @{many(sub {
        [
            one_of(map { my $x = $_; sub { expect($operators{$x}); $x } } keys %operators),
            call_like_expr(),
        ];
    })});
    reduce { ({
        type => 'infix_expr',
        callee => {
            type => 'name_expr',
            name => ['infix' . $b->[0]],
        },
        arguments => [$a, $b->[1]],
    }) } @operands;
}

sub call_like_expr {
    one_of(\&call_expr, \&primary_expr);
}

sub call_expr {
    my $callee = primary_expr();
    expect('left_parenthesis');
    my @arguments = eval { try \&expr };
    expect('right_parenthesis');
    {
        type => 'call_expr',
        callee => $callee,
        arguments => [@arguments],
    };
}

sub primary_expr {
    one_of(\&name_expr, \&string_expr, \&boolean_expr, \&if_expr, \&block_expr);
}

sub name_expr {
    my @xs = expect('identifier')->{value};
    push @xs, @{many(sub {
        expect('colon_colon');
        expect('identifier')->{value};
    })};
    { type => 'name_expr', name => [@xs] };
}

sub string_expr {
    my $value = expect('string')->{value};
    { type => 'string_expr', value => $value };
}

sub boolean_expr {
    my $value = expect('boolean')->{value};
    { type => 'boolean_expr', value => $value };
}

sub if_expr {
    expect('if');
    my $condition = expr();
    my $then = block_expr();
    my $else = eval { try sub {
        expect('else');
        my $else = block_expr();
    } };
    { type => 'if_expr', condition => $condition, then => $then, else => $else };
}

sub block_expr {
    expect('left_brace');
    my $stmts = many(\&stmt);
    expect('right_brace');
    { type => 'block_expr', stmts => $stmts };
}

sub stmt {
    one_of(\&expr_stmt);
}

sub expr_stmt {
    my $expr = expr();
    expect('semicolon');
    { type => 'expr_stmt', expr => $expr };
}

sub parse {
    my @tokens = @{shift()};
    my @decls = with_tokens \@tokens, sub {
        my $result = many(\&decl);
        expect('eof');
        @$result;
    };
    { type => 'module', decls => \@decls };
}

1;
