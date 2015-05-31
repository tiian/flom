#AM_CFLAGS =
#AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
AM_LDFLAGS = -module -Wl,--no-as-needed -Wl,-lglib-2.0 -Wl,--as-needed
dist_noinst_DATA = flom.i
libflom_java_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

flom_wrap.c: flom.i ../flom_errors.h.in ../flom_types.h ../flom_handle.h
	$(SWIG) -java -I.. flom.i

nodist_libflom_java_la_SOURCES = flom_wrap.c

lib_LTLIBRARIES = libflom-java.la
javadir = $(datadir)/$(PACKAGE)/java
java_DATA = flom.java
EXTRA_DIST = flom.java

#install-exec-hook:
#	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
#	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"

#uninstall-hook:
#	rm -f @PHP_INSTALL_PATH@/flom.so

