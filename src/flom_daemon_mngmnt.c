/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_daemon_mngmnt.h"
#include "flom_errors.h"
#include "flom_syslog.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON_MNGMNT



int flom_daemon_mngmnt(flom_config_t *config, flom_conns_t *conns, guint id)
{
    enum Exception { CONNS_GET_MSG_ERROR
                     , INTERNAL_ERROR
                     , DAEMON_MNGMNT_SHUTDOWN_ERROR
                     , UNRECOGNIZED_ACTION_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon_mngmnt\n"));
    TRY {
        struct flom_msg_s *msg;
        
        /* retrieve the arrived message */
        if (NULL == (msg = flom_conns_get_msg(conns, id)))
            THROW(CONNS_GET_MSG_ERROR);

        /* check message verb */
        if (FLOM_MSG_VERB_MNGMNT != msg->header.pvs.verb) {
            FLOM_TRACE(("flom_daemon_mngmnt: unsupported message verb "
                        "%d\n", msg->header.pvs.verb));
            THROW(INTERNAL_ERROR);
        } /* if (FLOM_MSG_VERB_MNGMNT != msg->header.pvs.verb) */

        /* check management action */
        switch (msg->body.mngmnt_8.action) {
            case FLOM_MSG_MNGMNT_ACTION_SHUTDOWN:
                if (FLOM_RC_OK != (
                        ret_cod = flom_daemon_mngmnt_shutdown(
                            config, conns, id)))
                    THROW(DAEMON_MNGMNT_SHUTDOWN_ERROR);
                break;
            default:
                FLOM_TRACE(("flom_daemon_mngmnt: unrecognized action %d\n",
                            msg->body.mngmnt_8.action));
                THROW(UNRECOGNIZED_ACTION_ERROR);
                break;
        } /* switch (msg->body.mngmnt_8.action) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_GET_MSG_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case DAEMON_MNGMNT_SHUTDOWN_ERROR:
                break;
            case UNRECOGNIZED_ACTION_ERROR:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon_mngmnt/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_daemon_mngmnt_shutdown(flom_config_t *config,
                                flom_conns_t *conns, guint id)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon_mngmnt_shutdown\n"));
    TRY {
        struct flom_msg_s *msg = flom_conns_get_msg(conns, id);
        int immediate = msg->body.mngmnt_8.action_data.shutdown.immediate;
        
        if (immediate) {
            FLOM_TRACE(("flom_daemon_mngmnt_shutdown: immediate shutdown, "
                        "exiting...\n"));
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM007N);
            exit(0);
        } else {
            FLOM_TRACE(("flom_daemon_mngmnt_shutdown: quiesce shutdown in "
                        "progress...\n"));
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM008N);
            flom_config_set_lifespan(NULL, FLOM_SHUTDOWN_QUIESCE_GRACE_TIME);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon_mngmnt_shutdown/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

