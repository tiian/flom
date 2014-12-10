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
#include <config.h>



#ifdef HAVE_STRING_H
/* strcasestr is not POSIX standard and needs GNU extensions... */
# define _GNU_SOURCE
# include <string.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
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
const gchar *FLOM_CONFIG_KEY_VERBOSE = _CONFIG_KEY_VERBOSE;
const gchar *FLOM_CONFIG_GROUP_RESOURCE = _CONFIG_GROUP_RESOURCE;
const gchar *FLOM_CONFIG_KEY_CREATE = _CONFIG_KEY_CREATE;
const gchar *FLOM_CONFIG_KEY_NAME = _CONFIG_KEY_NAME;
const gchar *FLOM_CONFIG_KEY_WAIT = _CONFIG_KEY_WAIT;
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
const gchar *FLOM_CONFIG_GROUP_NETWORK = _CONFIG_GROUP_NETWORK;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS = _CONFIG_KEY_DISCOVERY_ATTEMPTS;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT = _CONFIG_KEY_DISCOVERY_TIMEOUT;
const gchar *FLOM_CONFIG_KEY_DISCOVERY_TTL = _CONFIG_KEY_DISCOVERY_TTL;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME = _CONFIG_KEY_TCP_KEEPALIVE_TIME;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL = _CONFIG_KEY_TCP_KEEPALIVE_INTVL;
const gchar *FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES = _CONFIG_KEY_TCP_KEEPALIVE_PROBES;



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



void flom_config_reset()
{
    FLOM_TRACE(("flom_config_reset\n"));
    global_config.daemon_trace_file = NULL;
    global_config.command_trace_file = NULL;
    global_config.verbose = FALSE;
    global_config.resource_name = g_strdup(DEFAULT_RESOURCE_NAME);
    global_config.resource_wait = TRUE;
    global_config.resource_create = TRUE;
    global_config.resource_timeout = -1;
    global_config.resource_quantity = 1;
    global_config.lock_mode = FLOM_LOCK_MODE_EX;
    global_config.resource_idle_lifespan = 0;
    global_config.socket_name = NULL;
    global_config.daemon_lifespan = _DEFAULT_DAEMON_LIFESPAN;
    global_config.unicast_address = NULL;
    global_config.unicast_port = _DEFAULT_DAEMON_PORT;
    global_config.multicast_address = NULL;
    global_config.multicast_port = _DEFAULT_DAEMON_PORT;
    global_config.discovery_attempts = _DEFAULT_DISCOVERY_ATTEMPTS;
    global_config.discovery_timeout = _DEFAULT_DISCOVERY_TIMEOUT;
    global_config.discovery_ttl = _DEFAULT_DISCOVERY_TTL;
    global_config.tcp_keepalive_time = _DEFAULT_TCP_KEEPALIVE_TIME;
    global_config.tcp_keepalive_intvl = _DEFAULT_TCP_KEEPALIVE_INTVL;
    global_config.tcp_keepalive_probes = _DEFAULT_TCP_KEEPALIVE_PROBES;
}



