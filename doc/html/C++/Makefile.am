htmldir = $(docdir)/html/C++
dist_noinst_DATA = doxygen.conf.in Makefile.am
nodist_noinst_DATA = doxygen.conf
dist_html_DATA = *.html *.css *.png *.gif
CLEANFILES = doxygen.conf

doxygen.conf:	doxygen.conf.in
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@top_srcdir[@]|$(top_srcdir)|g' \
	$< >$@

index.html:	doxygen.conf $(top_srcdir)/src/FlomHandle.hh \
		$(top_srcdir)/src/flom.hh
	$(DOXYGEN) doxygen.conf

uninstall-hook:
	if test -d ${docdir}/html/C++; then rmdir ${docdir}/html/C++; fi

