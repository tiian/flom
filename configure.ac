AC_INIT([flom], [0.0.0], [tiian at users dot sourceforge dot net], [], [http://sourceforge.net/projects/flom/])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_CC
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
# AC_CONFIG_TESTDIR([tests])

# Installation PATH
test "x$prefix" = "xNONE" && prefix="/opt/flom"
test "x$exec_prefix" = "xNONE" -a "x$prefix" != "xNONE" && exec_prefix=$prefix

# Expanded variables can be used in C code and XML files
EXPANDED_SYSCONFDIR=$(eval echo $sysconfdir)
EXPANDED_LOCALSTATEDIR=$(eval echo $localstatedir)
EXPANDED_LIBDIR=$(eval echo $libdir)

AC_PRESERVE_HELP_ORDER
AC_ARG_ENABLE([trace],
        AS_HELP_STRING([--disable-trace],
                [disable trace feature (not suggested)]),
        [_TRACE="no"], [_TRACE="yes"; CPPFLAGS="$CPPFLAGS -D_TRACE"] )

#AM_PROG_CC_C_O

# Checking pkg-config is available
AC_CHECK_PROGS(PKG_CONFIG, [pkg-config], [])
if test -z $PKG_CONFIG 
then
        AC_MSG_ERROR([cannot find pkg-config program])
fi
# Checking glib2 is configured with pkg-config
AC_MSG_CHECKING([if glib2 is pkg-config configured])
$PKG_CONFIG glib-2.0
if test $? -ne 0
then
	AC_MSG_ERROR([it seems glib-2.0 is not configured with pkg-config])
else
	CPPFLAGS="$CPPFLAGS $($PKG_CONFIG --cflags glib-2.0)"
	LDFLAGS_GLIB="$($PKG_CONFIG --libs glib-2.0)"
	AC_MSG_RESULT([yes])
fi

# Vars useful for lixa-config utility program
eval "FLOM_CONFIG_INCLUDEDIR=$includedir"
eval "FLOM_CONFIG_LIBDIR=$libdir"
eval "FLOM_CONFIG_PREFIX=$prefix"
eval "FLOM_CONFIG_LOCALSTATEDIR=$localstatedir"
eval "FLOM_CONFIG_SYSCONFDIR=$sysconfdir"


# Checking pkg-config is available
AC_CHECK_PROGS(GDB, [gdb], [])
AC_CHECK_PROGS(VALGRIND, [valgrind], [])
AC_CHECK_PROGS(XSLTPROC, [xsltproc], [])
AC_CHECK_PROG(HAVE_DOT, [dot], [YES], [NO])

AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(glib.h)
AC_CHECK_HEADERS(stdarg.h stdint.h stdio.h stdlib.h string.h)
AC_CHECK_HEADERS(sys/inttypes.h sys/time.h sys/types.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(unistd.h)
#AC_CHECK_HEADERS(unistd.h)
#AC_CHECK_HEADERS(arpa/inet.h assert.h)
#AC_CHECK_HEADERS(fcntl.h)
#AC_CHECK_HEADERS(glib.h)
#AC_CHECK_HEADERS(libgen.h)
#AC_CHECK_HEADERS(netdb.h netinet/in.h netinet/tcp.h)
#AC_CHECK_HEADERS(poll.h pthread.h regex.h)
#AC_CHECK_HEADERS(signal.h stdarg.h stdint.h stdio.h stdlib.h string.h syslog.h)
#AC_CHECK_HEADERS(sys/mman.h sys/select.h sys/socket.h sys/stat.h)
#AC_CHECK_HEADERS(sys/time.h sys/types.h time.h)
#AC_CHECK_HEADERS(unistd.h uuid/uuid.h)

#AC_SEARCH_LIBS([sqrt], [m])

#AC_CHECK_TYPES([clock_t], [], [], [[#include <time.h>]])
#AC_CHECK_TYPES([in_port_t], [], [], [[#include <netinet/in.h>]])
#AC_CHECK_TYPES([nfds_t], [], [], [[#include <poll.h>]])
#AC_CHECK_TYPES(off_t)
#AC_CHECK_TYPES(pthread_t)
AC_CHECK_TYPES(pid_t)
AC_CHECK_TYPES(uint8_t)
AC_CHECK_TYPES(uint32_t)
AC_CHECK_TYPES(uintptr_t)
#AC_CHECK_TYPES([uuid_t], [], \
#	[AC_MSG_ERROR([cannot go on without uuid_t type])], \
#	[[#include <uuid/uuid.h>]])
#AC_CHECK_TYPES([in_addr_t], [], [], [[#include <netinet/in.h>]])
#AC_CHECK_TYPES([in_port_t], [], [], [[#include <netinet/in.h>]])

#AC_CHECK_SIZEOF([short int])
#AC_CHECK_SIZEOF([int])
#AC_CHECK_SIZEOF([long int])
#AC_CHECK_SIZEOF([long long int])
#AC_CHECK_SIZEOF([clock_t], [], [[#include <time.h>]])
#AC_CHECK_SIZEOF([in_port_t], [], [[#include <netinet/in.h>]])
#AC_CHECK_SIZEOF([off_t])
#AC_CHECK_SIZEOF([nfds_t], [], [[#include <poll.h>]])
#AC_CHECK_SIZEOF([pid_t], [], [[#include <sys/types.h>]])
#AC_CHECK_SIZEOF([pthread_t], [], [[#include <pthread.h>]])
#AC_CHECK_SIZEOF([size_t])
#AC_CHECK_SIZEOF([uintptr_t])
#AC_CHECK_SIZEOF([uint32_t])
#AC_CHECK_SIZEOF([uuid_t], [], [[#include <uuid/uuid.h>]])

AC_CHECK_FUNCS(vfprintf)

AC_SUBST(PACKAGE_BUGREPORT)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(LDFLAGS_GLIB)
AC_SUBST(EXPANDED_SYSCONFDIR)
AC_SUBST(EXPANDED_LOCALSTATEDIR)
AC_SUBST(EXPANDED_LIBDIR)
AC_SUBST(HAVE_DOT)
AC_SUBST(FLOM_CONFIG_INCLUDEDIR)
AC_SUBST(FLOM_CONFIG_LIBDIR)
AC_SUBST(FLOM_CONFIG_PREFIX)
AC_SUBST(FLOM_CONFIG_LOCALSTATEDIR)
AC_SUBST(FLOM_CONFIG_SYSCONFDIR)

AC_OUTPUT
