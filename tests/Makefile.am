SUBDIRS = src

# Interrupt the test at first failure
TESTSUITEFLAGS = "-e"

# Always generate package.m4 into the source directory, not into the
# build directory, since it must be distributed, along with testsuite,
# configure, etc.
$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	:;{ \
	  echo '# Signature of the current package.' && \
	  echo 'm4_define([AT_PACKAGE_NAME],      [$(PACKAGE_NAME)])' && \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [$(PACKAGE_TARNAME)])' && \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [$(PACKAGE_VERSION)])' && \
	  echo 'm4_define([AT_PACKAGE_STRING],    [$(PACKAGE_STRING)])' && \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [$(PACKAGE_BUGREPORT)])'; \
	  echo 'm4_define([AT_PACKAGE_URL],       [$(PACKAGE_URL)])'; \
	} > $@-t
	mv $@-t $@

EXTRA_DIST = $(TESTSUITE) atlocal.in \
	package.m4 \
	testsuite.at \
	api.at.in \
	api_perl.at.in \
	cmdopt.at.in \
	monitor.at \
	shutdown.at \
	incompopt.at.in \
	debug-features.at \
	tls.at.in \
	usecase.at.in \
	usecase-dist.at.in \
	usecase-hier.at \
	usecase-lt.at.in \
	usecase-num.at.in \
	usecase-seq.at \
	usecase-set.at.in \
	usecase-tms.at.in \
	vfs.at.in \
	scripts/*.sh

api.at: api.at.in
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	-e 's|@USER[@]|$(USER)|g' \
	-e 's|@STDBUF_O0[@]|$(STDBUF_O0)|g' \
	$< >$@

api_perl.at: api_perl.at.in
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	-e 's|@USER[@]|$(USER)|g' \
	$< >$@

cmdopt.at: cmdopt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_SYSTEM_CONFIG_FILENAME[@]|$(_SYSTEM_CONFIG_FILENAME)|g' \
	-e 's|@_CONFIG_GROUP_TRACE[@]|$(_CONFIG_GROUP_TRACE)|g' \
	-e 's|@_CONFIG_KEY_DAEMONTRACEFILE[@]|$(_CONFIG_KEY_DAEMONTRACEFILE)|g' \
	-e 's|@_CONFIG_KEY_COMMANDTRACEFILE[@]|$(_CONFIG_KEY_COMMANDTRACEFILE)|g' \
	-e 's|@_CONFIG_KEY_APPENDTRACEFILE[@]|$(_CONFIG_KEY_APPENDTRACEFILE)|g' \
	-e 's|@_CONFIG_KEY_VERBOSE[@]|$(_CONFIG_KEY_VERBOSE)|g' \
	-e 's|@_CONFIG_GROUP_RESOURCE[@]|$(_CONFIG_GROUP_RESOURCE)|g' \
	-e 's|@_CONFIG_KEY_NAME[@]|$(_CONFIG_KEY_NAME)|g' \
	-e 's|@_CONFIG_KEY_WAIT[@]|$(_CONFIG_KEY_WAIT)|g' \
	-e 's|@_CONFIG_KEY_TIMEOUT[@]|$(_CONFIG_KEY_TIMEOUT)|g' \
	-e 's|@_CONFIG_KEY_QUANTITY[@]|$(_CONFIG_KEY_QUANTITY)|g' \
	-e 's|@_CONFIG_KEY_CREATE[@]|$(_CONFIG_KEY_CREATE)|g' \
	-e 's|@_CONFIG_KEY_LOCK_MODE[@]|$(_CONFIG_KEY_LOCK_MODE)|g' \
	-e 's|@_CONFIG_KEY_IDLE_LIFESPAN[@]|$(_CONFIG_KEY_IDLE_LIFESPAN)|g' \
	-e 's|@_CONFIG_GROUP_DAEMON[@]|$(_CONFIG_GROUP_DAEMON)|g' \
	-e 's|@_CONFIG_KEY_SOCKET_NAME[@]|$(_CONFIG_KEY_SOCKET_NAME)|g' \
	-e 's|@_CONFIG_KEY_LIFESPAN[@]|$(_CONFIG_KEY_LIFESPAN)|g' \
	-e 's|@_CONFIG_KEY_UNICAST_ADDRESS[@]|$(_CONFIG_KEY_UNICAST_ADDRESS)|g' \
	-e 's|@_CONFIG_KEY_UNICAST_PORT[@]|$(_CONFIG_KEY_UNICAST_PORT)|g' \
	-e 's|@_CONFIG_KEY_MULTICAST_ADDRESS[@]|$(_CONFIG_KEY_MULTICAST_ADDRESS)|g' \
	-e 's|@_CONFIG_KEY_MULTICAST_PORT[@]|$(_CONFIG_KEY_MULTICAST_PORT)|g' \
	-e 's|@_CONFIG_KEY_MOUNT_POINT_VFS[@]|$(_CONFIG_KEY_MOUNT_POINT_VFS)|g' \
	-e 's|@_CONFIG_GROUP_MONITOR[@]|$(_CONFIG_GROUP_MONITOR)|g' \
	-e 's|@_CONFIG_KEY_IGNORED_SIGNALS[@]|$(_CONFIG_KEY_IGNORED_SIGNALS)|g' \
	-e 's|@_CONFIG_GROUP_NETWORK[@]|$(_CONFIG_GROUP_NETWORK)|g' \
	-e 's|@_CONFIG_KEY_NETWORK_INTERFACE[@]|$(_CONFIG_KEY_NETWORK_INTERFACE)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_ATTEMPTS[@]|$(_CONFIG_KEY_DISCOVERY_ATTEMPTS)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_TIMEOUT[@]|$(_CONFIG_KEY_DISCOVERY_TIMEOUT)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_TTL[@]|$(_CONFIG_KEY_DISCOVERY_TTL)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_TIME[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_TIME)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_INTVL[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_INTVL)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_PROBES[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_PROBES)|g' \
	-e 's|@_CONFIG_GROUP_TLS[@]|$(_CONFIG_GROUP_TLS)|g' \
	-e 's|@_CONFIG_KEY_TLS_CERTIFICATE[@]|$(_CONFIG_KEY_TLS_CERTIFICATE)|g' \
	-e 's|@_CONFIG_KEY_TLS_PRIVATE_KEY[@]|$(_CONFIG_KEY_TLS_PRIVATE_KEY)|g' \
	-e 's|@_CONFIG_KEY_TLS_CA_CERTIFICATE[@]|$(_CONFIG_KEY_TLS_CA_CERTIFICATE)|g' \
	-e 's|@_CONFIG_KEY_TLS_CHECK_PEER_ID[@]|$(_CONFIG_KEY_TLS_CHECK_PEER_ID)|g' \
	-e 's|@NET_IFACE[@]|$(NET_IFACE)|g' \
	-e 's|@TESTCASES[@]|$(TESTCASES)|g' \
	$< >$@

incompopt.at: incompopt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_ES_UNABLE_TO_EXECUTE_COMMAND[@]|$(_ES_UNABLE_TO_EXECUTE_COMMAND)|g' \
	$< >$@

tls.at: tls.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_CONFIG_GROUP_TLS[@]|$(_CONFIG_GROUP_TLS)|g' \
	-e 's|@_CONFIG_KEY_TLS_CERTIFICATE[@]|$(_CONFIG_KEY_TLS_CERTIFICATE)|g' \
	-e 's|@_CONFIG_KEY_TLS_PRIVATE_KEY[@]|$(_CONFIG_KEY_TLS_PRIVATE_KEY)|g' \
	-e 's|@_CONFIG_KEY_TLS_CA_CERTIFICATE[@]|$(_CONFIG_KEY_TLS_CA_CERTIFICATE)|g' \
	-e 's|@_CONFIG_KEY_TLS_CHECK_PEER_ID[@]|$(_CONFIG_KEY_TLS_CHECK_PEER_ID)|g' \
	$< >$@

usecase.at: usecase.at.in
	sed \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	$< >$@

usecase-num.at: usecase-num.at.in
	sed \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	$< >$@

usecase-set.at: usecase-set.at.in
	sed \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	$< >$@

usecase-dist.at: usecase-dist.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	$< >$@

usecase-lt.at: usecase-lt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	-e 's|@_ES_REQUESTER_CANT_WAIT[@]|$(_ES_REQUESTER_CANT_WAIT)|g' \
	-e 's|@NET_IFACE[@]|$(NET_IFACE)|g' \
	$< >$@

usecase-tms.at: usecase-tms.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@STDBUF_O0[@]|$(STDBUF_O0)|g' \
	$< >$@

vfs.at: vfs.at.in
	sed \
	-e 's|@abs_top_builddir[@]|$(abs_top_builddir)|g' \
	-e 's|@USER[@]|$(USER)|g' \
	$< >$@

TESTSUITE = $(srcdir)/testsuite

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)
installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)
clean-local:
	test ! -f '$(TESTSUITE)' || \
	  $(SHELL) '$(TESTSUITE)' --clean

AUTOM4TE = autom4te
AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(srcdir)/package.m4 \
	$(srcdir)/testsuite.at \
	$(srcdir)/api.at \
	$(srcdir)/api_perl.at \
	$(srcdir)/cmdopt.at \
	$(srcdir)/monitor.at \
	$(srcdir)/shutdown.at \
	$(srcdir)/incompopt.at \
	$(srcdir)/debug-features.at \
	$(srcdir)/tls.at \
	$(srcdir)/usecase.at \
	$(srcdir)/usecase-dist.at \
	$(srcdir)/usecase-lt.at \
	$(srcdir)/usecase-hier.at \
	$(srcdir)/usecase-num.at \
	$(srcdir)/usecase-seq.at \
	$(srcdir)/usecase-set.at \
	$(srcdir)/usecase-tms.at \
	$(srcdir)/vfs.at
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

DISTCLEANFILES = atconfig
CLEANFILES = api.at api_perl.at cmdopt.at incompopt.at tls.at usecase.at \
	usecase-dist.at usecase-lt.at usecase-num.at usecase-set.at \
	usecase-tms.at vfs.at
