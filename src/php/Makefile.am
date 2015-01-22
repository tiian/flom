AM_CFLAGS =
AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
AM_LDFLAGS = -module
dist_noinst_DATA = flom.i
# Unsure if it's better to use flom_php_module_la_LIBADD or 
# flom_php_module_la_LDADD
flom_php_module_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

flom_wrap.c: flom.i
	$(SWIG) -php -I.. flom.i

flom_php_module_la_SOURCES = flom_wrap.c

lib_LTLIBRARIES = flom-php-module.la
phpdir = $(datadir)/$(PACKAGE)/php
php_DATA = flom.php
EXTRA_DIST = flom.php php_flom.h

install-exec-hook:
	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"

uninstall-hook:
	rm -f @PHP_INSTALL_PATH@/flom.so

