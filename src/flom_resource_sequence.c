/*
 * Copyright (c) 2013-2018, Christian Ferrari <tiian@users.sourceforge.net>
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
#include "flom_resource_sequence.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_SEQUENCE



int flom_resource_sequence_can_lock(flom_resource_t *resource)
{    
    FLOM_TRACE(("flom_resource_sequence_can_lock: "
                "total_quantity=%d, locked_quantity=%d\n",
                resource->data.sequence.total_quantity,
                resource->data.sequence.locked_quantity));
    if (resource->data.sequence.total_quantity -
        resource->data.sequence.locked_quantity > 0)
        return TRUE;
    return FALSE;
}



guint flom_resource_sequence_get(flom_resource_t *resource)
{
    gpointer pop;
    guint ret_val;
    FLOM_TRACE(("flom_resource_sequence_get\n"));
    /* are there rolled back values? */
    if ((NULL == resource->data.sequence.rolled_back) ||
        (NULL == (pop = g_queue_pop_head(
                      resource->data.sequence.rolled_back)))) {
        ret_val = resource->data.sequence.next_value++;
        if (!ret_val) /* value 0 can not be stored in the g_queue */
            ret_val = resource->data.sequence.next_value++;
    } else
        ret_val = GPOINTER_TO_UINT(pop);
    FLOM_TRACE(("flom_resource_sequence_get: %u (rolled_back=%d, "
                "pop=%p)\n", ret_val, resource->data.sequence.rolled_back,
                pop));
    return ret_val;
}



