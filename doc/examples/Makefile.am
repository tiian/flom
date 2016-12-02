if COND_JAVAAPI
  MAYBE_JAVAAPI = java
endif
if COND_PERLAPI
  MAYBE_PERLAPI = perl
endif
if COND_PHPAPI
  MAYBE_PHPAPI = php
endif
if COND_PYTHONAPI
  MAYBE_PYTHONAPI = python
endif
SUBDIRS = $(MAYBE_JAVAAPI) $(MAYBE_PERLAPI) $(MAYBE_PHPAPI) $(MAYBE_PYTHONAPI) .
examplesdir = $(docdir)/examples
if COND_CPPAPI
  CPPSOURCE=BasicStatic.cc BasicDynamic.cc AdvancedStatic.cc \
	AdvancedDynamic.cc Transactional.cc
  CPPTARGET=BasicStatic BasicDynamic AdvancedStatic AdvancedDynamic Transactional
endif
dist_noinst_DATA = Makefile.am example_makefile.in README.in
dist_examples_DATA = README basic_static.c basic_dynamic.c \
	advanced_static.c advanced_dynamic.c transactional.c $(CPPSOURCE)
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
        -e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	$< >$@

uninstall-hook:
	if test -d ${docdir}/examples; then rmdir ${docdir}/examples; fi
