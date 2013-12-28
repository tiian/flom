AM_CPPFLAGS = -DSYSCONFDIR='"@sysconfdir@"'
AM_CFLAGS = -Wall
bin_PROGRAMS = flom

#CLEANFILES = flom_inst_conf.c
#flom_inst_conf.c: flom_inst_conf.c.in
#	sed \
#	-e 's|@PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
#	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
#	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
#	-e 's|@RELEASE_DATE[@]|$(RELEASE_DATE)|g' \
#	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
#	$< >$@

noinst_HEADERS = flom_client.h flom_config.h flom_conns.h flom_daemon.h \
	flom_defines.h flom_errors.h flom_exec.h flom_inst_conf.h \
	flom_locker.h flom_msg.h flom_resource_simple.h flom_rsrc.h \
	flom_trace.h
#dist_flom_SOURCES = flom_inst_conf.c.in
flom_SOURCES = main.c flom_client.c flom_config.c flom_conns.c flom_daemon.c \
	flom_errors.c flom_exec.c flom_inst_conf.c flom_locker.c flom_msg.c \
	flom_resource_simple.c flom_rsrc.c flom_trace.c

