AC_INIT([FLoM], [1.6.1-dev], [tiian at users dot sourceforge dot net], [], [https://github.com/tiian/flom])
AM_INIT_AUTOMAKE([-Wall -Werror])

AM_PROG_AR
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR(m4)

# LT Version numbers, remember to change them just *before* a release.
#
# From libtool manual:
#
# Here are a set of rules to help you update your library version information:
# 1. Start with version information of ‘0:0:0’ for each libtool library.
# 2. Update the version information only immediately before a public release 
#    of your software. More frequent updates are unnecessary, and only 
#    guarantee that the current interface number gets larger faster.
# 3. If the library source code has changed at all since the last update, then
#    increment revision (‘c :r :a ’ becomes ‘c :r + 1:a ’).
# 4. If any interfaces have been added, removed, or changed since the last
#    update, increment current, and set revision to 0.
# 5. If any interfaces have been added since the last public release, then
#    increment age.
# 6. If any interfaces have been removed since the last public release, then
#    set age to 0.
#
# libflom library version: LC_CURRENT -> c, LT_REVISION -> r, LT_AGE ->a
LT_CURRENT=2
LT_AGE=2
LT_REVISION=0

# hardwired configuration
_FLOM_PREFIX="flom"
_RELEASE_DATE="2023-05-03"
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
_CONFIG_KEY_APPENDTRACEFILE="AppendTraceFile"
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
_CONFIG_GROUP_MONITOR="Monitor"
_CONFIG_KEY_IGNORED_SIGNALS="IgnoredSignals"
_CONFIG_GROUP_NETWORK="Network"
_CONFIG_KEY_NETWORK_INTERFACE="NetworkInterface"
_CONFIG_KEY_DISCOVERY_ATTEMPTS="DiscoveryAttempts"
_CONFIG_KEY_DISCOVERY_TIMEOUT="DiscoveryTimeout"
_CONFIG_KEY_DISCOVERY_TTL="DiscoveryTTL"
_CONFIG_KEY_TCP_KEEPALIVE_TIME="TcpKeepaliveTime"
_CONFIG_KEY_TCP_KEEPALIVE_INTVL="TcpKeepaliveIntvl"
_CONFIG_KEY_TCP_KEEPALIVE_PROBES="TcpKeepaliveProbes"
_CONFIG_GROUP_TLS="TLS"
_CONFIG_KEY_TLS_CERTIFICATE="TlsCertificate"
_CONFIG_KEY_TLS_PRIVATE_KEY="TlsPrivateKey"
_CONFIG_KEY_TLS_CA_CERTIFICATE="TlsCaCertificate"
_CONFIG_KEY_TLS_CHECK_PEER_ID="TlsCheckPeerId"
_DEBUG_FEATURES_IPV6_MULTICAST_SERVER="IPv6.Multicast.Server"
_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT="IPv6.Multicast.Client"
_DEBUG_FEATURES_TLS_SERVER="TLS.Server"
_DEBUG_FEATURES_TLS_CLIENT="TLS.Client"
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
AC_DEFINE_UNQUOTED([_CONFIG_KEY_APPENDTRACEFILE], ["$_CONFIG_KEY_APPENDTRACEFILE"], [Label of "command trace file" key inside config files])
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
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_MONITOR], ["$_CONFIG_GROUP_MONITOR"], [Label of "Monitor" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_IGNORED_SIGNALS], ["$_CONFIG_KEY_IGNORED_SIGNALS"], [Label of "IgnoredSignals" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_NETWORK], ["$_CONFIG_GROUP_NETWORK"], [Label of "Network" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_NETWORK_INTERFACE], ["$_CONFIG_KEY_NETWORK_INTERFACE"], [Label of "NetworkInterface" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_ATTEMPTS], ["$_CONFIG_KEY_DISCOVERY_ATTEMPTS"], [Label of "DiscoverAttempts" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_TIMEOUT], ["$_CONFIG_KEY_DISCOVERY_TIMEOUT"], [Label of "DiscoverTimeout" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_DISCOVERY_TTL], ["$_CONFIG_KEY_DISCOVERY_TTL"], [Label of "DiscoverTTL" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_TIME], ["$_CONFIG_KEY_TCP_KEEPALIVE_TIME"], [Label of "TcpKeepaliveTime" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_INTVL], ["$_CONFIG_KEY_TCP_KEEPALIVE_INTVL"], [Label of "TcpKeepaliveIntvl" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TCP_KEEPALIVE_PROBES], ["$_CONFIG_KEY_TCP_KEEPALIVE_PROBES"], [Label of "TcpKeepaliveProbes" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_GROUP_TLS], ["$_CONFIG_GROUP_TLS"], [Label of "TLS" group inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TLS_CERTIFICATE], ["$_CONFIG_KEY_TLS_CERTIFICATE"], [Label of "TlsPeerCertificate" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TLS_PRIVATE_KEY], ["$_CONFIG_KEY_TLS_PRIVATE_KEY"], [Label of "TlsPeerPrivateKey" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TLS_CA_CERTIFICATE], ["$_CONFIG_KEY_TLS_CA_CERTIFICATE"], [Label of "TlsCaCertificate" key inside config files])
AC_DEFINE_UNQUOTED([_CONFIG_KEY_TLS_CHECK_PEER_ID], ["$_CONFIG_KEY_TLS_CHECK_PEER_ID"], [Label of "TlsSkipPeerId" key inside config files])
AC_DEFINE_UNQUOTED([_DEBUG_FEATURES_IPV6_MULTICAST_SERVER], ["$_DEBUG_FEATURES_IPV6_MULTICAST_SERVER"], [Label of "IPv6 Multicast Server" debug feature])
AC_DEFINE_UNQUOTED([_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT], ["$_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT"], [Label of "IPv6 Multicast Client" debug feature])
AC_DEFINE_UNQUOTED([_DEBUG_FEATURES_TLS_SERVER], ["$_DEBUG_FEATURES_TLS_SERVER"], [Label of "TLS Server" debug feature])
AC_DEFINE_UNQUOTED([_DEBUG_FEATURES_TLS_CLIENT], ["$_DEBUG_FEATURES_TLS_CLIENT"], [Label of "TLS Client" debug feature])
AC_DEFINE_UNQUOTED([_ES_REQUESTER_CANT_WAIT], [$_ES_REQUESTER_CANT_WAIT], [Exit status for cannot wait condition])
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
  doc/examples/java/Makefile
  doc/examples/perl/Makefile
  doc/examples/php/Makefile
  doc/examples/python/Makefile
  doc/html/Makefile
  doc/html/C/Makefile
  doc/html/C++/Makefile
  doc/html/java/Makefile
  doc/html/java/resources/Makefile
  doc/html/java/org/tiian/flom/Makefile
  etc/Makefile
  man/Makefile
  src/Makefile
  src/java/Makefile
  src/perl/Makefile
  src/php/Makefile
  src/python/Makefile
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
AC_ARG_ENABLE([cpp],
	AS_HELP_STRING([--disable-cpp],
		[do not prepare C++ API]),
	[CPPAPI="no"], [CPPAPI="yes"])