int flom_config_check()
{
    enum Exception { INVALID_OPTION
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_config_check\n"));
    TRY {
        flom_rsrc_type_t frt;
        
        /* check if configuration is for LOCAL and for NETWORK: it's not
           acceptable */
        if (NULL != flom_config_get_socket_name() &&
            (NULL != flom_config_get_unicast_address() ||
             NULL != flom_config_get_multicast_address())) {
            g_print("ERROR: flom can not be configured for local "
                    "(UNIX socket) and network (TCP-UDP/IP) communication "
                    "at the same time.\n");
            THROW(INVALID_OPTION);
        }
        /* if neither local nor network communication were configured,
           use default local */
        if (NULL == flom_config_get_socket_name() &&
            NULL == flom_config_get_unicast_address() &&
            NULL == flom_config_get_multicast_address()) {
            struct passwd *pwd = NULL;
            char *login = NULL;
            /* set UNIX socket name */
            global_config.socket_name = g_malloc(LOCAL_SOCKET_SIZE);
            pwd = getpwuid(getuid());
            if (NULL == pwd || NULL == pwd->pw_name)
                login = "nobody";
            else
                login = pwd->pw_name;
            snprintf(global_config.socket_name, LOCAL_SOCKET_SIZE,
                     "/tmp/flom-%s", login);
        }
        /* check options related to resource type */
        frt = flom_rsrc_get_type(flom_config_get_resource_name());
        /* check lock mode */
        if (FLOM_LOCK_MODE_EX != flom_config_get_lock_mode() &&
            FLOM_RSRC_TYPE_SIMPLE != frt && FLOM_RSRC_TYPE_HIER != frt) {
            if (flom_config_get_verbose())
                g_warning("This resource type (%d) support only exclusive "
                          "lock mode; specified value (%d) will be ignored\n",
                          frt, flom_config_get_lock_mode());
            flom_config_set_lock_mode(FLOM_LOCK_MODE_EX);
        }
        /* check quantity */
        if (1 != flom_config_get_resource_quantity() &&
            FLOM_RSRC_TYPE_NUMERIC != frt) {
            if (flom_config_get_verbose())
                g_warning("This resource type (%d) does not support quantity "
                          "lock option; specified value (%d) will be "
                          "ignored\n",
                          frt, flom_config_get_resource_quantity());
            flom_config_set_resource_quantity(1);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_OPTION:
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



void flom_config_print()
{
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_DAEMONTRACEFILE,
            NULL == flom_config_get_daemon_trace_file() ? FLOM_EMPTY_STRING :
            flom_config_get_daemon_trace_file());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_COMMANDTRACEFILE,
            NULL == flom_config_get_command_trace_file() ? FLOM_EMPTY_STRING :
            flom_config_get_command_trace_file());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_VERBOSE, flom_config_get_verbose());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_NAME,
            NULL == flom_config_get_resource_name() ? FLOM_EMPTY_STRING :
            flom_config_get_resource_name());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_WAIT, flom_config_get_resource_wait());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_TIMEOUT, flom_config_get_resource_timeout());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_QUANTITY, flom_config_get_resource_quantity());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_LOCK_MODE, flom_config_get_lock_mode());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_CREATE, flom_config_get_resource_create());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_IDLE_LIFESPAN,
            flom_config_get_resource_idle_lifespan());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_SOCKET_NAME,
            NULL == flom_config_get_socket_name() ? FLOM_EMPTY_STRING :
            flom_config_get_socket_name());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_LIFESPAN, flom_config_get_lifespan());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_UNICAST_ADDRESS,
            NULL == flom_config_get_unicast_address() ? FLOM_EMPTY_STRING :
            flom_config_get_unicast_address());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_UNICAST_PORT, flom_config_get_unicast_port());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_MULTICAST_ADDRESS,
            NULL == flom_config_get_multicast_address() ? FLOM_EMPTY_STRING :
            flom_config_get_multicast_address());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_DAEMON,
            FLOM_CONFIG_KEY_MULTICAST_PORT, flom_config_get_multicast_port());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_ATTEMPTS,
            flom_config_get_discovery_attempts());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_TIMEOUT,
            flom_config_get_discovery_timeout());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_DISCOVERY_TTL,
            flom_config_get_discovery_ttl());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME,
            flom_config_get_tcp_keepalive_time());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_INTVL,
            flom_config_get_tcp_keepalive_intvl());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_NETWORK,
            FLOM_CONFIG_KEY_TCP_KEEPALIVE_PROBES,
            flom_config_get_tcp_keepalive_probes());
}



void flom_config_free()
{
    FLOM_TRACE(("flom_config_free\n"));
    if (NULL != global_config.daemon_trace_file) {
        g_free(global_config.daemon_trace_file);
        global_config.daemon_trace_file = NULL;
    }
    if (NULL != global_config.command_trace_file) {
        g_free(global_config.command_trace_file);
        global_config.command_trace_file = NULL;
    }
    if (NULL != global_config.resource_name) {
        g_free(global_config.resource_name);
        global_config.resource_name = NULL;
    }
}



