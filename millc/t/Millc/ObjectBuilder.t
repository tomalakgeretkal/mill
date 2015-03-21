#!perl -T
use Millc::ObjectBuilder;
use Modern::Perl;
use Test::More tests => 2;

my $data = '';
open my $fh, '>:raw', \$data;

my $builder = Millc::ObjectBuilder->new();
$builder->dependency(['std', 'io']);
is($builder->string('Hello, world!'), 1);
$builder->write($fh);
is(
    $data,

    "\xDE\xAD\xBE\xEF" . # magic
    "\x00\x00\x00\x00\x01\x00" . # version

    "\x02\x00\x00\x00" . # string count
    "\x07\x00\x00\x00std::io" .
    "\x0D\x00\x00\x00Hello, world!" .

    "\x01\x00\x00\x00" . # dependency count
    "\x00\x00\x00\x00"
);
