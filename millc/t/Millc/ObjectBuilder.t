#!perl -T
use Millc::ObjectBuilder;
use Modern::Perl;
use Test::Differences;
use Test::More tests => 1;

my $data = '';
open my $fh, '>:raw', \$data;

my $builder = Millc::ObjectBuilder->new();

$builder->dependency('std::io');

$builder->string('std::io::writeln');
$builder->string('Hello, world!');

my $main_body =
    "\x01\x01\x00\x00\x00" . # push std::io::writeln
    "\x02\x02\x00\x00\x00" . # push "Hello, world!"
    "\x03\x01\x00\x00\x00" . # call
    "\x04" . # pop
    "\x05"; # return

$builder->subroutine('MAIN', 0, $main_body);

$builder->write($fh);
eq_or_diff(
    $data,

    "\xDE\xAD\xBE\xEF" . # magic
    "\x00\x00\x00\x00\x01\x00" . # version

    "\x04\x00\x00\x00" . # string count
    "\x07\x00\x00\x00std::io" .
    "\x10\x00\x00\x00std::io::writeln" .
    "\x0D\x00\x00\x00Hello, world!" .
    "\x04\x00\x00\x00MAIN" .

    "\x01\x00\x00\x00" . # dependency count
    "\x00\x00\x00\x00" .

    "\x01\x00\x00\x00" . # subroutine count

    "\x03\x00\x00\x00" . # MAIN
    "\x00\x00\x00\x00" . # parameter count
    pack('L<', length $main_body) . # body length
    $main_body # body
);
