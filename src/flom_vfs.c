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



#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif



#include "flom_vfs.h"
#include "flom_errors.h"
#include "flom_msg.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON_MNGMNT



/*
	How to umount VFS in case of issues:
	fusermount -u   mountpoint
	sudo umount -l  mountpoint
*/



const char *FLOM_VFS_ROOT_DIR_NAME = "/";
const char *FLOM_VFS_STATUS_DIR_NAME = "status";
const char *FLOM_VFS_INCUBATOR_DIR_NAME = "incubator";
const char *FLOM_VFS_LOCKERS_DIR_NAME = "lockers";
const char *FLOM_VFS_RESNAME_FILE_NAME = "resource_name";
const char *FLOM_VFS_RESTYPE_FILE_NAME = "resource_type";
const char *FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME = "holders";
const char *FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME = "waitings";
const char *FLOM_VFS_PEERNAME_FILE_NAME = "peer_name";
const char *FLOM_VFS_LOCKERS_LOCKMODE_FILE_NAME = "lock_mode";
const char *FLOM_VFS_LOCKERS_QUANTITY_FILE_NAME = "quantity";
const char *FLOM_VFS_LOCKERS_SEQUENCE_VALUE_FILE_NAME = "sequence_value";
const char *FLOM_VFS_LOCKERS_TIMESTAMP_VALUE_FILE_NAME = "timestamp_value";



flom_vfs_ram_tree_t flom_vfs_ram_tree;



