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



#include <stdio.h>
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_daemon.h"
#include "flom_errors.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON



int flom_daemon(const flom_config_t *config)
{
    enum Exception { FORK_ERROR
                     , WAIT_ERROR
                     , CHILD_PID_ERROR
                     , FORK_ERROR2
                     , SETSID_ERROR
                     , SIGNAL_ERROR
                     , FORK_ERROR3
                     , CHDIR_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon\n"));
    TRY {
        pid_t pid;
        
        /* creating a child process */
        if (-1 == (pid = fork())) {
            THROW(FORK_ERROR);
        } else if (0 != pid) {
            int status;
            pid_t child_pid;
            /* father process */
            FLOM_TRACE(("flom_daemon: started child process pid=" PID_T_FORMAT
                        "\n", pid));
            /* waiting termination: it's only a synchronization because
               while daemonizing the child will fork twice */
            if (-1 == (child_pid = wait(&status)))
                THROW(WAIT_ERROR);
            if (pid != child_pid) {
                FLOM_TRACE(("flom_daemon: something really wrong is happening;"
                            " pid=" PID_T_FORMAT ", child_pid=" PID_T_FORMAT
                            "\n", pid, child_pid));
                THROW(CHILD_PID_ERROR);
            }
        } else {
            pid_t pid;
            int i;
            FILE *dummy;
            
            /* child process, now daemonizing... */
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR2);
            } else if (0 != pid)
                exit(0);
            FLOM_TRACE(("flom_daemon: daemonizing... setsid()\n"));
            if (-1 == setsid())
                THROW(SETSID_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... signal()\n"));
            if (SIG_ERR == signal(SIGHUP, SIG_IGN))
                THROW(SIGNAL_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR3);
            } else if (0 != pid)
                exit(0);
            FLOM_TRACE(("flom_daemon: daemonizing... chdir()\n"));
            if (-1 == chdir("/"))
                THROW(CHDIR_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... umask()\n"));
            umask(0);
            FLOM_TRACE(("flom_daemon: daemonizing... close(0..63)\n"));
            for (i = 0; i < 64; ++i) close(i);
            if (NULL != config->trace_file)
                dummy = freopen(config->trace_file, "a", stderr);
            else
                dummy = freopen("/dev/null", "a", stderr);
            openlog("flom", LOG_PID, LOG_DAEMON);
            syslog(LOG_NOTICE, "flom daemon activated!");
            FLOM_TRACE(("flom_daemon: now daemonized!\n"));
            /* @@@ restart from here!!! */
            exit(0);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case FORK_ERROR:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case WAIT_ERROR:
                ret_cod = FLOM_RC_WAIT_ERROR;
                break;
            case CHILD_PID_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case FORK_ERROR2:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case SETSID_ERROR:
                ret_cod = FLOM_RC_SETSID_ERROR;
                break;
            case SIGNAL_ERROR:
                ret_cod = FLOM_RC_SIGNAL_ERROR;
                break;
            case FORK_ERROR3:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case CHDIR_ERROR:
                ret_cod = FLOM_RC_CHDIR_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



