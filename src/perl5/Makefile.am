
flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -perl5 -I.. flom.i
	perl Makefile.PL
	make

install-exec-hook:
	make install

uninstall-hook:
