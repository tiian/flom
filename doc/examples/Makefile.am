examplesdir = $(docdir)/examples
dist_noinst_DATA = Makefile.am example_makefile.in
dist_examples_DATA = README basic_static.c basic_dynamic.c \
	advanced_static.c advanced_dynamic.c
nodist_examples_DATA = example_makefile
example_makefile: example_makefile.in
	sed \
	-e 's|@GLIB2_LIBS[@]|$(GLIB2_LIBS)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	$< >$@
