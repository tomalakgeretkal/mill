use 5.006;
use strict;
use warnings FATAL => 'all';
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME             => 'Millc',
    VERSION_FROM     => 'lib/Millc.pm',
    MIN_PERL_VERSION => 5.006,
    CONFIGURE_REQUIRES => {
        'ExtUtils::MakeMaker' => 0,
    },
    BUILD_REQUIRES => {
        'Test::More' => 0,
    },
    PREREQ_PM => {
        'Data::GUID' => 0.048,
        'Modern::Perl' => 1.20150127,
        'Set::Scalar' => 1.29,
    },
    test => { TESTS => `zsh -c 'echo t/**/*.t'` },
    dist => { COMPRESS => 'gzip -9f', SUFFIX => 'gz' },
    clean => { FILES => 'Millc-*' },
);
