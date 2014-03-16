man1_MANS = flom.man
CLEANFILES = flom.man
flom.man: flom.man.in
	sed \
	-e 's|@docdir[@]|$(docdir)|g' \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
	-e 's|@_RELEASE_DATE[@]|$(_RELEASE_DATE)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@_SYSTEM_CONFIG_FILENAME[@]|$(_SYSTEM_CONFIG_FILENAME)|g' \
	-e 's|@_USER_CONFIG_FILENAME[@]|$(_USER_CONFIG_FILENAME)|g' \
	-e 's|@_DEFAULT_DAEMON_LIFESPAN[@]|$(_DEFAULT_DAEMON_LIFESPAN)|g' \
	-e 's|@_DEFAULT_DAEMON_PORT[@]|$(_DEFAULT_DAEMON_PORT)|g' \
	-e 's|@_DEFAULT_DISCOVERY_TIMEOUT[@]|$(_DEFAULT_DISCOVERY_TIMEOUT)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_TIME[@]|$(_DEFAULT_TCP_KEEPALIVE_TIME)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_INTVL[@]|$(_DEFAULT_TCP_KEEPALIVE_INTVL)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_PROBES[@]|$(_DEFAULT_TCP_KEEPALIVE_PROBES)|g' \
	-e 's|@_ES_UNABLE_TO_EXECUTE_COMMAND[@]|$(_ES_UNABLE_TO_EXECUTE_COMMAND)|g' \
	-e 's|@_ES_RESOURCE_BUSY[@]|$(_ES_RESOURCE_BUSY)|g' \
	-e 's|@_ES_GENERIC_ERROR[@]|$(_ES_GENERIC_ERROR)|g' \
	$< >$@
dist_man1_MANS = flom.man.in
