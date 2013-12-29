/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONFIG



/* global static objects */
flom_config_t global_config;



/* static strings */
const char *DEFAULT_RESOURCE_NAME = "_RESOURCE";
const char FLOM_SYSTEM_CONFIG_FILENAME[] = SYSTEM_CONFIG_FILENAME;
const char FLOM_USER_CONFIG_FILENAME[] = USER_CONFIG_FILENAME;
const char FLOM_DIR_FILE_SEPARATOR[] = DIR_FILE_SEPARATOR;


const char *FLOM_PACKAGE_BUGREPORT = PACKAGE_BUGREPORT;
const char *FLOM_PACKAGE_NAME = PACKAGE;
const char *FLOM_PACKAGE_VERSION = PACKAGE_VERSION;
const char *FLOM_PACKAGE_DATE = "2013-12-22";

const char FLOM_INSTALL_SYSCONFDIR[] = SYSCONFDIR;



void flom_config_reset()
{
    /* set UNIX socket name */
    snprintf(global_config.local_socket_path_name, LOCAL_SOCKET_SIZE,
             "/tmp/flom-%s", getlogin());
    global_config.daemon_trace_file = NULL;
    global_config.command_trace_file = NULL;
    global_config.idle_time = 5000; /* milliseconds */
    global_config.resource_name = DEFAULT_RESOURCE_NAME;
}



void flom_config_init(const char *user_config_file_name)
{
    /* retrieve configuration from system default config file */
    char system_config_filename[sizeof(FLOM_INSTALL_SYSCONFDIR) +
                                sizeof(FLOM_DIR_FILE_SEPARATOR) +
                                sizeof(FLOM_SYSTEM_CONFIG_FILENAME)];
    strcpy(system_config_filename, FLOM_INSTALL_SYSCONFDIR);
    strcat(system_config_filename, FLOM_DIR_FILE_SEPARATOR);
    strcat(system_config_filename, FLOM_SYSTEM_CONFIG_FILENAME);
    assert(sizeof(system_config_filename)>strlen(system_config_filename));
    FLOM_TRACE(("flom_config_init: looking for system wide config file '%s'\n",
                system_config_filename));
    flom_config_init_load(system_config_filename);
}



void flom_config_init_load(const char *config_file_name)
{
    FLOM_TRACE(("flom_config_init_load: loading file '%s'\n",
                config_file_name));
}
