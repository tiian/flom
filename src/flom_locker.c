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



#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_locker.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_LOCKER



void flom_locker_destroy(struct flom_locker_s *locker)
{
    if (NULL != locker) {
        g_free(locker->resource_name);
        close(locker->write_pipe);
        close(locker->read_pipe);
        g_free(locker);
    }
}



void flom_locker_array_init(flom_locker_array_t *lockers)
{
    lockers->n = 0;
    lockers->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)flom_locker_destroy);
}



void flom_locker_array_add(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    g_ptr_array_add(lockers->array, (gpointer)locker);
    lockers->n++;
}



gpointer flom_locker_loop(gpointer data)
{
    enum Exception { CONNS_INIT_ERROR
                     , CONNS_ADD_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , READ_ERROR1
                     , READ_ERROR2
                     , READ_ERROR3
                     , CONNS_IMPORT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_conns_t conns;
    
    FLOM_TRACE(("flom_locker_loop: new thread in progress...\n"));
    TRY {
        int loop = TRUE;
        int domain, client_fd;
        struct flom_locker_s *locker = (struct flom_locker_s *)data;
        struct flom_conn_data_s cd;

        /* as a first action, it marks the identifier */
        locker->thread = g_thread_self();
        /* initialize a connections object for this locker thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_init(&conns, AF_UNIX)))
            THROW(CONNS_INIT_ERROR);
        /* add the parent communication pipe to connections */
        memset(&cd, 0, sizeof(cd));
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               &conns, locker->read_pipe, sizeof(cd.sa),
                               &(cd.sa))))
            THROW(CONNS_ADD_ERROR);
        
        while (loop) {
            int ready_fd;
            nfds_t i;

            flom_conns_clean(&conns);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(
                                   &conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            ready_fd = poll(flom_conns_get_fds(&conns),
                            flom_conns_get_used(&conns),
                            global_config.idle_time);
            FLOM_TRACE(("flom_locker_loop: ready_fd=%d\n", ready_fd));
            /* @@@ */
        }
        /* pick-up socket domain from parent thread */
        if (sizeof(domain) != read(locker->read_pipe, &domain, sizeof(domain)))
            THROW(READ_ERROR1);
        flom_conns_set_domain(&conns, domain);
        /* pick-up connection file descriptor from parent thread */
        if (sizeof(client_fd) != read(
                locker->read_pipe, &client_fd, sizeof(client_fd)))
            THROW(READ_ERROR2);
        /* pick-up connection data from parent thread */
        if (sizeof(cd) != read(locker->read_pipe, &cd, sizeof(cd)))
            THROW(READ_ERROR3);
        /* import the connection passed by parent thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_import(
                               &conns, client_fd, &cd)))
            THROW(CONNS_IMPORT_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_INIT_ERROR:
            case CONNS_ADD_ERROR:
            case CONNS_SET_EVENTS_ERROR:
                break;
            case READ_ERROR1:
            case READ_ERROR2:
            case READ_ERROR3:
                ret_cod = FLOM_RC_READ_ERROR;
                break;
            case CONNS_IMPORT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* clean-up connections object */
    if (CONNS_INIT_ERROR < excp)
        flom_conns_free(&conns);
    FLOM_TRACE(("flom_locker_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    FLOM_TRACE(("flom_locker_loop: this thread completed service\n"));
    return data;
}
