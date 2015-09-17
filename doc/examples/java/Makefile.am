examplesdir = $(docdir)/examples/java
dist_noinst_DATA = Basic.java.in Advanced.java.in README.in
dist_examples_DATA = README
nodist_examples_DATA = Basic.java Advanced.java

Basic.java:	Basic.java.in
	sed \
	-e 's|@datadir[@]|$(datadir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	$< >$@

Advanced.java:	Advanced.java.in
	sed \
	-e 's|@datadir[@]|$(datadir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' \
	$< >$@

README:		README.in
	sed \
	-e 's|@libdir[@]|$(libdir)|g' \
	$< >$@

uninstall-hook:
	if test -d ${docdir}/examples/java; then rmdir ${docdir}/examples/java; fi
