/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_CONFIG_H
# define FLOM_CONFIG_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif



#include "flom_msg.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_CONFIG


/**
 * Number of chars accepted for a filename bound to AF_LOCAL socket
 */
#define LOCAL_SOCKET_SIZE sizeof(((struct sockaddr_un *)NULL)->sun_path)
/**
 * Used as backlog parameter for "listen" function call
 */
#define LISTEN_BACKLOG           100



/* configure dependent constant values */
/**
 * E-mail address as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_BUGREPORT;
/**
 * Name of the package as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_NAME;
/**
 * Version of the package as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_VERSION;
/**
 * Date of package release as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_DATE;
/**
 * Installation configuration dir (./configure output)
 */
extern const char FLOM_INSTALL_SYSCONFDIR[];
/**
 * Empty string for output messages
 */
extern const char *FLOM_EMPTY_STRING;
/**
 * NULL string for output messages
 */
extern const char *FLOM_NULL_STRING;


/**
 * Default name for a simple resource
 */
extern const gchar DEFAULT_RESOURCE_NAME[];
/**
 * Filename of system wide configuration file
 */
extern const gchar FLOM_SYSTEM_CONFIG_FILENAME[];
/**
 * Filename of user default configuration file
 */
extern const gchar FLOM_USER_CONFIG_FILENAME[];
/**
 * Separator used between directory and file names
 */
extern const gchar FLOM_DIR_FILE_SEPARATOR[];
/**
 * Separator used among elements inside a resource set name
 */
extern const gchar FLOM_RESOURCE_SET_SEPARATOR[];
/**
 * Separator used among elements inside a hierarchical resource name
 */
extern const gchar FLOM_HIER_RESOURCE_SEPARATOR[];



/**
 * Label associated to Trace group inside config files
 */
extern const gchar *FLOM_CONFIG_GROUP_TRACE;
/**
 * Label associated to DaemonTraceFile key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_DAEMONTRACEFILE;
/**
 * Label associated to CommandTraceFile key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_COMMANDTRACEFILE;
/**
 * Label associated to Verbose key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_VERBOSE;
/**
 * Label associated to "Resource" group inside config files
 */
extern const gchar *FLOM_CONFIG_GROUP_RESOURCE;
/**
 * Label associated to "Create" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_CREATE;
/**
 * Label associated to "Name" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_NAME;
/**
 * Label associated to "Wait" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_WAIT;
/**
 * Label associated to "Timeout" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TIMEOUT;
/**
 * Label associated to "Quantity" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_QUANTITY;
/**
 * Label associated to "LockMode" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_LOCK_MODE;
/**
 * Label associated to "Daemon" group inside config files
 */
extern const gchar *FLOM_CONFIG_GROUP_DAEMON;
/**
 * Label associated to "SocketName" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_SOCKET_NAME;
/**
 * Label associated to "Lifespan" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_LIFESPAN;
/**
 * Label associated to "UnicastAddress" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_UNICAST_ADDRESS;
/**
 * Label associated to "UnicastPort" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_UNICAST_PORT;
/**
 * Label associated to "MulticastAddress" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_MULTICAST_ADDRESS;
/**
 * Label associated to "MulticastPort" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_MULTICAST_PORT;
/**
 * Label associated to "Network" group inside config files
 */
extern const gchar *FLOM_CONFIG_GROUP_NETWORK;
/**
 * Label associated to "DiscoveryAttempts" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS;
/**
 * Label associated to "DiscoveryTimeout" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT;
/**
 * Label associated to "DiscoveryTTL" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_DISCOVERY_TTL;
/**
 * Label associated to "TcpKeepaliveTime" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME;
/**
 * Label associated to "TcpKeepaliveIntvl" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL;
/**
 * Label associated to "TcpKeepaliveProbes" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES;



/**
 * This type is useful for retrieving boolean values from configuration
 * and command options
 */
typedef enum flom_bool_value_e {
    /**
     * FALSE value
     */
    FLOM_BOOL_NO = FALSE,
    /**
     * TRUE value
     */
    FLOM_BOOL_YES = TRUE,
    /**
     * Invalid value
     */
    FLOM_BOOL_INVALID
} flom_bool_value_t;



/**
 * This struct contains all the values necessary for configuration
 */
