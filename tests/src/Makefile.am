AM_CPPFLAGS = -I../../src
AM_CFLAGS = -Wall
# C language case tests
case0000_SOURCES = case0000.c
case0001_SOURCES = case0001.c
case0002_SOURCES = case0002.c
# C++ language case tests
case1000_SOURCES = case1000.cc
case1001_SOURCES = case1001.cc
# C++ case tests executables are built conditionally (only if --disable-cppapi 
# was not specified at configure time)
if COND_CPPAPI
  MAYBE_CPPAPI=case1000 case1001
endif
LDADD = ../../src/libflom.la @GLIB2_LIBS@

noinst_PROGRAMS = case0000 case0001 case0002 $(MAYBE_CPPAPI)