int flom_config_init(const char *custom_config_filename)
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
        
        /* building system configuration filename */
        strcpy(system_config_filename, FLOM_INSTALL_SYSCONFDIR);
        strcat(system_config_filename, FLOM_DIR_FILE_SEPARATOR);
        strcat(system_config_filename, FLOM_SYSTEM_CONFIG_FILENAME);
        assert(sizeof(system_config_filename)>strlen(system_config_filename));
        ret_cod = flom_config_init_load(system_config_filename);
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
        ret_cod = flom_config_init_load(user_config_filename);
        if (FLOM_RC_OK != ret_cod &&
            FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR != ret_cod)
            THROW(CONFIG_INIT_LOAD_ERROR2);
        g_free(user_config_filename);
        user_config_filename = NULL;
        /* using custom config filename */
        if (NULL != custom_config_filename) {
            ret_cod = flom_config_init_load(custom_config_filename);
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



int flom_config_init_load(const char *config_file_name)
{
    enum Exception { G_KEY_FILE_NEW_ERROR
                     , G_KEY_FILE_LOAD_FROM_FILE_ERROR
                     , CONFIG_SET_VERBOSE_ERROR
                     , CONFIG_SET_RESOURCE_NAME_ERROR
                     , CONFIG_SET_RESOURCE_WAIT_ERROR
                     , CONFIG_SET_RESOURCE_TIMEOUT_ERROR
                     , CONFIG_SET_RESOURCE_QUANTITY_ERROR
                     , CONFIG_SET_RESOURCE_LOCK_MODE_ERROR
                     , CONFIG_SET_RESOURCE_CREATE_ERROR
                     , CONFIG_SET_RESOURCE_IDLE_LIFESPAN_ERROR
                     , CONFIG_SET_SOCKET_NAME_ERROR
                     , CONFIG_SET_DAEMON_LIFESPAN_ERROR
                     , CONFIG_SET_DAEMON_UNICAST_PORT_ERROR
                     , CONFIG_SET_DAEMON_MULTICAST_PORT_ERROR
                     , CONFIG_SET_DAEMON_DISCOVERY_ATTEMPTS_ERROR
                     , CONFIG_SET_DAEMON_DISCOVERY_TIMEOUT_ERROR
                     , CONFIG_SET_DAEMON_DISCOVERY_TTL_ERROR
                     , CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR
                     , CONFIG_SET_NETWORK_TCP_KEEPALIVE_INTVL_ERROR
                     , CONFIG_SET_NETWORK_TCP_KEEPALIVE_PROBES_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int print_file_name = FALSE;

    GKeyFile *gkf = NULL;
    GError *error = NULL;
    gchar *value = NULL;
    gint ivalue = 0;
    
    FLOM_TRACE(("flom_config_init_load\n"));
    TRY {
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
            flom_config_set_daemon_trace_file(value);
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
            flom_config_set_command_trace_file(value);
            value = NULL;
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
                flom_config_set_verbose(fbv);
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
            if (FLOM_RC_OK != (ret_cod =
                               flom_config_set_resource_name(value))) {
                print_file_name = TRUE;
                THROW(CONFIG_SET_RESOURCE_NAME_ERROR);
            } else {
                value = NULL;
            }
        }
        /* pick-up resource wait from configuration */
        if (NULL == (value = g_key_file_get_string(
                         gkf, FLOM_CONFIG_GROUP_RESOURCE,
                         FLOM_CONFIG_KEY_WAIT, &error))) {
            FLOM_TRACE(("flom_config_init_load/g_key_file_get_string"
                        "(...,%s,%s,...): code=%d, message='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_WAIT,
                        error->code,
                        error->message));
            g_error_free(error);
            error = NULL;
        } else {
            int throw_error = FALSE;
            flom_bool_value_t fbv;
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%s'\n",
                        FLOM_CONFIG_GROUP_RESOURCE,
                        FLOM_CONFIG_KEY_WAIT, value));
            if (FLOM_BOOL_INVALID == (
                    fbv = flom_bool_value_retrieve(value))) {
                print_file_name = TRUE;
                throw_error = TRUE;
            } else {
                flom_config_set_resource_wait(fbv);
            }
            g_free(value);
            value = NULL;
            if (throw_error) THROW(CONFIG_SET_RESOURCE_WAIT_ERROR);
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
            flom_config_set_resource_timeout(ivalue);
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
            flom_config_set_resource_quantity(ivalue);
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
                flom_config_set_lock_mode(flm);
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
                flom_config_set_resource_create(fbv);
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
            flom_config_set_resource_idle_lifespan(ivalue);
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
            if (FLOM_RC_OK != (ret_cod = flom_config_set_socket_name(value))) {
                print_file_name = TRUE;
                THROW(CONFIG_SET_SOCKET_NAME_ERROR);
            } else
                value = NULL;
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
            flom_config_set_lifespan(ivalue);
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
            flom_config_set_unicast_address(value);
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
            flom_config_set_unicast_port(ivalue);
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
            flom_config_set_multicast_address(value);
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
            flom_config_set_multicast_port(ivalue);
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
            flom_config_set_discovery_attempts(ivalue);
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
            flom_config_set_discovery_timeout(ivalue);
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
            flom_config_set_discovery_ttl(ivalue);
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
            if (throw_error) THROW(CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR);
        } else {
            FLOM_TRACE(("flom_config_init_load: %s[%s]='%d'\n",
                        FLOM_CONFIG_GROUP_NETWORK,
                        FLOM_CONFIG_KEY_TCP_KEEPALIVE_TIME, ivalue));
            flom_config_set_tcp_keepalive_time(ivalue);
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
            flom_config_set_tcp_keepalive_intvl(ivalue);
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
            flom_config_set_tcp_keepalive_probes(ivalue);
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
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_TIME_ERROR:
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_INTVL_ERROR:
            case CONFIG_SET_NETWORK_TCP_KEEPALIVE_PROBES_ERROR:
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
    if (NULL != gkf)
        g_key_file_free(gkf);
    FLOM_TRACE(("flom_config_init_load/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_config_set_socket_name(gchar *socket_name) {
    FLOM_TRACE(("flom_config_set_socket_name(%s)\n", socket_name));
    if (LOCAL_SOCKET_SIZE <= strlen(socket_name))
        return FLOM_RC_BUFFER_OVERFLOW;
    if (NULL != global_config.socket_name)
        g_free(global_config.socket_name);
    global_config.socket_name = socket_name;
    return FLOM_RC_OK;
}



int flom_config_set_resource_name(gchar *resource_name)
{
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_config_set_resource_name(%s)\n", resource_name));
    /* check resource name is not the default (reserved) */
    if (!strncmp(resource_name, DEFAULT_RESOURCE_NAME,
                 sizeof(DEFAULT_RESOURCE_NAME))) {
        g_print("ERROR: '%s' is a reserved resource name and can not be set "
                "by user\n", resource_name);
        ret_cod = FLOM_RC_INVALID_OPTION;
    } else if (FLOM_RSRC_TYPE_NULL == flom_rsrc_get_type(resource_name)) {
        FLOM_TRACE(("flom_config_set_resource_name: invalid resource "
                    "name '%s'\n", resource_name));
        g_print("ERROR: '%s' is not a valid name for a resource\n",
                resource_name);
        ret_cod = FLOM_RC_INVALID_OPTION;
    } else {
        if (NULL != global_config.resource_name)
            g_free(global_config.resource_name);
        global_config.resource_name = resource_name;
        ret_cod = FLOM_RC_OK;
    }
    return ret_cod;
}
