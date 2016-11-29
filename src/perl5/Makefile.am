dist_noinst_DATA = flom.i Makefile.PL.in README.in

# Produce FLoM wrapper using SWIG
flom_wrap.o: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -perl5 -const -I.. flom.i
	sed -i 's/*flom_/*/g' Flom.pm
	sed -i 's/sub FLOM_/sub /g' Flom.pm
	mkdir -p lib
	mv Flom.pm lib/
	perl Makefile.PL
	make -f builder

README: README.in ../../config.status
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	$< >$@

Makefile.PL: Makefile.PL.in ../../config.status
	sed \
        -e 's|@libdir[@]|$(libdir)|g' \
        -e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
        -e 's|@_FLOM_PREFIX[@]|$(_FLOM_PREFIX)|g' \
	-e 's|@GLIB2_PERL[@]|$(GLIB2_PERL)|g' \
	-e 's|@GTHREAD2_PERL[@]|$(GTHREAD2_PERL)|g' \
	-e 's|@DBUS1_PERL[@]|$(DBUS1_PERL)|g' \
	-e 's|@OPENSSL_PERL[@]|$(OPENSSL_PERL)|g' \
	-e 's|@CRYPTO_PERL[@]|$(CRYPTO_PERL)|g' \
	-e 's|@PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE_STRING[@]|$(PACKAGE_STRING)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	$< >$@

all-local: Makefile.PL README flom_wrap.o 

check-local:
	make -f builder test

clean-local:
	make -f builder realclean
	rm -f Makefile.PL
	rm -f README
	rm -f lib/Flom.pm
	rm -f flom_wrap.*

installcheck-local:
	make -f builder test

install-exec-hook:
	make -f builder install

dist:
	make -f builder dist

uninstall-hook:

