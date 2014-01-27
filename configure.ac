AC_INIT([flom], [0.1.5], [tiian at users dot sourceforge dot net], [], [http://sourceforge.net/projects/flom/])
AM_INIT_AUTOMAKE([-Wall -Werror])

# hardwired configuration
_RELEASE_DATE="2014-01-27"
_SYSTEM_CONFIG_FILENAME="flom.conf"
_USER_CONFIG_FILENAME=".flom"
_DIR_FILE_SEPARATOR="/"
_CONFIG_GROUP_TRACE="Trace"
_CONFIG_KEY_DAEMONTRACEFILE="DaemonTraceFile"
_CONFIG_KEY_COMMANDTRACEFILE="CommandTraceFile"
_CONFIG_GROUP_RESOURCE="Resource"
_CONFIG_KEY_NAME="Name"
_CONFIG_KEY_WAIT="Wait"
_CONFIG_KEY_TIMEOUT="Timeout"
_CONFIG_KEY_LOCK_MODE="LockMode"
_CONFIG_GROUP_COMMUNICATION="Communication"
_CONFIG_KEY_SOCKET_NAME="SocketName"
_ES_UNABLE_TO_EXECUTE_COMMAND=97
_ES_RESOURCE_BUSY=98
_ES_GENERIC_ERROR=99
# definition inside config.h for hardwired configuration
AC_DEFINE_UNQUOTED([_RELEASE_DATE], ["$_RELEASE_DATE"], [Date of release])
AC_DEFINE_UNQUOTED([_SYSTEM_CONFIG_FILENAME], ["$_SYSTEM_CONFIG_FILENAME"], [System wide configuration filename])
AC_DEFINE_UNQUOTED([_USER_CONFIG_FILENAME], ["$_USER_CONFIG_FILENAME"], [User default configuration filename])
AC_DEFINE_UNQUOTED([_DIR_FILE_SEPARATOR], ["$_DIR_FILE_SEPARATOR"], [String used to separate directory from file inside fully qualified path names])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_TRACE], ["$_CONFIG_GROUP_TRACE"], [Label of "trace" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DAEMONTRACEFILE], ["$_CONFIG_KEY_DAEMONTRACEFILE"], [Label of "daemon trace file" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_COMMANDTRACEFILE], ["$_CONFIG_KEY_COMMANDTRACEFILE"], [Label of "command trace file" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_RESOURCE], ["$_CONFIG_GROUP_RESOURCE"], [Label of "resource" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_NAME], ["$_CONFIG_KEY_NAME"], [Label of "Name" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_WAIT], ["$_CONFIG_KEY_WAIT"], [Label of "Wait" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TIMEOUT], ["$_CONFIG_KEY_TIMEOUT"], [Label of "Timeout" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_LOCK_MODE], ["$_CONFIG_KEY_LOCK_MODE"], [Label of "LockMode" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_COMMUNICATION], ["$_CONFIG_GROUP_COMMUNICATION"], [Label of "communication" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_SOCKET_NAME], ["$_CONFIG_KEY_SOCKET_NAME"], [Label of "SocketName" key inside config files])
AC_DEFINE_UNQUOTED([_ES_UNABLE_TO_EXECUTE_COMMAND], [$_ES_UNABLE_TO_EXECUTE_COMMAND], [Exit status for command execution error])
AC_DEFINE_UNQUOTED([_ES_RESOURCE_BUSY], [$_ES_RESOURCE_BUSY], [Exit status for busy resource condition])
AC_DEFINE_UNQUOTED([_ES_GENERIC_ERROR], [$_ES_GENERIC_ERROR], [Exit status for generic error])

AC_PROG_CC
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  etc/Makefile
  man/Makefile
  src/Makefile
  tests/Makefile
  tests/atlocal
])
AC_CONFIG_TESTDIR([tests])

# Installation PATH
# test "x$prefix" = "xNONE" && prefix="/opt/flom"
# test "x$exec_prefix" = "xNONE" -a "x$prefix" != "xNONE" && exec_prefix=$prefix

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
	LDFLAGS="$LDFLAGS $($PKG_CONFIG --libs glib-2.0) $($PKG_CONFIG --libs gthread-2.0)"
	AC_MSG_RESULT([yes])
