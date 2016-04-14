ACLOCAL_AMFLAGS="-Im4"
SUBDIRS = src man etc doc tests
dist_noinst_DATA = doxygen.conf.in
nodist_noinst_DATA = doxygen.conf
dist_doc_DATA = README COPYING ChangeLog TestLog
CLEANFILES = doxygen.conf

doxygen.conf: doxygen.conf.in
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@HAVE_DOT[@]|$(HAVE_DOT)|g' \
	$< >$@

uninstall-hook:
	if test -d ${docdir}; then rmdir ${docdir}; fi
	if test -d ${pkgdatadir}; then rmdir ${pkgdatadir}; fi
