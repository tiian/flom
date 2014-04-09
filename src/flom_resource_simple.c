/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_simple.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_SIMPLE



int flom_resource_simple_can_lock(flom_resource_t *resource,
                                  flom_lock_mode_t lock)
{
    static const flom_lock_mode_t lock_table[
        FLOM_LOCK_MODE_N][FLOM_LOCK_MODE_N] =
        { { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE } ,
          { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  FALSE } ,
          { TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE } ,
          { TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE } };
    
    GSList *p = NULL;
    flom_lock_mode_t old_lock;
    int can_lock = TRUE;
    
    FLOM_TRACE(("flom_resource_simple_can_lock: checking lock=%d\n", lock));
    p = resource->data.simple.holders;
    while (NULL != p) {
        old_lock = ((struct flom_rsrc_conn_lock_s *)p->data)->info.lock_mode;
        FLOM_TRACE(("flom_resource_simple_can_lock: current_lock=%d, "
                    "asked_lock=%d, lock_table[%d][%d]=%d\n",
                    old_lock, lock, old_lock, lock,
                    lock_table[old_lock][lock]));
        can_lock &= lock_table[old_lock][lock];
        if (!can_lock)
            break;
        else
            p = p->next;
    } /* while (NULL != p) */
    return can_lock;
}



int flom_resource_simple_inmsg(flom_resource_t *resource,
                               struct flom_conn_data_s *conn,
                               struct flom_msg_s *msg)
{
    enum Exception { MSG_FREE_ERROR1
                     , G_TRY_MALLOC_ERROR1
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , INVALID_OPTION
                     , RESOURCE_SIMPLE_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_resource_simple_inmsg\n"));
    TRY {
        flom_lock_mode_t new_lock = FLOM_LOCK_MODE_NL;
        int can_lock = TRUE;
        int can_wait = TRUE;
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                new_lock = msg->body.lock_8.resource.mode;
                can_wait = msg->body.lock_8.resource.wait;
                can_lock = flom_resource_simple_can_lock(
                    resource, new_lock);
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                if (can_lock) {
                    /* get the lock */
                    struct flom_rsrc_conn_lock_s *cl = NULL;
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_simple_inmsg: asked lock %d "
                                "can be assigned to connection %p\n",
                                new_lock, conn));
                    if (NULL == (cl = (struct flom_rsrc_conn_lock_s *)
                                 g_try_malloc(
                                     sizeof(struct flom_rsrc_conn_lock_s))))
                        THROW(G_TRY_MALLOC_ERROR1);
                    cl->info.lock_mode = new_lock;
                    cl->conn = conn;
                    resource->data.simple.holders = g_slist_prepend(
                        resource->data.simple.holders,
                        (gpointer)cl);
                    if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                           msg, FLOM_MSG_VERB_LOCK,
                                           2*FLOM_MSG_STEP_INCR,
                                           FLOM_RC_OK)))
                        THROW(MSG_BUILD_ANSWER_ERROR1);
                } else {
                    /* can't lock, enqueue */
                    if (can_wait) {
                        struct flom_rsrc_conn_lock_s *cl = NULL;
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_simple_inmsg: asked lock "
                                    "%d can not be assigned to connection %p, "
                                    "queing...\n", new_lock, conn));
                        if (NULL == (cl = (struct flom_rsrc_conn_lock_s *)
                                     g_try_malloc(
                                         sizeof(struct flom_rsrc_conn_lock_s))))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->info.lock_mode = new_lock;
                        cl->conn = conn;
                        g_queue_push_tail(
                            resource->data.simple.waitings,
                            (gpointer)cl);
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               2*FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_simple_inmsg: asked lock "
                                    "%d can not be assigned to connection %p, "
                                    "rejecting...\n", new_lock, conn));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               2*FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_BUSY)))
                            THROW(MSG_BUILD_ANSWER_ERROR3);
                    } /* if (msg->body.lock_8.resource.wait) */
                } /* if (can_lock) */
                break;
            case FLOM_MSG_VERB_UNLOCK:
                /* check lock is managed by this locker (this check will
                   trigger some issue if a client obtained more locks...) */
                if (g_strcmp0(flom_resource_get_name(resource),
                              msg->body.lock_8.resource.name)) {
                    FLOM_TRACE(("flom_resource_simple_inmsg: client wants to "
                                "unlock resource '%s' while it's locking "
                                "resource '%s'\n",
                                msg->body.lock_8.resource.name,
                                flom_resource_get_name(resource)));
                    THROW(INVALID_OPTION);
                }
                /* clean lock */
                if (FLOM_RC_OK != (ret_cod = flom_resource_simple_clean(
                                       resource, conn)))
                    THROW(RESOURCE_SIMPLE_CLEAN_ERROR);
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR2);
                flom_msg_init(msg);
                break;
            default:
                THROW(PROTOCOL_ERROR);
        } /* switch (msg->header.pvs.verb) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_FREE_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR1:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR2:
            case MSG_BUILD_ANSWER_ERROR3:
                break;
            case INVALID_OPTION:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case RESOURCE_SIMPLE_CLEAN_ERROR:
            case MSG_FREE_ERROR2:
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_simple_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_simple_clean(flom_resource_t *resource,
                               struct flom_conn_data_s *conn)
{
    enum Exception { NULL_OBJECT
                     , SIMPLE_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_simple_clean\n"));
    TRY {
        GSList *p = NULL;

        if (NULL == resource)
            THROW(NULL_OBJECT);
        /* check if the connection keeps a lock */
        p = resource->data.simple.holders;
        while (NULL != p) {
            if (((struct flom_rsrc_conn_lock_s *)p->data)->conn == conn)
                break;
            else
                p = p->next;
        } /* while (NULL != p) */
        if (NULL != p) {
            struct flom_rsrc_conn_lock_s *cl =
                (struct flom_rsrc_conn_lock_s *)p->data;
            FLOM_TRACE(("flom_resource_simple_clean: the client is holding "
                        "a lock mode %d, removing it...\n",
                        cl->info.lock_mode));
            FLOM_TRACE(("flom_resource_simple_clean: cl=%p\n", cl));
            resource->data.simple.holders = g_slist_remove(
                resource->data.simple.holders, cl);
            /* free the now useless connection lock record */
            g_free(cl);
            /*
            FLOM_TRACE(("flom_resource_simple_clean: g_slist_length=%u\n",
                        g_slist_length(resource->data.simple.holders)));
            */
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_simple_waitings(
                                   resource)))
                THROW(SIMPLE_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.simple.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_simple_clean: the client is "
                                "waiting for a lock mode %d, removing "
                                "it...\n", cl->info.lock_mode));
                    cl = g_queue_pop_nth(resource->data.simple.waitings, i);
                    if (NULL == cl) {
                        /* this should be impossibile because peek was ok
                           some rows above */
                        THROW(INTERNAL_ERROR);
                    } else {
                        /* free the now useless connection lock record */
                        g_free(cl);
                    }
                    break;
                } else
                    ++i;
            } while (TRUE);
        } /* if (NULL != p) */
                
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case SIMPLE_WAITINGS_ERROR:
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_simple_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_simple_free(flom_resource_t *resource)
{    
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_simple_free: cleaning-up holders list...\n"));
    while (NULL != resource->data.simple.holders) {
        struct flom_conn_lock_s *cl =
            (struct flom_conn_lock_s *)resource->data.simple.holders->data;
        resource->data.simple.holders = g_slist_remove(
            resource->data.simple.holders, cl);
        g_free(cl);
    }
    resource->data.simple.holders = NULL;
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_simple_free: cleaning-up waitings queue...\n"));
    while (!g_queue_is_empty(resource->data.simple.waitings)) {
        struct flom_conn_lock_s *cl =
            (struct flom_conn_lock_s *)g_queue_pop_head(
                resource->data.simple.waitings);
        g_free(cl);
    }
    g_queue_free(resource->data.simple.waitings);
    resource->data.simple.waitings = NULL;
}



