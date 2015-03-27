package Millc::Lex;
use Exporter 'import';
use List::Util 'min';
use Modern::Perl;

our @EXPORT_OK = qw(lex);

my @patterns = (
    [ qr/;/, 'semicolon' ],
    [ qr/\./, 'period' ],
    [ qr/\(/, 'left_parenthesis' ],
    [ qr/\)/, 'right_parenthesis' ],
    [ qr/{/, 'left_brace' ],
    [ qr/}/, 'right_brace' ],
    [ qr/::/, 'colon_colon' ],

    [ qr/".*?"/, 'string', sub { substr($_[0], 1, length($_[0]) - 2) } ],

    [ qr/_/, 'underscore' ],
    [ qr/use/, 'use' ],
    [ qr/MAIN/, 'main' ],
    [ qr/proc/, 'proc' ],

    [ qr/[a-zA-Z_][a-zA-Z0-9_]*/, 'identifier', sub { shift } ],
);

sub lex {
    my $code = shift;
    my @tokens;
    token: while ($code ne '') {
        my $space = qr/[ \n]|#\(.*?\)|#.*?\n/s;
        $code =~ s/^$space+|$space+$//gs;
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
