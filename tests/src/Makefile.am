AM_CPPFLAGS = -I../../src
AM_CFLAGS = -Wall
# C language case tests
case0000_SOURCES = case0000.c
case0001_SOURCES = case0001.c
case0002_SOURCES = case0002.c
# C++ language case tests
case1000_SOURCES = case1000.cc
case1001_SOURCES = case1001.cc
case1002_SOURCES = case1002.cc
# C++ case tests executables are built conditionally (only if --disable-cppapi 
# was not specified at configure time)
if COND_CPPAPI
  MAYBE_CPPAPI=case1000 case1001 case1002
endif
noinst_PROGRAMS = case0000 case0001 case0002 $(MAYBE_CPPAPI)
if COND_PHPAPI
  MAYBE_PHPAPI=case2000.php case2001.php case2002.php
endif
if COND_PYTHONAPI
  MAYBE_PYTHONAPI=case3000.py case3001.py case3002.py
endif
dist_noinst_DATA = case2000.php.in case2001.php.in case2002.php.in \
	$(MAYBE_PYTHONAPI)
noinst_DATA = $(MAYBE_PHPAPI) 

LDADD = ../../src/libflom.la @GLIB2_LIBS@

case2000.php:	case2000.php.in ../../config.status
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	$< >$@

case2001.php:	case2001.php.in ../../config.status
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	$< >$@

case2002.php:	case2002.php.in ../../config.status
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	$< >$@