typedef struct flom_config {
    /**
     * Path of UNIX socket / IP name or address used for connection
     */
    gchar             *socket_name;
    /**
     * Name of the file must be used to write trace messages from the daemon
     */
    gchar             *daemon_trace_file;
    /**
     * Name of the file must be used to write trace messages from the command
     */
    gchar             *command_trace_file;
    /**
     * Verbose execution: more messages will be printed on console
     */
    gint               verbose;
    /**
     * Daemon lifespan (milliseconds):
     * < 0 infinite, = 0 don't activate a daemon, > 0 after lifespan idle time
     * the activated daemon will terminate
     */
    gint               daemon_lifespan;
    /**
     * Name of the resource that must be locked
     */
    gchar             *resource_name;
    /**
     * The requester enqueues if the lock can not be obtained
     * (boolean value)
     */
    int                resource_wait;
    /**
     * The resource can be create if it was not previously created by another
     * requester
     * (boolean value)
     */
    int                resource_create;
    /**
     * The requester stay blocked for a maximum time if the resource and then
     * it will return (milliseconds as specified by poll POSIX function)
     */
    gint               resource_timeout;
    /**
     * Number of numeric resource to lock
     */
    gint               resource_quantity;
    /**
     * Lock mode as designed by VMS DLM
     */
    flom_lock_mode_t   lock_mode;
    /**
     * Daemon TCP/IP address
     */
    gchar             *unicast_address;
    /**
     * Daemon TCP/IP port
     */
    gint               unicast_port;
    /**
     * Daemon UDP/IP multicast address
     */
    gchar             *multicast_address;
    /**
     * Daemon UDP/IP multicast port
     */
    gint               multicast_port;
    /**
     * Discovery attempts: how many time a request will be issued
     */
    gint               discovery_attempts;
    /**
     * Discovery timeout for UDP/IP request (milliseconds)
     */
    gint               discovery_timeout;
    /**
     * Discovery TTL: hop limit for outgoing multicast datagrams
     */
    gint               discovery_ttl;
    /**
     * per socket value of parameter tcp_keepalive_time
     */
    gint               tcp_keepalive_time;
    /**
     * per socket value of parameter tcp_keepalive_intvl
     */
    gint               tcp_keepalive_intvl;
    /**
     * per socket value of parameter tcp_keepalive_probes
     */
    gint               tcp_keepalive_probes;
} flom_config_t;



/**
 * This is a global static object shared by all the application
 */
