#
# File: Makefile.am
# By  : Kevin L. Esteb
# Date: 14-Nov-2016
#
# a part of the configure system
#

flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -perl5 -const -I.. flom.i
	sed -i 's/*flom_/*/g' Flom.pm
	sed -i 's/sub FLOM_/sub /g' Flom.pm    
	perl Makefile.PL
	make

all-local: flom_wrap.c

clean-local:
	make realclean
	rm -f Flom.pm
	rm -f flom_wrap.*

install-exec-hook:
	make install

uninstall-hook:

