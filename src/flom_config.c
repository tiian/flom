/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_STRING_H
/* strcasestr is not POSIX standard and needs GNU extensions... */
# define _GNU_SOURCE
# include <string.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_IFADDRS_H
# include <ifaddrs.h>
#endif
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif
#ifdef HAVE_PWD_H
# include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_rsrc.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONFIG



/* global static objects */
flom_config_t global_config;



/* static strings */
const gchar DEFAULT_RESOURCE_NAME[] = "_RESOURCE";
const gchar FLOM_SYSTEM_CONFIG_FILENAME[] = _SYSTEM_CONFIG_FILENAME;
const gchar FLOM_USER_CONFIG_FILENAME[] = _USER_CONFIG_FILENAME;
const gchar FLOM_DIR_FILE_SEPARATOR[] = _DIR_FILE_SEPARATOR;
const gchar FLOM_RESOURCE_SET_SEPARATOR[] = _RESOURCE_SET_SEPARATOR;
const gchar FLOM_HIER_RESOURCE_SEPARATOR[] = _DIR_FILE_SEPARATOR;


const char *FLOM_PACKAGE_BUGREPORT = PACKAGE_BUGREPORT;
const char *FLOM_PACKAGE_NAME = PACKAGE;
const char *FLOM_PACKAGE_VERSION = PACKAGE_VERSION;
const char *FLOM_PACKAGE_DATE = _RELEASE_DATE;

const gchar FLOM_INSTALL_SYSCONFDIR[] = _SYSCONFDIR;

const char *FLOM_EMPTY_STRING = "";
const char *FLOM_NULL_STRING = "{null}";

const gchar *FLOM_CONFIG_GROUP_TRACE = _CONFIG_GROUP_TRACE;
const gchar *FLOM_CONFIG_KEY_DAEMONTRACEFILE = _CONFIG_KEY_DAEMONTRACEFILE;
const gchar *FLOM_CONFIG_KEY_COMMANDTRACEFILE = _CONFIG_KEY_COMMANDTRACEFILE;
const gchar *FLOM_CONFIG_KEY_APPENDTRACEFILE = _CONFIG_KEY_APPENDTRACEFILE;
const gchar *FLOM_CONFIG_KEY_VERBOSE = _CONFIG_KEY_VERBOSE;
const gchar *FLOM_CONFIG_GROUP_RESOURCE = _CONFIG_GROUP_RESOURCE;
const gchar *FLOM_CONFIG_KEY_CREATE = _CONFIG_KEY_CREATE;
const gchar *FLOM_CONFIG_KEY_NAME = _CONFIG_KEY_NAME;
const gchar *FLOM_CONFIG_KEY_TIMEOUT = _CONFIG_KEY_TIMEOUT;
const gchar *FLOM_CONFIG_KEY_QUANTITY = _CONFIG_KEY_QUANTITY;
const gchar *FLOM_CONFIG_KEY_LOCK_MODE = _CONFIG_KEY_LOCK_MODE;
const gchar *FLOM_CONFIG_KEY_IDLE_LIFESPAN = _CONFIG_KEY_IDLE_LIFESPAN;
const gchar *FLOM_CONFIG_GROUP_DAEMON = _CONFIG_GROUP_DAEMON;
const gchar *FLOM_CONFIG_KEY_SOCKET_NAME = _CONFIG_KEY_SOCKET_NAME;
const gchar *FLOM_CONFIG_KEY_LIFESPAN = _CONFIG_KEY_LIFESPAN;
const gchar *FLOM_CONFIG_KEY_UNICAST_ADDRESS = _CONFIG_KEY_UNICAST_ADDRESS;
const gchar *FLOM_CONFIG_KEY_UNICAST_PORT = _CONFIG_KEY_UNICAST_PORT;
const gchar *FLOM_CONFIG_KEY_MULTICAST_ADDRESS = _CONFIG_KEY_MULTICAST_ADDRESS;
const gchar *FLOM_CONFIG_KEY_MULTICAST_PORT = _CONFIG_KEY_MULTICAST_PORT;
const gchar *FLOM_CONFIG_GROUP_MONITOR = _CONFIG_GROUP_MONITOR;
const gchar *FLOM_CONFIG_KEY_IGNORED_SIGNALS = _CONFIG_KEY_IGNORED_SIGNALS;
const gchar *FLOM_CONFIG_GROUP_NETWORK = _CONFIG_GROUP_NETWORK;
const gchar *FLOM_CONFIG_KEY_NETWORK_INTERFACE = _CONFIG_KEY_NETWORK_INTERFACE;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS = _CONFIG_KEY_DISCOVERY_ATTEMPTS;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT = _CONFIG_KEY_DISCOVERY_TIMEOUT;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_TTL = _CONFIG_KEY_DISCOVERY_TTL;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME = _CONFIG_KEY_TCP_KEEPALIVE_TIME;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL = _CONFIG_KEY_TCP_KEEPALIVE_INTVL;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES = _CONFIG_KEY_TCP_KEEPALIVE_PROBES;
const gchar *FLOM_CONFIG_GROUP_TLS = _CONFIG_GROUP_TLS;
const gchar *FLOM_CONFIG_KEY_TLS_CERTIFICATE = _CONFIG_KEY_TLS_CERTIFICATE;
const gchar *FLOM_CONFIG_KEY_TLS_PRIVATE_KEY = _CONFIG_KEY_TLS_PRIVATE_KEY;
const gchar *FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE = _CONFIG_KEY_TLS_CA_CERTIFICATE;
const gchar *FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID = _CONFIG_KEY_TLS_CHECK_PEER_ID;



const gchar *SIGNAL_STRING_ARRAY[] = {
    /*  0 */ "",
    /*  1 */ "SIGHUP",
    /*  2 */ "SIGINT",
    /*  3 */ "SIGQUIT",
    /*  4 */ "SIGILL",
    /*  5 */ "SIGTRAP",
    /*  6 */ "SIGABRT",
    /*  7 */ "SIGBUS",
    /*  8 */ "SIGFPE",
    /*  9 */ "SIGKILL",
    /* 10 */ "SIGUSR1",
    /* 11 */ "SIGSEGV",
    /* 12 */ "SIGUSR2",
    /* 13 */ "SIGPIPE",
    /* 14 */ "SIGALRM",
    /* 15 */ "SIGTERM",
    /* 16 */ "SIGSTKFLT",
    /* 17 */ "SIGCHLD",
    /* 18 */ "SIGCONT",
    /* 19 */ "SIGSTOP",
    /* 20 */ "SIGTSTP",
    /* 21 */ "SIGTTIN",
    /* 22 */ "SIGTTOU",
    /* 23 */ "SIGURG",
    /* 24 */ "SIGXCPU",
    /* 25 */ "SIGXFSZ",
    /* 26 */ "SIGVTALRM",
    /* 27 */ "SIGPROF",
    /* 28 */ "SIGWINCH",
    /* 29 */ "SIGIO",
    /* 30 */ "SIGPWR",
    /* 31 */ "SIGSYS"
};
const size_t SIGNAL_STRING_ARRAY_SIZE =
    sizeof(SIGNAL_STRING_ARRAY)/sizeof(const gchar *);



