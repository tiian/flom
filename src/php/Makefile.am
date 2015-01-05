AM_CFLAGS =
AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
AM_LDFLAGS = -module
# Unsure if it's better to use flom_php_module_la_LIBADD or 
# flom_php_module_la_LDADD
flom_php_module_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

flom_wrap.c: flom.i
	$(SWIG) -php -I.. flom.i

lib_LTLIBRARIES = flom-php-module.la
flom_php_module_la_SOURCES = flom_wrap.c
# @@@ php_DATA = flom.php

install-exec-hook:
	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"
	echo "datadir="$(datadir)

uninstall-hook:
	rm -f @PHP_INSTALL_PATH@/flom.so

