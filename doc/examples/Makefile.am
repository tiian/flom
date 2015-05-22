if COND_PHPAPI
  MAYBE_PHPAPI = php
endif
if COND_PYTHONAPI
  MAYBE_PYTHONAPI = python
endif
SUBDIRS = . $(MAYBE_PHPAPI) $(MAYBE_PYTHONAPI)
examplesdir = $(docdir)/examples
if COND_CPPAPI
  CPPSOURCE=BasicStatic.cc BasicDynamic.cc AdvancedStatic.cc AdvancedDynamic.cc
  CPPTARGET=BasicStatic BasicDynamic AdvancedStatic AdvancedDynamic
endif
dist_noinst_DATA = Makefile.am example_makefile.in README.in
dist_examples_DATA = README basic_static.c basic_dynamic.c \
	advanced_static.c advanced_dynamic.c $(CPPSOURCE)
nodist_examples_DATA = example_makefile
example_makefile: example_makefile.in ../../config.status
	sed \
	-e 's|@CC[@]|$(CC)|g' \
	-e 's|@CXX[@]|$(CXX)|g' \
	-e 's|@CPPTARGET[@]|$(CPPTARGET)|g' \
	-e 's|@GLIB2_LIBS[@]|$(GLIB2_LIBS)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	$< >$@
README:	README.in ../../config.status
	sed \
	-e 's|@libdir[@]|$(libdir)|g' \
	$< >$@

