#!/bin/sh
#
# File: build.sh
# Date: 15-Nov-2016
# By  : Kevin L. Esteb
#
# run swig
#
swig -perl5 -const -I.. flom.i
#
# flatten the namespace - make it more perlish
#
sed -i 's/*flom_/*/g' Flom.pm
sed -i 's/sub FLOM_/sub /g' Flom.pm
#
# let perl create the makefile
#
perl Makefile.PL
make
make test
#
echo ""
echo "switch to root and run \"make install\""
echo ""
#
