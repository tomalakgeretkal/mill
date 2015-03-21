#!perl -T
use Millc::ObjectBuilder;
use Modern::Perl;
use Test::More tests => 1;

my $data = '';
open my $fh, '>:raw', \$data;

my $builder = Millc::ObjectBuilder->new();
my $i = $builder->string('foo');
my $j = $builder->string('bar');
$builder->write($fh);
is(
    $data,

    "\xDE\xAD\xBE\xEF" . # magic
    "\x00\x00\x00\x00\x01\x00" . # version

    "\x02\x00\x00\x00" . # string count
    "\x03\x00\x00\x00foo" .
    "\x03\x00\x00\x00bar"
);
