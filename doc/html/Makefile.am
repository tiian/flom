if COND_JAVAAPI
  MAYBE_JAVADOC = java
endif
SUBDIRS = $(MAYBE_JAVADOC) C C++ .
htmldir = $(docdir)/html
dist_noinst_DATA = doxygen_header.htm doxygen_footer.htm Makefile.am README

uninstall-hook:
	if test -d ${docdir}/html; then rmdir ${docdir}/html; fi

