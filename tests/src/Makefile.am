AM_CPPFLAGS = -I../../src
AM_CFLAGS = -Wall
# C language case tests
case0000_SOURCES = case0000.c
case0001_SOURCES = case0001.c
case0002_SOURCES = case0002.c
case0003_SOURCES = case0003.c
# C++ language case tests
case1000_SOURCES = case1000.cc
case1001_SOURCES = case1001.cc
case1002_SOURCES = case1002.cc
# Java language case tests
JAVA_SOURCE_FILES = case4000.java case4001.java case4002.java case4003.java
# PHP language case tests
PHP_SOURCE_FILES = case2000.php.in case2001.php.in case2002.php.in
# Python language case tests
PYTHON_SOURCE_FILES = case3000.py case3001.py case3002.py
# C++ case tests executables are built conditionally (only if --disable-cppapi 
# was not specified at configure time)
if COND_CPPAPI
  MAYBE_CPPAPI=case1000 case1001 case1002
endif
if COND_JAVAAPI
  MAYBE_JAVAAPI=case4000.class case4001.class case4002.class case4003.class
endif
if COND_PHPAPI
  MAYBE_PHPAPI=case2000.php case2001.php case2002.php
endif
if COND_PYTHONAPI
  MAYBE_PYTHONAPI=$(PYTHON_SOURCE_FILES)
endif
noinst_PROGRAMS = case0000 case0001 case0002 case0003 $(MAYBE_CPPAPI)
dist_noinst_DATA = $(JAVA_SOURCE_FILES) $(PHP_SOURCE_FILES) \
	$(PYTHON_SOURCE_FILES)
noinst_DATA = $(MAYBE_PHPAPI) $(MAYBE_JAVAAPI)

LDADD = ../../src/libflom.la @GLIB2_LIBS@ @OPENSSL_LIBS@

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

case4000.class:	case4000.java
	javac -cp $(abs_top_builddir)/src/java/flom.jar $<

case4001.class:	case4001.java
	javac -cp $(abs_top_builddir)/src/java/flom.jar $<

case4002.class:	case4002.java
	javac -cp $(abs_top_builddir)/src/java/flom.jar $<

case4003.class:	case4003.java
	javac -cp $(abs_top_builddir)/src/java/flom.jar $<
