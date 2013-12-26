man1_MANS = flom.man
CLEANFILES = flom.man
flom.man: flom.man.in
	sed \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
	-e 's|@RELEASE_DATE[@]|$(RELEASE_DATE)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	$< >$@
dist_man1_MANS = flom.man.in
