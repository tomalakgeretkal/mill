package Millc::Parse;
use Exporter 'import';
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
    one_of(\&use_decl, \&main_decl);
}

sub use_decl {
    expect('use');
    my $a = expect('identifier')->{value};
    expect('colon_colon');
    my $b = expect('identifier')->{value};
    expect('semicolon');
    { type => 'use_decl', module => [$a, $b] };
}

sub main_decl {
    expect('main');
    my $body = block_expr();
    { type => 'main_decl', body => $body };
}

sub expr {
    call_like_expr();
}

sub call_like_expr {
    one_of(\&call_expr, \&primary_expr);
}

sub call_expr {
    my $callee = primary_expr();
    expect('left_parenthesis');
    my $argument = expr();
    expect('right_parenthesis');
    { type => 'call_expr', callee => $callee, arguments => [$argument] };
}

sub primary_expr {
    one_of(\&name_expr, \&string_expr, \&block_expr);
}

sub name_expr {
    my $a = expect('identifier')->{value};
    expect('colon_colon');
    my $b = expect('identifier')->{value};
    { type => 'name_expr', name => [$a, $b] };
}

sub string_expr {
    my $value = expect('string')->{value};
    { type => 'string_expr', value => $value };
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
    push @tokens, { type => 'eof' };
    my @decls = with_tokens \@tokens, sub {
        my $result = many(\&decl);
        expect('eof');
        @$result;
    };
    { type => 'module', decls => \@decls },
}

1;