flom_bool_value_t flom_bool_value_retrieve(const gchar *text)
{
    /* parsing is case sensitive only on GNU systems */
    char *p = NULL;
    
    FLOM_TRACE(("flom_bool_value_retrieve: '%s'\n", text));
    /* check if 'yes', 'no' - any case - are in the text */
    if (NULL != (p = STRCASESTR(text, "no"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'no' here: '%s'\n", p));
        return FLOM_BOOL_NO;
    } else if (NULL != (p = STRCASESTR(text, "yes"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'yes' here: '%s'\n", p));
        return FLOM_BOOL_YES;
    /* check if 'y', 'n' - any case - are in the text */
    } else if (NULL != (p = STRCASESTR(text, "n"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'n' here: '%s'\n", p));
        return FLOM_BOOL_NO;
    } else if (NULL != (p = STRCASESTR(text, "y"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'y' here: '%s'\n", p));
        return FLOM_BOOL_YES;
    }
    return FLOM_BOOL_INVALID;
}



void flom_config_reset(flom_config_t *config)
{
    FLOM_TRACE(("flom_config_reset\n"));
    if (NULL == config)
        config = &global_config;
    config->daemon_trace_file = NULL;
    config->command_trace_file = NULL;
    config->append_trace_file = FALSE;
    config->verbose = FALSE;
    config->resource_name = g_strdup(DEFAULT_RESOURCE_NAME);
    config->resource_transactional = FALSE;
    config->resource_create = TRUE;
    config->resource_timeout = FLOM_NETWORK_WAIT_TIMEOUT;
    config->resource_quantity = 1;
    config->lock_mode = FLOM_LOCK_MODE_EX;
    config->resource_idle_lifespan = 0;
    config->socket_name = NULL;
    config->daemon_lifespan = _DEFAULT_DAEMON_LIFESPAN;
    config->unicast_address = NULL;
    config->unicast_port = _DEFAULT_DAEMON_PORT;
    config->multicast_address = NULL;
    config->multicast_port = _DEFAULT_DAEMON_PORT;
    config->network_interface = NULL;
    config->sin6_scope_id = 0;
    config->discovery_attempts = _DEFAULT_DISCOVERY_ATTEMPTS;
    config->discovery_timeout = _DEFAULT_DISCOVERY_TIMEOUT;
    config->discovery_ttl = _DEFAULT_DISCOVERY_TTL;
    config->tcp_keepalive_time = _DEFAULT_TCP_KEEPALIVE_TIME;
    config->tcp_keepalive_intvl = _DEFAULT_TCP_KEEPALIVE_INTVL;
    config->tcp_keepalive_probes = _DEFAULT_TCP_KEEPALIVE_PROBES;
    config->tls_certificate = NULL;
    config->tls_private_key = NULL;
    config->tls_ca_certificate = NULL;
    config->tls_check_peer_id = FALSE;
    sigemptyset(&config->ignored_signals);
}



int flom_config_check(flom_config_t *config)
{
    enum Exception { INVALID_OPTION1
                     , INVALID_OPTION2
                     , INVALID_OPTION3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_config_check\n"));
    TRY {
        flom_rsrc_type_t frt;

        if (NULL == config)
            config = &global_config;
        /* check if configuration is for LOCAL and for NETWORK: it's not
           acceptable */
        if (NULL != flom_config_get_socket_name(config) &&
            (NULL != flom_config_get_unicast_address(config) ||
             NULL != flom_config_get_multicast_address(config))) {
            g_print("ERROR: flom can not be configured for local "
                    "(UNIX socket) and network (TCP-UDP/IP) communication "
                    "at the same time.\n");
            THROW(INVALID_OPTION1);
        }
        /* network interface is useless if getifaddrs is not available */
#ifndef HAVE_GETIFADDRS
        if (NULL != flom_config_get_network_interface(config)) {
            g_print("ERROR: network interface can not be set because function "
                    "getifaddrs() is not available.\n");
            THROW(INVALID_OPTION2);
        }
#endif /* HAVE_GETIFADDRS */
        /* if neither local nor network communication were configured,
           use default local */
        if (NULL == flom_config_get_socket_name(config) &&
            NULL == flom_config_get_unicast_address(config) &&
            NULL == flom_config_get_multicast_address(config)) {
            struct passwd *pwd = NULL;
            char *login = NULL;
            /* set UNIX socket name */
            config->socket_name = g_malloc(LOCAL_SOCKET_SIZE);
            pwd = getpwuid(getuid());
            if (NULL == pwd || NULL == pwd->pw_name)
                login = "nobody";
            else
                login = pwd->pw_name;
            snprintf(config->socket_name, LOCAL_SOCKET_SIZE,
                     "/tmp/flom-%s", login);
        }
        /* check options related to resource type */
        frt = flom_rsrc_get_type(flom_config_get_resource_name(config));
        /* check lock mode */
        if (FLOM_LOCK_MODE_EX != flom_config_get_lock_mode(config) &&
            FLOM_RSRC_TYPE_SIMPLE != frt && FLOM_RSRC_TYPE_HIER != frt) {
            if (flom_config_get_verbose(config))
                g_warning("This resource type (%d) support only exclusive "
                          "lock mode; specified value (%d) will be ignored\n",
                          frt, flom_config_get_lock_mode(config));
            flom_config_set_lock_mode(config, FLOM_LOCK_MODE_EX);
        }
        /* check quantity */
        if (1 != flom_config_get_resource_quantity(config) &&
            FLOM_RSRC_TYPE_NUMERIC != frt) {
            if (flom_config_get_verbose(config))
                g_warning("This resource type (%d) does not support quantity "
                          "lock option; specified value (%d) will be "
                          "ignored\n",
                          frt, flom_config_get_resource_quantity(config));
            flom_config_set_resource_quantity(config, 1);
        }
        /* check peer id */
        if (flom_config_get_tls_check_peer_id(config) &&
            (NULL == flom_config_get_tls_certificate(config) ||
             NULL == flom_config_get_tls_private_key(config) ||
             NULL == flom_config_get_tls_ca_certificate(config))) {
            g_print("ERROR: TLS check peer id can not be specified without "
                    "full TLS configuration.\n");
            THROW(INVALID_OPTION3);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_OPTION1:
            case INVALID_OPTION2:
            case INVALID_OPTION3:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_config_check/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_config_print(flom_config_t *config)
{
    gchar *ignored_signals = NULL;
    
    /* default config */
    if (NULL == config)
        config = &global_config;
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_DAEMONTRACEFILE,
            NULL == flom_config_get_daemon_trace_file(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_daemon_trace_file(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_COMMANDTRACEFILE,
            NULL == flom_config_get_command_trace_file(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_command_trace_file(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_APPENDTRACEFILE,
            flom_config_get_append_trace_file(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_VERBOSE, flom_config_get_verbose(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_NAME,
            NULL == flom_config_get_resource_name(config) ? FLOM_EMPTY_STRING :
            flom_config_get_resource_name(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_TIMEOUT, flom_config_get_resource_timeout(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_QUANTITY,
            flom_config_get_resource_quantity(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_LOCK_MODE, flom_config_get_lock_mode(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_CREATE, flom_config_get_resource_create(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_IDLE_LIFESPAN,
            flom_config_get_resource_idle_lifespan(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_SOCKET_NAME,
            NULL == flom_config_get_socket_name(config) ? FLOM_EMPTY_STRING :
            flom_config_get_socket_name(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_LIFESPAN, flom_config_get_lifespan(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_UNICAST_ADDRESS,
            NULL == flom_config_get_unicast_address(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_unicast_address(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_UNICAST_PORT,
            flom_config_get_unicast_port(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_MULTICAST_ADDRESS,
            NULL == flom_config_get_multicast_address(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_multicast_address(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_MULTICAST_PORT,
            flom_config_get_multicast_port(config));
    ignored_signals = flom_config_get_ignored_signals_str(config);
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_MONITOR,
            FLOM_CONFIG_KEY_IGNORED_SIGNALS, ignored_signals);
    g_free(ignored_signals);
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_NETWORK_INTERFACE,
            NULL == flom_config_get_network_interface(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_network_interface(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS,
            flom_config_get_discovery_attempts(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT,
            flom_config_get_discovery_timeout(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_TTL,
            flom_config_get_discovery_ttl(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME,
            flom_config_get_tcp_keepalive_time(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL,
            flom_config_get_tcp_keepalive_intvl(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES,
            flom_config_get_tcp_keepalive_probes(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TLS,
            FLOM_CONFIG_KEY_TLS_CERTIFICATE,
            NULL == flom_config_get_tls_certificate(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_tls_certificate(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TLS,
            FLOM_CONFIG_KEY_TLS_PRIVATE_KEY,
            NULL == flom_config_get_tls_private_key(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_tls_private_key(config));
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TLS,
            FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE,
            NULL == flom_config_get_tls_ca_certificate(config) ?
            FLOM_EMPTY_STRING :
            flom_config_get_tls_ca_certificate(config));
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_TLS,
            FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID,
            flom_config_get_tls_check_peer_id(config));
}



void flom_config_free(flom_config_t *config)
{
    FLOM_TRACE(("flom_config_free\n"));
    if (NULL == config)
        config = &global_config;
    g_free(config->daemon_trace_file);
    config->daemon_trace_file = NULL;
    g_free(config->command_trace_file);
    config->command_trace_file = NULL;
    g_free(config->resource_name);
    config->resource_name = NULL;
    g_free(config->socket_name);
    config->socket_name = NULL;
    g_free(config->unicast_address);
    config->unicast_address = NULL;
    g_free(config->multicast_address);
    config->multicast_address = NULL;    
    g_free(config->network_interface);
    config->network_interface = NULL;
    g_free(config->tls_certificate);
    config->tls_certificate = NULL;
    g_free(config->tls_private_key);
    config->tls_private_key = NULL;
    g_free(config->tls_ca_certificate);
    config->tls_ca_certificate = NULL;
}



int flom_config_init(flom_config_t *config,
                     const char *custom_config_filename)
{
    enum Exception { CONFIG_INIT_LOAD_ERROR1
                     , CONFIG_INIT_LOAD_ERROR2
                     , CONFIG_INIT_LOAD_ERROR3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    /* retrieve configuration from system default config file */
    char system_config_filename[sizeof(FLOM_INSTALL_SYSCONFDIR) +
                                sizeof(FLOM_DIR_FILE_SEPARATOR) +
                                sizeof(FLOM_SYSTEM_CONFIG_FILENAME)];
    const gchar *home_dir = NULL;
    gchar *user_config_filename = NULL;

    FLOM_TRACE(("flom_config_init\n"));
    TRY {
        gsize ucf_size;

        /* default config object */
        if (NULL == config)
            config = &global_config;
        /* building system configuration filename */
        strcpy(system_config_filename, FLOM_INSTALL_SYSCONFDIR);
        strcat(system_config_filename, FLOM_DIR_FILE_SEPARATOR);
        strcat(system_config_filename, FLOM_SYSTEM_CONFIG_FILENAME);
        assert(sizeof(system_config_filename)>strlen(system_config_filename));
        ret_cod = flom_config_init_load(config, system_config_filename);
        if (FLOM_RC_OK != ret_cod &&
            FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR != ret_cod)
            THROW(CONFIG_INIT_LOAD_ERROR1);
        /* building user default configuration filename */
        home_dir = g_getenv("HOME");
        if (!home_dir)
            home_dir = g_get_home_dir();
        ucf_size = strlen(home_dir) + sizeof(FLOM_DIR_FILE_SEPARATOR) +
            sizeof(FLOM_USER_CONFIG_FILENAME);
        user_config_filename = g_malloc(ucf_size);
        g_stpcpy(
            g_stpcpy(
                g_stpcpy(user_config_filename, home_dir),
                FLOM_DIR_FILE_SEPARATOR),
            FLOM_USER_CONFIG_FILENAME);
        assert(ucf_size > strlen(user_config_filename));
        ret_cod = flom_config_init_load(config, user_config_filename);
        if (FLOM_RC_OK != ret_cod &&
            FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR != ret_cod)
            THROW(CONFIG_INIT_LOAD_ERROR2);
        g_free(user_config_filename);
        user_config_filename = NULL;
        /* using custom config filename */
        if (NULL != custom_config_filename) {
            ret_cod = flom_config_init_load(config, custom_config_filename);
            if (FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR == ret_cod)
                g_print("ERROR: error while loading file '%s'\n",
                        custom_config_filename);
            if (FLOM_RC_OK != ret_cod)
                THROW(CONFIG_INIT_LOAD_ERROR3);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONFIG_INIT_LOAD_ERROR1:
            case CONFIG_INIT_LOAD_ERROR2:
            case CONFIG_INIT_LOAD_ERROR3:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != user_config_filename)
        g_free(user_config_filename);
    FLOM_TRACE(("flom_config_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_config_init_load(flom_config_t *config,
                          const char *config_file_name)
{
    enum Exception {
        G_KEY_FILE_NEW_ERROR,
        G_KEY_FILE_LOAD_FROM_FILE_ERROR,
        CONFIG_SET_APPENDTRACEFILE_ERROR,
        CONFIG_SET_VERBOSE_ERROR,
        CONFIG_SET_RESOURCE_NAME_ERROR,
        CONFIG_SET_RESOURCE_WAIT_ERROR,
        CONFIG_SET_RESOURCE_TIMEOUT_ERROR,
        CONFIG_SET_RESOURCE_QUANTITY_ERROR,
        CONFIG_SET_RESOURCE_LOCK_MODE_ERROR,
        CONFIG_SET_RESOURCE_CREATE_ERROR,
        CONFIG_SET_RESOURCE_IDLE_LIFESPAN_ERROR,
        CONFIG_SET_SOCKET_NAME_ERROR,
        CONFIG_SET_DAEMON_LIFESPAN_ERROR,
        CONFIG_SET_DAEMON_UNICAST_PORT_ERROR,
        CONFIG_SET_DAEMON_MULTICAST_PORT_ERROR,
        CONFIG_SET_DAEMON_DISCOVERY_ATTEMPTS_ERROR,
        CONFIG_SET_DAEMON_DISCOVERY_TIMEOUT_ERROR,
        CONFIG_SET_DAEMON_DISCOVERY_TTL_ERROR,
        CONFIG_SET_MONITOR_IGNORED_SIGNALS_ERROR,
        CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR,
        CONFIG_SET_NETWORK_TCP_KEEPALIVE_INTVL_ERROR,
        CONFIG_SET_NETWORK_TCP_KEEPALIVE_PROBES_ERROR,
        CONFIG_SET_TLS_CERTIFICATE_ERROR,
        CONFIG_SET_TLS_PRIVATE_KEY_ERROR,
        CONFIG_SET_TLS_CA_CERTIFICATE_ERROR,
        CONFIG_SET_TLS_CHECK_PEER_ID_ERROR,
        NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int print_file_name = FALSE;

    GKeyFile *gkf = NULL;
    GError *error = NULL;
    gchar *value = NULL;
    gchar **list = NULL;
    gint ivalue = 0;
    
    FLOM_TRACE(("flom_config_init_load\n"));
    TRY {
        /* default config object */
        if (NULL == config)
            config = &global_config;
        /* create g_key_file object */
        if (NULL == (gkf = g_key_file_new()))
            THROW(G_KEY_FILE_NEW_ERROR);
        /* load configuration file */
        FLOM_TRACE(("flom_config_init_load: loading config from file '%s'\n",
                    config_file_name));
        if (!g_key_file_load_from_file(gkf, config_file_name,
                                       G_KEY_FILE_NONE, &error)) {
            if (NULL != error) {
                FLOM_TRACE(("flom_config_init_load/g_key_file_load_from_file:"
                            " code=%d, message='%s'\n", error->code,
                            error->message));
                g_error_free(error);
                error = NULL;
            }
            THROW(G_KEY_FILE_LOAD_FROM_FILE_ERROR);
        }
        /* pick-up daemon trace configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_DAEMONTRACEFILE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_DAEMONTRACEFILE, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_DAEMONTRACEFILE, value));
            flom_config_set_daemon_trace_file(config, value);
            g_free(value);
            value = NULL;
        }
        /* pick-up command trace configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_COMMANDTRACEFILE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_COMMANDTRACEFILE, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_COMMANDTRACEFILE, value));
            flom_config_set_command_trace_file(config, value);
            g_free(value);
            value = NULL;
        }
        /* pick-up append_trace_file from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_APPENDTRACEFILE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_APPENDTRACEFILE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_bool_value_t fbv;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_APPENDTRACEFILE, value));
            if (FLOM_BOOL_INVALID == (
                    fbv = flom_bool_value_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_append_trace_file(config, fbv);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_APPENDTRACEFILE_ERROR);
        }
        /* pick-up verbose mode from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TRACE,
                         FLOM_CONFIG_KEY_VERBOSE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_VERBOSE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_bool_value_t fbv;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TRACE,
                        FLOM_CONFIG_KEY_VERBOSE, value));
            if (FLOM_BOOL_INVALID == (
                    fbv = flom_bool_value_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_verbose(config, fbv);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_VERBOSE_ERROR);
        }
        /* pick-up resource name from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_RESOURCE,
                         FLOM_CONFIG_KEY_NAME, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_NAME, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_NAME, value));
            if (FLOM_RC_OK != (
                    ret_cod = flom_config_set_resource_name(config, value))) {
                print_file_name = TRUE;
                THROW(CONFIG_SET_RESOURCE_NAME_ERROR);
            } else {
                g_free(value);
                value = NULL;
            }
        }
        /* pick-up resource timeout from configuration */
        ivalue = g_key_file_get_integer(
            gkf, FLOM_CONFIG_GROUP_RESOURCE, FLOM_CONFIG_KEY_TIMEOUT, &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_TIMEOUT,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_TIMEOUT_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_TIMEOUT, ivalue));
            flom_config_set_resource_timeout(config, ivalue);
        }
        /* pick-up resource quantity from configuration */
        ivalue = g_key_file_get_integer(
            gkf, FLOM_CONFIG_GROUP_RESOURCE, FLOM_CONFIG_KEY_QUANTITY, &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_QUANTITY,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_QUANTITY_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_QUANTITY, ivalue));
            flom_config_set_resource_quantity(config, ivalue);
        }
        /* pick-up resource lock mode from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_RESOURCE,
                         FLOM_CONFIG_KEY_LOCK_MODE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_LOCK_MODE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_lock_mode_t flm;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_LOCK_MODE, value));
            if (FLOM_LOCK_MODE_INVALID == (
                    flm = flom_lock_mode_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_lock_mode(config, flm);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_LOCK_MODE_ERROR);
        }
        /* pick-up resource create from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_RESOURCE,
                         FLOM_CONFIG_KEY_CREATE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_CREATE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_bool_value_t fbv;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_CREATE, value));
            if (FLOM_BOOL_INVALID == (
                    fbv = flom_bool_value_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_resource_create(config, fbv);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_CREATE_ERROR);
        }
        /* pick-up resource idle lifespan from configuration */
        ivalue = g_key_file_get_integer(
            gkf, FLOM_CONFIG_GROUP_RESOURCE, FLOM_CONFIG_KEY_IDLE_LIFESPAN,
            &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_IDLE_LIFESPAN,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_IDLE_LIFESPAN_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_IDLE_LIFESPAN, ivalue));
            flom_config_set_resource_idle_lifespan(config, ivalue);
        }
        /* pick-up socket name configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_DAEMON,
                         FLOM_CONFIG_KEY_SOCKET_NAME, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_SOCKET_NAME, 
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_SOCKET_NAME, value));
            if (FLOM_RC_OK != (ret_cod = flom_config_set_socket_name(
                                   config, value))) {
                print_file_name = TRUE;
                THROW(CONFIG_SET_SOCKET_NAME_ERROR);
            } else {
                g_free(value);
                value = NULL;
            }
        }
        /* pick-up daemon lifespan from configuration */
        ivalue = g_key_file_get_integer(
            gkf, FLOM_CONFIG_GROUP_DAEMON, FLOM_CONFIG_KEY_LIFESPAN, &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_LIFESPAN,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_LIFESPAN_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_LIFESPAN, ivalue));
            flom_config_set_lifespan(config, ivalue);
        }
        /* pick-up unicast address configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_DAEMON,
                         FLOM_CONFIG_KEY_UNICAST_ADDRESS, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_UNICAST_ADDRESS,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_UNICAST_ADDRESS, value));
            flom_config_set_unicast_address(config, value);
            g_free(value);
            value = NULL;
        }
        /* pick-up unicast port from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_DAEMON,
                                        FLOM_CONFIG_KEY_UNICAST_PORT, &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_UNICAST_PORT,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_UNICAST_PORT_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_UNICAST_PORT, ivalue));
            flom_config_set_unicast_port(config, ivalue);
        }
        /* pick-up mulicast address configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_DAEMON,
                         FLOM_CONFIG_KEY_MULTICAST_ADDRESS, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_MULTICAST_ADDRESS,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_MULTICAST_ADDRESS, value));
            flom_config_set_multicast_address(config, value);
            g_free(value);
            value = NULL;
        }
        /* pick-up multicast port from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_DAEMON,
                                        FLOM_CONFIG_KEY_MULTICAST_PORT,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_MULTICAST_PORT,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_MULTICAST_PORT_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_DAEMON,
                        FLOM_CONFIG_KEY_MULTICAST_PORT, ivalue));
            flom_config_set_multicast_port(config, ivalue);
        }
        /* pick-up the signals that must be ignored by the monitor */
        if (NULL == (list = g_key_file_get_string_list(
                         gkf, FLOM_CONFIG_GROUP_MONITOR,
                         FLOM_CONFIG_KEY_IGNORED_SIGNALS, NULL, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_MONITOR,
                        FLOM_CONFIG_KEY_IGNORED_SIGNALS,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            flom_config_set_ignored_signals(config, list);
            g_strfreev(list);
            list = NULL;
        }
        /* pick-up network interface configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_NETWORK,
                         FLOM_CONFIG_KEY_NETWORK_INTERFACE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_NETWORK_INTERFACE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_NETWORK_INTERFACE, value));
            flom_config_set_network_interface(config, value);
            g_free(value);
            value = NULL;
        }
        /* pick-up discovery attempts from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_DISCOVERY_ATTEMPTS_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS, ivalue));
            flom_config_set_discovery_attempts(config, ivalue);
        }
        /* pick-up discovery timeout from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_DISCOVERY_TIMEOUT_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT, ivalue));
            flom_config_set_discovery_timeout(config, ivalue);
        }
        /* pick-up discovery timeout from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_DISCOVERY_TTL,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_TTL,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error) THROW(CONFIG_SET_DAEMON_DISCOVERY_TTL_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_DISCOVERY_TTL, ivalue));
            flom_config_set_discovery_ttl(config, ivalue);
        }
        /* pick-up tcp_keepalive_time from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error)
                THROW(CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME, ivalue));
            flom_config_set_tcp_keepalive_time(config, ivalue);
        }
        /* pick-up tcp_keepalive_intvl from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error)
                THROW(CONFIG_SET_NETWORK_TCP_KEEPALIVE_INTVL_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL, ivalue));
            flom_config_set_tcp_keepalive_intvl(config, ivalue);
        }
        /* pick-up tcp_keepalive_probes from configuration */
        ivalue = g_key_file_get_integer(gkf, FLOM_CONFIG_GROUP_NETWORK,
                                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES,
                                        &error);
        if (NULL != error) {
            int throw_error = FALSE;
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES,
                        error->code,
                        error->message));
            if (G_KEY_FILE_ERROR_KEY_NOT_FOUND != error->code) {
                print_file_name = throw_error = TRUE;
                g_print("%s\n", error->message);
            }
            g_error_free(error);
            error = NULL;
            if (throw_error)
                THROW(CONFIG_SET_NETWORK_TCP_KEEPALIVE_PROBES_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES, ivalue));
            flom_config_set_tcp_keepalive_probes(config, ivalue);
        }
        /* pick-up TLS peer certificate */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TLS,
                         FLOM_CONFIG_KEY_TLS_CERTIFICATE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CERTIFICATE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CERTIFICATE, value));
            if (FLOM_RC_OK != (ret_cod = flom_config_set_tls_certificate(
                                   config, value)))
                THROW(CONFIG_SET_TLS_CERTIFICATE_ERROR);
            g_free(value);
            value = NULL;
        }
        /* pick-up TLS peer private key */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TLS,
                         FLOM_CONFIG_KEY_TLS_PRIVATE_KEY, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_PRIVATE_KEY,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_PRIVATE_KEY, value));
            if (FLOM_RC_OK != (ret_cod = flom_config_set_tls_private_key(
                                   config, value)))
                THROW(CONFIG_SET_TLS_PRIVATE_KEY_ERROR);
            g_free(value);
            value = NULL;
        }
        /* pick-up TLS ca certificate */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TLS,
                         FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CA_CERTIFICATE, value));
            if (FLOM_RC_OK != (ret_cod = flom_config_set_tls_ca_certificate(
                                   config, value)))
                THROW(CONFIG_SET_TLS_CA_CERTIFICATE_ERROR);
            g_free(value);
            value = NULL;
        }
        /* pick-up TLS check peer id from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_TLS,
                         FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_bool_value_t fbv;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_TLS,
                        FLOM_CONFIG_KEY_TLS_CHECK_PEER_ID, value));
            if (FLOM_BOOL_INVALID == (
                    fbv = flom_bool_value_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_tls_check_peer_id(config, fbv);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_TLS_CHECK_PEER_ID_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_KEY_FILE_NEW_ERROR:
                ret_cod = FLOM_RC_G_KEY_FILE_NEW_ERROR;
                break;
            case G_KEY_FILE_LOAD_FROM_FILE_ERROR:
                ret_cod = FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR;
                break;
            case CONFIG_SET_APPENDTRACEFILE_ERROR:
            case CONFIG_SET_VERBOSE_ERROR:
            case CONFIG_SET_RESOURCE_NAME_ERROR:
            case CONFIG_SET_RESOURCE_WAIT_ERROR:
            case CONFIG_SET_RESOURCE_TIMEOUT_ERROR:
            case CONFIG_SET_RESOURCE_QUANTITY_ERROR:
            case CONFIG_SET_RESOURCE_LOCK_MODE_ERROR:
            case CONFIG_SET_RESOURCE_CREATE_ERROR:
            case CONFIG_SET_RESOURCE_IDLE_LIFESPAN_ERROR:
            case CONFIG_SET_SOCKET_NAME_ERROR:
            case CONFIG_SET_DAEMON_LIFESPAN_ERROR:
            case CONFIG_SET_DAEMON_UNICAST_PORT_ERROR:
            case CONFIG_SET_DAEMON_DISCOVERY_ATTEMPTS_ERROR:
            case CONFIG_SET_DAEMON_DISCOVERY_TIMEOUT_ERROR:
            case CONFIG_SET_DAEMON_DISCOVERY_TTL_ERROR:
            case CONFIG_SET_MONITOR_IGNORED_SIGNALS_ERROR:
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR:
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_INTVL_ERROR:
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_PROBES_ERROR:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case CONFIG_SET_TLS_CERTIFICATE_ERROR:
            case CONFIG_SET_TLS_PRIVATE_KEY_ERROR:
            case CONFIG_SET_TLS_CA_CERTIFICATE_ERROR:
                break;
            case CONFIG_SET_TLS_CHECK_PEER_ID_ERROR:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (print_file_name)
        g_print("ERROR: invalid configuration found in file '%s'\n",
                config_file_name);
    if (NULL != value)
        g_free(value);
    if (NULL != list)
        g_strfreev(list);
    if (NULL != gkf)
        g_key_file_free(gkf);
    FLOM_TRACE(("flom_config_init_load/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_config_clone(flom_config_t *config)
{
    enum Exception { NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_config_clone\n"));
    TRY {
        /* target object can't be NULL! */
        if (NULL == config)
            THROW(NULL_OBJECT);
        /* cloning the whole struct */
        memcpy(config, &global_config, sizeof(flom_config_t));
        /* duplicating dynamically allocated objects (strings) */
        config->daemon_trace_file = g_strdup(global_config.daemon_trace_file);
        config->command_trace_file =
            g_strdup(global_config.command_trace_file);
        config->resource_name = g_strdup(global_config.resource_name);
        config->socket_name = g_strdup(global_config.socket_name);
        config->unicast_address = g_strdup(global_config.unicast_address);
        config->multicast_address = g_strdup(global_config.multicast_address);
        config->network_interface = g_strdup(global_config.network_interface);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_config_clone/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_config_set_socket_name(flom_config_t *config,
                                const gchar *socket_name) {
    FLOM_TRACE(("flom_config_set_socket_name(%s)\n", socket_name));
    /* default config object */
    if (NULL == config)
        config = &global_config;
    if (NULL != socket_name && LOCAL_SOCKET_SIZE <= strlen(socket_name))
        return FLOM_RC_BUFFER_OVERFLOW;
    g_free(config->socket_name);
    config->socket_name = g_strdup(socket_name);
    return FLOM_RC_OK;
}



void flom_config_set_daemon_trace_file(
    flom_config_t *config, const gchar *daemon_trace_file)
{
    FLOM_TRACE(("flom_config_set_daemon_trace_file(%s)\n", daemon_trace_file));
    /* default config object */
    if (NULL == config)
        config = &global_config;
    if (NULL != config->daemon_trace_file)
        g_free(config->daemon_trace_file);
    config->daemon_trace_file = g_strdup(daemon_trace_file);
}



void flom_config_set_command_trace_file(
    flom_config_t *config, const gchar *command_trace_file) {
    FLOM_TRACE(("flom_config_set_command_trace_file(%s)\n",
                command_trace_file));
    /* default config object */
    if (NULL == config)
        config = &global_config;
    if (NULL != config->command_trace_file)
        g_free(config->command_trace_file);
    config->command_trace_file = g_strdup(command_trace_file);
}



int flom_config_set_resource_name(flom_config_t *config,
                                  const gchar *resource_name)
{
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int command_line = FALSE;
    flom_rsrc_type_t resource_type;
    
    FLOM_TRACE(("flom_config_set_resource_name(%s)\n", resource_name));
    /* default config object */
    if (NULL == config) {
        config = &global_config;
        command_line = TRUE;
    }
    /* check resource name is not the default (reserved) */
    if (!strncmp(resource_name, DEFAULT_RESOURCE_NAME,
                 sizeof(DEFAULT_RESOURCE_NAME))) {
        if (command_line)
            g_print("ERROR: '%s' is a reserved resource name and can not be "
                    "set by user\n", resource_name);
        ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
    } else if (FLOM_RSRC_TYPE_NULL == (
                   resource_type = flom_rsrc_get_type(resource_name))) {
        FLOM_TRACE(("flom_config_set_resource_name: invalid resource "
                    "name '%s'\n", resource_name));
        if (command_line)
            g_print("ERROR: '%s' is not a valid name for a resource\n",
                    resource_name);
        ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
    } else {
        if (NULL != config->resource_name)
            g_free(config->resource_name);
        config->resource_name = g_strdup(resource_name);
        ret_cod = FLOM_RC_OK;
    }
    /* check if the resource can support transactions */
    config->resource_transactional =
        flom_rsrc_get_transactional(resource_name);
    return ret_cod;
}



void flom_config_set_resource_create(flom_config_t *config, int value)
{
    if (NULL == config)
        global_config.resource_create = value;
    else
        config->resource_create = value;
}



void flom_config_set_resource_timeout(flom_config_t *config, gint timeout)
{
    if (NULL == config)
        global_config.resource_timeout = timeout;
    else
        config->resource_timeout = timeout;
}



void flom_config_set_resource_quantity(flom_config_t *config, gint value)
{
    if (0 > value) value = -value;
    if (0 != value) {
        if (NULL == config)
            global_config.resource_quantity = value;
        else
            config->resource_quantity = value;
    }
}



void flom_config_set_lock_mode(flom_config_t *config,
                               flom_lock_mode_t lock_mode)
{
    if (NULL == config)
        global_config.lock_mode = lock_mode;
    else
        config->lock_mode = lock_mode;
}



void flom_config_set_resource_idle_lifespan(flom_config_t *config, gint value)
{
    if (0 > value) value = -value;
    if (0 != value) {
        if (NULL == config)
            global_config.resource_idle_lifespan = value;
        else
            config->resource_idle_lifespan = value;
    }
}



void flom_config_set_unicast_address(flom_config_t *config,
                                     const gchar *address)
{
    if (NULL == config) {
        g_free(global_config.unicast_address);
        global_config.unicast_address = g_strdup(address);
    } else {
        g_free(config->unicast_address);
        config->unicast_address = g_strdup(address);
    }
}



void flom_config_set_unicast_port(flom_config_t *config, gint port)
{
    if (0 > port) port = -port;
    if (NULL == config)
        global_config.unicast_port = port;
    else
        config->unicast_port = port;
}



void flom_config_set_multicast_address(flom_config_t *config,
                                       const gchar *address)
{
    if (NULL == config) {
        g_free(global_config.multicast_address);
        global_config.multicast_address = g_strdup(address);
    } else {
        g_free(config->multicast_address);
        config->multicast_address = g_strdup(address);
    }
}



void flom_config_set_multicast_port(flom_config_t *config, gint port)
{
    if (0 > port) port = -port;
    if (NULL == config)
        global_config.multicast_port = port;
    else
        config->multicast_port = port;
}



void flom_config_set_ignored_signals(flom_config_t *config, gchar **list)
{
    int i, j;
    sigset_t *tmp;
    
    if (NULL == list) {
        FLOM_TRACE(("flom_config_set_ignored_signals: list is NULL!\n"));
        return;
    }
    if (NULL == config)
        tmp = &global_config.ignored_signals;
    else
        tmp = &config->ignored_signals;
    /* reset the signal set */
    sigemptyset(tmp);
    /* iterate the list of strings */
    for (i=0; list[i]!=NULL; ++i) {
        int signal_number;
        FLOM_TRACE(("flom_config_set_ignored_signals: list[%d]='%s'\n",
                    i, list[i]));
        /* check if it's a valid signal number */
        signal_number = (int)strtol(list[i], NULL, 10);
        if (signal_number<=0 || signal_number>=SIGNAL_STRING_ARRAY_SIZE)
            /* check if it's a valid string */
            for (j=1; j<SIGNAL_STRING_ARRAY_SIZE; ++j) {
                if (0 == strcmp(SIGNAL_STRING_ARRAY[j], list[i])) {
                    signal_number = j;
                    break;
                } /* if (0 == strcmp(SIGNAL_STRING_ARRAY[j], list[i])) */    
            } /* for (j=0; j<sizeof(SIGNAL_STRING_ARRAY)/ */
        if (signal_number>0 && signal_number<SIGNAL_STRING_ARRAY_SIZE) {
            if (0 != sigaddset(tmp, signal_number)) {
                FLOM_TRACE(("flom_config_set_ignored_signals/"
                            "sigaddset: errno=%d ('%s')\n",
                            errno, strerror(errno)));
            } else {
                FLOM_TRACE(("flom_config_set_ignored_signals: added "
                            "signal %d:%s to the exclusion list\n",
                            signal_number,
                            SIGNAL_STRING_ARRAY[signal_number]));
            }
        }
    } /* for (i=0; list[i]!=NULL; ++i) */
}



gchar *flom_config_get_ignored_signals_str(const flom_config_t *config)
{
    const sigset_t *sigset;
    int j;
    gchar *ret = NULL, *tmp = NULL;
    int first = TRUE;
    
    if (NULL == config)
        sigset = &global_config.ignored_signals;
    else
        sigset = &config->ignored_signals;
    ret = g_strconcat("", NULL);
    for (j=1; j<SIGNAL_STRING_ARRAY_SIZE; ++j) {
        if (sigismember(sigset, j)) {
            if (first) {
                first = FALSE;
                tmp = g_strconcat(ret, SIGNAL_STRING_ARRAY[j], NULL);
            } else
                tmp = g_strconcat(ret, ";", SIGNAL_STRING_ARRAY[j], NULL);
            g_free(ret);
            ret = tmp;
        }
    } /* for (j=1; j<sizeof(SIGNAL_STRING_ARRAY)/sizeof(const gchar *); ++j) */
    return ret;
}



void flom_config_print_signal_list(void) 
{
    int i=0,j;
    for (j=1; j<SIGNAL_STRING_ARRAY_SIZE; ++j) {
        g_print("%d) %s\t", j, SIGNAL_STRING_ARRAY[j]);
        if (++i%4 == 0)
            g_print("\n");
    }
    g_print("\n");
}



int flom_config_set_network_interface(flom_config_t *config,
                                      const gchar *value)
{
    enum Exception { GETIFADDRS_ERROR
                     , INVALID_IPV6_NETWORK_INTERFACE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
#ifdef HAVE_GETIFADDRS
    /* getifaddrs is not POSIX and we can not be sure it's available */
    struct ifaddrs *ifaddr = NULL, *ifa;
#endif /* HAVE_GETIFADDRS */
    
    FLOM_TRACE(("flom_config_set_network_interface\n"));
    TRY {
        int found = FALSE;
        
        /* set sin6_scope_id derived from network_interface */
#ifdef HAVE_GETIFADDRS
        /* getifaddrs is not POSIX and we can not be sure it's available */
        if (-1 == getifaddrs(&ifaddr)) {
            FLOM_TRACE(("flom_config_set_network_interface/getifaddrs(): "
                        "errno=%d '%s'\n", errno, strerror(errno)));
            THROW(GETIFADDRS_ERROR);
        } else {
            FLOM_TRACE_IFADDRS("flom_config_set_network_interface/"
                               "getifaddrs():", ifaddr);
        }
        /* search interface for IPv6 networking */
        for (ifa=ifaddr; NULL!=ifa; ifa=ifa->ifa_next) {
            if (NULL == ifa->ifa_addr) continue;
            if (AF_INET6 != ifa->ifa_addr->sa_family) continue;
            if (0 == strcmp(value, ifa->ifa_name)) {
                struct sockaddr_in6 sa6;
                memcpy(&sa6, ifa->ifa_addr, sizeof(sa6));
                FLOM_TRACE(("flom_config_set_network_interface: found "
                            "interface ifa_name='%s', sin6_scope_id=%u\n",
                            ifa->ifa_name, sa6.sin6_scope_id));
                FLOM_TRACE_SOCKADDR("flom_config_set_network_interface: "
                                    "associated IPv6 address is ",
                                    (struct sockaddr *)&sa6, sizeof(sa6));
                if (NULL == config)
                    global_config.sin6_scope_id = sa6.sin6_scope_id;
                else
                    config->sin6_scope_id = sa6.sin6_scope_id;
                found = TRUE;
                break;
            } /* if (0 == strcmp(value, ifa->ifa_name)) { */
        } /* for (ifa=ifaddr */
        if (!found) {
            FLOM_TRACE(("flom_config_set_network_interface: '%s' is not a "
                        "valid network interface for IPv6 adresses\n",
                        value));
            THROW(INVALID_IPV6_NETWORK_INTERFACE);
        }
#endif /* HAVE_GETIFADDRS */
        if (NULL == config) {
            g_free(global_config.network_interface);
            global_config.network_interface = g_strdup(value);
        } else {
            g_free(config->network_interface);
            config->network_interface = g_strdup(value);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETIFADDRS_ERROR:
                ret_cod = FLOM_RC_GETIFADDRS_ERROR;
                break;
            case INVALID_IPV6_NETWORK_INTERFACE:
                ret_cod = FLOM_RC_INVALID_IPV6_NETWORK_INTERFACE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
#ifdef HAVE_FREEIFADDRS
    /* freeifaddrs is not POSIX and we can not be sure it's available */
    if (NULL != ifaddr)
        freeifaddrs(ifaddr);
#endif /* HAVE_FREEIFADDRS */
    FLOM_TRACE(("flom_config_set_network_interface/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_config_set_sin6_scope_id(flom_config_t *config, uint32_t value)
{
    enum Exception { GETIFADDRS_ERROR
                     , INVALID_IPV6_SIN6_SCOPE_ID
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
#ifdef HAVE_GETIFADDRS
    /* getifaddrs is not POSIX and we can not be sure it's available */
    struct ifaddrs *ifaddr = NULL, *ifa;
#endif /* HAVE_GETIFADDRS */
    
    FLOM_TRACE(("flom_config_set_sin6_scope_id\n"));
    TRY {
        int found = FALSE;
        
        /* pick-up network_interface using sin6_scope_id as the key*/
#ifdef HAVE_GETIFADDRS
        /* getifaddrs is not POSIX and we can not be sure it's available */
        if (-1 == getifaddrs(&ifaddr)) {
            FLOM_TRACE(("flom_config_set_sin6_scope_id/getifaddrs(): "
                        "errno=%d '%s'\n", errno, strerror(errno)));
            THROW(GETIFADDRS_ERROR);
        } else {
            FLOM_TRACE_IFADDRS("flom_config_set_sin6_scope_id/"
                               "getifaddrs(): ", ifaddr);
        }
        /* search interface for IPv6 networking */
        for (ifa=ifaddr; NULL!=ifa; ifa=ifa->ifa_next) {
            struct sockaddr_in6 sa6;
            
            if (NULL == ifa->ifa_addr) continue;
            if (AF_INET6 != ifa->ifa_addr->sa_family) continue;
            memcpy(&sa6, ifa->ifa_addr, sizeof(sa6));
            if (value == sa6.sin6_scope_id) {
                FLOM_TRACE(("flom_config_set_sin6_scope_id: found "
                            "interface ifa_name='%s', sin6_scope_id=%u\n",
                            ifa->ifa_name, sa6.sin6_scope_id));
                FLOM_TRACE_SOCKADDR("flom_config_set_sin6_scope_id: "
                                    "associated IPv6 address is ",
                                    (struct sockaddr *)&sa6, sizeof(sa6));
                if (NULL == config) {
                    g_free(global_config.network_interface);
                    global_config.network_interface = g_strdup(ifa->ifa_name);
                    global_config.sin6_scope_id = sa6.sin6_scope_id;
                } else {
                    config->sin6_scope_id = sa6.sin6_scope_id;
                    g_free(config->network_interface);
                    config->network_interface = g_strdup(ifa->ifa_name);
                }
                found = TRUE;
                break;
            } /* if (0 == strcmp(value, ifa->ifa_name)) { */
        } /* for (ifa=ifaddr */
        if (!found) {
            FLOM_TRACE(("flom_config_set_sin6_scope_id: a valid network "
                        "interface has been found for sin6_scope_id='%u'\n",
                        value));
            THROW(INVALID_IPV6_SIN6_SCOPE_ID);
        }
#endif /* HAVE_GETIFADDRS */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETIFADDRS_ERROR:
                ret_cod = FLOM_RC_GETIFADDRS_ERROR;
                break;
            case INVALID_IPV6_SIN6_SCOPE_ID:
                ret_cod = FLOM_RC_INVALID_IPV6_NETWORK_INTERFACE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
#ifdef HAVE_FREEIFADDRS
    /* freeifaddrs is not POSIX and we can not be sure it's available */
    if (NULL != ifaddr)
        freeifaddrs(ifaddr);
#endif /* HAVE_FREEIFADDRS */
    FLOM_TRACE(("flom_config_set_sin6_scope_id/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_config_set_discovery_attempts(flom_config_t *config, gint attempts)
{
    if (0 > attempts) attempts = -attempts;
    if (0 == attempts) attempts = 1;
    if (NULL == config)
        global_config.discovery_attempts = attempts;
    else
        config->discovery_attempts = attempts;
}



void flom_config_set_discovery_timeout(flom_config_t *config, gint timeout)
{
    if (0 > timeout) timeout = -timeout;
    if (NULL == config)
        global_config.discovery_timeout = timeout;
    else
        config->discovery_timeout = timeout;
}



void flom_config_set_discovery_ttl(flom_config_t *config, gint ttl)
{
    if (0 > ttl) ttl = -ttl;
    if (0 == ttl) ttl = 1;
    if (NULL == config)
        global_config.discovery_ttl = ttl;
    else
        config->discovery_ttl = ttl;
}



void flom_config_set_tcp_keepalive_time(flom_config_t *config, gint value)
{
    if (0 > value) value = -value;
    if (NULL == config)
        global_config.tcp_keepalive_time = value;
    else
        config->tcp_keepalive_time = value;
}



void flom_config_set_tcp_keepalive_intvl(flom_config_t *config, gint value)
{
    if (0 > value) value = -value;
    if (NULL == config)
        global_config.tcp_keepalive_intvl = value;
    else
        config->tcp_keepalive_intvl = value;
}



void flom_config_set_tcp_keepalive_probes(flom_config_t *config, gint value)
{
    if (0 > value) value = -value;
    if (NULL == config)
        global_config.tcp_keepalive_probes = value;
    else
        config->tcp_keepalive_probes = value;
}



int flom_config_set_tls_certificate(flom_config_t *config, const gchar *value)
{
    char resolved_path[PATH_MAX+1];

    if (NULL == realpath(value, resolved_path))
        return FLOM_RC_REALPATH_ERROR;
    if (NULL == config) {
        g_free(global_config.tls_certificate);
        global_config.tls_certificate = g_strdup(resolved_path);
    } else {
        g_free(config->tls_certificate);
        config->tls_certificate = g_strdup(resolved_path);
    }
    return FLOM_RC_OK;
}



int flom_config_set_tls_private_key(flom_config_t *config,
                                     const gchar *value)
{
    char resolved_path[PATH_MAX+1];

    if (NULL == realpath(value, resolved_path))
        return FLOM_RC_REALPATH_ERROR;
    if (NULL == config) {
        g_free(global_config.tls_private_key);
        global_config.tls_private_key = g_strdup(resolved_path);
    } else {
        g_free(config->tls_private_key);
        config->tls_private_key = g_strdup(resolved_path);
    }
    return FLOM_RC_OK;
}



int flom_config_set_tls_ca_certificate(flom_config_t *config,
                                       const gchar *value)
{
    char resolved_path[PATH_MAX+1];

    if (NULL == realpath(value, resolved_path))
        return FLOM_RC_REALPATH_ERROR;
    if (NULL == config) {
        g_free(global_config.tls_ca_certificate);
        global_config.tls_ca_certificate = g_strdup(resolved_path);
    } else {
        g_free(config->tls_ca_certificate);
        config->tls_ca_certificate = g_strdup(resolved_path);
    }
    return FLOM_RC_OK;
}



