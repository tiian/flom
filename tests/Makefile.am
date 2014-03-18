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
	cmdopt.at.in \
	incompopt.at.in \
	usecase.at \
	usecase-dist.at.in \
	scripts/flom_*.sh

cmdopt.at: cmdopt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_SYSTEM_CONFIG_FILENAME[@]|$(_SYSTEM_CONFIG_FILENAME)|g' \
	-e 's|@_CONFIG_GROUP_TRACE[@]|$(_CONFIG_GROUP_TRACE)|g' \
	-e 's|@_CONFIG_KEY_DAEMONTRACEFILE[@]|$(_CONFIG_KEY_DAEMONTRACEFILE)|g' \
	-e 's|@_CONFIG_KEY_COMMANDTRACEFILE[@]|$(_CONFIG_KEY_COMMANDTRACEFILE)|g' \
	-e 's|@_CONFIG_GROUP_RESOURCE[@]|$(_CONFIG_GROUP_RESOURCE)|g' \
	-e 's|@_CONFIG_KEY_NAME[@]|$(_CONFIG_KEY_NAME)|g' \
	-e 's|@_CONFIG_KEY_WAIT[@]|$(_CONFIG_KEY_WAIT)|g' \
	-e 's|@_CONFIG_KEY_TIMEOUT[@]|$(_CONFIG_KEY_TIMEOUT)|g' \
	-e 's|@_CONFIG_KEY_LOCK_MODE[@]|$(_CONFIG_KEY_LOCK_MODE)|g' \
	-e 's|@_CONFIG_GROUP_DAEMON[@]|$(_CONFIG_GROUP_DAEMON)|g' \
	-e 's|@_CONFIG_KEY_SOCKET_NAME[@]|$(_CONFIG_KEY_SOCKET_NAME)|g' \
	-e 's|@_CONFIG_KEY_LIFESPAN[@]|$(_CONFIG_KEY_LIFESPAN)|g' \
	-e 's|@_CONFIG_KEY_UNICAST_ADDRESS[@]|$(_CONFIG_KEY_UNICAST_ADDRESS)|g' \
	-e 's|@_CONFIG_KEY_UNICAST_PORT[@]|$(_CONFIG_KEY_UNICAST_PORT)|g' \
	-e 's|@_CONFIG_KEY_MULTICAST_ADDRESS[@]|$(_CONFIG_KEY_MULTICAST_ADDRESS)|g' \
	-e 's|@_CONFIG_KEY_MULTICAST_PORT[@]|$(_CONFIG_KEY_MULTICAST_PORT)|g' \
	-e 's|@_CONFIG_GROUP_NETWORK[@]|$(_CONFIG_GROUP_NETWORK)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_ATTEMPTS[@]|$(_CONFIG_KEY_DISCOVERY_ATTEMPTS)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_TIMEOUT[@]|$(_CONFIG_KEY_DISCOVERY_TIMEOUT)|g' \
	-e 's|@_CONFIG_KEY_DISCOVERY_TTL[@]|$(_CONFIG_KEY_DISCOVERY_TTL)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_TIME[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_TIME)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_INTVL[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_INTVL)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_PROBES[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_PROBES)|g' \
	$< >$@

incompopt.at: incompopt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	$< >$@

usecase-dist.at: usecase-dist.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
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
	$(srcdir)/cmdopt.at \
	$(srcdir)/incompopt.at \
	$(srcdir)/usecase.at \
	$(srcdir)/usecase-dist.at
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

DISTCLEANFILES = atconfig
CLEANFILES = cmdopt.at incompopt.at usecase-dist.at
