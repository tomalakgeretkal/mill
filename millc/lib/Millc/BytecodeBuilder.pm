package Millc::BytecodeBuilder;
use Data::GUID;
use Modern::Perl;

sub new {
    bless {
        fh => $_[1],
        label_uses => {},
        label_offsets => {},
    };
}

sub fresh_label {
    my $label = Data::GUID->new->as_string;
    shift->{label_uses}->{$label} = [];
    $label;
}

sub save_label {
    my ($self, $label) = @_;
    my $offset = tell $self->{fh};
    for (@{$self->{label_uses}->{$label}}) {
        seek $self->{fh}, $_, 0;
        print { $self->{fh} } pack('L<', $offset);
    }
    delete $self->{label_uses}->{$label};
    $self->{label_offsets}->{$label} = $offset;
    seek $self->{fh}, $offset, 0;
}

sub push_global {
    my $fh = shift->{fh};
    print $fh "\x01";
    print $fh pack('L<', shift);
}

sub push_string {
    my $fh = shift->{fh};
    print $fh "\x02";
    print $fh pack('L<', shift);
}

sub push_boolean {
    my $fh = shift->{fh};
    print $fh "\x07";
    print $fh pack('C', shift() ? 1 : 0);
}

sub call {
    my $fh = shift->{fh};
    print $fh "\x03";
    print $fh pack('L<', shift);
}

sub pop {
    my $fh = shift->{fh};
    print $fh "\x04";
}

sub return {
    my $fh = shift->{fh};
    print $fh "\x05";
}

sub push_unit {
    my $fh = shift->{fh};
    print $fh "\x06";
}

sub push_parameter {
    my $fh = shift->{fh};
    print $fh "\x08";
    print $fh pack('L<', shift);
}

sub swap {
    my $fh = shift->{fh};
    print $fh "\x09";
}

sub conditional_jump {
    print { $_[0]->{fh} } "\x0A";
    goto &_jump;
}

sub unconditional_jump {
    print { $_[0]->{fh} } "\x0B";
    goto &_jump;
}

sub _jump {
    my ($self, $label) = @_;
    if (exists $self->{label_uses}->{$label}) {
        push @{$self->{label_uses}->{$label}}, tell($self->{fh});
        print { $self->{fh} } pack('L<', 0xFFFFFFFF);
    } else {
        my $offset = $self->{label_offsets}->{$label};
        print { $self->{fh} } pack('L<', $offset);
    }
}

1;
