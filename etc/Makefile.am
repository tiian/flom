# flom.conf.in should be distributed but not installed
# flom.conf should be installed but not distributed
# the best I was able to obtain was:
#	flom.conf.in is distributed and installed
#	flom.conf    is installed but not distributed
dist_doc_DATA = flom.conf.in
nodist_doc_DATA = flom.conf 
CLEANFILES = flom.conf
flom.conf : flom.conf.in
	sed \
        -e 's|@sysconfdir[@]|$(sysconfdir)|g' \
        -e 's|@_USER_CONFIG_FILENAME[@]|$(_USER_CONFIG_FILENAME)|g' \
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
	-e 's|@_CONFIG_KEY_DISCOVERY_TIMEOUT[@]|$(_CONFIG_KEY_DISCOVERY_TIMEOUT)|g' \
	-e 's|@_DEFAULT_DAEMON_LIFESPAN[@]|$(_DEFAULT_DAEMON_LIFESPAN)|g' \
	-e 's|@_DEFAULT_DAEMON_PORT[@]|$(_DEFAULT_DAEMON_PORT)|g' \
	-e 's|@_CONFIG_GROUP_NETWORK[@]|$(_CONFIG_GROUP_NETWORK)|g' \
	-e 's|@_DEFAULT_DISCOVERY_TIMEOUT[@]|$(_DEFAULT_DISCOVERY_TIMEOUT)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_TIME[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_TIME)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_INTVL[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_INTVL)|g' \
	-e 's|@_CONFIG_KEY_TCP_KEEPALIVE_PROBES[@]|$(_CONFIG_KEY_TCP_KEEPALIVE_PROBES)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_TIME[@]|$(_DEFAULT_TCP_KEEPALIVE_TIME)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_INTVL[@]|$(_DEFAULT_TCP_KEEPALIVE_INTVL)|g' \
	-e 's|@_DEFAULT_TCP_KEEPALIVE_PROBES[@]|$(_DEFAULT_TCP_KEEPALIVE_PROBES)|g' \
	$< >$@
