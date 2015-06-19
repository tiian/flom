#AM_CFLAGS =
AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
JAVA_PKGDIR = org/tiian/flom
AM_LDFLAGS = -module -Wl,--no-as-needed -Wl,-lglib-2.0 -Wl,--as-needed
libflom_java_la_LIBADD = @GLIB2_LIBS@ ../libflom.la

libflom_java_la_SOURCES = flom_java.h flom_java.c
dist_noinst_HEADERS = flom_java.h
dist_noinst_DATA = FlomErrorCodes.awk
javadir = $(datadir)/$(PACKAGE)/java
dist_java_DATA = flom.jar
lib_LTLIBRARIES = libflom_java.la

$(JAVA_PKGDIR)/FlomErrorCodes.java:	FlomErrorCodes.awk ../flom_errors.h
	awk -f $^ > $@

$(JAVA_PKGDIR)/FlomErrorCodes.class:	$(JAVA_PKGDIR)/FlomErrorCodes.java
	javac $<

$(JAVA_PKGDIR)/FlomHandle.class:	$(JAVA_PKGDIR)/FlomHandle.java
	javac $<

$(JAVA_PKGDIR)/FlomException.class:	$(JAVA_PKGDIR)/FlomException.java
	javac $<

flom_java.h:	flom.jar
	rm -f $@ ; javah -o $@ -classpath $< org.tiian.flom.FlomHandle org.tiian.flom.FlomException

flom.jar:	$(JAVA_PKGDIR)/FlomErrorCodes.class $(JAVA_PKGDIR)/FlomHandle.class $(JAVA_PKGDIR)/FlomException.class
	jar cf $@ $^

#javadir = $(datadir)/$(PACKAGE)/java
#java_DATA = flom.java
#EXTRA_DIST = flom.java

#install-exec-hook:
#	ln -sf $(DESTDIR)$(libdir)/flom-php-module.so \
#	@PHP_INSTALL_PATH@/flom.so || echo "WARNING: module flom.so not installed in PHP default path @PHP_INSTALL_PATH@"

#uninstall-hook:
#	rm -f @PHP_INSTALL_PATH@/flom.so

