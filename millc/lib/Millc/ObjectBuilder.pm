package Millc::ObjectBuilder;
use Modern::Perl;
use Set::Scalar;

sub new {
    bless {
        strings => [],
    };
}

sub string {
    push(shift->{strings}, shift) - 1;
}

sub write {
    my ($self, $fh) = @_;

    print $fh "\xDE\xAD\xBE\xEF"; # magic
    print $fh pack('S<S<S<', 0, 0, 1); # version

    print $fh pack('L<', scalar @{$self->{strings}});
    for (@{$self->{strings}}) {
        print $fh pack('L<', length $_);
        print $fh $_;
    }
}

1;