extern flom_config_t global_config;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Interpret a string and extract the boolean value
     * @param text IN string to interpret
     * @return a boolean value
     */
    flom_bool_value_t flom_bool_value_retrieve(const gchar *text);
    


    /**
     * Reset config
     */
    void flom_config_reset();
    


    /**
     * Check configuration after config file values and command line option
     * overrides
     * @return a reason code
     */
    int flom_config_check();


    
    /**
     * Print config using "g_print"
     */
    void flom_config_print();



    /**
     * Release all memory allocated by global config object
     */
    void flom_config_free();


    
    /**
     * Initialize configuration (global) object retrieving data from
     * configuration files
     * @param custom_config_filename IN filename of user configuration file
     * @return a reason code
     */
    int flom_config_init(const char *custom_config_filename);



    /**
     * Load a configuration file, parse it and initialize global configuration
     * as described in the config file
     * @param config_file_name IN configuration file to open and parse
     * @return a reason code
     */
    int flom_config_init_load(const char *config_file_name);


    
    /**
     * Set daemon_socket_name
     * @param socket_name IN set the new value for socket_name properties
     * @return a reason code
     */
    int flom_config_set_socket_name(gchar *socket_name);



    /**
     * Retrieve the socket_name specified for daemon process
     * @return socket_name
     */
    static inline const gchar *flom_config_get_socket_name(void) {
        return global_config.socket_name; }


    
    /**
     * Set trace_file in config object
     * @param daemon_trace_file IN set the new value for trace_file properties
     */
    static inline void flom_config_set_daemon_trace_file(
        gchar *daemon_trace_file) {
        if (NULL != global_config.daemon_trace_file)
            g_free(global_config.daemon_trace_file);
        global_config.daemon_trace_file = daemon_trace_file; }



    /**
     * Retrieve the trace file specified for daemon process
     * @return trace file name
     */
    static inline const gchar *flom_config_get_daemon_trace_file(void) {
        return global_config.daemon_trace_file; }


    
    /**
     * Set trace_file in config object
     * @param command_trace_file IN set the new value for trace_file properties
     */
    static inline void flom_config_set_command_trace_file(
        gchar *command_trace_file) {
        if (NULL != global_config.command_trace_file)
            g_free(global_config.command_trace_file);
        global_config.command_trace_file = command_trace_file; }



    /**
     * Retrieve the trace file specified for command process
     * @return trace file name
     */
    static inline const gchar *flom_config_get_command_trace_file(void) {
        return global_config.command_trace_file; }


    
    /**
     * Set "verbose" config parameter
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_verbose(gint value) {
        global_config.verbose = value;
    }



    /**
     * Get "verbose" config parameter
     * @return a boolean value
     */
    static inline gint flom_config_get_verbose(void) {
        return global_config.verbose;
    }


    
    /**
     * Set "daemon_lifespan" config parameter
     * @param timeout IN milliseconds
     */
    static inline void flom_config_set_lifespan(gint timeout) {
        global_config.daemon_lifespan = timeout;
    }



    /**
     * Get "daemon_lifespan" config parameter
     * @return current timeout in milliseconds
     */
    static inline gint flom_config_get_lifespan(void) {
        return global_config.daemon_lifespan;
    }


    
    /**
     * Set resource_name in config object
     * @param resource_name IN set the new value for resource_name properties
     * @return a reason code
     */
    int flom_config_set_resource_name(gchar *resource_name);



    /**
     * Retrieve the trace file specified for command process
     * @return trace file name
     */
    static inline const gchar *flom_config_get_resource_name(void) {
        return global_config.resource_name; }



    /**
     * Set "resource_wait" config parameter
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_resource_wait(int value) {
        global_config.resource_wait = value;
    }



    /**
     * Get "resource_wait" config parameter
     * @return a boolean value
     */
    static inline int flom_config_get_resource_wait(void) {
        return global_config.resource_wait;
    }


    
    /**
     * Set "resource_create" config parameter
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_resource_create(int value) {
        global_config.resource_create = value;
    }



    /**
     * Get "resource_create" config parameter
     * @return a boolean value
     */
    static inline int flom_config_get_resource_create(void) {
        return global_config.resource_create;
    }


    
    /**
     * Set "resource_timeout" config parameter
     * @param timeout IN milliseconds
     */
    static inline void flom_config_set_resource_timeout(gint timeout) {
        if (0 > timeout) timeout = -timeout;
        global_config.resource_timeout = timeout;
    }



    /**
     * Get "resource_timeout" config parameter
     * @return current timeout in milliseconds
     */
    static inline gint flom_config_get_resource_timeout(void) {
        return global_config.resource_timeout;
    }


    
    /**
     * Set "resource_quantity" config parameter
     * @param value IN number of resource to lock
     */
    static inline void flom_config_set_resource_quantity(gint value) {
        if (0 > value) value = -value;
        if (0 != value) global_config.resource_quantity = value;
    }



    /**
     * Get "resource_quantity" config parameter
     * @return current quantity
     */
    static inline gint flom_config_get_resource_quantity(void) {
        return global_config.resource_quantity;
    }


    
    /**
     * Set "lock_mode" config parameter
     * @param lock_mode IN lock mode
     */
    static inline void flom_config_set_lock_mode(flom_lock_mode_t lock_mode) {
        global_config.lock_mode = lock_mode;
    }



    /**
     * Get "lock_mode" config parameter
     * @return current lock mode
     */
    static inline flom_lock_mode_t flom_config_get_lock_mode(void) {
        return global_config.lock_mode;
    }


    
    /**
     * Set unicast_address in config object
     * @param address IN set the new value for unicast_address property
     */
    static inline void flom_config_set_unicast_address(gchar *address) {
        if (NULL != global_config.unicast_address)
            g_free(global_config.unicast_address);
        global_config.unicast_address = address; }



    /**
     * Retrieve the unicast address specified for daemon process
     * @return TCP/IP unicast address
     */
    static inline const gchar *flom_config_get_unicast_address(void) {
        return global_config.unicast_address; }


    
    /**
     * Set TCP/IP unicast port config parameter
     * @param port IN TCP/IP port
     */
    static inline void flom_config_set_unicast_port(gint port) {
        if (0 > port) port = -port;
        global_config.unicast_port = port;
    }



    /**
     * Get TCP/IP unicast port config parameter
     * @return current unicast port
     */
    static inline gint flom_config_get_unicast_port(void) {
        return global_config.unicast_port;
    }


    
    /**
     * Set multicast_address in config object
     * @param address IN set the new value for multicast_address property
     */
    static inline void flom_config_set_multicast_address(gchar *address) {
        if (NULL != global_config.multicast_address)
            g_free(global_config.multicast_address);
        global_config.multicast_address = address; }



    /**
     * Retrieve the multicast address specified for daemon process
     * @return UDP/IP multicast address
     */
    static inline const gchar *flom_config_get_multicast_address(void) {
        return global_config.multicast_address; }


    
    /**
     * Set UDP/IP multicast port config parameter
     * @param port IN UDP/IP port
     */
    static inline void flom_config_set_multicast_port(gint port) {
        if (0 > port) port = -port;
        global_config.multicast_port = port;
    }



    /**
     * Get UDP/IP multicast port config parameter
     * @return current multicast port
     */
    static inline gint flom_config_get_multicast_port(void) {
        return global_config.multicast_port;
    }


    
    /**
     * Set UDP/IP discovery attempts config parameter
     * @param attempts IN new value
     */
    static inline void flom_config_set_discovery_attempts(gint attempts) {
        if (0 > attempts) attempts = -attempts;
        if (0 == attempts) attempts = 1;
        global_config.discovery_attempts = attempts;
    }



    /**
     * Get UDP/IP discovery request attempts config parameter
     * @return current value
     */
    static inline gint flom_config_get_discovery_attempts(void) {
        return global_config.discovery_attempts;
    }


    
    /**
     * Set UDP/IP discovery request timeout config parameter
     * @param timeout IN new value
     */
    static inline void flom_config_set_discovery_timeout(gint timeout) {
        if (0 > timeout) timeout = -timeout;
        global_config.discovery_timeout = timeout;
    }



    /**
     * Get UDP/IP discovery request timeout config parameter
     * @return current value
     */
    static inline gint flom_config_get_discovery_timeout(void) {
        return global_config.discovery_timeout;
    }


    
    /**
     * Set UDP/IP discovery request TTL config parameter
     * @param ttl IN new value
     */
    static inline void flom_config_set_discovery_ttl(gint ttl) {
        if (0 > ttl) ttl = -ttl;
        if (0 == ttl) ttl = 1;
        global_config.discovery_ttl = ttl;
    }



    /**
     * Get UDP/IP discovery request ttl config parameter
     * @return current value
     */
    static inline gint flom_config_get_discovery_ttl(void) {
        return global_config.discovery_ttl;
    }


    
    /**
     * Set tcp_keepalive_time parameter value for socket SO_KEEPALIVE feature
     * @param value IN new value
     */
    static inline void flom_config_set_tcp_keepalive_time(gint value) {
        if (0 > value) value = -value;
        global_config.tcp_keepalive_time = value;
    }



    /**
     * Get tcp_keepalive_time parameter value for socket SO_KEEPALIVE feature
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_time(void) {
        return global_config.tcp_keepalive_time;
    }


    
    /**
     * Set tcp_keepalive_intvl parameter value for socket SO_KEEPALIVE feature
     * @param value IN new value
     */
    static inline void flom_config_set_tcp_keepalive_intvl(gint value) {
        if (0 > value) value = -value;
        global_config.tcp_keepalive_intvl = value;
    }



    /**
     * Get tcp_keepalive_intvl parameter value for socket SO_KEEPALIVE feature
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_intvl(void) {
        return global_config.tcp_keepalive_intvl;
    }


    
    /**
     * Set tcp_keepalive_probes parameter value for socket SO_KEEPALIVE feature
     * @param value IN new value
     */
    static inline void flom_config_set_tcp_keepalive_probes(gint value) {
        if (0 > value) value = -value;
        global_config.tcp_keepalive_probes = value;
    }



    /**
     * Get tcp_keepalive_probes parameter value for socket SO_KEEPALIVE feature
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_probes(void) {
        return global_config.tcp_keepalive_probes;
    }


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_CONFIG_H */
