AM_CPPFLAGS = -D_SYSCONFDIR='"@sysconfdir@"'
AM_CFLAGS = -Wall
flom_LDADD=@GLIB2_LIBS@
bin_PROGRAMS = flom

noinst_HEADERS = flom_client.h flom_config.h flom_conns.h flom_daemon.h \
	flom_daemon_mngmnt.h \
	flom_defines.h flom_errors.h flom_exec.h \
	flom_locker.h flom_msg.h flom_resource_hier.h \
	flom_resource_numeric.h flom_resource_set.h \
	flom_resource_simple.h flom_rsrc.h \
	flom_syslog.h flom_trace.h
flom_SOURCES = main.c flom_client.c flom_config.c flom_conns.c flom_daemon.c \
	flom_daemon_mngmnt.c \
	flom_errors.c flom_exec.c flom_locker.c flom_msg.c \
	flom_resource_hier.c flom_resource_numeric.c flom_resource_set.c \
	flom_resource_simple.c flom_rsrc.c flom_trace.c

