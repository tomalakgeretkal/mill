package Millc::ObjectBuilder;
use Modern::Perl;
use Set::Scalar;

sub new {
    bless {
        strings => [],
        name => undef,
        dependencies => [],
        subroutines => [],
    };
}

sub string {
    push(shift->{strings}, shift) - 1;
}

sub name {
    my ($self, $name) = @_;
    $self->{name} = $self->string($name);
}

sub dependency {
    my ($self, $module) = @_;
    push @{$self->{dependencies}}, $self->string($module);
}

sub subroutine {
    my ($self, $name, $parameter_count, $body) = @_;
    push @{$self->{subroutines}}, {
        name => $self->string($name),
        parameter_count => $parameter_count,
        body => $body,
    };
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

    print $fh pack('L<', $self->{name});

    print $fh pack('L<', scalar @{$self->{dependencies}});
    for (@{$self->{dependencies}}) {
        print $fh pack('L<', $_);
    }

    print $fh pack('L<', scalar @{$self->{subroutines}});
    for (@{$self->{subroutines}}) {
        print $fh pack('L<', $_->{name});
        print $fh pack('L<', $_->{parameter_count});
        print $fh pack('L<', length $_->{body});
        print $fh $_->{body};
    }
}

1;
