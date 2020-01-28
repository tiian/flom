/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif



#include "flom_errors.h"
#include "flom_exec.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_EXEC


int flom_exec(gchar **const command_argv, const char *element,
              int *child_status)
{
    enum Exception { COMMAND_ARGV_IS_NULL
                     , FORK_ERROR
                     , MALLOC_ERROR
                     , EXECVP_ERROR
                     , WAIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    pid_t pid = -1;
    gchar *local_element = NULL;
    
    FLOM_TRACE(("flom_exec\n"));
    TRY {

        if (NULL == command_argv) {
            FLOM_TRACE(("flom_exec: command_argv cannot be NULL\n"));
            THROW(COMMAND_ARGV_IS_NULL);
        }
        
        /* fork */
        if (-1 == (pid = fork())) {
            THROW(FORK_ERROR);
        } else if (0 == pid) {
            /* child process, preparing for execv... */
            const char *path = command_argv[0];
            char **argv;
            guint i, num, el;
            num = g_strv_length(command_argv);
            /* check additional option (resource set locked element) **/
            if (NULL  == element)
                el = 0;
            else {
                el = 1;
                local_element = g_strdup((const gchar *)element);
            }
            for (i=0; i<num; ++i)
                FLOM_TRACE(("flom_exec-child: command_argv[%u]='%s'\n",
                            i, command_argv[i]));
            if (el)
                FLOM_TRACE(("flom_exec-child: element='%s'\n", local_element));
            if (NULL == (argv = (char **)malloc((++num+el)* sizeof(char *))))
                THROW(MALLOC_ERROR);
            for (i=0; i<num-1; ++i) {
                argv[i] = command_argv[i];
            }
            if (el)
                argv[num-1] = local_element;
            argv[num-1+el] = (char *)NULL;
            FLOM_TRACE(("flom_exec-child: path='%s'\n", path));
            for (i=0; i<num-1+el; ++i)
                FLOM_TRACE(("flom_exec-child: argv[%u]='%s'\n", i, argv[i]));
            /* execvp */
            if (-1 == execvp(path, argv)) {
                /* print a warning on terminal */
                g_warning("Unable to execute command '%s'\n", path);
                THROW(EXECVP_ERROR);
            }
        } else {
            int status;
            pid_t child_pid;
            
            /* father process */
            FLOM_TRACE(("flom_exec-father: child pid=" PID_T_FORMAT "\n",
                        pid));
            /* waiting child termination */
            FLOM_TRACE(("flom_exec-father: waiting child termination\n"));
            if (-1 == (child_pid = wait(&status)))
                THROW(WAIT_ERROR);
            *child_status = WEXITSTATUS(status);
            FLOM_TRACE(("flom_exec-father: child exit status is %d\n",
                        *child_status));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case COMMAND_ARGV_IS_NULL:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case FORK_ERROR:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case MALLOC_ERROR:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case EXECVP_ERROR:
                ret_cod = FLOM_RC_EXECVP_ERROR;
                break;
            case WAIT_ERROR:
                ret_cod = FLOM_RC_WAIT_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_exec-%s/excp=%d/"
                "ret_cod=%d/errno=%d\n",
                (0 == pid ? "child" : "father"), 
                excp, ret_cod, errno));
    /* free dynamic memory */
    g_free(local_element);
    if (0 == pid && FLOM_RC_OK != ret_cod)
        exit(FLOM_ES_UNABLE_TO_EXECUTE_COMMAND);
    return ret_cod;
}

