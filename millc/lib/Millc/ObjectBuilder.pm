package Millc::ObjectBuilder;
use Modern::Perl;
use Set::Scalar;

sub new {
    bless {
        strings => [],
        dependencies => [],
    };
}

sub string {
    push(shift->{strings}, shift) - 1;
}

sub dependency {
    my $self = shift;
    my $module = join('::', @{shift()});
    my $id = $self->string($module);
    push @{$self->{dependencies}}, $id;
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

    print $fh pack('L<', scalar @{$self->{dependencies}});
    for (@{$self->{dependencies}}) {
        printf $fh pack('L<', $_);
    }
}

1;
