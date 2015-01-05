# Transform this in a real Makefile.am with libtool thing!
lib_LTLIBRARIES = flom-php-module.la
flom_php_module_la_SOURCES = flom_wrap.c
AM_CPPFLAGS = @PHP_INCLUDES@
AM_LDFLAGS = -module
# Unsure if it's better to use flom_php_module_la_LIBADD or 
# flom_php_module_la_LDADD
flom_php_module_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

flom_wrap.c: flom.i
	$(SWIG) -php -I.. flom.i

install-exec-hook:
	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so @PHP_INSTALL_PATH@/flom.so

#uninstall-hook:
#	rm -f @PHP_INSTALL_PATH@/flom.so

