/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_set.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_SET



int flom_resource_set_can_lock(flom_resource_t *resource, guint *element)
{
    guint i;
    int found = FALSE;
    
    FLOM_TRACE(("flom_resource_set_can_lock\n"));
    /* loop around elements starting from the index element */
    for (i=0; i<resource->data.set.elements->len; ++i) {
        /* i loops between 0 and len-1
           j loops between index and index-1 traversing len-1 and 0 */
        guint j = (i + resource->data.set.index) %
            resource->data.set.elements->len;
        struct flom_rsrc_data_set_element_s *rdse =
            &g_array_index(resource->data.set.elements,
                           struct flom_rsrc_data_set_element_s, j);
        if (NULL == rdse->conn) {
            *element = j;
            found = TRUE;
            break;
        }
    } /* for i */
    return found;
}



int flom_resource_set_init(flom_resource_t *resource,
                           const gchar *name)
{
    enum Exception { G_STRDUP_ERROR
                     , G_ARRAY_NEW_ERROR
                     , RSRC_GET_ELEMENTS_ERROR
                     , G_QUEUE_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_set_init\n"));
    TRY {
        if (NULL == (resource->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR);
        FLOM_TRACE(("flom_resource_set_init: initialized resource ('%s')\n",
                    resource->name));
        
        if (NULL == (resource->data.set.elements = g_array_new(
                         FALSE, FALSE,
                         sizeof(struct flom_rsrc_data_set_element_s))))
            THROW(G_ARRAY_NEW_ERROR);
        if (FLOM_RC_OK != (ret_cod =
                           flom_rsrc_get_elements(
                               name, resource->data.set.elements)))
            THROW(RSRC_GET_ELEMENTS_ERROR);
        resource->data.set.index = 0;
        if (NULL == (resource->data.set.waitings = g_queue_new()))
            THROW(G_QUEUE_NEW_ERROR);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case G_ARRAY_NEW_ERROR:
                ret_cod = FLOM_RC_G_ARRAY_NEW_ERROR;
                break;
            case RSRC_GET_ELEMENTS_ERROR:
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
    FLOM_TRACE(("flom_resource_set_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_set_inmsg(flom_resource_t *resource,
                            flom_conn_t *conn,
                            struct flom_msg_s *msg)
{
    enum Exception { MSG_FREE_ERROR1
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , INVALID_OPTION
                     , RESOURCE_SET_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_resource_set_inmsg\n"));
    TRY {
        int can_lock = TRUE;
        int can_wait = TRUE;
        guint element;
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                can_lock = flom_resource_set_can_lock(resource, &element);
                can_wait = msg->body.lock_8.resource.wait;
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                if (can_lock) {
                    /* get the lock */
                    struct flom_rsrc_data_set_element_s *rdse =
                        &g_array_index(resource->data.set.elements,
                                       struct flom_rsrc_data_set_element_s,
                                       element);
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_set_inmsg: element %u ('%s') "
                                "can be assigned to connection %p\n",
                                element, rdse->name, conn));
                    /* track locker connection */
                    rdse->conn = conn;
                    /* move to next element for next locker (round robin) */
                    resource->data.set.index = (resource->data.set.index + 1) %
                        resource->data.set.elements->len;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                           msg, FLOM_MSG_VERB_LOCK,
                                           conn->last_step +
                                           FLOM_MSG_STEP_INCR,
                                           FLOM_RC_OK, rdse->name)))
                        THROW(MSG_BUILD_ANSWER_ERROR1);
                } else {
                    /* can't lock, enqueue */
                    if (can_wait) {
                        struct flom_rsrc_conn_lock_s *cl = NULL;
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_set_inmsg: there is no "
                                    "available element for "
                                    "connection %p, queing...\n", conn));
                        if (NULL == (cl = flom_rsrc_conn_lock_new()))
                            THROW(G_TRY_MALLOC_ERROR);
                        cl->conn = conn;
                        g_queue_push_tail(
                            resource->data.set.waitings,
                            (gpointer)cl);
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               conn->last_step +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_set_inmsg: there is no "
                                    "available element for connection %p, "
                                    "rejecting...\n", conn));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               conn->last_step +
                                               FLOM_MSG_STEP_INCR,
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
                    FLOM_TRACE(("flom_resource_set_inmsg: client wants to "
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
                if (FLOM_RC_OK != (ret_cod = flom_resource_set_clean(
                                       resource, conn)))
                    THROW(RESOURCE_SET_CLEAN_ERROR);
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
            case MSG_BUILD_ANSWER_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR2:
            case MSG_BUILD_ANSWER_ERROR3:
                break;
            case INVALID_OPTION:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case RESOURCE_SET_CLEAN_ERROR:
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
    FLOM_TRACE(("flom_resource_set_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_set_clean(flom_resource_t *resource,
                            flom_conn_t *conn)
{
    enum Exception { NULL_OBJECT
                     , SET_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_set_clean\n"));
    TRY {
        guint element;
        int found = FALSE;
        struct flom_rsrc_data_set_element_s *rdse = NULL;

        if (NULL == resource)
            THROW(NULL_OBJECT);
        /* check if the connection keeps a lock */
        for (element=0; element<resource->data.set.elements->len;
             ++element) {
            rdse = &g_array_index(resource->data.set.elements,
                                  struct flom_rsrc_data_set_element_s,
                                  element);
            if (rdse->conn == conn) {
                found = TRUE;
                break;
            }
        } /* for (element=0; ... */
        if (found) {
            FLOM_TRACE(("flom_resource_set_clean: the client is holding "
                        "element %u ('%s'), removing connection %p from "
                        "it...\n", element, rdse->name, conn));
            rdse->conn = NULL;
            if (FLOM_RC_OK != (ret_cod = flom_resource_set_waitings(
                                   resource)))
                THROW(SET_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.set.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_set_clean: the client is "
                                "waiting to a lock an element, "
                                "removing it...\n"));
                    cl = g_queue_pop_nth(resource->data.set.waitings, i);
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
            case SET_WAITINGS_ERROR:
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
    FLOM_TRACE(("flom_resource_set_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_set_free(flom_resource_t *resource)
{
    guint i;
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_set_free: cleaning-up elements array...\n"));
    for (i=0; i<resource->data.set.elements->len; ++i) {
        struct flom_rsrc_data_set_element_s *rdse =
            &g_array_index(resource->data.set.elements,
                           struct flom_rsrc_data_set_element_s, i);
        FLOM_TRACE(("flom_resource_set_free: removing element %u ('%s')\n",
                    i, rdse->name));
        g_free(rdse->name);
        rdse->name = NULL;
        rdse->conn = NULL;
    }
    /* free array structure */
    g_array_free(resource->data.set.elements, TRUE);
    resource->data.set.elements = NULL;
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_set_free: cleaning-up waitings queue...\n"));
    while (!g_queue_is_empty(resource->data.set.waitings)) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)g_queue_pop_head(
                resource->data.set.waitings);
        flom_rsrc_conn_lock_delete(cl);
    }
    g_queue_free(resource->data.set.waitings);
    resource->data.set.waitings = NULL;
    resource->data.set.index = 0;
    /* releasing resource name */
    if (NULL != resource->name)
        g_free(resource->name);
    resource->name = NULL;
}



int flom_resource_set_waitings(flom_resource_t *resource)
{
    enum Exception { INTERNAL_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    
    FLOM_TRACE(("flom_resource_set_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        
        /* check if there is any connection waiting for a lock */
        do {
            guint element;
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.set.waitings, i);
            if (NULL == cl)
                break;
            /* try to apply this lock... */
            if (flom_resource_set_can_lock(resource, &element)) {
                struct flom_rsrc_data_set_element_s *rdse =
                    &g_array_index(resource->data.set.elements,
                                   struct flom_rsrc_data_set_element_s,
                                   element);
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.set.waitings, i);
                if (NULL == cl)
                    /* this should be impossibile because peek was ok
                       some rows above */
                    THROW(INTERNAL_ERROR);
                FLOM_TRACE(("flom_resource_set_waitings: element %u ('%s') "
                            "can be assigned to connection %p\n",
                            element, rdse->name, cl->conn));
                /* send a message to the client that's waiting the lock */
                flom_msg_init(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                       &msg, FLOM_MSG_VERB_LOCK,
                                       3*FLOM_MSG_STEP_INCR,
                                       FLOM_RC_OK, rdse->name)))
                    THROW(MSG_BUILD_ANSWER_ERROR);
                if (FLOM_RC_OK != (
                        ret_cod = flom_msg_serialize(
                            &msg, buffer, sizeof(buffer), &to_send)))
                    THROW(MSG_SERIALIZE_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_tcp_send(
                                       flom_tcp_get_sockfd(
                                           flom_conn_get_tcp(cl->conn)),
                                       buffer, to_send)))
                    THROW(MSG_SEND_ERROR);
                cl->conn->last_step = msg.header.pvs.step;
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
                    THROW(MSG_FREE_ERROR);                
                /* track locker connection */
                rdse->conn = cl->conn;
                /* move to next element for next locker (round robin) */
                resource->data.set.index = (resource->data.set.index + 1) %
                    resource->data.set.elements->len;
                /* free cl */
                flom_rsrc_conn_lock_delete(cl);
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
    FLOM_TRACE(("flom_resource_set_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

