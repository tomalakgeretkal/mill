package Millc::BytecodeBuilder;
use Modern::Perl;

sub new {
    bless {
        fh => $_[1],
    };
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

1;
