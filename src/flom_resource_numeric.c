/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_numeric.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_NUMERIC



int flom_resource_numeric_can_lock(flom_resource_t *resource,
                                   gint quantity)
{    
    FLOM_TRACE(("flom_resource_numeric_can_lock: checking quantity=%d "
                "(total_quantity=%d, locked_quantity=%d)\n",
                quantity, resource->data.numeric.total_quantity,
                resource->data.numeric.locked_quantity));
    if (resource->data.numeric.total_quantity -
        resource->data.numeric.locked_quantity >= quantity)
        return TRUE;
    return FALSE;
}



int flom_resource_numeric_init(flom_resource_t *resource,
                               const gchar *name)
{
    enum Exception { G_STRDUP_ERROR
                     , RSRC_GET_NUMBER_ERROR
                     , G_QUEUE_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_numeric_init\n"));
    TRY {
        if (NULL == (resource->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR);
        FLOM_TRACE(("flom_resource_numeric_init: initialized resource ('%s')\n",
                    resource->name));

        if (FLOM_RC_OK != (ret_cod = flom_rsrc_get_number(
                               name, FLOM_RSRC_TYPE_NUMERIC,
                               &(resource->data.numeric.total_quantity))))
                    THROW(RSRC_GET_NUMBER_ERROR);
        resource->data.numeric.locked_quantity = 0;
        resource->data.numeric.holders = NULL;
        if (NULL == (resource->data.numeric.waitings = g_queue_new()))
            THROW(G_QUEUE_NEW_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case RSRC_GET_NUMBER_ERROR:
                break;
            case G_QUEUE_NEW_ERROR:
                ret_cod = FLOM_RC_G_QUEUE_NEW_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_numeric_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_numeric_inmsg(flom_resource_t *resource,
                                flom_conn_t *conn,
                                struct flom_msg_s *msg,
                                struct timeval *next_deadline)
{
    enum Exception { MSG_FREE_ERROR1
                     , G_TRY_MALLOC_ERROR1
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , INVALID_OPTION
                     , RESOURCE_NUMERIC_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_resource_numeric_inmsg\n"));
    TRY {
        int can_lock = TRUE;
        int can_wait = TRUE;
        int impossible_lock = FALSE;
        gint new_quantity = 0;
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                new_quantity = msg->body.lock_8.resource.quantity;
                can_lock = flom_resource_numeric_can_lock(
                    resource, new_quantity);
                if (new_quantity > resource->data.numeric.total_quantity) {
                    can_wait = FALSE;
                    impossible_lock = TRUE;
                } else
                    can_wait = msg->body.lock_8.resource.wait;
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                if (can_lock) {
                    /* get the lock */
                    struct flom_rsrc_conn_lock_s *cl = NULL;
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_numeric_inmsg: asked lock "
                                "quantity %d can be assigned to connection "
                                "%p\n", new_quantity, conn));
                    if (NULL == (cl = flom_rsrc_conn_lock_new()))
                        THROW(G_TRY_MALLOC_ERROR1);
                    cl->info.quantity = new_quantity;
                    cl->conn = conn;
                    resource->data.numeric.holders = g_slist_prepend(
                        resource->data.numeric.holders,
                        (gpointer)cl);
                    resource->data.numeric.locked_quantity += new_quantity;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                           msg, FLOM_MSG_VERB_LOCK,
                                           flom_conn_get_last_step(conn) +
                                           FLOM_MSG_STEP_INCR,
                                           FLOM_RC_OK, NULL)))
                        THROW(MSG_BUILD_ANSWER_ERROR1);
                } else {
                    /* can't lock, enqueue */
                    if (can_wait) {
                        struct flom_rsrc_conn_lock_s *cl = NULL;
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_numeric_inmsg: asked "
                                    "quantity %d can not be assigned to "
                                    "connection %p, queing...\n",
                                    new_quantity, conn));
                        if (NULL == (cl = flom_rsrc_conn_lock_new()))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->info.quantity = new_quantity;
                        cl->conn = conn;
                        g_queue_push_tail(
                            resource->data.numeric.waitings,
                            (gpointer)cl);
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_numeric_inmsg: asked "
                                    "quantity %d can not be assigned to "
                                    "connection %p, rejecting...\n",
                                    new_quantity, conn));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               impossible_lock ?
                                               FLOM_RC_LOCK_IMPOSSIBLE :
                                               FLOM_RC_LOCK_BUSY, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR3);
                    } /* if (msg->body.lock_8.resource.wait) */
                } /* if (can_lock) */
                break;
            case FLOM_MSG_VERB_UNLOCK:
                /* check lock is managed by this locker (this check will
                   trigger some issue if a client obtained more locks...) */
                if (g_strcmp0(flom_resource_get_name(resource),
                              msg->body.unlock_8.resource.name)) {
                    FLOM_TRACE(("flom_resource_numeric_inmsg: client wants to "
                                "unlock resource '%s' while it's locking "
                                "resource '%s'\n",
                                msg->body.unlock_8.resource.name,
                                flom_resource_get_name(resource)));
                    syslog(LOG_WARNING, FLOM_SYSLOG_FLM009W,
                           msg->body.unlock_8.resource.name,
                           flom_resource_get_name(resource));
                    THROW(INVALID_OPTION);
                }
                /* clean lock */
                if (FLOM_RC_OK != (ret_cod = flom_resource_numeric_clean(
                                       resource, conn)))
                    THROW(RESOURCE_NUMERIC_CLEAN_ERROR);
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
            case RESOURCE_NUMERIC_CLEAN_ERROR:
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
    FLOM_TRACE(("flom_resource_numeric_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_numeric_clean(flom_resource_t *resource,
                                flom_conn_t *conn)
{
    enum Exception { NULL_OBJECT
                     , NUMERIC_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_numeric_clean\n"));
    TRY {
        GSList *p = NULL;

        if (NULL == resource)
            THROW(NULL_OBJECT);
        /* check if the connection keeps a lock */
        p = resource->data.numeric.holders;
        while (NULL != p) {
            if (((struct flom_rsrc_conn_lock_s *)p->data)->conn == conn)
                break;
            else
                p = p->next;
        } /* while (NULL != p) */
        if (NULL != p) {
            struct flom_rsrc_conn_lock_s *cl =
                (struct flom_rsrc_conn_lock_s *)p->data;
            FLOM_TRACE(("flom_resource_numeric_clean: the client is holding "
                        "a lock with quantity %d, removing it...\n",
                        cl->info.quantity));
            FLOM_TRACE(("flom_resource_numeric_clean: cl=%p\n", cl));
            resource->data.numeric.holders = g_slist_remove(
                resource->data.numeric.holders, cl);
            resource->data.numeric.locked_quantity -= cl->info.quantity;
            /* free the now useless connection lock record */
            flom_rsrc_conn_lock_delete(cl);
            /*
            FLOM_TRACE(("flom_resource_numeric_clean: g_slist_length=%u\n",
                        g_slist_length(resource->data.numeric.holders)));
            */
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_numeric_waitings(
                                   resource)))
                THROW(NUMERIC_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.numeric.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_numeric_clean: the client is "
                                "waiting for a lock with quantity %d, "
                                "removing it...\n", cl->info.quantity));
                    cl = g_queue_pop_nth(resource->data.numeric.waitings, i);
                    if (NULL == cl) {
                        /* this should be impossibile because peek was ok
                           some rows above */
                        THROW(INTERNAL_ERROR);
                    } else {
                        /* free the now useless connection lock record */
                        flom_rsrc_conn_lock_delete(cl);
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
            case NUMERIC_WAITINGS_ERROR:
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
    FLOM_TRACE(("flom_resource_numeric_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_numeric_free(flom_resource_t *resource)
{    
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_numeric_free: cleaning-up holders list...\n"));
    while (NULL != resource->data.numeric.holders) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)
            resource->data.numeric.holders->data;
        resource->data.numeric.holders = g_slist_remove(
            resource->data.numeric.holders, cl);
        flom_rsrc_conn_lock_delete(cl);
    }
    resource->data.numeric.holders = NULL;
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_numeric_free: cleaning-up waitings queue...\n"));
    while (!g_queue_is_empty(resource->data.numeric.waitings)) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)g_queue_pop_head(
                resource->data.numeric.waitings);
        flom_rsrc_conn_lock_delete(cl);
    }
    g_queue_free(resource->data.numeric.waitings);
    resource->data.numeric.waitings = NULL;
    resource->data.numeric.total_quantity =
        resource->data.numeric.locked_quantity = 0;
    /* releasing resource name */
    if (NULL != resource->name)
        g_free(resource->name);
    resource->name = NULL;
}



int flom_resource_numeric_waitings(flom_resource_t *resource)
{
    enum Exception { INTERNAL_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    
    FLOM_TRACE(("flom_resource_numeric_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        
        /* check if there is any connection waiting for a lock */
        do {
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.numeric.waitings, i);
            if (NULL == cl)
                break;
            /* try to apply this lock... */
            if (flom_resource_numeric_can_lock(resource, cl->info.quantity)) {
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.numeric.waitings, i);
                if (NULL == cl)
                    /* this should be impossibile because peek was ok
                       some rows above */
                    THROW(INTERNAL_ERROR);
                FLOM_TRACE(("flom_resource_numeric_waitings: asked lock "
                            "quantity %d can be assigned to connection "
                            "%p\n", cl->info.quantity, cl->conn));
                /* send a message to the client that's waiting the lock */
                flom_msg_init(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                       &msg, FLOM_MSG_VERB_LOCK,
                                       3*FLOM_MSG_STEP_INCR,
                                       FLOM_RC_OK, NULL)))
                    THROW(MSG_BUILD_ANSWER_ERROR);
                if (FLOM_RC_OK != (
                        ret_cod = flom_msg_serialize(
                            &msg, buffer, sizeof(buffer), &to_send)))
                    THROW(MSG_SERIALIZE_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_conn_send(
                                       cl->conn, buffer, to_send)))
                    THROW(MSG_SEND_ERROR);
                flom_conn_set_last_step(cl->conn, msg.header.pvs.step);
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
                    THROW(MSG_FREE_ERROR);                
                /* insert into holders */
                resource->data.numeric.holders = g_slist_prepend(
                    resource->data.numeric.holders,
                    (gpointer)cl);
                resource->data.numeric.locked_quantity += cl->info.quantity;
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
        flom_rsrc_conn_lock_delete(cl);
    }
    FLOM_TRACE(("flom_resource_numeric_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

