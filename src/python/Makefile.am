AM_CFLAGS =
AM_CPPFLAGS = @PYTHON_INCLUDES@ -I ..
AM_LDFLAGS = -module -Wl,--no-as-needed -Wl,-lglib-2.0 -Wl,--as-needed
dist_noinst_DATA = flom.i

flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -python -I.. flom.i

#nodist__flom_so_SOURCES = flom_wrap.c

# FIX ME!!! @@@
_flom.so:	flom_wrap.c setup.py
	python setup.py build_ext --inplace
	
#
# Build command is:
# python setup.py build_ext --inplace
#

#phpdir = $(datadir)/$(PACKAGE)/php
#php_DATA = flom.php
#EXTRA_DIST = flom.php php_flom.h

#install-exec-hook:
#	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
#	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"

#uninstall-hook:
#	rm -f @PHP_INSTALL_PATH@/flom.so

