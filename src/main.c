/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#define FLOM_TRACE_MODULE FLOM_TRACE_MOD_GENERIC



#include "flom_config.h"
#include "flom_connect.h"
#include "flom_exec.h"
#include "flom_trace.h"



static gboolean print_version = FALSE;
static gchar **command_argv = NULL;
/* command line options */
static GOptionEntry entries[] =
{
    { "version", 'v', 0, G_OPTION_ARG_NONE, &print_version, "Print package info and exit", NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command_argv, "Command must be executed under flom control" },
    { NULL }
};



int main (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *option_context;
    int child_status = 0;

    flom_config_t config;
    
    FLOM_TRACE_INIT;

    option_context = g_option_context_new("-- command to execute");
    g_option_context_add_main_entries(option_context, entries, NULL);
    if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(1);
    }
    g_option_context_free(option_context);

    /* @@@ remove me
    if (command_argv != NULL) {
        guint i, num;
        num = g_strv_length (command_argv);
        for (i = 0; i < num; ++i) {
            g_print ("argv[%u]: %s\n", i, command_argv[i]);
        }
    }
    */

    flom_config_reset(&config);
    flom_connect(&config);
    
    flom_exec(command_argv, &child_status);
    
    g_strfreev (command_argv);
    command_argv = NULL;
    
	return child_status;
}
