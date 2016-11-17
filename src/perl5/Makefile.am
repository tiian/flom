
flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -perl5 -const -I.. flom.i
	sed -i 's/*flom_/*/g' Flom.pm
	sed -i 's/sub FLOM_/sub /g' Flom.pm    
	perl Makefile.PL
	make -f builder

all-local: flom_wrap.c

clean-local:
	make -f builder realclean
	rm -f Flom.pm
	rm -f flom_wrap.*

install-exec-hook:
	make -f builder install

uninstall-hook:

