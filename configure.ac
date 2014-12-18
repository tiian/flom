AC_INIT([FLoM], [0.9.1-dev], [tiian at users dot sourceforge dot net], [], [http://sourceforge.net/projects/flom/])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR(m4)

# LT Version numbers, remember to change them just *before* a release.
#   (Interfaces removed:    CURRENT++, AGE=0, REVISION=0)
#   (Interfaces added:      CURRENT++, AGE++, REVISION=0)
#   (No interfaces changed:                   REVISION++)
#
# libflom library version
LT_CURRENT=0
LT_AGE=0
LT_REVISION=0

# hardwired configuration
_RELEASE_DATE="2014-12-04"
_SYSTEM_CONFIG_FILENAME="flom.conf"
_USER_CONFIG_FILENAME=".flom"
_DIR_FILE_SEPARATOR="/"
_RESOURCE_SET_SEPARATOR="."
_DEFAULT_DAEMON_LIFESPAN=5000
_DEFAULT_DAEMON_PORT=28015
_DEFAULT_DISCOVERY_ATTEMPTS=2
_DEFAULT_DISCOVERY_TIMEOUT=500
_DEFAULT_DISCOVERY_TTL=1
_DEFAULT_TCP_KEEPALIVE_TIME=60
_DEFAULT_TCP_KEEPALIVE_INTVL=10
_DEFAULT_TCP_KEEPALIVE_PROBES=6
_CONFIG_GROUP_TRACE="Trace"
_CONFIG_KEY_DAEMONTRACEFILE="DaemonTraceFile"
_CONFIG_KEY_COMMANDTRACEFILE="CommandTraceFile"
_CONFIG_KEY_VERBOSE="Verbose"
_CONFIG_GROUP_RESOURCE="Resource"
_CONFIG_KEY_CREATE="Create"
_CONFIG_KEY_NAME="Name"
_CONFIG_KEY_WAIT="Wait"
_CONFIG_KEY_TIMEOUT="Timeout"
_CONFIG_KEY_QUANTITY="Quantity"
_CONFIG_KEY_LOCK_MODE="LockMode"
_CONFIG_KEY_IDLE_LIFESPAN="IdleLifespan"
_CONFIG_GROUP_DAEMON="Daemon"
_CONFIG_KEY_SOCKET_NAME="SocketName"
_CONFIG_KEY_LIFESPAN="Lifespan"
_CONFIG_KEY_UNICAST_ADDRESS="UnicastAddress"
_CONFIG_KEY_UNICAST_PORT="UnicastPort"
_CONFIG_KEY_MULTICAST_ADDRESS="MulticastAddress"
_CONFIG_KEY_MULTICAST_PORT="MulticastPort"
_CONFIG_GROUP_NETWORK="Network"
_CONFIG_KEY_DISCOVERY_ATTEMPTS="DiscoveryAttempts"
_CONFIG_KEY_DISCOVERY_TIMEOUT="DiscoveryTimeout"
_CONFIG_KEY_DISCOVERY_TTL="DiscoveryTTL"
_CONFIG_KEY_TCP_KEEPALIVE_TIME="TcpKeepaliveTime"
_CONFIG_KEY_TCP_KEEPALIVE_INTVL="TcpKeepaliveIntvl"
_CONFIG_KEY_TCP_KEEPALIVE_PROBES="TcpKeepaliveProbes"
_ES_REQUESTER_CANT_WAIT=96
_ES_UNABLE_TO_EXECUTE_COMMAND=97
_ES_RESOURCE_BUSY=98
_ES_GENERIC_ERROR=99
# definition inside config.h for hardwired configuration
AC_DEFINE_UNQUOTED([_RELEASE_DATE], ["$_RELEASE_DATE"], [Date of release])
AC_DEFINE_UNQUOTED([_SYSTEM_CONFIG_FILENAME], ["$_SYSTEM_CONFIG_FILENAME"], [System wide configuration filename])
AC_DEFINE_UNQUOTED([_USER_CONFIG_FILENAME], ["$_USER_CONFIG_FILENAME"], [User default configuration filename])
AC_DEFINE_UNQUOTED([_DIR_FILE_SEPARATOR], ["$_DIR_FILE_SEPARATOR"], [String used to separate directory from file inside fully qualified path names])
AC_DEFINE_UNQUOTED([_RESOURCE_SET_SEPARATOR], ["$_RESOURCE_SET_SEPARATOR"], [String used to separate elements inside resource set names])
AC_DEFINE_UNQUOTED([_DEFAULT_DAEMON_LIFESPAN], [$_DEFAULT_DAEMON_LIFESPAN], [Default value for option daemon lifespan (milliseconds)])
AC_DEFINE_UNQUOTED([_DEFAULT_DAEMON_PORT], [$_DEFAULT_DAEMON_PORT], [Default port used to reach flom daemon using TCP and UDP (multicast)])
AC_DEFINE_UNQUOTED([_DEFAULT_DISCOVERY_ATTEMPTS], [$_DEFAULT_DISCOVERY_ATTEMPTS], [Default number of attempts used to for auto discovery feature])
AC_DEFINE_UNQUOTED([_DEFAULT_DISCOVERY_TIMEOUT], [$_DEFAULT_DISCOVERY_TIMEOUT], [Default discovery timeout for UDP/IP (multicast) query (milliseconds)])
AC_DEFINE_UNQUOTED([_DEFAULT_DISCOVERY_TTL], [$_DEFAULT_DISCOVERY_TTL], [Default TTL: hop limit for multicast datagrams (UDP/IP) - auto-discovery query])
AC_DEFINE_UNQUOTED([_DEFAULT_TCP_KEEPALIVE_TIME], [$_DEFAULT_TCP_KEEPALIVE_TIME], [Per socket override of tcp_keepalive_time parameter associated to SO_KEEPALIVE])
AC_DEFINE_UNQUOTED([_DEFAULT_TCP_KEEPALIVE_INTVL], [$_DEFAULT_TCP_KEEPALIVE_INTVL], [Per socket override of tcp_keepalive_intvl parameter associated to SO_KEEPALIVE])
AC_DEFINE_UNQUOTED([_DEFAULT_TCP_KEEPALIVE_PROBES], [$_DEFAULT_TCP_KEEPALIVE_PROBES], [Per socket override of tcp_keepalive_probes parameter associated to SO_KEEPALIVE])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_TRACE], ["$_CONFIG_GROUP_TRACE"], [Label of "trace" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DAEMONTRACEFILE], ["$_CONFIG_KEY_DAEMONTRACEFILE"], [Label of "daemon trace file" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_COMMANDTRACEFILE], ["$_CONFIG_KEY_COMMANDTRACEFILE"], [Label of "command trace file" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_VERBOSE], ["$_CONFIG_KEY_VERBOSE"], [Label of "Verbose" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_RESOURCE], ["$_CONFIG_GROUP_RESOURCE"], [Label of "resource" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_CREATE], ["$_CONFIG_KEY_CREATE"], [Label of "Create" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_NAME], ["$_CONFIG_KEY_NAME"], [Label of "Name" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_WAIT], ["$_CONFIG_KEY_WAIT"], [Label of "Wait" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TIMEOUT], ["$_CONFIG_KEY_TIMEOUT"], [Label of "Timeout" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_QUANTITY], ["$_CONFIG_KEY_QUANTITY"], [Label of "Quantity" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_LOCK_MODE], ["$_CONFIG_KEY_LOCK_MODE"], [Label of "LockMode" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_IDLE_LIFESPAN], ["$_CONFIG_KEY_IDLE_LIFESPAN"], [Label of "IdleLifespan" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_DAEMON], ["$_CONFIG_GROUP_DAEMON"], [Label of "Daemon" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_SOCKET_NAME], ["$_CONFIG_KEY_SOCKET_NAME"], [Label of "SocketName" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_LIFESPAN], ["$_CONFIG_KEY_LIFESPAN"], [Label of "Lifespan" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_UNICAST_ADDRESS], ["$_CONFIG_KEY_UNICAST_ADDRESS"], [Label of "UnicastAddress" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_UNICAST_PORT], ["$_CONFIG_KEY_UNICAST_PORT"], [Label of "UnicastPort" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_MULTICAST_ADDRESS], ["$_CONFIG_KEY_MULTICAST_ADDRESS"], [Label of "MulticastAddress" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_MULTICAST_PORT], ["$_CONFIG_KEY_MULTICAST_PORT"], [Label of "MulticastPort" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_NETWORK], ["$_CONFIG_GROUP_NETWORK"], [Label of "Network" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_ATTEMPTS], ["$_CONFIG_KEY_DISCOVERY_ATTEMPTS"], [Label of "DiscoverAttempts" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_TIMEOUT], ["$_CONFIG_KEY_DISCOVERY_TIMEOUT"], [Label of "DiscoverTimeout" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_TTL], ["$_CONFIG_KEY_DISCOVERY_TTL"], [Label of "DiscoverTTL" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_TIME], ["$_CONFIG_KEY_TCP_KEEPALIVE_TIME"], [Label of "TcpKeepaliveTime" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_INTVL], ["$_CONFIG_KEY_TCP_KEEPALIVE_INTVL"], [Label of "TcpKeepaliveIntvl" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_PROBES], ["$_CONFIG_KEY_TCP_KEEPALIVE_PROBES"], [Label of "TcpKeepaliveProbes" key inside config files])
AC_DEFINE_UNQUOTED([_ES_REQUESTER_CANT_WAIT], [$_ES_REQUESTER_CANT_WAIT], [Exit status for can't wait condition])
AC_DEFINE_UNQUOTED([_ES_UNABLE_TO_EXECUTE_COMMAND], [$_ES_UNABLE_TO_EXECUTE_COMMAND], [Exit status for command execution error])
AC_DEFINE_UNQUOTED([_ES_RESOURCE_BUSY], [$_ES_RESOURCE_BUSY], [Exit status for busy resource condition])
AC_DEFINE_UNQUOTED([_ES_GENERIC_ERROR], [$_ES_GENERIC_ERROR], [Exit status for generic error])

AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  doc/examples/Makefile
  doc/html/Makefile
  etc/Makefile
  man/Makefile
  src/Makefile
  tests/atlocal
  tests/Makefile
  tests/src/Makefile
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
AC_ARG_ENABLE([cppapi],
	AS_HELP_STRING([--disable-cppapi],
		[do not prepare C++ API]),
	[CPPAPI="no"], [CPPAPI="yes"])
#AM_PROG_CC_C_O
# C++ stuff is conditional
AM_CONDITIONAL([COND_CPPAPI], [test "$CPPAPI" = "yes"])

# Checking pkg-config is available
AC_CHECK_PROGS(PKG_CONFIG, [pkg-config], [])
if test -z $PKG_CONFIG 
then
        AC_MSG_ERROR([cannot find pkg-config program])
fi
# checking glib-2.0 and gthread-2.0 availability and compatibility using  
# pkg-config
# Starting with glib 2.32, gthread must no more been specified as documented
# here: https://developer.gnome.org/glib/2.32/glib-compiling.html
# Until glib 2.30, gthread must be specified as documented here:
# https://developer.gnome.org/glib/2.30/glib-compiling.html
pkg-config --atleast-version=2.32 glib-2.0
if test $? -eq 0
then
	PKG_CHECK_MODULES([GLIB2], [glib-2.0 >= 2.32])
else
#	PKG_CHECK_MODULES([GLIB2], [glib-2.0 >= 2.22 gthread-2.0 >= 2.22])
	PKG_CHECK_MODULES([GLIB2], [gthread-2.0 >= 2.22])
fi
CPPFLAGS="$CPPFLAGS $GLIB2_CFLAGS"
#GLIB2_LIBS automatically set by PKG_CHECK_MODULES macro

# Checking the availability of some utility programs
AC_CHECK_PROGS(DOXYGEN, [doxygen], [true])
AC_CHECK_PROG(HAVE_DOT, [dot], [YES], [NO])

AC_CHECK_HEADERS(assert.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(glib.h)
AC_CHECK_HEADERS(netdb.h netinet/in.h netinet/tcp.h)
AC_CHECK_HEADERS(poll.h pwd.h)
AC_CHECK_HEADERS(regex.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(stdarg.h stdint.h stdio.h stdlib.h string.h syslog.h)
AC_CHECK_HEADERS(sys/stat.h sys/socket.h)
AC_CHECK_HEADERS(sys/time.h sys/types.h sys/un.h sys/wait.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(unistd.h)
#AC_CHECK_HEADERS(unistd.h)
#AC_CHECK_HEADERS(arpa/inet.h assert.h)
#AC_CHECK_HEADERS(glib.h)
#AC_CHECK_HEADERS(libgen.h)
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

AC_CHECK_FUNCS(signal)
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strcasestr)
AC_CHECK_FUNCS(vsnprintf)

AC_SUBST(GLIB2_LIBS)
AC_SUBST(GTHREAD2_LIBS)
AC_SUBST(HAVE_DOT)

# substitution of optional (--disable) features 
AC_SUBST(CPPAPI)

# substitution of LibTool values for library version
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_AGE)
AC_SUBST(LT_REVISION)

# substitution inside *.in for hardwired configuration
AC_SUBST(_RELEASE_DATE)
AC_SUBST(_SYSTEM_CONFIG_FILENAME)
AC_SUBST(_USER_CONFIG_FILENAME)
AC_SUBST(_CONFIG_GROUP_TRACE)
AC_SUBST(_CONFIG_KEY_DAEMONTRACEFILE)
AC_SUBST(_CONFIG_KEY_COMMANDTRACEFILE)
AC_SUBST(_CONFIG_KEY_VERBOSE)
AC_SUBST(_CONFIG_GROUP_RESOURCE)
AC_SUBST(_CONFIG_KEY_CREATE)
AC_SUBST(_CONFIG_KEY_NAME)
AC_SUBST(_CONFIG_KEY_WAIT)
AC_SUBST(_CONFIG_KEY_TIMEOUT)
AC_SUBST(_CONFIG_KEY_QUANTITY)
AC_SUBST(_CONFIG_KEY_LOCK_MODE)
AC_SUBST(_CONFIG_KEY_IDLE_LIFESPAN)
AC_SUBST(_CONFIG_GROUP_DAEMON)
AC_SUBST(_CONFIG_KEY_SOCKET_NAME)
AC_SUBST(_CONFIG_KEY_LIFESPAN)
AC_SUBST(_CONFIG_KEY_UNICAST_ADDRESS)
AC_SUBST(_CONFIG_KEY_UNICAST_PORT)
AC_SUBST(_CONFIG_KEY_MULTICAST_ADDRESS)
AC_SUBST(_CONFIG_KEY_MULTICAST_PORT)
AC_SUBST(_CONFIG_GROUP_NETWORK)
AC_SUBST(_CONFIG_KEY_DISCOVERY_ATTEMPTS)
AC_SUBST(_CONFIG_KEY_DISCOVERY_TIMEOUT)
AC_SUBST(_CONFIG_KEY_DISCOVERY_TTL)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_TIME)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_INTVL)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_PROBES)
AC_SUBST(_DEFAULT_DAEMON_LIFESPAN)
AC_SUBST(_DEFAULT_DAEMON_PORT)
AC_SUBST(_DEFAULT_DISCOVERY_ATTEMPTS)
AC_SUBST(_DEFAULT_DISCOVERY_TIMEOUT)
AC_SUBST(_DEFAULT_DISCOVERY_TTL)
AC_SUBST(_DEFAULT_TCP_KEEPALIVE_TIME)
AC_SUBST(_DEFAULT_TCP_KEEPALIVE_INTVL)
AC_SUBST(_DEFAULT_TCP_KEEPALIVE_PROBES)
AC_SUBST(_ES_REQUESTER_CANT_WAIT)
AC_SUBST(_ES_UNABLE_TO_EXECUTE_COMMAND)
AC_SUBST(_ES_RESOURCE_BUSY)
AC_SUBST(_ES_GENERIC_ERROR)
AC_SUBST(_RESOURCE_SET_SEPARATOR)
AC_SUBST(_DIR_FILE_SEPARATOR)
AC_OUTPUT
