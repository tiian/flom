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



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_config.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RSRC



/* global static objects */
regex_t global_res_name_preg[FLOM_RSRC_TYPE_N];



int global_res_name_preg_init()
{
    enum Exception { SNPRINTF_ERROR
                     , REGCOMP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("global_res_name_preg_init\n"));
    TRY {
        int reg_error;
        char reg_errbuf[200];
        char reg_expr[1000];
        flom_rsrc_type_t i;
        const char *reg_str[FLOM_RSRC_TYPE_N] = {
            "^_$" /* this is a dummy value */ ,
            "^%s|([[:alpha:][:digit:]])+$" };

        for (i=FLOM_RSRC_TYPE_NULL; i<FLOM_RSRC_TYPE_N; ++i) {
            /* preparing regular expression */
            if (sizeof(reg_expr) <= snprintf(
                    reg_expr, sizeof(reg_expr), reg_str[i],
                    DEFAULT_RESOURCE_NAME))
                THROW(SNPRINTF_ERROR);
            FLOM_TRACE(("global_res_name_preg_init: regular expression for "
                        "type %d is '%s'\n", i, reg_expr));
            reg_error = regcomp(global_res_name_preg+i, reg_expr,
                                REG_EXTENDED|REG_NOSUB|REG_NEWLINE);
            if (0 != reg_error) {
                regerror(reg_error, global_res_name_preg+i,
                         reg_errbuf, sizeof(reg_errbuf));
                FLOM_TRACE(("global_res_name_preg_init: regcomp returned %d "
                            "('%s') instead of 0\n", reg_error, reg_errbuf));
                THROW(REGCOMP_ERROR);
            }
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SNPRINTF_ERROR:
                ret_cod = FLOM_RC_SNPRINTF_ERROR;
                break;
            case REGCOMP_ERROR:
                ret_cod = FLOM_RC_REGCOMP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("global_res_name_preg_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



flom_rsrc_type_t flom_rsrc_get_type(const gchar *resource_name)
{
    int reg_error;
    char reg_errbuf[200];
    flom_rsrc_type_t i;
    flom_rsrc_type_t ret_cod = FLOM_RSRC_TYPE_NULL;
    
    FLOM_TRACE(("flom_rsrc_get_type\n"));

    for (i=FLOM_RSRC_TYPE_NULL+1; i<FLOM_RSRC_TYPE_N; ++i) {
        reg_error = regexec(global_res_name_preg+i,
                            resource_name, 0, NULL, 0);
        regerror(reg_error, global_res_name_preg+i, reg_errbuf,
                 sizeof(reg_errbuf));
        FLOM_TRACE(("flom_rsrc_get_type: regexec returned "
                    "%d ('%s') for string '%s'\n",
                    reg_error, reg_errbuf, resource_name));
        if (0 == reg_error) {
            ret_cod = i;
            break;
        } /* if (0 == reg_error) */
    } /* for (i=FLOM_RSRC_RES_TYPE_NULL+1; ... */
    FLOM_TRACE(("flom_rsrc_get_type/ret_cod=%d\n", ret_cod));
    return ret_cod;
}



int flom_resource_init(flom_resource_t *resource,
                       flom_rsrc_type_t type, const gchar *name)
{
    enum Exception { OUT_OF_RANGE
                     , G_QUEUE_NEW_ERROR
                     , UNKNOW_RESOURCE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_init\n"));
    TRY {
        if (FLOM_RSRC_TYPE_NULL >= type || FLOM_RSRC_TYPE_N <= type)
            THROW(OUT_OF_RANGE);
        resource->type = type;
        resource->name = g_strdup(name);
        FLOM_TRACE(("flom_resource_init: initialized resource ('%s',%d)\n",
                    resource->name, resource->type));

        switch (resource->type) {
            case FLOM_RSRC_TYPE_SIMPLE:
                resource->data.simple.holders = NULL;
                if (NULL == (resource->data.simple.waitings = g_queue_new()))
                    THROW(G_QUEUE_NEW_ERROR);
                resource->inmsg = flom_resource_simple_inmsg;
                resource->clean = flom_resource_simple_clean;
                break;
            default:
                THROW(UNKNOW_RESOURCE);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case G_QUEUE_NEW_ERROR:
                ret_cod = FLOM_RC_G_QUEUE_NEW_ERROR;
                break;
            case UNKNOW_RESOURCE:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_free(flom_resource_t *resource)
{
    FLOM_TRACE(("flom_resource_free\n"));
    if (NULL != resource->name)
        g_free(resource->name);
    resource->name = NULL;
}



int flom_resource_simple_inmsg(flom_resource_t *resource,
                               struct flom_conn_data_s *conn,
                               struct flom_msg_s *msg)
{
    enum Exception { MSG_FREE_ERROR
                     , G_TRY_MALLOC_ERROR1
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    static const flom_lock_type_t lock_table[
        FLOM_LOCK_TYPE_N][FLOM_LOCK_TYPE_N] =
        { { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE } ,
          { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  FALSE } ,
          { TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE } ,
          { TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE } };
    
    FLOM_TRACE(("flom_resource_simple_inmsg\n"));
    TRY {
        GSList *p = NULL;
        flom_lock_type_t old_lock , new_lock = FLOM_LOCK_TYPE_NL;
        int can_lock = TRUE;
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                p = resource->data.simple.holders;
                new_lock = msg->body.lock_8.resource.type;
                while (NULL != p) {
                    old_lock = ((struct flom_rsrc_conn_lock_s *)
                                p->data)->lock_type;
                    FLOM_TRACE(("flom_resource_simple_inmsg: current_lock=%d, "
                                "asked_lock=%d, lock_table[%d][%d]=%d\n",
                                old_lock, new_lock, old_lock, new_lock,
                                lock_table[old_lock][new_lock]));
                    can_lock &= lock_table[old_lock][new_lock];
                    if (!can_lock)
                        break;
                    else
                        p = p->next;
                } /* while (NULL != p) */
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR);
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
                    cl->lock_type = new_lock;
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
                    if (msg->body.lock_8.resource.wait) {
                        struct flom_rsrc_conn_lock_s *cl = NULL;
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_simple_inmsg: asked lock %d "
                                    "can not be assigned to connection %p, "
                                    "queing...\n", new_lock, conn));
                        if (NULL == (cl = (struct flom_rsrc_conn_lock_s *)
                                     g_try_malloc(
                                         sizeof(struct flom_rsrc_conn_lock_s))))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->lock_type = new_lock;
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
                        FLOM_TRACE(("flom_resource_simple_inmsg: asked lock %p "
                                    "can not be assigned to connection %p, "
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
                break;
            default:
                THROW(PROTOCOL_ERROR);
        } /* switch (msg->header.pvs.verb) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_FREE_ERROR:
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
            FLOM_TRACE(("flom_resource_simple_clean: the client is holding "
                        "a lock of type %d, removing it...\n",
                        ((struct flom_rsrc_conn_lock_s *)p->data)->lock_type));
            resource->data.simple.holders = g_slist_remove(
                resource->data.simple.holders, p->data);
            /*
            FLOM_TRACE(("flom_resource_simple_clean: g_slist_length=%u\n",
                        g_slist_length(resource->data.simple.holders)));
            */
            /* check if the some other clients can get a lock now */
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
                /* try to apply this lock... */
                /* @@@ extract the logic from simple_inmsg and put in a
                   distinct function ... */
            } while (TRUE);
            
        }
                
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case SIMPLE_WAITINGS_ERROR:
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



int flom_resource_simple_waitings(flom_resource_t *resource)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_simple_waitings\n"));
    TRY {
        /* @@@ */
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
    FLOM_TRACE(("flom_resource_simple_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

