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



#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_hier.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_HIER



int flom_resource_hier_can_lock(struct flom_rsrc_data_hier_element_s *node,
                                flom_lock_mode_t lock, gchar **level_name)
{
    static const flom_lock_mode_t lock_table[
        FLOM_LOCK_MODE_N][FLOM_LOCK_MODE_N] =
        { { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE } ,
          { TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  FALSE } ,
          { TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, TRUE,  FALSE, FALSE } ,
          { TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE } ,
          { TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE } };
    
    GSList *p;
    flom_lock_mode_t old_lock;
    int can_lock = TRUE;
    
    FLOM_TRACE(("flom_resource_hier_can_lock: node->name='%s', "
                "level_name='%s', checking lock=%d\n", node->name,
                *level_name != NULL ? *level_name : FLOM_NULL_STRING, lock));
    if (NULL != *level_name) {
        if (g_strcmp0(*level_name, node->name)) {
            FLOM_TRACE(("flom_resource_hier_can_lock: node->name is "
                        "different than level_name, leaving...\n"));
            return can_lock;
        } /* if (g_strcmp0(*level_name, node->name)) */
    } /* if (NULL != *level_name) */
    /* check locks kept by all the holders of this level */
    p = node->holders;
    while (NULL != p) {
        old_lock = ((struct flom_rsrc_conn_lock_s *)p->data)->info.lock_mode;
        FLOM_TRACE(("flom_resource_hier_can_lock: current_lock=%d, "
                    "asked_lock=%d, lock_table[%d][%d]=%d\n",
                    old_lock, lock, old_lock, lock,
                    lock_table[old_lock][lock]));
        can_lock &= lock_table[old_lock][lock];
        if (!can_lock)
            break;
        else
            p = p->next;
    } /* while (NULL != p) */
    /* checking must go deeper? */
    if (can_lock) {
        guint i;
        /* resource name levels are terminated, check the locks kept by
           all leaves */
        for (i=0; i<node->leaves->len; ++i) {
            can_lock = flom_resource_hier_can_lock(
                g_ptr_array_index(node->leaves, i), lock,
                NULL == *level_name ? level_name : level_name+1);
            if (!can_lock)
                break;
        } /* for (i=0; i<node->leaves.len; ++i) */
    } /* if (can_lock) */
    return can_lock;
}



