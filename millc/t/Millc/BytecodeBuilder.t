#!perl -T
use Millc::BytecodeBuilder;
use Modern::Perl;
use Test::Differences;
use Test::More tests => 1;

my $data = '';
open my $fh, '>:raw', \$data;
my $builder = Millc::BytecodeBuilder->new($fh);

$builder->push_module_member(0);
$builder->push_string(1);
$builder->call(1);
$builder->pop();
$builder->return();

eq_or_diff(
    $data,
    "\x01\x00\x00\x00\x00" .
    "\x02\x01\x00\x00\x00" .
    "\x03\x01\x00\x00\x00" .
    "\x04" .
    "\x05"
);