flom_vfs_ram_node_t *flom_vfs_ram_node_create(const char *name,
                                              const char *content)
{
    enum Exception { G_TRY_MALLOC_ERROR
                     , G_STRDUP_ERROR1
                     , G_STRDUP_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_vfs_ram_node_t *tmp = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_node_create: name='%s'\n", name));
    TRY {
        /* allocate the node object */
        if (NULL == (tmp = g_try_malloc(sizeof(flom_vfs_ram_node_t))))
            THROW(G_TRY_MALLOC_ERROR);
        /* duplicate the string for the name */
        if (NULL == (tmp->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR1);
        /* duplicate the string for the content */
        if (NULL == content) {
            tmp->content = NULL;
        } else if (NULL == (tmp->content = g_strdup(content)))
            THROW(G_STRDUP_ERROR2);
        tmp->ctime = tmp->mtime = time(NULL);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case G_STRDUP_ERROR1:
            case G_STRDUP_ERROR2:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        /* in case of failure, the object must be cleaned up */
        if (excp < NONE && excp > G_TRY_MALLOC_ERROR) {
            if (NULL != tmp->content)
                g_free(tmp->content); /* should never happen */
            if (NULL != tmp->name)
                g_free(tmp->name);
            if (NULL != tmp)
                g_free(tmp);
            tmp = NULL;
        }
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_vfs_ram_node_create/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return tmp;
}



void flom_vfs_ram_node_destroy(flom_vfs_ram_node_t *node)
{
    if (NULL != node) {
        FLOM_TRACE(("flom_vfs_ram_node_destroy: ino=" FUSE_INO_T_FORMAT "\n",
                    (fuse_ino_t *)node));
        if (NULL != node->name) {
            FLOM_TRACE(("flom_vfs_ram_node_destroy: name='%s'\n",
                        node->name));
            g_free(node->name);
            node->name = NULL;
        }
        if (NULL != node->content) {
            FLOM_TRACE(("flom_vfs_ram_node_destroy: content='%s'\n",
                        node->content));
            g_free(node->content);
            node->content = NULL;
        }
        g_free(node);
    } else {
        FLOM_TRACE(("flom_vfs_ram_node_destroy: node == NULL!\n"));
    }
}



int flom_vfs_ram_tree_init(int activate)
{
    enum Exception { BYPASS_ACTIVATION
                     , NODE_CREATE_ERROR1
                     , G_NODE_NEW_ERROR1
                     , NODE_CREATE_ERROR2
                     , G_NODE_PREPEND_DATA2
                     , NODE_CREATE_ERROR3
                     , G_NODE_PREPEND_DATA3
                     , NODE_CREATE_ERROR4
                     , G_NODE_PREPEND_DATA4
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    flom_vfs_ram_node_t *tmp_root_node = NULL;
    flom_vfs_ram_node_t *tmp_status_node = NULL;
    flom_vfs_ram_node_t *tmp_lockers_node = NULL;
    flom_vfs_ram_node_t *tmp_incubator_node = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_init(activate=%d)\n", activate));
    TRY {
        GNode *tmp, *tmp_status;

        if (!activate) {
            /* VFS feature must not be activated, ram tree must stay empty */
            flom_vfs_ram_tree.active = FALSE;
            flom_vfs_ram_tree.root = NULL;
            THROW(BYPASS_ACTIVATION);
        }
        
        /* initialize the global semaphore */
        g_mutex_init(&flom_vfs_ram_tree.mutex);
        /* lock it immediately! */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        
        /* create the data block for the first node, root dir */
        if (NULL == (tmp_root_node = flom_vfs_ram_node_create(
                         FLOM_VFS_ROOT_DIR_NAME, NULL)))
            THROW(NODE_CREATE_ERROR1);
        
        /* create the node as the root node in the tree */
        if (NULL == (flom_vfs_ram_tree.root = g_node_new(tmp_root_node)))
            THROW(G_NODE_NEW_ERROR1);

        /* now the ram tree can be considered active */
        flom_vfs_ram_tree.active = TRUE;

        /* create the data block for the second node, status dir */
        if (NULL == (tmp_status_node = flom_vfs_ram_node_create(
                         FLOM_VFS_STATUS_DIR_NAME, NULL)))
            THROW(NODE_CREATE_ERROR2);
        /* append the node to the tree, create a child */
        if (NULL == (tmp = g_node_prepend_data(
                         flom_vfs_ram_tree.root, tmp_status_node)))
            THROW(G_NODE_PREPEND_DATA2);
        tmp_status = tmp;   /* save it to attach incubator dir later on */
        
        /* create the data block for the third node, lockers dir */
        if (NULL == (tmp_lockers_node = flom_vfs_ram_node_create(
                         FLOM_VFS_LOCKERS_DIR_NAME, NULL)))
            THROW(NODE_CREATE_ERROR3);
        /* append the node to the tree, create a child */
        if (NULL == (tmp = g_node_prepend_data(
                         tmp, tmp_lockers_node)))
            THROW(G_NODE_PREPEND_DATA3);
        
        /* create the data block for the fourth node, incubator dir */
        if (NULL == (tmp_incubator_node = flom_vfs_ram_node_create(
                         FLOM_VFS_INCUBATOR_DIR_NAME, NULL)))
            THROW(NODE_CREATE_ERROR4);
        /* append the node to the tree, create a child */
        if (NULL == (tmp = g_node_prepend_data(
                         tmp_status, tmp_incubator_node)))
            THROW(G_NODE_PREPEND_DATA4);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BYPASS_ACTIVATION:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NODE_CREATE_ERROR1:
            case NODE_CREATE_ERROR2:
            case NODE_CREATE_ERROR3:
            case NODE_CREATE_ERROR4:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case G_NODE_NEW_ERROR1:
                ret_cod = FLOM_RC_G_NODE_NEW_ERROR;
                break;
            case G_NODE_PREPEND_DATA2:
            case G_NODE_PREPEND_DATA3:
            case G_NODE_PREPEND_DATA4:
                ret_cod = FLOM_RC_G_NODE_PREPEND_DATA_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        /* recovery memory */
        if (excp > NODE_CREATE_ERROR1 && excp <= G_NODE_NEW_ERROR1)
            flom_vfs_ram_node_destroy(tmp_root_node);
        if (excp > NODE_CREATE_ERROR2 && excp <= G_NODE_PREPEND_DATA2)
            flom_vfs_ram_node_destroy(tmp_status_node);
        if (excp > NODE_CREATE_ERROR3 && excp <= G_NODE_PREPEND_DATA3)
            flom_vfs_ram_node_destroy(tmp_lockers_node);
        if (excp > NODE_CREATE_ERROR4 && excp <= G_NODE_PREPEND_DATA4)
            flom_vfs_ram_node_destroy(tmp_incubator_node);
    } /* TRY-CATCH */
    /* unlock the semaphore to leave access to the object to others */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}




gboolean flom_vfs_ram_tree_cleanup_iter(GNode *n, gpointer data) {
    FLOM_TRACE(("flom_vfs_ram_tree_cleanup_iter: removing data for node %p\n",
                n));
    flom_vfs_ram_node_destroy((flom_vfs_ram_node_t *)n->data);
    return FALSE;
}



void flom_vfs_ram_tree_cleanup(GNode *node, int locked)
{
    FLOM_TRACE(("flom_vfs_ram_tree_cleanup(node=%p)\n", node));
    if (flom_vfs_ram_tree.active) {
        int is_root = NULL == node || flom_vfs_ram_tree.root == node;
        /* lock it immediately! */
        if (!locked)
            g_mutex_lock(&flom_vfs_ram_tree.mutex);
        /* traverse the tree to remove data */
        g_node_traverse(
            is_root ? flom_vfs_ram_tree.root : node,
            G_PRE_ORDER, G_TRAVERSE_ALL,
            -1, flom_vfs_ram_tree_cleanup_iter, NULL);
        /* destroy the tree */
        g_node_destroy(is_root ? flom_vfs_ram_tree.root : node);
        if (is_root)
            flom_vfs_ram_tree.root = NULL;
        /* unlock the semaphore to leave access to the object to others */
        if (!locked)
            g_mutex_unlock(&flom_vfs_ram_tree.mutex);
        if (is_root) {
            g_mutex_clear(&flom_vfs_ram_tree.mutex);
            /* deactivate it! */
            flom_vfs_ram_tree.active = FALSE;
        }
    }
}



GNode *flom_vfs_ram_tree_find(gpointer data)
{
    enum Exception { NOT_ACTIVE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GNode *result = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_find: data=%p\n", data));
    TRY {
        GNode *tmp = NULL;

        if (!flom_vfs_ram_tree.active)
            THROW(NOT_ACTIVE);
        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;

        /* if data == NULL, the caller wants the root of the tree */
        if (NULL == data) {
            tmp = flom_vfs_ram_tree.root;
            FLOM_TRACE(("flom_vfs_ram_tree_find: %p is for root in "
                        "ram tree, tmp->data=%p, tmp->data->name='%s'\n",
                        data, tmp->data,
                        ((flom_vfs_ram_node_t *)tmp->data)->name));
        } else if (NULL == (tmp = g_node_find(flom_vfs_ram_tree.root,
                                            G_PRE_ORDER,
                                            G_TRAVERSE_ALL, data))) {
            /* search the data */
            FLOM_TRACE(("flom_vfs_ram_tree_find: %p was not found in "
                        "ram tree\n", data));
        } else {
            FLOM_TRACE(("flom_vfs_ram_tree_find: %p was found in "
                        "ram tree, tmp->data=%p, tmp->data->name='%s'\n",
                        data, tmp->data,
                        ((flom_vfs_ram_node_t *)tmp->data)->name));
        }
        result = tmp;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NOT_ACTIVE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_find/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return result;
}



gboolean flom_vfs_ram_tree_find_parent_iter(
    GNode *node, gpointer data) {
    
    struct flom_vfs_ram_tree_parent_child_s *parent_child =
        (struct flom_vfs_ram_tree_parent_child_s *)data;
    GNode *tmp;

    FLOM_TRACE(("flom_vfs_ram_tree_find_parent_iter: "
                "child->name='%s', node->name='%s'\n",
                ((flom_vfs_ram_node_t *)parent_child->child->data)->name,
                ((flom_vfs_ram_node_t *)node->data)->name));
    /* check if one of the child matches node, if yes we got the parent */
    tmp = g_node_find_child(node, G_TRAVERSE_ALL,
                            parent_child->child->data);
    if (NULL != tmp) {
        FLOM_TRACE(("flom_vfs_ram_tree_find_parent_iter: '%s' is the "
                    "parent of '%s'\n", 
                    ((flom_vfs_ram_node_t *)node->data)->name,
                    ((flom_vfs_ram_node_t *)parent_child->child->data)->name));
        parent_child->parent = node;
        return TRUE;
    }
    else
        return FALSE;
}



GNode *flom_vfs_ram_tree_find_parent(GNode *node, int locked)
{
    enum Exception { INACTIVE_FEATURE
                     , NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GNode *result = NULL;
    int must_unlock = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_find_parent: node=%p\n", node));
    TRY {
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        
        if (NULL == node || NULL == node->data)
            THROW(NULL_OBJECT);
        FLOM_TRACE(("flom_vfs_ram_tree_find_parent: node=%p, name='%s'\n",
                    node, ((flom_vfs_ram_node_t *)node->data)->name));
        
        /* lock the tree to avoid conflicts */
        if (!locked) {
            g_mutex_lock(&flom_vfs_ram_tree.mutex);
            must_unlock = TRUE;
        }
    
        /* if node is root, no parent */
        if (G_NODE_IS_ROOT(node)) {
            FLOM_TRACE(("flom_vfs_ram_tree_find_parent: root node does not "
                        "have a parent\n"));
            /* return root itself */
            result = node;
        } else {
            struct flom_vfs_ram_tree_parent_child_s parent_child;
            parent_child.child = node;
            parent_child.parent = NULL;
            /* traverse the three to find where node is a child */
            g_node_traverse(flom_vfs_ram_tree.root, G_IN_ORDER,
                            G_TRAVERSE_NON_LEAVES, -1,
                            flom_vfs_ram_tree_find_parent_iter, &parent_child);
            result = parent_child.parent;
        }
        FLOM_TRACE(("flom_vfs_ram_tree_find_parent: result=%p, name='%s'\n",
                    result, ((flom_vfs_ram_node_t *)result->data)->name));

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (must_unlock)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_find_parent/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return result;
}



void flom_vfs_ino_name_pair_destroy_notify(gpointer data)
{
    struct flom_vfs_ino_name_pair_s *tmp = data;
    if (NULL != tmp && NULL != tmp->name) {
        FLOM_TRACE(("flom_vfs_ino_name_pair_destroy_notify: tmp=%p, "
                    "tmp->name='%s'\n", tmp, tmp->name));
        g_free(tmp->name);
        tmp->name = NULL;
    }
}



GNode *flom_vfs_ram_tree_find_child_by_name(GNode *root, const char *name)
{
    enum Exception { INACTIVE_FEATURE
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GNode *result = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_find_child_by_name\n"));
    TRY {
        guint i=0, n=0;
        GNode *node;

        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        /* traverse all the children of the node */
        n = g_node_n_children(root);
        for (i=0; i<n; i++) {
            node = g_node_nth_child(root, i);
            if (NULL == node)
                break;
            FLOM_TRACE(("flom_vfs_ram_tree_find_child_by_name: "
                        "name='%s'\n",
                        ((flom_vfs_ram_node_t *)node->data)->name));
            
            if (!strcmp(((flom_vfs_ram_node_t *)node->data)->name, name)) {
                result = node;
                break;
            }
        } /* for (i=0; i<n; i++) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_find_child_by_name/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return result;
}



struct flom_vfs_ram_tree_find_node_by_name_s {
    const char *name;
    GNode *node;
};



gboolean flom_vfs_ram_tree_find_node_by_name_iter(
    GNode *node, gpointer data) {
    
    struct flom_vfs_ram_tree_find_node_by_name_s *name_and_node =
        (struct flom_vfs_ram_tree_find_node_by_name_s *)data;

    FLOM_TRACE(("flom_vfs_ram_tree_find_node_by_name_iter: "
                "node->name='%s', name='%s'\n",
                ((flom_vfs_ram_node_t *)node->data)->name,
                name_and_node->name));
    /* check if the two names match */
    if (!strcmp(((flom_vfs_ram_node_t *)node->data)->name,
                name_and_node->name)) {
        FLOM_TRACE(("flom_vfs_ram_tree_find_node_by_name_iter: node "
                    "found!\n"));
        name_and_node->node = node;
        return TRUE;
    } else
        return FALSE;
}



int flom_vfs_ram_tree_find_node_by_name(GNode *start, const char *name,
                                        int locked, GNode **result)
{
    enum Exception { INACTIVE_FEATURE
                     , OBJ_NOT_FOUND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_vfs_ram_tree_find_node_by_name(name='%s')\n", name));
    TRY {
        struct flom_vfs_ram_tree_find_node_by_name_s name_and_node;
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);

        /* lock the tree to avoid conflicts */
        if (!locked)
            g_mutex_lock(&flom_vfs_ram_tree.mutex);

        name_and_node.name = name;
        name_and_node.node = NULL;
        g_node_traverse(start, G_PRE_ORDER, G_TRAVERSE_ALL, -1, 
                        flom_vfs_ram_tree_find_node_by_name_iter,
                        (gpointer)(&name_and_node));
        if (NULL == name_and_node.node) {
            *result = NULL;
            THROW(OBJ_NOT_FOUND_ERROR);
        } else
            *result = name_and_node.node;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case OBJ_NOT_FOUND_ERROR:
                ret_cod = FLOM_RC_OBJ_NOT_FOUND_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (!locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_find_node_by_name/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



GArray *flom_vfs_ram_tree_retrieve_children(gpointer data)
{
    enum Exception { INACTIVE_FEATURE
                     , G_ARRAY_NEW_ERROR
                     , NODE_NOT_FOUND
                     , G_STRDUP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GArray *result = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children: data=%p\n", data));
    TRY {
        GNode *node = NULL;
        guint i, n;

        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;

        /* allocate the array for the result */
        if (NULL == (result = g_array_new(
                         FALSE, FALSE, sizeof(
                             struct flom_vfs_ino_name_pair_s))))
            THROW(G_ARRAY_NEW_ERROR);
        /* associate the destroyer */
        g_array_set_clear_func(result, flom_vfs_ino_name_pair_destroy_notify);
        
        /* if data == NULL, the caller wants the root of the tree */
        if (NULL == data) {
            node = flom_vfs_ram_tree.root;
            FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children: %p is for root "
                        "in ram tree, node->data=%p, node->data->name='%s'\n",
                        data, node->data,
                        ((flom_vfs_ram_node_t *)node->data)->name));
        } else if (NULL == (node = g_node_find(flom_vfs_ram_tree.root,
                                            G_PRE_ORDER,
                                            G_TRAVERSE_ALL, data))) {
            /* search the data */
            FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children: %p was not "
                        "found in ram tree\n", data));
        } else {
            FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children: %p was found in "
                        "ram tree, node->data=%p, node->data->name='%s'\n",
                        data, node->data,
                        ((flom_vfs_ram_node_t *)node->data)->name));
        }
        if (NULL == node)
            THROW(NODE_NOT_FOUND);
        /* scan all the children of the node */
        n = g_node_n_children(node);
        for (i=0; i<n; i++) {
            GNode *child = g_node_nth_child(node, i);
            flom_vfs_ram_node_t *node_in_ram;
            struct flom_vfs_ino_name_pair_s pair;
            if (NULL == child)
                break;
            node_in_ram = (flom_vfs_ram_node_t *)child->data;
            pair.ino = (fuse_ino_t)node_in_ram;
            if (NULL == (pair.name = g_strdup(node_in_ram->name)))
                THROW(G_STRDUP_ERROR);
            g_array_append_val(result, pair);
            FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children: appended ("
                        FUSE_INO_T_FORMAT ",'%s')\n", pair.ino, pair.name));
        } /* for (i=0; i<n; i++) */

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case G_ARRAY_NEW_ERROR:
                ret_cod = FLOM_RC_G_ARRAY_NEW_ERROR;
                break;
            case NODE_NOT_FOUND:
                ret_cod = FLOM_RC_OBJ_NOT_FOUND_ERROR;
                break;
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
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_retrieve_children/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return result;
}



int flom_vfs_ram_tree_update_mtime(GNode *node)
{
    enum Exception { INACTIVE_FEATURE
                     , NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_vfs_ram_tree_update_mtime(node=%p)\n", node));
    TRY {
        flom_vfs_ram_node_t *node_data;
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        
        if (NULL == node)
            THROW(NULL_OBJECT);

        node_data = (flom_vfs_ram_node_t *)node->data;
        node_data->mtime = time(NULL);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_vfs_ram_tree_update_mtime/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_add_locker(flom_uid_t uid, const char *resource_name,
                                 const char *resource_type)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME
                     , RAM_NODE_UID_DIR_ERROR
                     , G_NODE_UID_DIR_APPEND_DATA
                     , RAM_TREE_UPDATE_MTIME
                     , RAM_NODE_RESOUCE_NAME_FILE_ERROR
                     , G_NODE_RESOUCE_NAME_FILE_APPEND_DATA
                     , RAM_NODE_RESOURCE_TYPE_FILE_ERROR
                     , G_NODE_RESOURCE_TYPE_FILE_APPEND_DATA
                     , RAM_NODE_HOLDERS_DIR_ERROR
                     , G_NODE_HOLDERS_DIR_APPEND_DATA
                     , RAM_NODE_WAITINGS_DIR_ERROR
                     , G_NODE_WAITINGS_DIR_APPEND_DATA
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_vfs_ram_node_t *tmp_ram_node_uid_dir = NULL;
    flom_vfs_ram_node_t *tmp_ram_node_resource_name_file = NULL;
    flom_vfs_ram_node_t *tmp_ram_node_resource_type_file = NULL;
    flom_vfs_ram_node_t *tmp_ram_node_holders_dir = NULL;
    flom_vfs_ram_node_t *tmp_ram_node_waitings_dir = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker(uid=" FLOM_UID_T_FORMAT
                ", resource_name='%s', resource_type='%s')\n",
                uid, resource_name, resource_type));
        
    TRY {
        GNode *lockers_node = NULL;
        GNode *tmp_node_uid_dir = NULL;
        GNode *tmp_node_resource_name_file = NULL;
        GNode *tmp_node_resource_type_file = NULL;
        GNode *tmp_node_holders_dir = NULL;
        GNode *tmp_node_waitings_dir = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        char string_buffer[FLOM_VFS_STD_BUFFER_SIZE];
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME);
        /* create the data block for the locker node */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, uid);
        if (NULL == (tmp_ram_node_uid_dir = flom_vfs_ram_node_create(
                         uid_buffer, NULL)))
            THROW(RAM_NODE_UID_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_uid_dir = g_node_append_data(
                         lockers_node, tmp_ram_node_uid_dir)))
            THROW(G_NODE_UID_DIR_APPEND_DATA);
        tmp_ram_node_uid_dir = NULL;
        /* update modification time for the parent dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(lockers_node)))
            THROW(RAM_TREE_UPDATE_MTIME);
        /* create the data block for the resource_name file node */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", resource_name);
        if (NULL == (
                tmp_ram_node_resource_name_file = flom_vfs_ram_node_create(
                         FLOM_VFS_RESNAME_FILE_NAME,
                         string_buffer)))
            THROW(RAM_NODE_RESOUCE_NAME_FILE_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_resource_name_file = g_node_append_data(
                         tmp_node_uid_dir, tmp_ram_node_resource_name_file)))
            THROW(G_NODE_RESOUCE_NAME_FILE_APPEND_DATA);
        tmp_ram_node_resource_name_file = NULL;
        /* create the data block for the resource_type file node */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", resource_type);
        if (NULL == (
                tmp_ram_node_resource_type_file = flom_vfs_ram_node_create(
                         FLOM_VFS_RESTYPE_FILE_NAME,
                         string_buffer)))
            THROW(RAM_NODE_RESOURCE_TYPE_FILE_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_resource_type_file = g_node_append_data(
                         tmp_node_uid_dir, tmp_ram_node_resource_type_file)))
            THROW(G_NODE_RESOURCE_TYPE_FILE_APPEND_DATA);
        tmp_ram_node_resource_type_file = NULL;
        /* create the data block for the holders node */
        if (NULL == (tmp_ram_node_holders_dir = flom_vfs_ram_node_create(
                         FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME, NULL)))
            THROW(RAM_NODE_HOLDERS_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_holders_dir = g_node_append_data(
                         tmp_node_uid_dir, tmp_ram_node_holders_dir)))
            THROW(G_NODE_HOLDERS_DIR_APPEND_DATA);
        tmp_ram_node_uid_dir = NULL;
        /* create the data block for the waitings node */
        if (NULL == (tmp_ram_node_waitings_dir = flom_vfs_ram_node_create(
                         FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME, NULL)))
            THROW(RAM_NODE_WAITINGS_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_waitings_dir = g_node_append_data(
                         tmp_node_uid_dir, tmp_ram_node_waitings_dir)))
            THROW(G_NODE_WAITINGS_DIR_APPEND_DATA);
        tmp_ram_node_uid_dir = NULL;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME:
                break;
            case RAM_NODE_UID_DIR_ERROR:
            case RAM_NODE_RESOUCE_NAME_FILE_ERROR:
            case RAM_NODE_RESOURCE_TYPE_FILE_ERROR:
            case RAM_NODE_HOLDERS_DIR_ERROR:
            case RAM_NODE_WAITINGS_DIR_ERROR:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case RAM_TREE_UPDATE_MTIME:
                break;
            case G_NODE_UID_DIR_APPEND_DATA:
            case G_NODE_RESOUCE_NAME_FILE_APPEND_DATA:
            case G_NODE_RESOURCE_TYPE_FILE_APPEND_DATA:
            case G_NODE_HOLDERS_DIR_APPEND_DATA:
            case G_NODE_WAITINGS_DIR_APPEND_DATA:
                ret_cod = FLOM_RC_G_NODE_APPEND_DATA_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        /* recover memory in case of error */
        if (excp != NONE) {
            if (NULL != tmp_ram_node_uid_dir)
                flom_vfs_ram_node_destroy(tmp_ram_node_uid_dir);
            else if (NULL != tmp_ram_node_resource_name_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_resource_name_file);
            else if (NULL != tmp_ram_node_resource_type_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_resource_type_file);
            else if (NULL != tmp_ram_node_holders_dir)
                flom_vfs_ram_node_destroy(tmp_ram_node_holders_dir);
        } /* if (excp != NONE) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_del_locker(flom_uid_t uid)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_del_locker(uid=" FLOM_UID_T_FORMAT "\n",
                uid));
    TRY {
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        GNode *lockers_node = NULL;
        GNode *locker_dir_node = NULL;

        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME1);
        /* create the nome of the subdir */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, uid);
        /* locate the specific locker subdir */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               lockers_node, uid_buffer, TRUE,
                               &locker_dir_node)))
            THROW(FIND_NODE_BY_NAME2);
        flom_vfs_ram_tree_cleanup(locker_dir_node, TRUE);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_del_locker/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_add_locker_conn(flom_uid_t locker_uid,
                                      flom_uid_t conn_uid,
                                      int is_holder,
                                      const char *peer_name,
                                      flom_lock_mode_t lock_mode,
                                      const gint *quantity,
                                      const gchar *sequence_value,
                                      const gchar *timestamp_value)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , FIND_NODE_BY_NAME3
                     , RAM_NODE_CONN_UID_DIR_ERROR
                     , G_NODE_UID_CONN_DIR_APPEND_DATA
                     , RAM_TREE_UPDATE_MTIME
                     , ADD_LOCKER_CONN_FILE1
                     , ADD_LOCKER_CONN_FILE2
                     , ADD_LOCKER_CONN_FILE3
                     , ADD_LOCKER_CONN_FILE4
                     , ADD_LOCKER_CONN_FILE5
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    flom_vfs_ram_node_t *tmp_ram_node_conn_uid_dir = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_conn(locker_uid="
                FLOM_UID_T_FORMAT ", conn_uid=" FLOM_UID_T_FORMAT
                ", is_holder=%d, peer_name='%s')\n",
                locker_uid, conn_uid, is_holder, peer_name));
    TRY {
        GNode *lockers_node = NULL;
        GNode *specific_locker_node = NULL;
        GNode *children_node = NULL;
        GNode *tmp_node_conn_uid_dir = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        char string_buffer[FLOM_VFS_STD_BUFFER_SIZE];

        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME1);
        /* locate the directory of the specific locker */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, locker_uid);
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               lockers_node, uid_buffer,
                               TRUE, &specific_locker_node)))
            THROW(FIND_NODE_BY_NAME2);
        /* locate the holders/waitings directory */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               specific_locker_node,
                               is_holder ? FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME :
                               FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME,
                               TRUE, &children_node)))
            THROW(FIND_NODE_BY_NAME3);
        /* create the data block for the conn node */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (NULL == (tmp_ram_node_conn_uid_dir = flom_vfs_ram_node_create(
                         uid_buffer, NULL)))
            THROW(RAM_NODE_CONN_UID_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_conn_uid_dir = g_node_append_data(
                         children_node, tmp_ram_node_conn_uid_dir)))
            THROW(G_NODE_UID_CONN_DIR_APPEND_DATA);
        tmp_ram_node_conn_uid_dir = NULL;
        /* update modification time for the parent dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(children_node)))
            THROW(RAM_TREE_UPDATE_MTIME);

        snprintf(string_buffer, sizeof(string_buffer), "%s\n", peer_name);
        if (NULL != peer_name) {
            if (FLOM_RC_OK != (ret_cod =
                               flom_vfs_ram_tree_add_locker_conn_file(
                                   locker_uid, conn_uid, TRUE,
                                   FLOM_VFS_PEERNAME_FILE_NAME,
                                   string_buffer)))
                THROW(ADD_LOCKER_CONN_FILE1);
        }
        
        if (FLOM_LOCK_MODE_INVALID != lock_mode) {
            if (FLOM_RC_OK != (ret_cod =
                               flom_vfs_ram_tree_add_locker_conn_file(
                                   locker_uid, conn_uid, TRUE,
                                   FLOM_VFS_LOCKERS_LOCKMODE_FILE_NAME,
                                   flom_lock_mode_long_string(lock_mode))))
                THROW(ADD_LOCKER_CONN_FILE2);
        }

        if (NULL != quantity) {
            snprintf(string_buffer, sizeof(string_buffer), "%d\n", *quantity);
            if (FLOM_RC_OK != (ret_cod =
                               flom_vfs_ram_tree_add_locker_conn_file(
                                   locker_uid, conn_uid, TRUE,
                                   FLOM_VFS_LOCKERS_QUANTITY_FILE_NAME,
                                   string_buffer)))
                THROW(ADD_LOCKER_CONN_FILE3);
        }
        
        if (NULL != sequence_value) {
            if (FLOM_RC_OK != (ret_cod =
                               flom_vfs_ram_tree_add_locker_conn_file(
                                   locker_uid, conn_uid, TRUE,
                                   FLOM_VFS_LOCKERS_SEQUENCE_VALUE_FILE_NAME,
                                   sequence_value)))
                THROW(ADD_LOCKER_CONN_FILE4);
        }

        if (NULL != timestamp_value) {
            if (FLOM_RC_OK != (ret_cod =
                               flom_vfs_ram_tree_add_locker_conn_file(
                                   locker_uid, conn_uid, TRUE,
                                   FLOM_VFS_LOCKERS_TIMESTAMP_VALUE_FILE_NAME,
                                   timestamp_value)))
                THROW(ADD_LOCKER_CONN_FILE5);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
            case FIND_NODE_BY_NAME3:
            case RAM_NODE_CONN_UID_DIR_ERROR:
            case G_NODE_UID_CONN_DIR_APPEND_DATA:
            case RAM_TREE_UPDATE_MTIME:
            case ADD_LOCKER_CONN_FILE1:
            case ADD_LOCKER_CONN_FILE2:
            case ADD_LOCKER_CONN_FILE3:
            case ADD_LOCKER_CONN_FILE4:
            case ADD_LOCKER_CONN_FILE5:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        if (excp != NONE) {
            if (NULL != tmp_ram_node_conn_uid_dir)
                flom_vfs_ram_node_destroy(tmp_ram_node_conn_uid_dir);
            /*
            else if (NULL != tmp_ram_node_conn_peer_name_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_conn_peer_name_file);
            */
        } /* if (excp != NONE) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_conn/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_add_locker_conn_file(flom_uid_t locker_uid,
                                           flom_uid_t conn_uid,
                                           int already_locked,
                                           const char *file_name,
                                           const char *file_content)
{
    enum Exception { INACTIVE_FEATURE
                     , NULL_OBJECT
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , FIND_NODE_BY_NAME3
                     , RAM_NODE_FILE_ERROR
                     , G_NODE_FILE_APPEND_DATA
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    flom_vfs_ram_node_t *tmp_ram_node_conn_file = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_conn_file\n"));
    TRY {
        GNode *lockers_node = NULL;
        GNode *specific_locker_node = NULL;
        GNode *specific_conn_node = NULL;
        GNode *tmp_node_file = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        if (NULL == file_name || NULL == file_content)
            THROW(NULL_OBJECT);
        if (!already_locked) {
            /* lock the tree to avoid conflicts */
            g_mutex_lock(&flom_vfs_ram_tree.mutex);
            locked = TRUE;
        }
        
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME1);
        /* locate the directory of the specific locker */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, locker_uid);
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               lockers_node, uid_buffer,
                               TRUE, &specific_locker_node)))
            THROW(FIND_NODE_BY_NAME2);
        /* locate the directory of the specific connection */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               specific_locker_node, uid_buffer,
                               TRUE, &specific_conn_node)))
            THROW(FIND_NODE_BY_NAME3);
        /* create the data block for the file node */
        if (NULL == (
                tmp_ram_node_conn_file =
                flom_vfs_ram_node_create(file_name, file_content)))
            THROW(RAM_NODE_FILE_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_file = g_node_append_data(
                         specific_conn_node,
                         tmp_ram_node_conn_file)))
            THROW(G_NODE_FILE_APPEND_DATA);
        tmp_ram_node_conn_file = NULL;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
            case FIND_NODE_BY_NAME3:
            case RAM_NODE_FILE_ERROR:
            case G_NODE_FILE_APPEND_DATA:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        if (excp != NONE) {
            if (NULL != tmp_ram_node_conn_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_conn_file);
        } /* if (excp != NONE) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_conn_file/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_del_conn(flom_uid_t conn_uid,
                               int incubating)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , FIND_PARENT
                     , RAM_TREE_UPDATE_MTIME
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_del_conn(conn_uid="
                FLOM_UID_T_FORMAT ")\n", conn_uid));
    TRY {
        GNode *lockers_node = NULL;
        GNode *specific_conn_node = NULL;
        GNode *parent_node = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               incubating ? FLOM_VFS_INCUBATOR_DIR_NAME :
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME1);
        /* locate the directory of the specific connection */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               lockers_node, uid_buffer,
                               TRUE, &specific_conn_node)))
            THROW(FIND_NODE_BY_NAME2);
        /* retrieve the father of the node that will be deleted; it can be
           either "holders" or "waitings" dir */
        if (NULL == (parent_node = flom_vfs_ram_tree_find_parent(
                         specific_conn_node, TRUE)))
            THROW(FIND_PARENT);
        /* update modification time for the parent dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(parent_node)))
            THROW(RAM_TREE_UPDATE_MTIME);
        /* remove the node/dir associated to the connection */
        flom_vfs_ram_tree_cleanup(specific_conn_node, TRUE);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
                break;
            case FIND_PARENT:
                ret_cod = FLOM_RC_OBJ_NOT_FOUND_ERROR;
                break;
            case RAM_TREE_UPDATE_MTIME:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_del_conn/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_move_locker_conn(flom_uid_t conn_uid)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , FIND_PARENT
                     , OBJ_CORRUPTED
                     , FIND_GRANDPARENT
                     , FIND_NODE_BY_NAME3
                     , RAM_TREE_UPDATE_MTIME1
                     , RAM_TREE_UPDATE_MTIME2
                     , RAM_TREE_UPDATE_MTIME3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_move_locker_conn(conn_uid="
                FLOM_UID_T_FORMAT ")\n"));
    TRY {
        GNode *lockers_node = NULL;
        GNode *specific_conn_node = NULL;
        GNode *parent_node = NULL;
        GNode *grandparent_node = NULL;
        GNode *holders_node = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        flom_vfs_ram_node_t *parent_node_data;
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
        /* locate the directory with lockers */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_LOCKERS_DIR_NAME,
                               TRUE, &lockers_node)))
            THROW(FIND_NODE_BY_NAME1);
        /* locate the directory of the specific connection */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               lockers_node, uid_buffer,
                               TRUE, &specific_conn_node)))
            THROW(FIND_NODE_BY_NAME2);
        /* locate the father of the node that will be moved; it must be
           the "waitings" dir, otherwise the situation is not consistent */
        if (NULL == (parent_node = flom_vfs_ram_tree_find_parent(
                         specific_conn_node, TRUE)))
            THROW(FIND_PARENT);
        /* check the parent is "waitings" */
        parent_node_data = (flom_vfs_ram_node_t *)parent_node->data;
        if (0 != strcmp(parent_node_data->name,
                        FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME)) {
            FLOM_TRACE(("flom_vfs_ram_tree_move_locker_conn: parent node name "
                        "should be '%s', but it is '%s'! The RAM tree is no "
                        "more consistent\n",
                        FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME,
                        parent_node_data->name));
            THROW(OBJ_CORRUPTED);
        }
        /* locate the grandfather of the node that will be moved */
        if (NULL == (grandparent_node = flom_vfs_ram_tree_find_parent(
                         parent_node, TRUE)))
            THROW(FIND_GRANDPARENT);
        /* locate the holders node in the same locker */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               grandparent_node,
                               FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME,
                               TRUE, &holders_node)))
            THROW(FIND_NODE_BY_NAME3);
        /* unlink the node from "waitings" */
        g_node_unlink(specific_conn_node);
        /* update modification time for the OLD parent (waitings) dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(parent_node)))
            THROW(RAM_TREE_UPDATE_MTIME1);
        /* append the node to the holders */
        g_node_append(holders_node, specific_conn_node);
        /* update modification time for the NEW parent (holders) dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(holders_node)))
            THROW(RAM_TREE_UPDATE_MTIME2);
        /* update modification time for the moved dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(specific_conn_node)))
            THROW(RAM_TREE_UPDATE_MTIME3);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
            case FIND_NODE_BY_NAME3:
                break;
            case FIND_PARENT:
            case FIND_GRANDPARENT:
                ret_cod = FLOM_RC_OBJ_NOT_FOUND_ERROR;
                break;
            case OBJ_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case RAM_TREE_UPDATE_MTIME1:
            case RAM_TREE_UPDATE_MTIME2:
            case RAM_TREE_UPDATE_MTIME3:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_move_locker_conn/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_add_incubator_conn(flom_uid_t conn_uid,
                                         const char *peer_name,
                                         const char *resource_name,
                                         const char *resource_type)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME
                     , RAM_NODE_UID_DIR_ERROR
                     , G_NODE_UID_DIR_APPEND_DATA
                     , RAM_TREE_UPDATE_MTIME
                     , VFS_RAM_TREE_ADD_FILE1
                     , VFS_RAM_TREE_ADD_FILE2
                     , VFS_RAM_TREE_ADD_FILE3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    flom_vfs_ram_node_t *tmp_ram_node_uid_dir = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_incubator_conn("
                "conn_uid=" FLOM_UID_T_FORMAT ", "
                "peer_name='%s', "
                "resource_name='%s', "
                "resource_type='%s')\n",
                conn_uid, peer_name, resource_name, resource_type));
    TRY {
        GNode *incubator_node = NULL;
        GNode *tmp_node_uid_dir = NULL;
        char uid_buffer[SIZEOF_FLOM_UID_T * 3];
        char string_buffer[FLOM_VFS_STD_BUFFER_SIZE];
        
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;

        /* locate the directory of the incubator */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               flom_vfs_ram_tree.root,
                               FLOM_VFS_INCUBATOR_DIR_NAME,
                               TRUE, &incubator_node)))
            THROW(FIND_NODE_BY_NAME);
        /* create the data block for the incubator node */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (NULL == (tmp_ram_node_uid_dir = flom_vfs_ram_node_create(
                         uid_buffer, NULL)))
            THROW(RAM_NODE_UID_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_uid_dir = g_node_append_data(
                         incubator_node, tmp_ram_node_uid_dir)))
            THROW(G_NODE_UID_DIR_APPEND_DATA);
        tmp_ram_node_uid_dir = NULL;
        /* update modification time for the parent dir */
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_update_mtime(incubator_node)))
            THROW(RAM_TREE_UPDATE_MTIME);
        /* create the file for peer_name */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", peer_name);
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_add_file(
                               tmp_node_uid_dir,
                               FLOM_VFS_PEERNAME_FILE_NAME,
                               string_buffer)))
            THROW(VFS_RAM_TREE_ADD_FILE1);
        /* create the file for resource_name */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", resource_name);
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_add_file(
                               tmp_node_uid_dir,
                               FLOM_VFS_RESNAME_FILE_NAME,
                               string_buffer)))
            THROW(VFS_RAM_TREE_ADD_FILE2);
        /* create the file for resource_type */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", resource_type);
        if (FLOM_RC_OK != (ret_cod =
                           flom_vfs_ram_tree_add_file(
                               tmp_node_uid_dir,
                               FLOM_VFS_RESTYPE_FILE_NAME,
                               string_buffer)))
            THROW(VFS_RAM_TREE_ADD_FILE3);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INACTIVE_FEATURE:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case FIND_NODE_BY_NAME:
            case RAM_NODE_UID_DIR_ERROR:
            case G_NODE_UID_DIR_APPEND_DATA:
            case RAM_TREE_UPDATE_MTIME:
            case VFS_RAM_TREE_ADD_FILE1:
            case VFS_RAM_TREE_ADD_FILE2:
            case VFS_RAM_TREE_ADD_FILE3:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        if (excp != NONE) {
            if (NULL != tmp_ram_node_uid_dir)
                flom_vfs_ram_node_destroy(tmp_ram_node_uid_dir);
        }
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_add_incubator_conn/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_vfs_ram_tree_add_file(GNode *dir,
                               const char *file_name,
                               const char *file_content)
{
    enum Exception { NULL_OBJECT1
                     , NULL_OBJECT2
                     , RAM_NODE_FILE_ERROR
                     , G_NODE_FILE_APPEND_DATA
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_vfs_ram_node_t *tmp_ram_node_file = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_file("
                "dir=%p, file_name='%s', file_content='%s'\n",
                dir, file_name, file_content));
    TRY {
        GNode *tmp_node_file = NULL;
        
        if (NULL == dir)
            THROW(NULL_OBJECT1);
        if (NULL == file_name || NULL == file_content)
            THROW(NULL_OBJECT2);
        
        /* create the data block for the file node */
        if (NULL == (
                tmp_ram_node_file =
                flom_vfs_ram_node_create(file_name, file_content)))
            THROW(RAM_NODE_FILE_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_file = g_node_append_data(
                         dir, tmp_ram_node_file)))
            THROW(G_NODE_FILE_APPEND_DATA);
        tmp_ram_node_file = NULL;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case RAM_NODE_FILE_ERROR:
            case G_NODE_FILE_APPEND_DATA:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
        if (excp != NONE) {
            if (NULL != tmp_ram_node_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_file);
        } /* if (excp != NONE) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_vfs_ram_tree_add_file/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

