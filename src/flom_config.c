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
const char *DEFAULT_RESOURCE_NAME = "_RESOURCE";
flom_config_t global_config;



void flom_config_reset()
{
    /* set UNIX socket name */
    snprintf(global_config.local_socket_path_name, LOCAL_SOCKET_SIZE,
             "/tmp/flom-%s", getlogin());
    global_config.trace_file = NULL;
    global_config.idle_time = 5000; /* milliseconds */
    global_config.resource_name = DEFAULT_RESOURCE_NAME;
}