#AM_PROG_CC_C_O
# C++ stuff is conditional
# disable C++ if a C++ compiler is not installed
AC_CHECK_PROGS([HAVE_CXX], [$CXX], [no])
if test "$HAVE_CXX" = "no"
then
	CPPAPI="no"
fi
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
GLIB2_PERL=""
GTHREAD2_PERL=""
GLIB2_PYTHON=""
GTHREAD2_PYTHON=""
pkg-config --atleast-version=2.32 glib-2.0
if test $? -eq 0
then
	PKG_CHECK_MODULES([GLIB2], [glib-2.0 >= 2.32])
	GLIB2_PERL="glib-2.0"
	GTHREAD2_PERL="glib-2.0"
	GLIB2_PYTHON="glib-2.0"
	GTHREAD2_PYTHON="glib-2.0"
else
	PKG_CHECK_MODULES([GLIB2], [gthread-2.0 >= 2.22])
	GLIB2_PERL="glib-2.0"
	GTHREAD2_PERL="gthread-2.0"
	GLIB2_PYTHON="glib-2.0"
	GTHREAD2_PYTHON="gthread-2.0"
fi
CPPFLAGS="$CPPFLAGS $GLIB2_CFLAGS"
#GLIB2_LIBS automatically set by PKG_CHECK_MODULES macro

# DBUS check
DBUS1_PERL=""
DBUS1_PYTHON=""
pkg-config --atleast-version=1.2.16 dbus-1
if test $? -eq 0
then
	PKG_CHECK_MODULES([DBUS1], [dbus-1 >= 1.2.16])
	DBUS1_PERL="dbus-1"
	DBUS1_PYTHON="dbus-1"
	CPPFLAGS="$CPPFLAGS $DBUS1_CFLAGS"
else
        AC_MSG_ERROR([cannot find dbus package at level 1.2.16 or higher])
fi

