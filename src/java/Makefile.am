#AM_CFLAGS =
AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
JAVA_PKGDIR = org/tiian/flom
AM_LDFLAGS = -module -Wl,--no-as-needed -Wl,-lglib-2.0 -Wl,--as-needed
libflom_java_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

nodist_libflom_java_la_SOURCES = FlomHandle.c FlomHandle.h

$(JAVA_PKGDIR)/FlomHandle.class:	$(JAVA_PKGDIR)/FlomHandle.java
	javac $(JAVA_PKGDIR)/FlomHandle.java

FlomHandle.h:	$(JAVA_PKGDIR)/FlomHandle.class
	javah -o FlomHandle.h org.tiian.flom.FlomHandle


lib_LTLIBRARIES = libflom-java.la
#javadir = $(datadir)/$(PACKAGE)/java
#java_DATA = flom.java
#EXTRA_DIST = flom.java

#install-exec-hook:
#	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
#	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"

#uninstall-hook:
#	rm -f @PHP_INSTALL_PATH@/flom.so

