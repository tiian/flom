/*
 * Copyright (c) 2013-2024, Christian Ferrari <tiian@users.sourceforge.net>
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
#include <config.h>



#include "flom_defines.h"



#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_daemon_mngmnt.h"
#include "flom_fuse.h"
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



#ifdef HAVE_LIBFUSE
/* Version for FUSE 2 */
gpointer flom_daemon_mngmnt_activate_vfs(gpointer data)
{
    enum Exception { FUSE_PARSE_CMDLINE
                     , FUSE_MOUNT
                     , FUSE_LOWLEVEL_NEW
                     , FUSE_SET_SIGNAL_HANDLERS
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int argc = 2;
    char *argv[] = { "flom", (char *)data };
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_chan *ch;
    struct fuse_session *se;
    char *mountpoint = NULL;
    
    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]\n"));
    TRY {
        int ret = -1;

        /* setting common values */
        flom_fuse_common_values.uid = getuid();
        flom_fuse_common_values.gid = getgid();
        
        FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]/fuse_parse_cmdline\n"));
        if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != 0)
            THROW(FUSE_PARSE_CMDLINE);

        if ((ch = fuse_mount(mountpoint, &args)) == NULL)
            THROW(FUSE_MOUNT);

        syslog(LOG_INFO, FLOM_SYSLOG_FLM022I, (char *)data);


        if (NULL == (se = fuse_lowlevel_new(
                         &args, &fuse_callback_functions,
                         sizeof(fuse_callback_functions), NULL)))
            THROW(FUSE_LOWLEVEL_NEW);

        /*
         * This function might conflict with FLoM signal management that's not
         * the default one; FUSE usage is introduced by version 1.7.0 as
         * experimental, but this function is not called, hoping it's not
         * a mandatory step (2023-11-17)
        if (fuse_set_signal_handlers(se) != 0)
            THROW(FUSE_SET_SIGNAL_HANDLERS);
        */
        
        fuse_session_add_chan(se, ch);
        ret = fuse_session_loop(se);
        FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]: "
                    "fuse_session_loop return code is %d\n", ret));
        /* 
         * This function might conflict with FLoM signal management that's not
         * the default one; FUSE usage is introduced by version 1.7.0 as
         * experimental, but this function is not called, hoping it's not
         * a mandatory step (2023-11-17)
        fuse_remove_signal_handlers(se);
        */
        fuse_session_remove_chan(ch);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case FUSE_PARSE_CMDLINE:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]: "
                            "fuse_parse_cmdline() error\n"));
                break;
            case FUSE_MOUNT:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]: "
                            "fuse_mount() error\n"));
                break;
            case FUSE_LOWLEVEL_NEW:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]: "
                            "fuse_lowlevel_new() error\n"));
                break;
            case FUSE_SET_SIGNAL_HANDLERS:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]: "
                            "fuse_set_signal_handlers() error\n"));
                break;                
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* recovery allocated memory */

    if (excp >= FUSE_LOWLEVEL_NEW)
        fuse_session_destroy(se);
    if (excp >= FUSE_MOUNT)
        fuse_unmount(mountpoint, ch);
    if (excp > FUSE_PARSE_CMDLINE) {
        free(mountpoint);
        fuse_opt_free_args(&args);
    }

    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE2]/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return NULL;
}
#elif HAVE_LIBFUSE3
/* Version for FUSE 3 */
gpointer flom_daemon_mngmnt_activate_vfs(gpointer data)
{
    enum Exception { FUSE_PARSE_CMDLINE
                     , FUSE_SESSION_NEW
                     , FUSE_SET_SIGNAL_HANDLERS
                     , FUSE_SESSION_MOUNT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    int argc = 2;
    char *argv[] = { "flom", (char *)data };
    
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_cmdline_opts opts;
    struct fuse_session *se;
    
    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]\n"));
    TRY {
        int ret = -1;

        /* setting common values */
        flom_fuse_common_values.uid = getuid();
        flom_fuse_common_values.gid = getgid();

        if (fuse_parse_cmdline(&args, &opts) != 0)
            THROW(FUSE_PARSE_CMDLINE);

        syslog(LOG_INFO, FLOM_SYSLOG_FLM022I, (char *)data);
        se = fuse_session_new(&args, &fuse_callback_functions,
                              sizeof(fuse_callback_functions), NULL);
        if (se == NULL)
            THROW(FUSE_SESSION_NEW);

        /*
         * This function might conflict with FLoM signal management that's not
         * the default one; FUSE usage is introduced by version 1.7.0 as
         * experimental, but this function is not called, hoping it's not
         * a mandatory step (2023-11-17)
        if (fuse_set_signal_handlers(se) != 0)
            THROW(FUSE_SET_SIGNAL_HANDLERS);
        */

        if (fuse_session_mount(se, opts.mountpoint) != 0)
            THROW(FUSE_SESSION_MOUNT);

        ret = fuse_session_loop(se);
        FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]: "
                    "fuse_session_loop return code is %d", ret));

        fuse_session_unmount(se);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case FUSE_PARSE_CMDLINE:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]: "
                            "fuse_parse_cmdline() error\n"));
                break;
            case FUSE_SESSION_NEW:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]: "
                            "fuse_session_new() error\n"));
                break;
            case FUSE_SET_SIGNAL_HANDLERS:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]: "
                            "fuse_set_signal_handlers() error\n"));
                break;
            case FUSE_SESSION_MOUNT:
                FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]: "
                            "fuse_session_mount() error\n"));
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* memory recovery */
    if (excp >= FUSE_SESSION_MOUNT) {
        fuse_remove_signal_handlers(se);
    }
        /* 
         * This function might conflict with FLoM signal management that's not
         * the default one; FUSE usage is introduced by version 1.7.0 as
         * experimental, but this function is not called, hoping it's not
         * a mandatory step (2023-11-17)
         if (excp >= FUSE_SET_SIGNAL_HANDLERS) {
         fuse_session_destroy(se);
         }
        */
    if (excp > FUSE_PARSE_CMDLINE) {
        free(opts.mountpoint);
        fuse_opt_free_args(&args);
    }
    FLOM_TRACE(("flom_daemon_mngmnt_activate_vfs[FUSE3]/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return NULL;
}
#else
# error No HAVE_LIBFUSEx defined!
#endif