# OpenSSL check
OPENSSL_PERL=""
CRYPTO_PERL=""
OPENSSL_PYTHON=""
CRYPTO_PYTHON=""
pkg-config --atleast-version=0.9.8k openssl
if test $? -eq 0
then
	PKG_CHECK_MODULES([OPENSSL], [openssl >= 0.9.8k])
	OPENSSL_PERL="ssl"
	CRYPTO_PERL="crypto"
	OPENSSL_PYTHON="ssl"
	CRYPTO_PYTHON="crypto"
	CPPFLAGS="$CPPFLAGS $OPENSSL_CFLAGS"
	TLS_INIT_FUNCTION="SSL_library_init"
else
	AC_MSG_ERROR([cannot find openssl package at level 0.9.8k or higher])
fi
# Newer versions of OpenSSL (1.1.x)
pkg-config --atleast-version=1.1.0g openssl
if test $? -eq 0
then
	TLS_INIT_FUNCTION="OPENSSL_init_ssl"
fi

# Checking stdbuf is available
AC_CHECK_PROGS(STDBUF, [stdbuf], [])
if test "xxx$STDBUF" = "xxxstdbuf"
then
	STDBUF_O0="$STDBUF -o0"
fi
AC_MSG_NOTICE([STDBUF_O0=$STDBUF_O0])
# Checking the availability of some utility programs
AC_CHECK_PROGS(DOXYGEN, [doxygen], [true])
AC_CHECK_PROG(HAVE_DOT, [dot], [YES], [NO])

# @@@ old style, remove me
# Checking the availability of SWIG (Simplified Wrapper and Interface Generator)
# AC_CHECK_PROGS(SWIG, [swig], [noswig])

# Check SWIG availability
m4_include(m4/ax_pkg_swig.m4)
AX_PKG_SWIG([1.3.40])

# Checking the availability of JRE
AC_CHECK_PROG(HAVE_JAVA, [java], [yes], [no])
# Checking the availability of JDK
AC_CHECK_PROG(HAVE_JAVAC, [javac], [yes], [no])
AC_CHECK_PROG(HAVE_JAVAH, [javah], [yes], [no])
AM_CONDITIONAL([COND_HAVE_JAVAH], [test "$HAVE_JAVAH" = "yes"])
if test "$HAVE_JAVAH" = "no"
then
	AC_MSG_NOTICE([javah not found, assuming Java 9 or higher])
fi
AC_CHECK_PROG(HAVE_JAR, [jar], [yes], [no])
AC_CHECK_PROG(HAVE_JAVADOC, [javadoc], [yes], [no])
# checking if javadoc support HTML5
if test "$HAVE_JAVADOC" = "yes"
then
	AC_MSG_CHECKING([if javadoc supports HTML5])
	javadoc -help | grep html5 >/dev/null
	JAVADOC_HTML5=$?
	if test $JAVADOC_HTML5 -eq 0
	then
        	AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
	AM_CONDITIONAL([COND_JAVADOC_HTML5], [test $JAVADOC_HTML5 -eq 0])
else
	AM_CONDITIONAL([COND_JAVADOC_HTML5], [false])
fi
# Build Java wrapper?
AC_ARG_ENABLE([java],
	AS_HELP_STRING([--disable-java],
		[do not prepare Java binding API]),
	[JAVA="no"], [JAVA="yes"])
# Java stuff is conditional
if test "$JAVA" = "yes" -a "$HAVE_JAVA" = "yes" -a "$HAVE_JAVAC" = "yes" -a "$HAVE_JAR" = "yes" -a "$HAVE_JAVADOC" = "yes"
then
	# JNI include directory resolution
	AX_JNI_INCLUDE_DIR
	for JNI_INCLUDE_DIR in $JNI_INCLUDE_DIRS
	do
		CPPFLAGS="$CPPFLAGS -I$JNI_INCLUDE_DIR"
	done
	AC_CHECK_HEADERS(jni.h jni_md.h)
	# OK, all Java stuff is OK!
	JAVAAPI="yes"
else
	JAVAAPI="no"
fi
AM_CONDITIONAL([COND_JAVAAPI], [test "$JAVAAPI" = "yes"])

# Checking the availability of Perl5 tools
AC_CHECK_PROG(HAVE_PERL, [perl], [yes], [no])
# Build Perl wrapper?
AC_ARG_ENABLE([perl],
	AS_HELP_STRING([--disable-perl],
		[do not prepare Perl binding API]),
	[PERL="no"], [PERL="yes"])
# Perl stuff is conditional
if test "$HAVE_PERL" = "yes" -a "$PERL" = "yes" -a "$SWIG" != "noswig"
then
	PERLAPI="yes"
