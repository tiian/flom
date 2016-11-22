
flom_wrap.o: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -perl5 -const -I.. flom.i
	sed -i 's/*flom_/*/g' Flom.pm
	sed -i 's/sub FLOM_/sub /g' Flom.pm    
	perl Makefile.PL
	make -f builder

Makefile.PL: Makefile.PL.in ../../config.status
	sed \
	-e 's|@GLIB2_PYTHON[@]|$(GLIB2_PYTHON)|g' \
	-e 's|@GTHREAD2_PYTHON[@]|$(GTHREAD2_PYTHON)|g' \
	-e 's|@DBUS1_PYTHON[@]|$(DBUS1_PYTHON)|g' \
	-e 's|@OPENSSL_PYTHON[@]|$(OPENSSL_PYTHON)|g' \
	-e 's|@CRYPTO_PYTHON[@]|$(CRYPTO_PYTHON)|g' \
	-e 's|@PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE_STRING[@]|$(PACKAGE_STRING)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	$< >$@

all-local: Makefile.PL flom_wrap.o

check-local:
	make -f builder test

clean-local:
	make -f builder realclean
	rm -f Makefile.PL
	rm -f Flom.pm
	rm -f flom_wrap.*

installcheck-local:
	make -f builder test

install-exec-hook:
	make -f builder install

uninstall-hook:

