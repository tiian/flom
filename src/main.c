/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
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

#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#define FLOM_TRACE_MODULE FLOM_TRACE_MOD_GENERIC



#include "flom_config.h"
#include "flom_client.h"
#include "flom_conns.h"
#include "flom_debug_features.h"
#include "flom_errors.h"
#include "flom_exec.h"
#include "flom_rsrc.h"
#include "flom_trace.h"



static gboolean print_version = FALSE;
static gboolean verbose = FALSE;
static char *config_file = NULL;
static gchar *socket_name = NULL;
static gchar *resource_name = NULL;
static gint resource_timeout = FLOM_NETWORK_WAIT_TIMEOUT;
static gint resource_quantity = 0;
static gchar *resource_wait = NULL;
static gchar *resource_create = NULL;
static gint resource_idle_lifespan = 0;
static gchar *lock_mode = NULL;
static gint daemon_lifespan = _DEFAULT_DAEMON_LIFESPAN;
static gchar *unicast_address = NULL;
static gint unicast_port = _DEFAULT_DAEMON_PORT;
static gchar *multicast_address = NULL;
static gint multicast_port = _DEFAULT_DAEMON_PORT;
static gchar *network_interface = NULL;
static gint discovery_attempts = _DEFAULT_DISCOVERY_ATTEMPTS;
static gint discovery_timeout = _DEFAULT_DISCOVERY_TIMEOUT;
static gint discovery_ttl = _DEFAULT_DISCOVERY_TTL;
static gint tcp_keepalive_time = _DEFAULT_TCP_KEEPALIVE_TIME;
static gint tcp_keepalive_intvl = _DEFAULT_TCP_KEEPALIVE_INTVL;
static gint tcp_keepalive_probes = _DEFAULT_TCP_KEEPALIVE_PROBES;
static gint quiesce_exit = 0;
static gint immediate_exit = 0;
static gchar *command_trace_file = NULL;
static gchar *daemon_trace_file = NULL;
static gchar *append_trace_file = NULL;
static gchar *debug_feature = NULL;
static gchar **command_argv = NULL;
/* command line options */
static GOptionEntry entries[] =
{
    { "version", 'v', 0, G_OPTION_ARG_NONE, &print_version, "Print package info and exit", NULL },
    { "verbose", 'V', 0, G_OPTION_ARG_NONE, &verbose, "Activate verbose messages", NULL },
    { "config-file", 'c', 0, G_OPTION_ARG_STRING, &config_file, "User configuration file name", NULL },
    { "resource-name", 'r', 0, G_OPTION_ARG_STRING, &resource_name, "Specify the name of the resource to be locked", NULL },
    { "resource-wait", 'w', 0, G_OPTION_ARG_STRING, &resource_wait, "Specify if the command enques when the resource is already locked (accepted values 'yes', 'no')", NULL },
    { "resource-timeout", 'o', 0, G_OPTION_ARG_INT, &resource_timeout, "Specify maximum wait time (milliseconds) if a resource is already locked", NULL },
    { "resource-quantity", 'q', 0, G_OPTION_ARG_INT, &resource_quantity, "Specify how many numeric resources must be locked", NULL },
    { "resource-create", 'e', 0, G_OPTION_ARG_STRING, &resource_create, "Specify if the command can create the resource to lock (accepted values 'yes', 'no')", NULL },
    { "resource-idle-lifespan", 'i', 0, G_OPTION_ARG_INT, &resource_idle_lifespan, "Specify how long (milliseconds) a resource will be kept after usage termination", NULL },
    { "lock-mode", 'l', 0, G_OPTION_ARG_STRING, &lock_mode, "Resource lock mode ('NL', 'CR', 'CW', 'PR', 'PW', 'EX')", NULL },
    { "socket-name", 's', 0, G_OPTION_ARG_STRING, &socket_name, "Daemon/command communication socket name", NULL },
    { "daemon-lifespan", 'd', 0, G_OPTION_ARG_INT, &daemon_lifespan, "Specify minimum lifespan of the flom daemon (if activated)", NULL },
    { "unicast-address", 'a', 0, G_OPTION_ARG_STRING, &unicast_address, "Daemon TCP/IP address", NULL },
    { "unicast-port", 'p', 0, G_OPTION_ARG_INT, &unicast_port, "Daemon TCP/IP port", NULL },
    { "multicast-address", 'A', 0, G_OPTION_ARG_STRING, &multicast_address, "Daemon UDP/IP (multicast) address", NULL },
    { "multicast-port", 'P', 0, G_OPTION_ARG_INT, &multicast_port, "Daemon UDP/IP (multicast) port", NULL },
    { "network-interface", 'n', 0, G_OPTION_ARG_STRING, &network_interface, "Network interface that must be used for IPv6 link local addresses", NULL },
    { "discovery-attempts", 'D', 0, G_OPTION_ARG_INT, &discovery_attempts, "UDP/IP (multicast) max number of requests", NULL },
    { "discovery-timeout", 'I', 0, G_OPTION_ARG_INT, &discovery_timeout, "UDP/IP (multicast) request timeout", NULL },
    { "discovery-ttl", 0, 0, G_OPTION_ARG_INT, &discovery_ttl, "UDP/IP (multicast) hop limit", NULL },
    { "tcp-keepalive-time", 0, 0, G_OPTION_ARG_INT, &tcp_keepalive_time, "Local override for SO_KEEPALIVE feature", NULL },
    { "tcp-keepalive-intvl", 0, 0, G_OPTION_ARG_INT, &tcp_keepalive_intvl, "Local override for SO_KEEPALIVE feature", NULL },
    { "tcp-keepalive-probes", 0, 0, G_OPTION_ARG_INT, &tcp_keepalive_probes, "Local override for SO_KEEPALIVE feature", NULL },
    { "daemon-trace-file", 't', 0, G_OPTION_ARG_STRING, &daemon_trace_file, "Specify daemon (background process) trace file name (absolute path required)", NULL },
    { "command-trace-file", 'T', 0, G_OPTION_ARG_STRING, &command_trace_file, "Specify command (foreground process) trace file name (absolute path required)", NULL },
    { "append-trace-file", 0, 0, G_OPTION_ARG_STRING, &append_trace_file, "Specify if the trace file(s) must be appended or truncated for every execution (accepted values 'yes', 'no')", NULL },
    { "quiesce-exit", 'x', 0, G_OPTION_ARG_NONE, &quiesce_exit, "Start daemon termination completing current requests", NULL },
    { "immediate-exit", 'X', 0, G_OPTION_ARG_NONE, &immediate_exit, "Start daemon termination immediately and interrupting current requests", NULL },
    { "debug-feature", 0, 0, G_OPTION_ARG_STRING, &debug_feature, "Debug execution, specify the debug feature to execute", NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command_argv, "Command must be executed under flom control" },
    { NULL }
};