else
	PERLAPI="no"
fi
AM_CONDITIONAL([COND_PERLAPI], [test "$PERLAPI" = "yes"])

# Checking the availability of PHP tools
AC_CHECK_PROG(HAVE_PHP, [php], [yes], [no])
AC_CHECK_PROG(HAVE_PHP_CONFIG, [php-config], [yes], [no])
if test "$HAVE_PHP_CONFIG" = "yes"
then
	AC_MSG_CHECKING([for PHP7])
	PHP_VERNUM=$(php-config --vernum)
	if test $PHP_VERNUM -ge 70000
	then
        	AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
fi
AM_CONDITIONAL([COND_PHP7], [test $PHP_VERNUM -ge 70000])
# Build PHP wrapper?
AC_ARG_ENABLE([php],
	AS_HELP_STRING([--disable-php],
		[do not prepare PHP binding API]),
	[PHP="no"], [PHP="yes"])
# PHP stuff is conditional
if test "$HAVE_PHP" = "yes" -a "$HAVE_PHP_CONFIG" = "yes" -a "$PHP" = "yes" \
	-a "$SWIG" != "noswig"
then
	PHPAPI="yes"
	PHP_INCLUDES=$(php-config --includes)
	PHP_INSTALL_PATH=$(php-config --extension-dir)
else
	PHPAPI="no"
fi
AM_CONDITIONAL([COND_PHPAPI], [test "$PHPAPI" = "yes"])

# @@@ old style, remove me
# Checking the availability of Python tools
# AC_CHECK_PROG(HAVE_PYTHON, [python], [yes], [no])
#AC_CHECK_PROG(HAVE_PYTHON_CONFIG, [python-config], [yes], [no])

# Check Python availability
m4_include(m4/ax_python_devel.m4)
AX_PYTHON_DEVEL([>= '2.6.5'])
# Check SWIG support for Python
m4_include(m4/ax_swig_python.m4)
AX_SWIG_PYTHON

# Build Python wrapper?
AC_ARG_ENABLE([python],
	AS_HELP_STRING([--disable-python],
		[do not prepare Python binding API]),
	[BUILD_PYTHON="no"], [BUILD_PYTHON="yes"])
# Python stuff is conditional
if test "z$PYTHON" != "z" -a "$BUILD_PYTHON" = "yes" -a "z$SWIG" != "z"
then
	PYTHONAPI="yes"
	PYTHON_INCLUDES=$PYTHON_CPPFLAGS
else
	PYTHONAPI="no"
fi
AM_CONDITIONAL([COND_PYTHONAPI], [test "$PYTHONAPI" = "yes"])


AC_CHECK_PROG(HAVE_DBUS_UUIDGEN, [dbus-uuidgen], [yes], [no])
if test "$HAVE_DBUS_UUIDGEN" = "no"
then
	AC_MSG_ERROR([cannot find dbus-uuidgen utility])
fi

# networktest
AC_ARG_ENABLE([testcases],
	AS_HELP_STRING([--disable-testcases],
		[do not execute test cases that require network]),
	[TESTCASES="no"], [TESTCASES="yes"])
if test "z$TESTCASES" != "z" -a "$TESTCASES" = "yes"
then
	# find a valid network interface
	m4_include(m4/ax_net_iface.m4)
	AX_NET_IFACE
fi

AC_DEFINE_UNQUOTED([_ES_GENERIC_ERROR], [$_ES_GENERIC_ERROR], [Exit status for generic error])

AC_CHECK_HEADERS(arpa/inet.h assert.h)
AC_CHECK_HEADERS(dbus/dbus.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(glib.h)
AC_CHECK_HEADERS(ifaddrs.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(netdb.h)
AC_CHECK_HEADERS(netinet/in.h)
AC_CHECK_HEADERS(netinet/tcp.h)
AC_CHECK_HEADERS(openssl/ssl.h)
AC_CHECK_HEADERS(openssl/err.h)
AC_CHECK_HEADERS(openssl/x509v3.h)
AC_CHECK_HEADERS(poll.h pwd.h)
AC_CHECK_HEADERS(regex.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(stdarg.h)
AC_CHECK_HEADERS(stdint.h)
AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(syslog.h)
AC_CHECK_HEADERS(sys/file.h)
AC_CHECK_HEADERS(sys/stat.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(sys/un.h)
AC_CHECK_HEADERS(sys/wait.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(unistd.h)

AC_CHECK_TYPES(pid_t)
AC_CHECK_TYPES(uint8_t)
AC_CHECK_TYPES(uint32_t)
AC_CHECK_TYPES(uint64_t)
AC_CHECK_TYPES(uintptr_t)
AC_CHECK_TYPES([struct sockaddr_un], [], [], [[#include <sys/un.h>]])

AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long int])
AC_CHECK_SIZEOF([pid_t], [], [[#include <sys/types.h>]])
AC_CHECK_SIZEOF([size_t])
AC_CHECK_SIZEOF([socklen_t])
AC_CHECK_SIZEOF([ssize_t])

AC_CHECK_LIB([dbus-1], [dbus_get_local_machine_id])
AC_CHECK_LIB([ssl], [$TLS_INIT_FUNCTION])

AC_CHECK_FUNCS(freeifaddrs)
AC_CHECK_FUNCS(getifaddrs)
AC_CHECK_FUNCS(signal)
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strcasestr)
AC_CHECK_FUNCS(vsnprintf)
AC_CHECK_FUNCS(TLSv1_method)
AC_CHECK_FUNCS(TLSv1_1_method)
AC_CHECK_FUNCS(TLSv1_2_method)
AC_CHECK_FUNCS(TLS_method)

AC_SUBST(GLIB2_LIBS)
AC_SUBST(GTHREAD2_LIBS)
AC_SUBST(DBUS1_LIBS)
AC_SUBST(HAVE_DOT)
AC_SUBST(STDBUF_O0)

# substitution of the current logged user
AC_SUBST(USER)

# substitution of optional (--disable) features 
AC_SUBST(CPPAPI)
AC_SUBST(JAVAAPI)
AC_SUBST(PERLAPI)
AC_SUBST(PHPAPI)
AC_SUBST(PHP_INCLUDES)
AC_SUBST(PHP_INSTALL_PATH)
AC_SUBST(PYTHON)
AC_SUBST(PYTHONAPI)
AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(GLIB2_PERL)
AC_SUBST(GTHREAD2_PERL)
AC_SUBST(DBUS1_PERL)
AC_SUBST(OPENSSL_PERL)
AC_SUBST(CRYPTO_PERL)
AC_SUBST(GLIB2_PYTHON)
AC_SUBST(GTHREAD2_PYTHON)
AC_SUBST(DBUS1_PYTHON)
AC_SUBST(OPENSSL_PYTHON)
AC_SUBST(CRYPTO_PYTHON)
AC_SUBST(TESTCASES)

# substitution of LibTool values for library version
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_AGE)
AC_SUBST(LT_REVISION)

# substitution inside *.in for hardwired configuration
AC_SUBST(_FLOM_PREFIX)
AC_SUBST(_RELEASE_DATE)
AC_SUBST(_SYSTEM_CONFIG_FILENAME)
AC_SUBST(_USER_CONFIG_FILENAME)
AC_SUBST(_CONFIG_GROUP_TRACE)
AC_SUBST(_CONFIG_KEY_DAEMONTRACEFILE)
AC_SUBST(_CONFIG_KEY_COMMANDTRACEFILE)
AC_SUBST(_CONFIG_KEY_APPENDTRACEFILE)
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
AC_SUBST(_CONFIG_GROUP_MONITOR)
AC_SUBST(_CONFIG_KEY_IGNORED_SIGNALS)
AC_SUBST(_CONFIG_GROUP_NETWORK)
AC_SUBST(_CONFIG_KEY_NETWORK_INTERFACE)
AC_SUBST(_CONFIG_KEY_DISCOVERY_ATTEMPTS)
AC_SUBST(_CONFIG_KEY_DISCOVERY_TIMEOUT)
AC_SUBST(_CONFIG_KEY_DISCOVERY_TTL)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_TIME)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_INTVL)
AC_SUBST(_CONFIG_KEY_TCP_KEEPALIVE_PROBES)
AC_SUBST(_CONFIG_GROUP_TLS)
AC_SUBST(_CONFIG_KEY_TLS_CERTIFICATE)
AC_SUBST(_CONFIG_KEY_TLS_PRIVATE_KEY)
AC_SUBST(_CONFIG_KEY_TLS_CA_CERTIFICATE)
AC_SUBST(_CONFIG_KEY_TLS_CHECK_PEER_ID)
AC_SUBST(_DEBUG_FEATURES_IPV6_MULTICAST_SERVER)
AC_SUBST(_DEBUG_FEATURES_IPV6_MULTICAST_CLIENT)
AC_SUBST(_DEBUG_FEATURES_TLS_SERVER)
AC_SUBST(_DEBUG_FEATURES_TLS_CLIENT)
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