int flom_resource_simple_waitings(flom_resource_t *resource)
{
    enum Exception { INTERNAL_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    
    FLOM_TRACE(("flom_resource_simple_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        
        /* check if there is any connection waiting for a lock */
        do {
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.simple.waitings, i);
            if (NULL == cl)
                break;
            /* try to apply this lock... */
            if (flom_resource_simple_can_lock(resource, cl->info.lock_mode)) {
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.simple.waitings, i);
                if (NULL == cl)
                    /* this should be impossibile because peek was ok
                       some rows above */
                    THROW(INTERNAL_ERROR);
                /* send a message to the client thatìs waiting the lock */
                flom_msg_init(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                       &msg, FLOM_MSG_VERB_LOCK,
                                       3*FLOM_MSG_STEP_INCR,
                                       FLOM_RC_OK)))
                    THROW(MSG_BUILD_ANSWER_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                                       &msg, buffer, sizeof(buffer), &to_send)))
                    THROW(MSG_SERIALIZE_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_msg_send(
                                       cl->conn->fd, buffer, to_send)))
                    THROW(MSG_SEND_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
                    THROW(MSG_FREE_ERROR);                
                /* insert into holders */
                resource->data.simple.holders = g_slist_prepend(
                    resource->data.simple.holders,
                    (gpointer)cl);
                cl = NULL;
            } else
                ++i;
        } while (TRUE);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR:
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != cl) {
        g_free(cl);
    }
    FLOM_TRACE(("flom_resource_simple_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

