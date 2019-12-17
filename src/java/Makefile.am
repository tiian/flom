#AM_CFLAGS =
AM_CPPFLAGS = @PHP_INCLUDES@ -I ..
JAVA_PKGDIR = org/tiian/flom
AM_LDFLAGS = -module -Wl,--no-as-needed -Wl,-lglib-2.0 -Wl,--as-needed
libflom_java_la_LIBADD = @GLIB2_LIBS@ @DBUS1_LIBS@ @OPENSSL_LIBS@ ../libflom.la

#libflom_java_la_SOURCES = flom_java.c org_tiian_flom_FlomHandle.h org_tiian_flom_FlomErrorCodes.h
libflom_java_la_SOURCES = flom_java.c
dist_noinst_HEADERS = flom_java_FlomHandle.h flom_java_FlomErrorCodes.h
dist_noinst_DATA = FlomErrorCodes.awk FlomLockModes.awk \
	$(JAVA_PKGDIR)/FlomException.java $(JAVA_PKGDIR)/FlomHandle.java
javadir = $(datadir)/$(PACKAGE)/java
dist_java_DATA = flom.jar
lib_LTLIBRARIES = libflom_java.la

$(JAVA_PKGDIR)/FlomErrorCodes.java:	FlomErrorCodes.awk ../flom_errors.h
	mkdir -p $(JAVA_PKGDIR)
	awk -f $^ > $@

$(JAVA_PKGDIR)/FlomLockModes.java:	FlomLockModes.awk ../flom_types.h
	mkdir -p $(JAVA_PKGDIR)
	awk -f $^ > $@

$(JAVA_PKGDIR)/FlomLockModes.class:	$(JAVA_PKGDIR)/FlomLockModes.java
	javac $<

$(JAVA_PKGDIR)/FlomErrorCodes.class: $(JAVA_PKGDIR)/FlomErrorCodes.java
	javac $<

$(JAVA_PKGDIR)/FlomHandle.class: $(JAVA_PKGDIR)/FlomHandle.java
	javac $<

$(JAVA_PKGDIR)/FlomException.class:	$(JAVA_PKGDIR)/FlomException.java
	javac $<

flom.jar:	$(JAVA_PKGDIR)/FlomErrorCodes.class $(JAVA_PKGDIR)/FlomLockModes.class $(JAVA_PKGDIR)/FlomHandle.class $(JAVA_PKGDIR)/FlomException.class
	jar cf $@ $^

uninstall-hook:
	if test -d ${pkgdatadir}/java; then rmdir ${pkgdatadir}/java; fi