int flom_resource_sequence_init(flom_resource_t *resource,
                                const gchar *name)
{
    enum Exception { G_STRDUP_ERROR
                     , RSRC_GET_NUMBER_ERROR
                     , G_QUEUE_NEW_ERROR1
                     , G_QUEUE_NEW_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_sequence_init\n"));
    TRY {
        if (NULL == (resource->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR);
        FLOM_TRACE(("flom_resource_sequence_init: initialized resource "
                    "('%s')\n", resource->name));

        if (FLOM_RC_OK != (ret_cod = flom_rsrc_get_number(
                               name, FLOM_RSRC_TYPE_SEQUENCE,
                               &(resource->data.sequence.total_quantity))))
                    THROW(RSRC_GET_NUMBER_ERROR);
        resource->data.sequence.locked_quantity = 0;
        resource->data.sequence.next_value = 1;
        /* is this sequence transactional? */
        if (flom_rsrc_get_transactional(resource->name)) {
            if (NULL == (resource->data.sequence.rolled_back = g_queue_new()))
                THROW(G_QUEUE_NEW_ERROR1);
        } else
            resource->data.sequence.rolled_back = NULL;
        resource->data.sequence.holders = NULL;
        if (NULL == (resource->data.sequence.waitings = g_queue_new()))
            THROW(G_QUEUE_NEW_ERROR2);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case RSRC_GET_NUMBER_ERROR:
                break;
            case G_QUEUE_NEW_ERROR1:
            case G_QUEUE_NEW_ERROR2:
                ret_cod = FLOM_RC_G_QUEUE_NEW_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_sequence_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_sequence_inmsg(flom_resource_t *resource,
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
                     , OBJ_CORRUPTED
                     , RESOURCE_SEQUENCE_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_resource_sequence_inmsg\n"));
    TRY {
        int can_lock = TRUE;
        int can_wait = TRUE;
        int impossible_lock = FALSE;
        gchar element[30]; /* it must contain a guint */
        GSList *p;
        struct flom_rsrc_conn_lock_s *cl = NULL;
        
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                can_lock = flom_resource_sequence_can_lock(resource);
                can_wait = msg->body.lock_8.resource.wait;
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                if (can_lock) {
                    /* get the lock */
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_sequence_inmsg: asked lock "
                                "can be assigned to connection "
                                "%p\n", conn));
                    if (NULL == (cl = flom_rsrc_conn_lock_new()))
                        THROW(G_TRY_MALLOC_ERROR1);
                    cl->info.sequence_value =
                        flom_resource_sequence_get(resource);
                    snprintf(element, sizeof(element), "%u",
                             cl->info.sequence_value);
                    cl->rollback = TRUE;
                    cl->conn = conn;
                    resource->data.sequence.holders = g_slist_prepend(
                        resource->data.sequence.holders,
                        (gpointer)cl);
                    resource->data.sequence.locked_quantity++;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                           msg, FLOM_MSG_VERB_LOCK,
                                           flom_conn_get_last_step(conn) +
                                           FLOM_MSG_STEP_INCR,
                                           FLOM_RC_OK, element)))
                        THROW(MSG_BUILD_ANSWER_ERROR1);
                } else {
                    /* can't lock, enqueue */
                    if (can_wait) {
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_sequence_inmsg: "
                                    "lock can not be assigned to "
                                    "connection %p, queing...\n", conn));
                        if (NULL == (cl = flom_rsrc_conn_lock_new()))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->conn = conn;
                        g_queue_push_tail(
                            resource->data.sequence.waitings,
                            (gpointer)cl);
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_sequence_inmsg: asked "
                                    "lock can not be assigned to "
                                    "connection %p, rejecting...\n", conn));
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
                    FLOM_TRACE(("flom_resource_sequence_inmsg: client wants "
                                "to unlock resource '%s' while it's locking "
                                "resource '%s'\n",
                                msg->body.unlock_8.resource.name,
                                flom_resource_get_name(resource)));
                    syslog(LOG_WARNING, FLOM_SYSLOG_FLM009W,
                           msg->body.unlock_8.resource.name,
                           flom_resource_get_name(resource));
                    THROW(INVALID_OPTION);
                }
                /* commit the sequence to avoid re-use */
                if (NULL == (p = flom_rsrc_conn_find(
                                 resource->data.sequence.holders, conn))) {
                    FLOM_TRACE(("flom_resource_sequence_inmsg: unable to "
                                "find lock holder for connection %p\n",
                                conn));
                    THROW(OBJ_CORRUPTED);
                }
                cl = (struct flom_rsrc_conn_lock_s *)p->data;
                cl->rollback = msg->body.unlock_8.resource.rollback;
                /* clean lock */
                if (FLOM_RC_OK != (ret_cod = flom_resource_sequence_clean(
                                       resource, conn)))
                    THROW(RESOURCE_SEQUENCE_CLEAN_ERROR);
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
            case OBJ_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case RESOURCE_SEQUENCE_CLEAN_ERROR:
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
    FLOM_TRACE(("flom_resource_sequence_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_sequence_clean(flom_resource_t *resource,
                                 flom_conn_t *conn)
{
    enum Exception { NULL_OBJECT
                     , SEQUENCE_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_sequence_clean\n"));
    TRY {
        GSList *p = NULL;

        if (NULL == resource)
            THROW(NULL_OBJECT);
        /* check if the connection keeps a lock */
        if (NULL != (p = flom_rsrc_conn_find(
                         resource->data.sequence.holders,conn))) {
            struct flom_rsrc_conn_lock_s *cl =
                (struct flom_rsrc_conn_lock_s *)p->data;
            FLOM_TRACE(("flom_resource_sequence_clean: the client is holding "
                        "a%s lock with sequence %u, removing it...\n",
                        cl->rollback ? "n uncommitted" : " commited",
                        cl->info.sequence_value));
            if ((NULL != resource->data.sequence.rolled_back) &&
                cl->rollback) {
                /* put the rolled back value in the queue */
                g_queue_push_tail(resource->data.sequence.rolled_back,
                                  GUINT_TO_POINTER(cl->info.sequence_value));
            } /* if (cl->rollback) */
            FLOM_TRACE(("flom_resource_sequence_clean: cl=%p\n", cl));
            resource->data.sequence.holders = g_slist_remove(
                resource->data.sequence.holders, cl);
            resource->data.sequence.locked_quantity--;
            /* free the now useless connection lock record */
            flom_rsrc_conn_lock_delete(cl);
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_sequence_waitings(
                                   resource)))
                THROW(SEQUENCE_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.sequence.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_sequence_clean: the client is "
                                "waiting for a lock, removing it...\n"));
                    cl = g_queue_pop_nth(resource->data.sequence.waitings, i);
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
            case SEQUENCE_WAITINGS_ERROR:
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
    FLOM_TRACE(("flom_resource_sequence_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_sequence_free(flom_resource_t *resource)
{    
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_sequence_free: cleaning-up holders list...\n"));
    while (NULL != resource->data.sequence.holders) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)
            resource->data.sequence.holders->data;
        resource->data.sequence.holders = g_slist_remove(
            resource->data.sequence.holders, cl);
        flom_rsrc_conn_lock_delete(cl);
    }
    resource->data.sequence.holders = NULL;
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_sequence_free: cleaning-up waitings "
                "queue...\n"));
    while (!g_queue_is_empty(resource->data.sequence.waitings)) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)g_queue_pop_head(
                resource->data.sequence.waitings);
        flom_rsrc_conn_lock_delete(cl);
    }
    g_queue_free(resource->data.sequence.waitings);
    /* clean-up rolled back queue... */
    if (NULL != resource->data.sequence.rolled_back) {
        FLOM_TRACE(("flom_resource_sequence_free: cleaning-up rolled back "
                    "queue...\n"));
        g_queue_free(resource->data.sequence.rolled_back);
    }
    
    resource->data.sequence.waitings = NULL;
    resource->data.sequence.total_quantity =
        resource->data.sequence.locked_quantity = 0;
    /* releasing resource name */
    if (NULL != resource->name)
        g_free(resource->name);
    resource->name = NULL;
}



int flom_resource_sequence_waitings(flom_resource_t *resource)
{
    enum Exception { INTERNAL_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    
    FLOM_TRACE(("flom_resource_sequence_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        gchar element[30]; /* it must contain a guint */
        
        /* check if there is any connection waiting for a lock */
        do {
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.sequence.waitings, i);
            if (NULL == cl)
                break;
            /* try to apply this lock... */
            if (flom_resource_sequence_can_lock(resource)) {
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.sequence.waitings, i);
                if (NULL == cl)
                    /* this should be impossibile because peek was ok
                       some rows above */
                    THROW(INTERNAL_ERROR);
                FLOM_TRACE(("flom_resource_sequence_waitings: asked lock "
                            "can be assigned to connection %p\n",
                            cl->conn));
                cl->info.sequence_value = flom_resource_sequence_get(resource);
                snprintf(element, sizeof(element), "%u",
                         cl->info.sequence_value);
                cl->rollback = TRUE;
                /* send a message to the client that's waiting the lock */
                flom_msg_init(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                       &msg, FLOM_MSG_VERB_LOCK,
                                       3*FLOM_MSG_STEP_INCR,
                                       FLOM_RC_OK, element)))
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
                resource->data.sequence.holders = g_slist_prepend(
                    resource->data.sequence.holders,
                    (gpointer)cl);
                resource->data.sequence.locked_quantity++;
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
    FLOM_TRACE(("flom_resource_sequence_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