int main (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *option_context;
    int child_status = 0;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_conn_data_s cd;
    char *locked_element = NULL;

    option_context = g_option_context_new("[-- command to execute]");
    g_option_context_add_main_entries(option_context, entries, NULL);
    if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
        g_printerr("option parsing failed: %s\n", error->message);
        exit(FLOM_ES_GENERIC_ERROR);
    }
    g_option_context_free(option_context);

    if (print_version) {
        g_print("FLoM: Free LOck Manager\n"
                "Copyright (c) 2013-2015, Christian Ferrari; "
                "all rights reserved.\n"
                "License: GPL (GNU Public License) version 2\n"
                "Package name: %s; package version: %s; release date: %s\n"
                "Access http://sourceforge.net/projects/flom/ for "
                "project community activities\n",
                FLOM_PACKAGE_NAME, FLOM_PACKAGE_VERSION, FLOM_PACKAGE_DATE);
        exit(FLOM_ES_OK);
    }

    /* initialize trace destination if necessary */
    FLOM_TRACE_INIT;
    
    /* initialize regular expression table */
    if (FLOM_RC_OK != (ret_cod = global_res_name_preg_init())) {
        g_printerr("global_res_name_preg_init: ret_cod=%d\n", ret_cod);
        exit(FLOM_ES_GENERIC_ERROR);
    }

    /* reset global configuration */
    flom_config_reset(NULL);
    /* initialize configuration with standard system, standard user and
       user customized config files */
    if (FLOM_RC_OK != (ret_cod = flom_config_init(NULL, config_file))) {
        g_printerr("flom_config_init: ret_cod=%d\n", ret_cod);
        exit(FLOM_ES_GENERIC_ERROR);
    }
    /* overrides configuration with command line passed arguments */
    if (NULL != daemon_trace_file)
        flom_config_set_daemon_trace_file(NULL, daemon_trace_file);
    if (NULL != command_trace_file)
        flom_config_set_command_trace_file(NULL, command_trace_file);
    if (verbose)
        flom_config_set_verbose(NULL, verbose);
    if (NULL != resource_name)
        if (FLOM_RC_OK != (ret_cod = flom_config_set_resource_name(
                               NULL, resource_name))) {
            g_printerr("flom_config_set_resource_name: ret_cod=%d\n", ret_cod);
            exit(FLOM_ES_GENERIC_ERROR);
        }
    if (0 > resource_quantity)
        g_printerr("Resource quantity ignored because negative values (%d) "
                   "are meaningless\n", resource_quantity);
    else if (0 < resource_quantity)
        flom_config_set_resource_quantity(NULL, resource_quantity);

    if (NULL != resource_wait) {
        flom_bool_value_t fbv;
        if (FLOM_BOOL_INVALID == (
                fbv = flom_bool_value_retrieve(resource_wait))) {
            g_printerr("resource-wait: '%s' is an invalid value\n",
                       resource_wait);
            exit(FLOM_ES_GENERIC_ERROR);
        }
        flom_config_set_resource_wait(NULL, fbv);
    }
    if (FLOM_NETWORK_WAIT_TIMEOUT != resource_timeout) {
        /* timeout is useless if no wait was specified */
        if (FLOM_BOOL_NO == flom_config_get_resource_wait(NULL))
            g_printerr("Timeout ignored because 'no wait' behavior was "
                       "specified\n");
        else
            flom_config_set_resource_timeout(NULL, resource_timeout);
    }
    if (NULL != lock_mode) {
        flom_lock_mode_t flm;
        if (FLOM_LOCK_MODE_INVALID == (
                flm = flom_lock_mode_retrieve(lock_mode))) {
            g_printerr("lock-mode: '%s' is an invalid value\n", lock_mode);
            exit(FLOM_ES_GENERIC_ERROR);
        }
        flom_config_set_lock_mode(NULL, flm);
    }
    if (NULL != resource_create) {
        flom_bool_value_t fbv;
        if (FLOM_BOOL_INVALID == (
                fbv = flom_bool_value_retrieve(resource_create))) {
            g_print("resource-create: '%s' is an invalid value\n",
                    resource_create);
            exit(FLOM_ES_GENERIC_ERROR);
        }
        flom_config_set_resource_create(NULL, fbv);
    }
    flom_config_set_resource_idle_lifespan(NULL, resource_idle_lifespan);
    if (NULL != socket_name) {
        if (FLOM_RC_OK != (ret_cod = flom_config_set_socket_name(
                               NULL, socket_name))) {
            g_printerr("socket-name: '%s' is an invalid value\n", socket_name);
            g_printerr("flom_client_connect: ret_cod=%d (%s)\n",
                       ret_cod, flom_strerror(ret_cod));
            exit(FLOM_ES_GENERIC_ERROR);
        }
    }
    if (_DEFAULT_DAEMON_LIFESPAN != daemon_lifespan) {
        flom_config_set_lifespan(NULL, daemon_lifespan);
    }
    if (NULL != unicast_address) {
        flom_config_set_unicast_address(NULL, unicast_address);
    }
    if (_DEFAULT_DAEMON_PORT != unicast_port) {
        flom_config_set_unicast_port(NULL, unicast_port);
    }
    if (NULL != multicast_address) {
        flom_config_set_multicast_address(NULL, multicast_address);
    }
    if (_DEFAULT_DAEMON_PORT != multicast_port) {
        flom_config_set_multicast_port(NULL, multicast_port);
    }
    if (NULL != network_interface) {
        flom_config_set_network_interface(NULL, network_interface);
    }
    if (_DEFAULT_DISCOVERY_ATTEMPTS != discovery_attempts) {
        flom_config_set_discovery_attempts(NULL, discovery_attempts);
    }
    if (_DEFAULT_DISCOVERY_TIMEOUT != discovery_timeout) {
        flom_config_set_discovery_timeout(NULL, discovery_timeout);
    }
    if (_DEFAULT_DISCOVERY_TTL != discovery_ttl) {
        flom_config_set_discovery_ttl(NULL, discovery_ttl);
    }
    if (_DEFAULT_TCP_KEEPALIVE_TIME != tcp_keepalive_time) {
        flom_config_set_tcp_keepalive_time(NULL, tcp_keepalive_time);
    }
    if (_DEFAULT_TCP_KEEPALIVE_INTVL != tcp_keepalive_intvl) {
        flom_config_set_tcp_keepalive_intvl(NULL, tcp_keepalive_intvl);
    }
    if (_DEFAULT_TCP_KEEPALIVE_PROBES != tcp_keepalive_probes) {
        flom_config_set_tcp_keepalive_probes(NULL, tcp_keepalive_probes);
    }
    if (NULL != append_trace_file) {
        flom_bool_value_t fbv;
        if (FLOM_BOOL_INVALID == (
                fbv = flom_bool_value_retrieve(append_trace_file))) {
            g_printerr("append-trace-file: '%s' is an invalid value\n",
                       append_trace_file);
            exit(FLOM_ES_GENERIC_ERROR);
        }
        flom_config_set_append_trace_file(NULL, fbv);
    }    
    if (NULL != flom_config_get_command_trace_file(NULL))
        /* change trace destination if necessary */
        FLOM_TRACE_REOPEN(flom_config_get_command_trace_file(NULL),
                          flom_config_get_append_trace_file(NULL));

    /* print configuration */
    if (flom_config_get_verbose(NULL))
        flom_config_print(NULL);

    /* check configuration */
    if (FLOM_RC_OK != (ret_cod = flom_config_check(NULL))) {
        g_printerr("Configuration is not valid, cannot go on!\n");
        exit(FLOM_ES_GENERIC_ERROR);        
    }

    /* check if the command is asking the execution of a debug feature */
    if (NULL != debug_feature)
        exit(FLOM_RC_OK == flom_debug_features(debug_feature) ? 0 : 1);
    
    /* check if the command is asking daemon termination */
    if (quiesce_exit || immediate_exit) {
        g_print("Starting FLoM daemon %s shutdown...\n",
                immediate_exit ? "immediate" : "quiesce");
        flom_client_shutdown(NULL, immediate_exit);
        exit(0);
    }
    
    /* check the command is not null */
    if (NULL == command_argv) {
        g_printerr("No command to execute!\n");
        exit(FLOM_ES_UNABLE_TO_EXECUTE_COMMAND);        
    }

    /* open connection to a valid flom lock manager... */
    if (FLOM_RC_OK != (ret_cod = flom_client_connect(NULL, &cd, TRUE))) {
        g_printerr("flom_client_connect: ret_cod=%d (%s)\n",
                   ret_cod, flom_strerror(ret_cod));
        exit(FLOM_ES_GENERIC_ERROR);
    }

    /* sending lock command */
    ret_cod = flom_client_lock(NULL, &cd,
                               flom_config_get_resource_timeout(NULL),
                               &locked_element);
    switch (ret_cod) {
        case FLOM_RC_OK: /* OK, go on */
            if (flom_config_get_verbose(NULL) && NULL != locked_element)
                g_print("Locked element is '%s'\n", locked_element);
            break;
        case FLOM_RC_LOCK_BUSY: /* busy */
            g_printerr("Resource already locked, the lock cannot be "
                       "obtained\n");
            /* gracefully disconnect from daemon */
            if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(&cd))) {
                g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                          ret_cod, flom_strerror(ret_cod));
            }
            exit(FLOM_ES_RESOURCE_BUSY);
            break;
        case FLOM_RC_LOCK_CANT_WAIT: /* can't wait, leaving... */
            g_printerr("The resource could be available in the future, "
                       "but the requester can't wait\n");
            /* gracefully disconnect from daemon */
            if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(&cd))) {
                g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                           ret_cod, flom_strerror(ret_cod));
            }
            exit(FLOM_ES_REQUESTER_CANT_WAIT);
            break;
        case FLOM_RC_LOCK_IMPOSSIBLE: /* impossible */
            g_printerr("Resource will never satisfy the request, the lock "
                       "cannot be obtained\n");
            /* gracefully disconnect from daemon */
            if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(&cd))) {
                g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                          ret_cod, flom_strerror(ret_cod));
            }
            exit(FLOM_ES_GENERIC_ERROR);
            break;
        case FLOM_RC_NETWORK_TIMEOUT: /* timeout expired, busy resource */
            g_printerr("The lock was not obtained because timeout "
                       "(%d milliseconds) expired\n",
                       flom_config_get_resource_timeout(NULL));
            /* gracefully disconnect from daemon */
            if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(&cd))) {
                g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                           ret_cod, flom_strerror(ret_cod));
            }
            exit(FLOM_ES_RESOURCE_BUSY);
            break;            
        default:
            g_printerr("flom_client_lock: ret_cod=%d (%s)\n",
                       ret_cod, flom_strerror(ret_cod));
            exit(FLOM_ES_GENERIC_ERROR);
    } /* switch (ret_cod) */

    /* execute the command */
    if (FLOM_RC_OK != (ret_cod = flom_exec(command_argv, locked_element,
                                           &child_status))) {
        guint i, num;
        g_printerr("Unable to execute command: '");
        num = g_strv_length(command_argv);
        for (i=0; i<num; ++i)
            g_print("%s", command_argv[i]);
        g_print("'\n");
        exit(FLOM_ES_UNABLE_TO_EXECUTE_COMMAND);
    }

    /* releasing locked element memory */
    g_free(locked_element);
    locked_element = NULL;
    
    /* sending unlock command */
    if (FLOM_RC_OK != (ret_cod = flom_client_unlock(NULL, &cd))) {
        g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                   ret_cod, flom_strerror(ret_cod));
        exit(FLOM_ES_GENERIC_ERROR);
    }

    /* gracefully disconnect from daemon */
    if (FLOM_RC_OK != (ret_cod = flom_client_disconnect(&cd))) {
        g_printerr("flom_client_unlock: ret_cod=%d (%s)\n",
                   ret_cod, flom_strerror(ret_cod));
    }

    g_strfreev (command_argv);
    command_argv = NULL;

    /* release config data */
    flom_config_free(NULL);
    /* release regular expression data */
    global_res_name_preg_free();
    
	return child_status;
}
