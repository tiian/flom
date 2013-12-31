SUBDIRS = src man etc doc
# Unfortunately doxygen.conf.in is installed too (it should be only distributed)
# I was not able to fix it!
dist_doc_DATA = README doxygen.conf.in
nodist_doc_DATA = doxygen.conf
CLEANFILES = doxygen.conf
doxygen.conf: doxygen.conf.in
	sed \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@HAVE_DOT[@]|$(HAVE_DOT)|g' \
	$< >$@

