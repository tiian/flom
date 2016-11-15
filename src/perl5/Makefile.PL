#
# File: Makefile.PL
# By  : Kevin L. Esteb (kevin@kesteb.us)
# Date: 11-Nov-2016
#
# Create a makefile for the Perl5 bindings.
#
# swig -perl5 -I.. flom.i
# perl Makefile.PL
# make
# make install
#
# On Debian make sure these packages are installed:
#
#    pkg-config
#    libgnome2-dev
#

use ExtUtils::MakeMaker;
WriteMakefile(
    'ABSTRACT' => 'Perl5 bindings for FLoM',
    'AUTHOR'   => 'Kevin L. Esteb (kevin@kesteb.us)',
    'VERSION'  => '1.3.4',
    'LICENSE'  => 'GPL_2',
    'NAME'     => 'Flom',
    'LIBS'     => '-lglib-2.0 -lflom',
    'OBJECT'   => 'flom_wrap.o'
);

