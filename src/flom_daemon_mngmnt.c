/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#define FUSE_USE_VERSION 26


#include <config.h>



#ifdef HAVE_FUSE_LOWLEVEL_H
# include <fuse_lowlevel.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_daemon_mngmnt.h"
#include "flom_vfs.h"
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



gpointer flom_daemon_mngmnt_activate_vfs(gpointer data)
{
    enum Exception { VFS_CHECK_UID_INODE_INTEGRITY
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int argc = 2;
    char *argv[] = { "flom", "/tmp/prova" };
    
    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs\n"));
    TRY {
        struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
        struct fuse_chan *ch;
        char *mountpoint;
        int err = -1;

        /* checking consistency before starting VFS */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_check_uid_inode_integrity()))
            THROW(VFS_CHECK_UID_INODE_INTEGRITY);

        /* setting common values */
        flom_vfs_common_values.uid = getuid();
        flom_vfs_common_values.gid = getgid();
        flom_vfs_common_values.time = time(NULL);
        
        if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
            (ch = fuse_mount(mountpoint, &args)) != NULL) {
            struct fuse_session *se;

            se = fuse_lowlevel_new(&args, &fuse_callback_functions,
                                   sizeof(fuse_callback_functions), NULL);
            if (se != NULL) {
                if (fuse_set_signal_handlers(se) != -1) {
                    fuse_session_add_chan(se, ch);
                    err = fuse_session_loop(se);
                    fuse_remove_signal_handlers(se);
                    fuse_session_remove_chan(ch);
                }
                fuse_session_destroy(se);
            }
            fuse_unmount(mountpoint, ch);
        };
        fuse_opt_free_args(&args);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case VFS_CHECK_UID_INODE_INTEGRITY:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return NULL;
}
