examplesdir = $(docdir)/examples/php
dist_noinst_DATA = basic.php.in advanced.php.in transactional.php.in README.in
dist_examples_DATA = README
nodist_examples_DATA = basic.php advanced.php transactional.php

basic.php: basic.php.in ../../../config.status
	sed \
	-e 's|@datadir[@]|$(datadir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	$< >$@

advanced.php: advanced.php.in ../../../config.status
	sed \
	-e 's|@datadir[@]|$(datadir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	$< >$@

transactional.php: transactional.php.in ../../../config.status
	sed \
	-e 's|@datadir[@]|$(datadir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	$< >$@

README: README.in ../../../config.status
	sed \
	-e 's|@libdir[@]|$(libdir)|g' \
	$< >$@

uninstall-hook:
	if test -d ${docdir}/examples/php; then rmdir ${docdir}/examples/php; fi
