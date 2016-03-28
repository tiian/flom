/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
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
/**
 * Grace time (milliseconds) used for quiesce shutdown
 */
#define FLOM_SHUTDOWN_QUIESCE_GRACE_TIME    100
/**
 * Maximum number of idle periods before locker termination
 */
#define FLOM_LOCKER_MAX_IDLE_PERIODS          1



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
 * Label associated to AppendTraceFile key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_APPENDTRACEFILE;
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
 * Label associated to "IdleLifespan" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_IDLE_LIFESPAN;
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
 * Label associated to "NetworkInterface" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_NETWORK_INTERFACE;
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
 * Label associated to TLS group inside config files
 */
extern const gchar *FLOM_CONFIG_GROUP_TLS;
/**
 * Label associated to "TlsPeerCertificate" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TLS_CERTIFICATE;
/**
 * Label associated to "TlsPeerPrivateKey" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TLS_PRIVATE_KEY;
/**
 * Label associated to "TlsCaCertificate" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE;
/**
 * Label associated to "TlsCheckPeerId" key inside config files
 */
extern const gchar *FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID;




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
typedef struct flom_config_s {
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
     * Append trace file, boolean value: TRUE=append, FALSE=truncate
     */
    gint               append_trace_file;
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
     * The resource will be kept for at least this milliseconds value after
     * last usage
     */
    gint               resource_idle_lifespan;
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
     * Network interface that must be used to reach IPv6 link local addresses
     */
    gchar             *network_interface;
    /**
     * IPv6 interface scope ID derived from @ref network_interface
     */
    uint32_t           sin6_scope_id;
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
    /**
     * name of the file that contains the X.509 certificate assigned to this
     * peer
     */
    gchar             *tls_certificate;
    /**
     * name of the file that contains the private key of this peer
     */
    gchar             *tls_private_key;
    /**
     * name of the file that contains the X.509 certificate of the
     * certification authority used to sign the certificate of this peer
     */
    gchar             *tls_ca_certificate;
    /**
     * check if the CommonName (CN) of the peer certificate matches the peer
     * unique identifier
     */
    gint               tls_check_peer_id;
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
     * @param config OUT configuration object, NULL for global config
     */
    void flom_config_reset(flom_config_t *config);
    


    /**
     * Check configuration after config file values and command line option
     * overrides
     * @param config IN configuration object, NULL for global config
     * @return a reason code
     */
    int flom_config_check(flom_config_t *config);


    
    /**
     * Print config using "g_print"
     * @param config IN configuration object, NULL for global config
     */
    void flom_config_print(flom_config_t *config);



    /**
     * Release all memory allocated by a config object
     * @param config IN/OUT configuration object, NULL for global config
     */
    void flom_config_free(flom_config_t *config);


    
    /**
     * Initialize configuration (global) object retrieving data from
     * configuration files
     * @param config IN/OUT configuration object, NULL for global config
     * @param custom_config_filename IN filename of user configuration file
     *        (NULL means there's no a custom configuration file)
     * @return a reason code
     */
    int flom_config_init(flom_config_t *config,
                         const char *custom_config_filename);



    /**
     * Load a configuration file, parse it and initialize global configuration
     * as described in the config file
     * @param config IN/OUT configuration object, NULL for global config
     * @param config_file_name IN configuration file to open and parse
     * @return a reason code
     */
    int flom_config_init_load(flom_config_t *config,
                              const char *config_file_name);



    /**
     * Clone global configuration to a local configuration object
     * @param config IN destination configuration object
     * @return a reason code
     */
    int flom_config_clone(flom_config_t *config);
    

    
    /**
     * Set daemon_socket_name
     * @param config IN/OUT configuration object, NULL for global config
     * @param socket_name IN set the new value for socket_name properties
     * @return a reason code
     */
    int flom_config_set_socket_name(flom_config_t *config,
                                    const gchar *socket_name);



    /**
     * Retrieve the socket_name specified for daemon process
     * @param config IN/OUT configuration object, NULL for global config
     * @return socket_name
     */
    static inline const gchar *flom_config_get_socket_name(
        flom_config_t *config) {
        return NULL == config ?
            global_config.socket_name : config->socket_name;
    }


    
    /**
     * Set trace_file in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param daemon_trace_file IN set the new value for trace_file properties
     */
    void flom_config_set_daemon_trace_file(
        flom_config_t *config, const gchar *daemon_trace_file);



    /**
     * Retrieve the trace file specified for daemon process
     * @param config IN/OUT configuration object, NULL for global config
     * @return trace file name
     */
    static inline const gchar *flom_config_get_daemon_trace_file(
        flom_config_t *config) {
        return NULL == config ?
            global_config.daemon_trace_file : config->daemon_trace_file;
    }


    
    /**
     * Set trace_file in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param command_trace_file IN set the new value for trace_file properties
     */
    void flom_config_set_command_trace_file(
        flom_config_t *config, const gchar *command_trace_file);



    /**
     * Retrieve the trace file specified for command process
     * @param config IN/OUT configuration object, NULL for global config
     * @return trace file name
     */
    static inline const gchar *flom_config_get_command_trace_file(
        flom_config_t *config) {
        return NULL == config ?
            global_config.command_trace_file : config->command_trace_file;
    }


    
    /**
     * Set "append_trace_file" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_append_trace_file(
        flom_config_t *config, gint value) {
        if (NULL == config)
            global_config.append_trace_file = value;
        else
            config->append_trace_file = value;
    }



    /**
     * Get "append_trace_file" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return a boolean value
     */
    static inline gint flom_config_get_append_trace_file(
        flom_config_t *config) {
        return NULL == config ?
            global_config.append_trace_file : config->append_trace_file;
    }


    
    /**
     * Set "verbose" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_verbose(
        flom_config_t *config, gint value) {
        if (NULL == config)
            global_config.verbose = value;
        else
            config->verbose = value;
    }



    /**
     * Get "verbose" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return a boolean value
     */
    static inline gint flom_config_get_verbose(flom_config_t *config) {
        return NULL == config ?
            global_config.verbose : config->verbose;
    }


    
    /**
     * Set "daemon_lifespan" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param timeout IN milliseconds
     */
    static inline void flom_config_set_lifespan(
        flom_config_t *config, gint timeout) {
        if (NULL == config)
            global_config.daemon_lifespan = timeout;
        else
            config->daemon_lifespan = timeout;
    }



    /**
     * Get "daemon_lifespan" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current timeout in milliseconds
     */
    static inline gint flom_config_get_lifespan(flom_config_t *config) {
        return NULL == config ?
            global_config.daemon_lifespan : config->daemon_lifespan;
    }


    
    /**
     * Set resource_name in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param resource_name IN set the new value for resource_name properties
     * @return a reason code
     */
    int flom_config_set_resource_name(flom_config_t *config,
                                      const gchar *resource_name);



    /**
     * Get "resource_name" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return trace file name
     */
    static inline const gchar *flom_config_get_resource_name(
        flom_config_t *config) {
        return NULL == config ?
            global_config.resource_name : config->resource_name;
    }



    /**
     * Set "resource_wait" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new (boolean) value
     */
    void flom_config_set_resource_wait(flom_config_t *config, int value);



    /**
     * Get "resource_wait" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return a boolean value
     */
    static inline int flom_config_get_resource_wait(flom_config_t *config) {
        return NULL == config ?
            global_config.resource_wait : config->resource_wait;
    }


    
    /**
     * Set "resource_create" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new (boolean) value
     */
    void flom_config_set_resource_create(flom_config_t *config, int value);



    /**
     * Get "resource_create" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return a boolean value
     */
    static inline int flom_config_get_resource_create(flom_config_t *config) {
        return NULL == config ?
            global_config.resource_create : config->resource_create;
    }


    
    /**
     * Set "resource_timeout" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param timeout IN milliseconds
     */
    void flom_config_set_resource_timeout(flom_config_t *config, gint timeout);



    /**
     * Get "resource_timeout" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current timeout in milliseconds
     */
    static inline gint flom_config_get_resource_timeout(
        flom_config_t *config) {
        return NULL == config ?
            global_config.resource_timeout : config->resource_timeout;
    }


    
    /**
     * Set "resource_quantity" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN number of resource to lock
     */
    void flom_config_set_resource_quantity(flom_config_t *config, gint value);



    /**
     * Get "resource_quantity" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current quantity
     */
    static inline gint flom_config_get_resource_quantity(
        flom_config_t *config) {
        return NULL == config ?
            global_config.resource_quantity : config->resource_quantity;
    }


    
    /**
     * Set "lock_mode" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param lock_mode IN lock mode
     */
    void flom_config_set_lock_mode(flom_config_t *config,
                                   flom_lock_mode_t lock_mode);



    /**
     * Get "lock_mode" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current lock mode
     */
    static inline flom_lock_mode_t flom_config_get_lock_mode(
        flom_config_t *config) {
        return NULL == config ?
            global_config.lock_mode : config->lock_mode;
    }


    
    /**
     * Set "resource_idle_lifespan" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN number of resource to lock
     */
    void flom_config_set_resource_idle_lifespan(flom_config_t *config,
                                                gint value);



    /**
     * Get "resource_idle_lifespan" config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current quantity
     */
    static inline gint flom_config_get_resource_idle_lifespan(
        flom_config_t *config) {
        return NULL == config ?
            global_config.resource_idle_lifespan :
            config->resource_idle_lifespan;
    }


    
    /**
     * Set unicast_address in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param address IN set the new value for unicast_address property
     */
    void flom_config_set_unicast_address(flom_config_t *config,
                                         const gchar *address);
        


    /**
     * Retrieve the unicast address specified for daemon process
     * @param config IN/OUT configuration object, NULL for global config
     * @return TCP/IP unicast address
     */
    static inline const gchar *flom_config_get_unicast_address(
        flom_config_t *config) {
        return NULL == config ?
            global_config.unicast_address : config->unicast_address;
    }


    
    /**
     * Set TCP/IP unicast port config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param port IN TCP/IP port
     */
    void flom_config_set_unicast_port(flom_config_t *config, gint port);



    /**
     * Get TCP/IP unicast port config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current unicast port
     */
    static inline gint flom_config_get_unicast_port(flom_config_t *config) {
        return NULL == config ?
            global_config.unicast_port : config->unicast_port;
    }


    
    /**
     * Set multicast_address in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param address IN set the new value for multicast_address property
     */
    void flom_config_set_multicast_address(
        flom_config_t *config, const gchar *address);



    /**
     * Retrieve the multicast address specified for daemon process
     * @param config IN/OUT configuration object, NULL for global config
     * @return UDP/IP multicast address
     */
    static inline const gchar *flom_config_get_multicast_address(
        flom_config_t *config) {
        return NULL == config ?
            global_config.multicast_address : config->multicast_address;
    }


    
    /**
     * Set UDP/IP multicast port config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param port IN UDP/IP port
     */
    void flom_config_set_multicast_port(flom_config_t *config, gint port);



    /**
     * Get UDP/IP multicast port config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current multicast port
     */
    static inline gint flom_config_get_multicast_port(flom_config_t *config) {
        return NULL == config ?
            global_config.multicast_port : config->multicast_port;
    }


    
    /**
     * Set network_interface in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN set the new value for network_interface property
     * @return a reason code
     */
    int flom_config_set_network_interface(flom_config_t *config,
                                          const gchar *value);
        


    /**
     * Retrieve the network interface that must be used to reach IPv6 link
     * local addresses
     * @param config IN/OUT configuration object, NULL for global config
     * @return network interface
     */
    static inline const gchar *flom_config_get_network_interface(
        flom_config_t *config) {
        return NULL == config ?
            global_config.network_interface : config->network_interface;
    }



    /**
     * Directly set the IPv6 scope id (typecally retrieved by an incoming
     * connection)
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN set the new value for sin6_scope_id property
     * @return
     */
    int flom_config_set_sin6_scope_id(flom_config_t *config, uint32_t value);

    

    /**
     * Retrieve the IPv6 scope ID associated to the network interface
     * @param config IN/OUT configuration object, NULL for global config
     * @return IPv6 scope ID
     */
    static inline uint32_t flom_config_get_sin6_scope_id(
        flom_config_t *config) {
        return NULL == config ?
            global_config.sin6_scope_id : config->sin6_scope_id;
    }

    
    
    /**
     * Set UDP/IP discovery attempts config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param attempts IN new value
     */
    void flom_config_set_discovery_attempts(flom_config_t *config,
                                            gint attempts);



    /**
     * Get UDP/IP discovery request attempts config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_discovery_attempts(
        flom_config_t *config) {
        return NULL == config ?
            global_config.discovery_attempts : config->discovery_attempts;
    }


    
    /**
     * Set UDP/IP discovery request timeout config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param timeout IN new value
     */
    void flom_config_set_discovery_timeout(flom_config_t *config,
                                           gint timeout);



    /**
     * Get UDP/IP discovery request timeout config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_discovery_timeout(
        flom_config_t *config) {
        return NULL == config ?
            global_config.discovery_timeout : config->discovery_timeout;
    }


    
    /**
     * Set UDP/IP discovery request TTL config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @param ttl IN new value
     */
    void flom_config_set_discovery_ttl(flom_config_t *config, gint ttl);



    /**
     * Get UDP/IP discovery request ttl config parameter
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_discovery_ttl(flom_config_t *config) {
        return NULL == config ?
            global_config.discovery_ttl : config->discovery_ttl;
    }


    
    /**
     * Set tcp_keepalive_time parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new value
     */
    void flom_config_set_tcp_keepalive_time(flom_config_t *config, gint value);



    /**
     * Get tcp_keepalive_time parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_time(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tcp_keepalive_time : config->tcp_keepalive_time;
    }


    
    /**
     * Set tcp_keepalive_intvl parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new value
     */
    void flom_config_set_tcp_keepalive_intvl(flom_config_t *config,
                                             gint value);



    /**
     * Get tcp_keepalive_intvl parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_intvl(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tcp_keepalive_intvl : config->tcp_keepalive_intvl;
    }


    
    /**
     * Set tcp_keepalive_probes parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new value
     */
    void flom_config_set_tcp_keepalive_probes(flom_config_t *config,
                                              gint value);



    /**
     * Get tcp_keepalive_probes parameter value for socket SO_KEEPALIVE feature
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_tcp_keepalive_probes(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tcp_keepalive_probes : config->tcp_keepalive_probes;
    }



    /**
     * Set tls_certificate in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN set the new value for tls_certificate property
     * @return a reason code
     */
    int flom_config_set_tls_certificate(flom_config_t *config,
                                        const gchar *value);
        


    /**
     * Get tls_certificate value
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gchar *flom_config_get_tls_certificate(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tls_certificate : config->tls_certificate;
    }
    

    
    /**
     * Set tls_private_key in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN set the new value for tls_private_key property
     * @return a reason code
     */
    int flom_config_set_tls_private_key(flom_config_t *config,
                                        const gchar *value);
        


    /**
     * Get tls_private_key value
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gchar *flom_config_get_tls_private_key(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tls_private_key : config->tls_private_key;
    }
    

    
    /**
     * Set tls_ca_certificate in config object
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN set the new value for tls_ca_certificate property
     * @return a reason code
     */
    int flom_config_set_tls_ca_certificate(flom_config_t *config,
                                           const gchar *value);
        


    /**
     * Get tls_ca_certificate value
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gchar *flom_config_get_tls_ca_certificate(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tls_ca_certificate : config->tls_ca_certificate;
    }
    

    
    /**
     * Set tls_check_peer_id parameter value
     * @param config IN/OUT configuration object, NULL for global config
     * @param value IN new (boolean) value
     */
    static inline void flom_config_set_tls_check_peer_id(
        flom_config_t *config, gint value) {
        if (NULL == config)
            global_config.tls_check_peer_id = value;
        else
            config->tls_check_peer_id = value;
    }



    /**
     * Get tls_check_peer_id parameter value
     * @param config IN/OUT configuration object, NULL for global config
     * @return current value
     */
    static inline gint flom_config_get_tls_check_peer_id(
        flom_config_t *config) {
        return NULL == config ?
            global_config.tls_check_peer_id : config->tls_check_peer_id;
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
