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
SUBDIRS = . $(MAYBE_JAVAAPI) $(MAYBE_PERLAPI) $(MAYBE_PHPAPI) $(MAYBE_PYTHONAPI)
AM_CPPFLAGS = -D_SYSCONFDIR='"@sysconfdir@"'
AM_CFLAGS = -Wall
lib_LTLIBRARIES = libflom.la
libflom_la_LDFLAGS = -version-info @LT_CURRENT@:@LT_REVISION@:@LT_AGE@
flom_LDADD=@GLIB2_LIBS@ @DBUS1_LIBS@ @FUSE_LIBS@ @OPENSSL_LIBS@ libflom.la
flom_LDFLAGS = -Wl,-rpath -Wl,$(libdir)
bin_PROGRAMS = flom
EXTRA_DIST = flom_errors.h.in
BUILT_SOURCES = flom_errors.h
CLEANFILES = flom_errors.h
if COND_CPPAPI
  INST_CPPAPI=flom.hh FlomHandle.hh
else
  NOINST_CPPAPI=flom.hh FlomHandle.hh
endif

flom_errors.h: flom_errors.h.in
	sed \
	-e 's|@_ES_REQUESTER_CANT_WAIT[@]|$(_ES_REQUESTER_CANT_WAIT)|g' \
	-e 's|@_ES_UNABLE_TO_EXECUTE_COMMAND[@]|$(_ES_UNABLE_TO_EXECUTE_COMMAND)|g' \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	$< >$@

dist_include_HEADERS = flom.h flom_handle.h flom_types.h \
	$(INST_CPPAPI)
nodist_include_HEADERS = flom_errors.h
noinst_HEADERS = flom_client.h flom_config.h flom_conn.h flom_conns.h \
	flom_debug_features.h flom_daemon.h flom_daemon_mngmnt.h \
	flom_daemon_mngmnt_vfs.h flom_defines.h flom_exec.h \
	flom_locker.h flom_msg.h flom_resource_hier.h \
	flom_resource_numeric.h flom_resource_sequence.h flom_resource_set.h \
	flom_resource_simple.h flom_resource_timestamp.h flom_rsrc.h \
	flom_syslog.h flom_tcp.h flom_tls.h flom_trace.h $(NOINST_CPPAPI)

libflom_la_SOURCES = flom_client.c flom_config.c flom_conn.c flom_conns.c \
	flom_daemon.c flom_daemon_mngmnt.c flom_daemon_mngmnt_vfs.c \
	flom_errors.c flom_locker.c \
	flom_msg.c flom_handle.c \
	flom_resource_hier.c flom_resource_numeric.c flom_resource_sequence.c \
	flom_resource_set.c flom_resource_simple.c flom_resource_timestamp.c \
	flom_rsrc.c flom_tcp.c flom_tls.c flom_trace.c

flom_SOURCES = main.c flom_exec.c flom_debug_features.c
