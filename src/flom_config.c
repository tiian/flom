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


const char *FLOM_PACKAGE_BUGREPORT = PACKAGE_BUGREPORT;
const char *FLOM_PACKAGE_NAME = PACKAGE;
const char *FLOM_PACKAGE_VERSION = PACKAGE_VERSION;
const char *FLOM_PACKAGE_DATE = _RELEASE_DATE;

const gchar FLOM_INSTALL_SYSCONFDIR[] = _SYSCONFDIR;

const gchar *FLOM_CONFIG_GROUP_TRACE = _CONFIG_GROUP_TRACE;
const gchar *FLOM_CONFIG_KEY_DAEMONTRACEFILE = _CONFIG_KEY_DAEMONTRACEFILE;
const gchar *FLOM_CONFIG_KEY_COMMANDTRACEFILE = _CONFIG_KEY_COMMANDTRACEFILE;
const gchar *FLOM_CONFIG_GROUP_RESOURCE = _CONFIG_GROUP_RESOURCE;
const gchar *FLOM_CONFIG_KEY_NAME = _CONFIG_KEY_NAME;
const gchar *FLOM_CONFIG_KEY_WAIT = _CONFIG_KEY_WAIT;
const gchar *FLOM_CONFIG_KEY_TIMEOUT = _CONFIG_KEY_TIMEOUT;
const gchar *FLOM_CONFIG_KEY_LOCK_MODE = _CONFIG_KEY_LOCK_MODE;



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
    struct passwd *pwd = NULL;
    char *login = NULL;
    
    FLOM_TRACE(("flom_config_reset\n"));
    /* set UNIX socket name */
    pwd = getpwuid(getuid());
    if (NULL == pwd || NULL == pwd->pw_name)
        login = "nobody";
    else
        login = pwd->pw_name;
    snprintf(global_config.local_socket_path_name, LOCAL_SOCKET_SIZE,
             "/tmp/flom-%s", login);
    global_config.daemon_trace_file = NULL;
    global_config.command_trace_file = NULL;
    global_config.idle_time = 5000; /* milliseconds */
    global_config.resource_name = g_strdup(DEFAULT_RESOURCE_NAME);
    global_config.resource_wait = TRUE;
    global_config.resource_timeout = FLOM_NETWORK_WAIT_TIMEOUT;
    global_config.lock_mode = FLOM_LOCK_MODE_EX;
}



void flom_config_print()
{
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_DAEMONTRACEFILE,
            NULL == flom_config_get_daemon_trace_file() ? "" :
            flom_config_get_daemon_trace_file());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_TRACE,
            FLOM_CONFIG_KEY_COMMANDTRACEFILE,
            NULL == flom_config_get_command_trace_file() ? "" :
            flom_config_get_command_trace_file());
    g_print("[%s]/%s='%s'\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_NAME,
            NULL == flom_config_get_resource_name() ? "" :
            flom_config_get_resource_name());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_WAIT, flom_config_get_resource_wait());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_TIMEOUT, flom_config_get_resource_timeout());
    g_print("[%s]/%s=%d\n", FLOM_CONFIG_GROUP_RESOURCE,
            FLOM_CONFIG_KEY_LOCK_MODE, flom_config_get_lock_mode());
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
                     , CONFIG_SET_RESOURCE_NAME_ERROR
                     , CONFIG_SET_RESOURCE_WAIT_ERROR
                     , CONFIG_SET_RESOURCE_TIMEOUT_ERROR
                     , CONFIG_SET_LOCK_MODE_ERROR
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
            if (throw_error) THROW(CONFIG_SET_LOCK_MODE_ERROR);
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
            case CONFIG_SET_RESOURCE_NAME_ERROR:
                break;
            case CONFIG_SET_RESOURCE_WAIT_ERROR:
            case CONFIG_SET_RESOURCE_TIMEOUT_ERROR:
            case CONFIG_SET_LOCK_MODE_ERROR:
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
