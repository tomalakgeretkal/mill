package Millc::Lex;
use Exporter 'import';
use List::Util 'min';
use Modern::Perl;

our @EXPORT_OK = qw(lex);

my @patterns = (
    [ qr/::/, 'colon_colon' ],
    [ qr/:/, 'colon' ],
    [ qr/;/, 'semicolon' ],
    [ qr/\./, 'period' ],
    [ qr/\(/, 'left_parenthesis' ],
    [ qr/\)/, 'right_parenthesis' ],
    [ qr/{/, 'left_brace' ],
    [ qr/}/, 'right_brace' ],
    [ qr/~/, 'tilde' ],
    [ qr/\+/, 'plus' ],
    [ qr/-/, 'minus' ],

    [ qr/".*?"/, 'string', sub { substr($_[0], 1, length($_[0]) - 2) } ],

    [ qr/false/, 'boolean', sub { 0 } ],
    [ qr/else/, 'else' ],
    [ qr/MAIN/, 'main' ],
    [ qr/proc/, 'proc' ],
    [ qr/true/, 'boolean', sub { 1 } ],
    [ qr/if/, 'if' ],
    [ qr/use/, 'use' ],
    [ qr/_/, 'underscore' ],

    [ qr/[a-zA-Z_][a-zA-Z0-9_]*/, 'identifier', sub { shift } ],

    [ qr/\z/, 'eof' ],
);

sub lex {
    my $code = shift;
    my @tokens;
    token: while ($code ne '') {
        my $space = qr/[ \n]|#(\((?:(?-1)|.)*?\))|#.*?\n/s;
        $code =~ s/^$space+//s;
        for (@patterns) {
            my ($pattern, $type, $value) = @$_;
            if ($code =~ /^$pattern/) {
                push @tokens, {
                    type => $type,
                    $value ? (value => $value->($&)) : (),
                };
                $code = substr($code, length($&));
                next token;
            }
        }
        die 'invalid token: ' . substr($code, 0, min(15, index($code, "\n")));
    }
    @tokens;
}

1;
