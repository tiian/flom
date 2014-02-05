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
	scripts/flom_*.sh

cmdopt.at: cmdopt.at.in
	sed \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	-e 's|@_SYSTEM_CONFIG_FILENAME[@]|$(_SYSTEM_CONFIG_FILENAME)|g' \
	$< >$@

incompopt.at: incompopt.at.in
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
	$(srcdir)/usecase.at
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@

DISTCLEANFILES = atconfig
CLEANFILES = cmdopt.at incompopt.at