int flom_resource_hier_add_locker(flom_resource_t *resource,
                                  struct flom_rsrc_conn_lock_s *cl,
                                  gchar **splitted_name)
{
    enum Exception { G_TRY_MALLOC_ERROR
                     , G_STRDUP_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_hier_add_locker\n"));
    TRY {
        struct flom_rsrc_data_hier_element_s *node = resource->data.hier.root;
        gchar **level_name;
        int node_is_leaf = TRUE;
        int continue_search = TRUE;

        flom_resource_hier_trace(resource);
        for (level_name=splitted_name; *level_name; ++level_name) {
            node_is_leaf = TRUE;
            FLOM_TRACE(("flom_resource_hier_add_locker: "
                        "*level_name='%s', node->name='%s'\n",
                        STRORNULL(*level_name), STRORNULL(node->name)));
            if (continue_search && !g_strcmp0(*level_name, node->name)) {
                /* go one level depth */
                guint i;
                continue_search = FALSE;
                for (i=0; i<node->leaves->len; ++i) {
                    struct flom_rsrc_data_hier_element_s *leaf =
                        g_ptr_array_index(node->leaves, i);
                    node_is_leaf = FALSE;
                    FLOM_TRACE(("flom_resource_hier_add_locker: "
                                "*(level_name+1)='%s', leaf->name='%s'\n",
                                STRORNULL(*(level_name+1)),
                                STRORNULL(leaf->name)));
                    if (!g_strcmp0(*(level_name+1), leaf->name)) {
                        node = leaf;
                        continue_search = TRUE;
                        break;
                    } /* if (!g_strcmp0(*(level_name+1), leaf->name)) */
                } /* for (i=0; i<node->leaves->len; ++i) */
            } else {
                break;
            } /* if (!g_strcmp0(*level_name, node->name)) */
        } /* for (level_name=splitted_name; *level_name; ++level_name) */

        /* check if the tree must be expanded */
        if (NULL != *level_name && TRUE == node_is_leaf) {
            /* new resource is longer than previous ones, "node" points to
               a leaf tree node and must be extended */
            struct flom_rsrc_data_hier_element_s *frdhe;
            for (; *level_name; ++level_name) {
                FLOM_TRACE(("flom_resource_hier_add_locker: adding node "
                            "*level_name='%s'\n", *level_name));
                /* allocating a new leaf */
                if (NULL == (frdhe = (struct flom_rsrc_data_hier_element_s *)
                             g_try_malloc(
                                 sizeof(struct flom_rsrc_data_hier_element_s))))
                    THROW(G_TRY_MALLOC_ERROR);
                if (NULL == (frdhe->name = g_strdup(*level_name)))
                    THROW(G_STRDUP_ERROR);
                frdhe->holders = NULL;
                frdhe->leaves = g_ptr_array_new();
                /* link father to this element */
                g_ptr_array_add(node->leaves, frdhe);
                node = frdhe;
            } /* for (; *level_name; ++level_name) */
        } else if (NULL != *level_name && FALSE == node_is_leaf) {
            /* this is an internal error that should never happen */
            THROW(INTERNAL_ERROR);
        }
        /* if (NULL == *level_name)
           FALSE == node_is_leaf:
           new resource is shorter than previous ones, "node" points to
           a non leaf tree node
           TRUE == node_is_leaf:
           new resource is exactly matches a previous one, "node" points
           to a leaf tree node */
        node->holders = g_slist_prepend(node->holders, (gpointer)cl);

        flom_resource_hier_trace(resource);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
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
    FLOM_TRACE(("flom_resource_hier_add_locker/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_hier_init(flom_resource_t *resource,
                            const gchar *name)
{
    enum Exception { INVALID_RESOURCE_NAME
                     , G_STRDUP_ERROR1
                     , G_STRSPLIT_ERROR
                     , G_TRY_MALLOC_ERROR
                     , G_STRDUP_ERROR2
                     , G_QUEUE_NEW_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    gchar **splitted_name = NULL;
    
    FLOM_TRACE(("flom_resource_hier_init\n"));
    TRY {
        gchar **level_name;
        int i = 0;
        size_t sep_len = strlen(FLOM_HIER_RESOURCE_SEPARATOR);
        struct flom_rsrc_data_hier_element_s *father = NULL;        
        
        if (0 != strncmp(name, FLOM_HIER_RESOURCE_SEPARATOR, sep_len)) {
            FLOM_TRACE(("flom_resource_hier_init: '%s' does not start with "
                        "'%s'\n", name, FLOM_HIER_RESOURCE_SEPARATOR));
            THROW(INVALID_RESOURCE_NAME);
        }
            
        if (NULL == (resource->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR1);
        FLOM_TRACE(("flom_resource_hier_init: initializing resource ('%s')\n",
                    resource->name));
        /* prepare splitted name */
        if (NULL == (splitted_name = g_strsplit(
                         resource->name+sep_len,
                         FLOM_HIER_RESOURCE_SEPARATOR, 0)))
            THROW(G_STRSPLIT_ERROR);
        /* prepare tree structure */
        for (level_name = splitted_name; *level_name; level_name++) {
            struct flom_rsrc_data_hier_element_s *frdhe;
            FLOM_TRACE(("flom_resource_hier_init: level %d is '%s'\n",
                        i, *level_name));
            /* allocating a new leaf */
            if (NULL == (frdhe = (struct flom_rsrc_data_hier_element_s *)
                         g_try_malloc(
                             sizeof(struct flom_rsrc_data_hier_element_s))))
                THROW(G_TRY_MALLOC_ERROR);
            if (NULL == (frdhe->name = g_strdup(*level_name)))
                THROW(G_STRDUP_ERROR2);
            frdhe->holders = NULL;
            frdhe->leaves = g_ptr_array_new();
            if (!i) {
                /* link root to this element */
                resource->data.hier.root = frdhe;
                father = frdhe;
            } else {
                /* link father to this element */
                g_ptr_array_add(father->leaves, frdhe);
                father = frdhe;
            }
            i++;
        } /* for (name = ... */
        if (NULL == (resource->data.hier.waitings = g_queue_new()))
            THROW(G_QUEUE_NEW_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_RESOURCE_NAME:
                ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
                break;
            case G_STRDUP_ERROR1:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case G_STRSPLIT_ERROR:
                ret_cod = FLOM_RC_G_STRSPLIT_ERROR;
                break;
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case G_STRDUP_ERROR2:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
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
    /* free allocated memory */
    if (NULL != splitted_name) {
        g_strfreev(splitted_name);
        splitted_name = NULL;
    }
    FLOM_TRACE(("flom_resource_hier_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_hier_inmsg(flom_resource_t *resource,
                             flom_conn_t *conn,
                             struct flom_msg_s *msg)
{
    enum Exception { G_STRSPLIT_ERROR
                     , G_STRDUP_ERROR
                     , MSG_FREE_ERROR1
                     , G_TRY_MALLOC_ERROR1
                     , RESOURCE_HIER_ADD_LOCKER_ERROR
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , INVALID_RESOURCE_NAME
                     , RESOURCE_HIER_CHANGE_NAME_ERROR
                     , RESOURCE_HIER_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    gchar **splitted_name = NULL;
    gchar *resource_name = NULL;
    
    FLOM_TRACE(("flom_resource_hier_inmsg\n"));
    TRY {
        flom_lock_mode_t new_lock = FLOM_LOCK_MODE_NL;
        int can_lock = TRUE;
        int can_wait = TRUE;
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                new_lock = msg->body.lock_8.resource.mode;
                can_wait = msg->body.lock_8.resource.wait;
                /* create resource splitted name */
                if (NULL == (splitted_name = g_strsplit(
                                 msg->body.lock_8.resource.name+
                                 strlen(FLOM_HIER_RESOURCE_SEPARATOR),
                                 FLOM_HIER_RESOURCE_SEPARATOR, 0)))
                    THROW(G_STRSPLIT_ERROR);
                /* duplicate resource name before message reset */
                if (NULL == (resource_name = g_strdup(
                                 msg->body.lock_8.resource.name)))
                    THROW(G_STRDUP_ERROR);
                FLOM_TRACE(("flom_resource_hier_inmsg: connection %p wants "
                            "to lock resource '%s' using mode %d\n",
                            conn, resource_name, new_lock));
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                can_lock = flom_resource_hier_can_lock(
                    resource->data.hier.root, new_lock, splitted_name);
                if (can_lock) {
                    /* get the lock */
                    struct flom_rsrc_conn_lock_s *cl = NULL;
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_hier_inmsg: asked lock %d "
                                "can be assigned to connection %p\n",
                                new_lock, conn));
                    if (NULL == (cl = flom_rsrc_conn_lock_new()))
                        THROW(G_TRY_MALLOC_ERROR1);
                    cl->info.lock_mode = new_lock;
                    cl->conn = conn;
                    cl->name = NULL;
                    if (FLOM_RC_OK != (
                            ret_cod = flom_resource_hier_add_locker(
                                resource, cl, splitted_name)))
                        THROW(RESOURCE_HIER_ADD_LOCKER_ERROR);
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
                        FLOM_TRACE(("flom_resource_hier_inmsg: asked lock "
                                    "%d can not be assigned to connection %p, "
                                    "queing...\n", new_lock, conn));
                        if (NULL == (cl = flom_rsrc_conn_lock_new()))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->info.lock_mode = new_lock;
                        cl->conn = conn;
                        cl->name = resource_name;
                        resource_name = NULL;
                        FLOM_TRACE(("flom_resource_hier_inmsg: "
                                    "g_queue_get_length()=%u (before)\n",
                                    g_queue_get_length(
                                        resource->data.hier.waitings)));
                        g_queue_push_tail(
                            resource->data.hier.waitings,
                            (gpointer)cl);
                        FLOM_TRACE(("flom_resource_hier_inmsg: "
                                    "g_queue_get_length()=%u (after)\n",
                                    g_queue_get_length(
                                        resource->data.hier.waitings)));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_hier_inmsg: asked lock "
                                    "%d can not be assigned to connection %p, "
                                    "rejecting...\n", new_lock, conn));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_BUSY, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR3);
                    } /* if (msg->body.lock_8.resource.wait) */
                } /* if (can_lock) */
                break;
            case FLOM_MSG_VERB_UNLOCK:
                /* check resource name */
                if (flom_resource_hier_compare_name(
                        resource, msg->body.unlock_8.resource.name)) {
                    FLOM_TRACE(("flom_resource_hier_inmsg: arrived unlock "
                                "verb for resource '%s', but this locker "
                                "is managing resource '%s'\n",
                                STRORNULL(msg->body.unlock_8.resource.name),
                                STRORNULL(resource->data.hier.root->name)));
                    syslog(LOG_WARNING, FLOM_SYSLOG_FLM009W,
                           STRORNULL(msg->body.unlock_8.resource.name),
                           STRORNULL(resource->data.hier.root->name));
                    THROW(INVALID_RESOURCE_NAME);
                }
                /* set the asked unlock name at resource level */
                if (FLOM_RC_OK != (ret_cod = flom_resource_hier_change_name(
                                       resource,
                                       msg->body.unlock_8.resource.name)))
                    THROW(RESOURCE_HIER_CHANGE_NAME_ERROR);
                /* clean lock */
                if (FLOM_RC_OK != (ret_cod = flom_resource_hier_clean(
                                       resource, conn)))
                    THROW(RESOURCE_HIER_CLEAN_ERROR);
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
            case G_STRSPLIT_ERROR:
                ret_cod = FLOM_RC_G_STRSPLIT_ERROR;
                break;
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case MSG_FREE_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR1:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case RESOURCE_HIER_ADD_LOCKER_ERROR:
            case MSG_BUILD_ANSWER_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR2:
            case MSG_BUILD_ANSWER_ERROR3:
                break;
            case INVALID_RESOURCE_NAME:
                ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
                break;
            case RESOURCE_HIER_CHANGE_NAME_ERROR:
            case RESOURCE_HIER_CLEAN_ERROR:
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
    /* free allocated memory */
    if (NULL != splitted_name) {
        g_strfreev(splitted_name);
        splitted_name = NULL;
    }
    if (NULL != resource_name) {
        g_free(resource_name);
        resource_name = NULL;
    }
    FLOM_TRACE(("flom_resource_hier_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_hier_gc(
    struct flom_rsrc_data_hier_element_s *element)
{
    guint i;

    /* scan leaves before */
    if (NULL != element->leaves) {
        FLOM_TRACE(("flom_resource_hier_gc: element '%s' has %u children\n",
                    element->name, element->leaves->len));
        i = 0;
        while (i<element->leaves->len) {
            struct flom_rsrc_data_hier_element_s *child =
                g_ptr_array_index(element->leaves, i);
            flom_resource_hier_gc(child);
            /* check if the leaf is really a leaf or an internal node */
            if (NULL == child->leaves || 0 == child->leaves->len) {
                FLOM_TRACE(("flom_resource_hier_gc: child '%s' is a terminal "
                            "node (a leaf) with holders=%p\n",
                            child->name, child->holders));
                if (NULL == child->holders) {
                    FLOM_TRACE(("flom_resource_hier_gc: releasing leaf '%s'\n",
                                child->name));
                    g_free(child->name);
                    child->name = NULL;
                    if (NULL != child->leaves)
                        g_ptr_array_free(child->leaves, TRUE);
                    g_ptr_array_remove_index(element->leaves, i);
                    g_free(child);
                    continue;
                } /* if (NULL == node->holders) */
            } /* if (NULL == node->leaves || 0 == node->leaves->len) */
            i++;
        } /* while (i<element->leaves.len) */
    } /* if (NULL != element->leaves) */
}
    

    
int flom_resource_hier_clean(flom_resource_t *resource,
                             flom_conn_t *conn)
{
    enum Exception { NULL_OBJECT
                     , G_STRSPLIT_ERROR
                     , INVALID_RESOURCE_NAME
                     , HIER_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    gchar **splitted_name = NULL;
    
    FLOM_TRACE(("flom_resource_hier_clean\n"));
    TRY {
        GSList *p = NULL;
        gchar **level_name = NULL;
        size_t sep_len = strlen(FLOM_HIER_RESOURCE_SEPARATOR);
        struct flom_rsrc_data_hier_element_s *node;
            
        if (NULL == resource)
            THROW(NULL_OBJECT);
        FLOM_TRACE(("flom_resource_hier_clean: removing resource '%s'\n",
                    resource->name));
        flom_resource_hier_trace(resource);
        /* prepare splitted name */
        if (NULL == (splitted_name = g_strsplit(
                         resource->name+sep_len,
                         FLOM_HIER_RESOURCE_SEPARATOR, 0)))
            THROW(G_STRSPLIT_ERROR);
        /* search leaf node for this resource name */
        node = resource->data.hier.root;
        for (level_name = splitted_name; *level_name; ++level_name) {
            FLOM_TRACE(("flom_resource_hier_clean: *level_name='%s', "
                        "node->name='%s'\n", STRORNULL(*level_name),
                        STRORNULL(node->name)));
            if (!g_strcmp0(*level_name, node->name)) {
                /* go one level depth */
                guint i;
                int found = FALSE;
                for (i=0; i<node->leaves->len; ++i) {
                    struct flom_rsrc_data_hier_element_s *leaf =
                        g_ptr_array_index(node->leaves, i);
                    FLOM_TRACE(("flom_resource_hier_clean: "
                                "*(level_name+1)='%s', leaf->name='%s'\n",
                                STRORNULL(*(level_name+1)),
                                STRORNULL(leaf->name)));
                    if (!g_strcmp0(*(level_name+1), leaf->name)) {
                        node = leaf;
                        found = TRUE;
                        FLOM_TRACE(("flom_resource_hier_clean: "
                                    "*(level_name+1)==leaf->name\n"));
                        break;
                    } /* if (!g_strcmp0(*(level_name+1), leaf->name)) */
                } /* for (i=0; i<node->leaves->len; ++i) */
                if (!found && NULL != *(level_name+1)) {
                    FLOM_TRACE(("flom_resource_hier_clean: unable to locate "
                                "node for resource level '%s', it was "
                                "probably already cleaned-up\n",
                                STRORNULL(*(level_name+1))));
                    break;
                }
            } /* if (!g_strcmp0(*level_name, node->name)) */
        } /* for (level_name = splitted_name; *level_name; ++level_name) */
        assert(NULL != node);
        /* check if the connection keeps a lock */
        p = node->holders;
        while (NULL != p) {
            if (((struct flom_rsrc_conn_lock_s *)p->data)->conn == conn)
                break;
            else
                p = p->next;
        } /* while (NULL != p) */
        if (NULL != p) {
            struct flom_rsrc_conn_lock_s *cl =
                (struct flom_rsrc_conn_lock_s *)p->data;
            FLOM_TRACE(("flom_resource_hier_clean: the client is holding "
                        "a lock mode %d, removing it...\n",
                        cl->info.lock_mode));
            FLOM_TRACE(("flom_resource_hier_clean: cl=%p\n", cl));
            node->holders = g_slist_remove(node->holders, cl);
            /* free the now useless connection lock record */
            flom_rsrc_conn_lock_delete(cl);
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_hier_waitings(
                                   resource)))
                THROW(HIER_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.hier.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_hier_clean: the client is "
                                "waiting for a lock mode %d, removing "
                                "it...\n", cl->info.lock_mode));
                    cl = g_queue_pop_nth(resource->data.hier.waitings, i);
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
        FLOM_TRACE(("flom_resource_hier_clean: node->holders=%p\n",
                    node->holders));
        /* calling garbage collector */
        flom_resource_hier_gc(resource->data.hier.root);
        flom_resource_hier_trace(resource);
                
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case G_STRSPLIT_ERROR:
                ret_cod = FLOM_RC_G_STRSPLIT_ERROR;
                break;
            case INVALID_RESOURCE_NAME:
                ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
                break;
            case HIER_WAITINGS_ERROR:
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
    /* free allocated memory */
    if (NULL != splitted_name) {
        g_strfreev(splitted_name);
        splitted_name = NULL;
    }
    FLOM_TRACE(("flom_resource_hier_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_hier_free(flom_resource_t *resource)
{
    /* removing resource tree */
    flom_resource_hier_free_element(resource->data.hier.root);
    g_free(resource->data.hier.root);
    
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_hier_free: cleaning-up waitings queue...\n"));
    while (!g_queue_is_empty(resource->data.hier.waitings)) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)g_queue_pop_head(
                resource->data.hier.waitings);
        flom_rsrc_conn_lock_delete(cl);
    }
    g_queue_free(resource->data.hier.waitings);
    resource->data.hier.waitings = NULL;
    if (NULL != resource->name) {
        g_free(resource->name);
        resource->name = NULL;
    }
}



void flom_resource_hier_free_element(
    struct flom_rsrc_data_hier_element_s *element)
{
    guint i;

    FLOM_TRACE(("flom_resource_hier_free_element: diving the tree...\n"));
    /* scan leaves and free them before */
    if (NULL != element->leaves) {
        for (i=0; i<element->leaves->len; ++i) {
            flom_resource_hier_free_element(
                g_ptr_array_index(element->leaves, i));
            g_free(g_ptr_array_index(element->leaves, i));
        } /* for (i=0; i<element->leaves.len; ++i) */
    } /* if (NULL != element->leaves) */
    g_ptr_array_free(element->leaves, TRUE);

    FLOM_TRACE(("flom_resource_hier_free_element: cleaning element '%s'\n",
                element->name));    
    
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_hier_free_element: cleaning-up holders array "
                "of lists...\n"));
    while (NULL != element->holders) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)element->holders->data;
        element->holders = g_slist_remove(element->holders, cl);
        flom_rsrc_conn_lock_delete(cl);
    } /* while (NULL != element->holders) */
    element->holders = NULL;

    /* releasing resource name */
    if (NULL != element->name) {
        g_free(element->name);
        element->name = NULL;
    }
}



int flom_resource_hier_compare_name(const flom_resource_t *resource,
                                    const gchar *name)
{
    gchar **splitted_name = NULL;
    int ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
    size_t sep_len = strlen(FLOM_HIER_RESOURCE_SEPARATOR);
    
    if (0 != strncmp(name, FLOM_HIER_RESOURCE_SEPARATOR, sep_len)) {
        FLOM_TRACE(("flom_resource_hier_init: '%s' does not start with "
                    "'%s'\n", name, FLOM_HIER_RESOURCE_SEPARATOR));
    } else if (NULL == (splitted_name = g_strsplit(
                            name+sep_len, FLOM_HIER_RESOURCE_SEPARATOR, 0))) {
        FLOM_TRACE(("flom_resource_hier_compare_name: WARNING g_strsplit "
                    "returned NULL\n"));
    } else {
        FLOM_TRACE(("flom_resource_hier_compare_name: "
                    "splitted_name[0]='%s', "
                    "resource->..root->name[0]='%s'\n",
                    splitted_name[0], resource->data.hier.root->name));
        ret_cod = g_strcmp0(splitted_name[0],
                            resource->data.hier.root->name);
    }
    /* release allocated memory */
    g_strfreev(splitted_name);
    return ret_cod;
}



int flom_resource_hier_change_name(flom_resource_t *resource,
                                   const gchar *name)
{
    enum Exception { G_STRDUP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_hier_change_name: substituting '%s' with "
                "'%s'\n", STRORNULL(resource->name), STRORNULL(name)));
    TRY {
        if (!g_strcmp0(resource->name, name)) {
            FLOM_TRACE(("flom_resource_hier_change_name: strings are equal, "
                        "bypassing...\n"));
        } else {
            if (NULL != resource->name) {
                g_free(resource->name);
                resource->name = NULL;
            }
            if (NULL == (resource->name = g_strdup(name)))
                THROW(G_STRDUP_ERROR);
        } /* if (!g_strcmp0(resource->name, name)) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_hier_change_name/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_hier_waitings(flom_resource_t *resource)
{
    enum Exception { G_STRSPLIT_ERROR
                     , RESOURCE_HIER_ADD_LOCKER_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    gchar **splitted_name = NULL;
    
    FLOM_TRACE(("flom_resource_hier_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        size_t sep_len = strlen(FLOM_HIER_RESOURCE_SEPARATOR);
        
        /* check if there is any connection waiting for a lock */
        do {
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.hier.waitings, i);
            if (NULL == cl) {
                FLOM_TRACE(("flom_resource_hier_waitings: NULL cl for "
                            "i=%u\n", i));
                break;
            }
            assert(NULL != cl->name);
            /* prepare splitted resource name */
            if (NULL == (splitted_name = g_strsplit(
                             cl->name+sep_len,
                             FLOM_HIER_RESOURCE_SEPARATOR, 0)))
                THROW(G_STRSPLIT_ERROR);
            /* try to apply this lock... */
            if (flom_resource_hier_can_lock(
                    resource->data.hier.root, cl->info.lock_mode,
                    splitted_name)) {
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.hier.waitings, i);
                /* this should be impossibile because peek was ok
                   some rows above */
                assert(NULL != cl);
                if (FLOM_RC_OK != (
                        ret_cod = flom_resource_hier_add_locker(
                            resource, cl, splitted_name)))
                    THROW(RESOURCE_HIER_ADD_LOCKER_ERROR);
                /* send a message to the client that is waiting the lock */
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
                flom_msg_trace(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_tcp_send(
                                       flom_tcp_get_sockfd(
                                           flom_conn_get_tcp(cl->conn)),
                                       buffer, to_send)))
                    THROW(MSG_SEND_ERROR);
                flom_conn_set_last_step(cl->conn, msg.header.pvs.step);
                cl = NULL;
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
                    THROW(MSG_FREE_ERROR);                
            } else
                ++i;
        } while (TRUE);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRSPLIT_ERROR:
                ret_cod = FLOM_RC_G_STRSPLIT_ERROR;
                break;
            case RESOURCE_HIER_ADD_LOCKER_ERROR:
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
    /* free allocated memory */
    if (NULL != splitted_name) {
        g_strfreev(splitted_name);
        splitted_name = NULL;
    }
    if (NULL != cl) {
        flom_rsrc_conn_lock_delete(cl);
        cl = NULL;
    }
    FLOM_TRACE(("flom_resource_hier_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_hier_trace(const flom_resource_t *resource)
{
    FLOM_TRACE(("flom_resource_hier_trace: name='%s', type=%d\n",
                resource->name, resource->type));
    assert(FLOM_RSRC_TYPE_HIER == resource->type);
    /* bypass the recursive CPU intensive function if the module is not
       traced */
    if (FLOM_TRACE_MODULE & flom_trace_mask)
        flom_resource_hier_trace_node(resource->data.hier.root, 0);
}


void flom_resource_hier_trace_node(
    struct flom_rsrc_data_hier_element_s *node, int level)
{
    int leaf_id, holders;
    char *tabs;
    GSList *p;

    switch (level) {
        case 0: tabs = ""; break;
        case 1: tabs = "  "; break;
        case 2: tabs = "    "; break;
        case 3: tabs = "      "; break;
        case 4: tabs = "        "; break;
        case 5: tabs = "          "; break;
        case 6: tabs = "            "; break;
        case 7: tabs = "              "; break;
        case 8: tabs = "                "; break;
        case 9: tabs = "                  "; break;
        case 10: tabs = "                    "; break;
        default: tabs = "---------------------+"; break;
    } /* switch (level) */
    /* compute number of holders */
    p = node->holders;
    holders = 0;
    while (NULL != p) {
        holders++;
        p = p->next;
    }
    FLOM_TRACE(("flom_resource_hier_trace_node: %s%s%s (holders=%d)\n",
                tabs, FLOM_HIER_RESOURCE_SEPARATOR, node->name, holders));
    /* looping on all leafs */
    for (leaf_id=0; leaf_id<node->leaves->len; ++leaf_id) {
        struct flom_rsrc_data_hier_element_s *leaf =
            g_ptr_array_index(node->leaves, leaf_id);
        flom_resource_hier_trace_node(leaf, level+1);
    } /* for (leaf_id=0; leaf_id<node->leaves->len; ++leaf_id) */
}
