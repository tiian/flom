SUBDIRS = src man
dist_doc_DATA = README doxygen.conf.in

CLEANFILES = doxygen.conf
doxygen.conf: doxygen.conf.in
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@HAVE_DOT[@]|$(HAVE_DOT)|g' \
	$< >$@
doc_DATA = doxygen.conf
