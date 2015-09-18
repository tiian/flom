SUBDIRS = resources org/tiian/flom .
htmldir = $(docdir)/html/java
dist_noinst_DATA = Makefile.am
#dist_html_DATA = *.html *.css *.png *.gif
dist_html_DATA = package-list *.html *.css

package-list:	$(top_srcdir)/src/java/org/tiian/flom/*.java
	javadoc $^

uninstall-hook:
	if test -d ${docdir}/html/java/resources; then rmdir ${docdir}/html/java/resources; fi
	if test -d ${docdir}/html/java/org/tiian/flom; then rmdir ${docdir}/html/java/org/tiian/flom; fi
	if test -d ${docdir}/html/java/org/tiian; then rmdir ${docdir}/html/java/org/tiian; fi
	if test -d ${docdir}/html/java/org; then rmdir ${docdir}/html/java/org; fi
	if test -d ${docdir}/html/java; then rmdir ${docdir}/html/java; fi
