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
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#define FLOM_TRACE_MODULE FLOM_TRACE_MOD_GENERIC



#include "flom_config.h"
#include "flom_connect.h"
#include "flom_errors.h"
#include "flom_exec.h"
#include "flom_rsrc.h"
#include "flom_trace.h"



static gboolean print_version = FALSE;
static char *trace_file = NULL;
static gchar **command_argv = NULL;
/* command line options */
static GOptionEntry entries[] =
{
    { "version", 'v', 0, G_OPTION_ARG_NONE, &print_version, "Print package info and exit", NULL },
    { "trace-file", 't', 0, G_OPTION_ARG_STRING, &trace_file, "Specify daemon trace file name (absolute path required)", NULL },
    { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command_argv, "Command must be executed under flom control" },
    { NULL }
};



int main (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *option_context;
    int child_status = 0;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE_INIT;
    
    option_context = g_option_context_new("-- command to execute");
    g_option_context_add_main_entries(option_context, entries, NULL);
    if (!g_option_context_parse(option_context, &argc, &argv, &error)) {
        g_print("option parsing failed: %s\n", error->message);
        exit(1);
    }
    g_option_context_free(option_context);

    flom_config_reset();
    flom_config_set_trace_file(trace_file);

    if (FLOM_RC_OK != (ret_cod = global_res_name_preg_init())) {
        g_print("global_res_name_preg_init: ret_cod=%d\n", ret_cod);
        exit(1);
    }

    if (FLOM_RC_OK != (ret_cod = flom_connect())) {
        g_print("flom_connect: ret_cod=%d (%s)\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    if (FLOM_RC_OK != (ret_cod = flom_exec(command_argv, &child_status))) {
        g_print("flom_exec: ret_cod=%d\n", ret_cod);
        exit(1);
    }
    
    g_strfreev (command_argv);
    command_argv = NULL;
    
	return child_status;
}