fi

# Checking pkg-config is available
AC_CHECK_PROGS(GDB, [gdb], [])
AC_CHECK_PROGS(VALGRIND, [valgrind], [])
AC_CHECK_PROGS(XSLTPROC, [xsltproc], [])
AC_CHECK_PROG(HAVE_DOT, [dot], [YES], [NO])

AC_CHECK_HEADERS(assert.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(glib.h)
AC_CHECK_HEADERS(netinet/in.h)
AC_CHECK_HEADERS(poll.h pwd.h)
AC_CHECK_HEADERS(regex.h)
AC_CHECK_HEADERS(stdarg.h stdint.h stdio.h stdlib.h string.h syslog.h)
AC_CHECK_HEADERS(sys/stat.h sys/socket.h)
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/un.h sys/wait.h)
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
#AC_CHECK_TYPES([uuid_t], [], \
#	[AC_MSG_ERROR([cannot go on without uuid_t type])], \
#	[[#include <uuid/uuid.h>]])
#AC_CHECK_TYPES([in_addr_t], [], [], [[#include <netinet/in.h>]])
#AC_CHECK_TYPES([in_port_t], [], [], [[#include <netinet/in.h>]])
AC_CHECK_TYPES(pid_t)
AC_CHECK_TYPES(uint8_t)
AC_CHECK_TYPES(uint32_t)
AC_CHECK_TYPES(uintptr_t)
AC_CHECK_TYPES([struct sockaddr_un], [], [], [[#include <sys/un.h>]])

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long int])
AC_CHECK_SIZEOF([pid_t], [], [[#include <sys/types.h>]])
AC_CHECK_SIZEOF([size_t])
AC_CHECK_SIZEOF([ssize_t])
#AC_CHECK_SIZEOF([short int])
#AC_CHECK_SIZEOF([long long int])
#AC_CHECK_SIZEOF([clock_t], [], [[#include <time.h>]])
#AC_CHECK_SIZEOF([in_port_t], [], [[#include <netinet/in.h>]])
#AC_CHECK_SIZEOF([off_t])
#AC_CHECK_SIZEOF([nfds_t], [], [[#include <poll.h>]])
#AC_CHECK_SIZEOF([pthread_t], [], [[#include <pthread.h>]])
#AC_CHECK_SIZEOF([uintptr_t])
#AC_CHECK_SIZEOF([uint32_t])
#AC_CHECK_SIZEOF([uuid_t], [], [[#include <uuid/uuid.h>]])

AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strcasestr)
AC_CHECK_FUNCS(vsnprintf)

AC_SUBST(LDFLAGS_GLIB)
AC_SUBST(HAVE_DOT)

# substitution inside *.in for hardwired configuration
AC_SUBST(_RELEASE_DATE)
AC_SUBST(_SYSTEM_CONFIG_FILENAME)
AC_SUBST(_USER_CONFIG_FILENAME)
AC_SUBST(_CONFIG_GROUP_TRACE)
AC_SUBST(_CONFIG_KEY_DAEMONTRACEFILE)
AC_SUBST(_CONFIG_KEY_COMMANDTRACEFILE)
AC_SUBST(_CONFIG_GROUP_RESOURCE)
AC_SUBST(_CONFIG_KEY_NAME)
AC_SUBST(_CONFIG_KEY_WAIT)
AC_SUBST(_CONFIG_KEY_TIMEOUT)
AC_SUBST(_CONFIG_KEY_LOCK_MODE)
AC_SUBST(_CONFIG_GROUP_COMMUNICATION)
AC_SUBST(_CONFIG_KEY_SOCKET_NAME)
AC_SUBST(_ES_UNABLE_TO_EXECUTE_COMMAND)
AC_SUBST(_ES_RESOURCE_BUSY)
AC_SUBST(_ES_GENERIC_ERROR)

AC_OUTPUT
