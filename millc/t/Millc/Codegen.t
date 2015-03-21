#!perl -T
use Millc::BytecodeBuilder;
use Millc::Codegen 'codegen';
use Millc::Lex 'lex';
use Millc::Name 'resolve';
use Millc::ObjectBuilder;
use Millc::Parse 'parse';
use Modern::Perl;
use Test::Differences;
use Test::More tests => 1;

my $hello_world = <<EOC;
use std::io;

MAIN {
    io::writeln("Hello, world!");
}
EOC

my $ast = resolve(parse([lex($hello_world)]));

my $object_builder = Millc::ObjectBuilder->new();
codegen($object_builder, 'Millc::BytecodeBuilder', $ast);
my $data = '';
open my $fh, '>:raw', \$data;
$object_builder->write($fh);

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
    "\x11\x00\x00\x00" . # body length
    "\x01\x01\x00\x00\x00" . # push std::io::writeln
    "\x02\x02\x00\x00\x00" . # push "Hello, world!"
    "\x03\x01\x00\x00\x00" . # call
    "\x04" . # pop
    "\x05" # return
);
