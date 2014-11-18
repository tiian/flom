dist_data_DATA = doxygen.conf.in doxygen_header.htm doxygen_footer.htm \
	Makefile.am
nodist_data_DATA = doxygen.conf
dist_doc_DATA = *.html *.css *.png *.gif
CLEANFILES = doxygen.conf

doxygen.conf:	doxygen.conf.in
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@top_srcdir[@]|$(top_srcdir)|g' \
	$< >$@

index.html:	doxygen.conf \
		$(top_srcdir)/src/flom.h \
		$(top_srcdir)/src/flom_errors.h \
		$(top_srcdir)/src/flom_handle.h \
		$(top_srcdir)/src/flom_handle.h
	$(DOXYGEN) doxygen.conf
