man1_MANS = flom.man
CLEANFILES = flom.man
flom.man: flom.man.in
	sed \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
	-e 's|@RELEASE_DATE[@]|$(RELEASE_DATE)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@SYSTEM_CONFIG_FILENAME[@]|$(SYSTEM_CONFIG_FILENAME)|g' \
	-e 's|@USER_CONFIG_FILENAME[@]|$(USER_CONFIG_FILENAME)|g' \
	$< >$@
dist_man1_MANS = flom.man.in
