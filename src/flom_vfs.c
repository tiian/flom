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

flom_vfs_common_values_t flom_vfs_common_values;



struct fuse_lowlevel_ops fuse_callback_functions = {
	.lookup		= flom_vfs_lookup,
	.getattr	= hello_ll_getattr,
	.getxattr	= hello_ll_getxattr,
	.readdir	= flom_vfs_readdir,
	.open		= flom_vfs_open,
	.read		= flom_vfs_read,
};



const char *FLOM_VFS_ROOT_DIR_NAME = "/";
const char *FLOM_VFS_STATUS_DIR_NAME = "status";
const char *FLOM_VFS_LOCKERS_DIR_NAME = "lockers";
const char *FLOM_VFS_LOCKERS_RESNAME_FILE_NAME = "resource_name";
const char *FLOM_VFS_LOCKERS_RESTYPE_FILE_NAME = "resource_type";
const char *FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME = "holders";
const char *FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME = "waitings";
const char *FLOM_VFS_LOCKERS_HOLDERS_PEERNAME_FILE_NAME = "peer_name";



int flom_vfs_stat(fuse_ino_t ino, struct stat *stbuf)
{
    GNode *node_in_ram;
    
    FLOM_TRACE(("flom_vfs_stat: ino=" FLOM_UID_T_FORMAT "\n", ino));
    
    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    if (NULL == (node_in_ram = flom_vfs_ram_tree_find(
                     FLOM_VFS_INO_ROOT_DIR == ino ?
                     NULL : (gpointer)ino))) {
        FLOM_TRACE(("flom_vfs_stat: ino=" FLOM_UID_T_FORMAT
                    ", not found\n", ino));
    } else {
        flom_vfs_ram_node_t *data = (flom_vfs_ram_node_t *)node_in_ram->data;
        if (flom_vfs_ram_node_is_dir(data)) {
            stbuf->st_ino = ino;
            stbuf->st_mode = S_IFDIR | 0550;
            stbuf->st_nlink = 2;
            stbuf->st_uid = flom_vfs_common_values.uid;
            stbuf->st_gid = flom_vfs_common_values.gid;
            stbuf->st_ctime = flom_vfs_ram_node_get_ctime(data);
            stbuf->st_mtime = flom_vfs_ram_node_get_mtime(data);
        } else {
            stbuf->st_ino = ino;
            stbuf->st_mode = S_IFREG | 0440;
            stbuf->st_nlink = 1;
            stbuf->st_uid = flom_vfs_common_values.uid;
            stbuf->st_gid = flom_vfs_common_values.gid;
            stbuf->st_ctime = flom_vfs_ram_node_get_ctime(data);
            stbuf->st_mtime = flom_vfs_ram_node_get_mtime(data);
            stbuf->st_size = strlen(flom_vfs_ram_node_get_content(data));
        }
    }
    
    /* retrieve the type associated to the inode and the FLoM unique id */
    /* @@@ remove me
    flom_vfs_inode_type_t type;
    flom_uid_t uid;
    flom_vfs_inode_to_uid(ino, &type, &uid);
    
	stbuf->st_ino = ino;

    switch (type) {
        case FLOM_VFS_ROOT_DIR:
        case FLOM_VFS_STATUS_DIR:
        case FLOM_VFS_LOCKERS_DIR:
            stbuf->st_mode = S_IFDIR | 0550;
            stbuf->st_nlink = 2;
            stbuf->st_uid = flom_vfs_common_values.uid;
            stbuf->st_gid = flom_vfs_common_values.gid;
            stbuf->st_atime = stbuf->st_ctime = stbuf->st_mtime =
                flom_vfs_common_values.time;
            break;
        default:
            FLOM_TRACE(("flom_vfs_stat: error for type=%d\n", type));
            return -1;
            } *//* switch (type) */
    
/*
	case 2:
		stbuf->st_mode = S_IFREG | 0440;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = 
		stbuf->st_ctime = 
		stbuf->st_mtime = time( NULL );
		stbuf->st_size = strlen(hello_str);
		break;
*/
	return 0;
}

void hello_ll_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
	if (flom_vfs_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}


/*
static void hello_ll_getxattr(fuse_req_t req, fuse_ino_t ino,
		const char *name, size_t size)
{
	fuse_reply_buf(req, NULL, 0);
}
*/
void hello_ll_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                                                          size_t size)
{
        (void)size;
        //assert(ino == 2);
        if (strcmp(name, "hello_ll_getxattr_name") == 0)
        {
                const char *buf = "hello_ll_getxattr_value";
                fuse_reply_buf(req, buf, strlen(buf));
        }
        else
        {
                fuse_reply_err(req, ENOTSUP);
        }
}



void flom_vfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;
    GNode *parent_node = NULL;
    GNode *child_node = NULL;

    FLOM_TRACE(("flom_vfs_lookup: parent=" FLOM_UID_T_FORMAT ", name='%s'\n",
                parent, name));

    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    parent_node = flom_vfs_ram_tree_find(FLOM_VFS_INO_ROOT_DIR == parent ?
                                         NULL : (gpointer)parent);
    if (NULL == parent_node) {
        FLOM_TRACE(("flom_vfs_lookup: parent=" FLOM_UID_T_FORMAT
                    ", not found\n", parent));
		fuse_reply_err(req, ENOENT);
    } else {
        /* search what node below parent has name "name" */
        if (NULL == (child_node =
                     flom_vfs_ram_tree_find_child_by_name(
                         parent_node, name))) {
            FLOM_TRACE(("flom_vfs_lookup: child with name '%s' not found\n",
                        name));
            fuse_reply_err(req, ENOENT);
        } else {
            flom_vfs_ram_node_t *child_in_ram =
                (flom_vfs_ram_node_t *)child_node->data;
            memset(&e, 0, sizeof(e));
            e.ino = (fuse_ino_t)child_in_ram;
            e.attr_timeout = 1.0;
            e.entry_timeout = 1.0;
            flom_vfs_stat(e.ino, &e.attr);
            
            fuse_reply_entry(req, &e);
        }
    }
    
    /*
    if (FLOM_VFS_INO_ROOT_DIR == parent &&
        0 == strcmp(name, FLOM_VFS_STATUS_DIR_NAME)) {
        // status dir
		memset(&e, 0, sizeof(e));
		e.ino = FLOM_VFS_INO_STATUS_DIR;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		flom_vfs_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
    } else if (FLOM_VFS_INO_STATUS_DIR == parent &&
               0 == strcmp(name, FLOM_VFS_LOCKERS_DIR_NAME)) {
        // lockers dir
		memset(&e, 0, sizeof(e));
		e.ino = FLOM_VFS_INO_LOCKERS_DIR;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		flom_vfs_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
    } else {
		fuse_reply_err(req, ENOENT);
    }
    */
}

struct dirbuf {
	char *p;
	size_t size;
};

void dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name,
		       fuse_ino_t ino)
{
	struct stat stbuf;
	size_t oldsize = b->size;
	b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
	b->p = (char *) realloc(b->p, b->size);
	memset(&stbuf, 0, sizeof(stbuf));
	stbuf.st_ino = ino;
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,
			  b->size);
}

#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

void flom_vfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                      off_t off, struct fuse_file_info *fi)
{
    GNode *node, *parent;
    flom_vfs_ram_node_t *node_in_ram;
    GArray *children = NULL;
    FLOM_TRACE(("flom_vfs_readdir: ino=" FUSE_INO_T_FORMAT "\n", ino));

    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    node = flom_vfs_ram_tree_find(FLOM_VFS_INO_ROOT_DIR == ino ?
                                  NULL : (gpointer)ino);
    parent = flom_vfs_ram_tree_find_parent(node);
    if (NULL == node) {
        /* the inode does not exists! */
		fuse_reply_err(req, ENOENT);        
    } else {
        node_in_ram = (flom_vfs_ram_node_t *)node->data;
        if (!flom_vfs_ram_node_is_dir(node_in_ram)) {
            FLOM_TRACE(("flom_vfs_readdir: inode=" FUSE_INO_T_FORMAT
                        " is not a dir\n", ino));
            fuse_reply_err(req, ENOTDIR); 
        } else {
            /* it's a dir, retrieve all the children */
            guint i;
            struct dirbuf b;

            memset(&b, 0, sizeof(b));
            children = flom_vfs_ram_tree_retrieve_children(
                FLOM_VFS_INO_ROOT_DIR == ino ?
                NULL : (gpointer)ino);
            dirbuf_add(req, &b, ".", ino);
            dirbuf_add(req, &b, "..",
                       (fuse_ino_t)((flom_vfs_ram_node_t *)parent->data));
            for (i=0; i<children->len; ++i) {
                struct flom_vfs_ino_name_pair_s *element;
                element = &g_array_index(
                    children,
                    struct flom_vfs_ino_name_pair_s, i);
                dirbuf_add(req, &b, element->name,
                           element->ino);
                FLOM_TRACE(("flom_vfs_readdir: name='%s', ino="
                            FUSE_INO_T_FORMAT "\n",
                            element->name, element->ino));
            }
            reply_buf_limited(req, b.p, b.size, off, size);
            free(b.p);
        }
        /* clean-up the array */
        if (NULL != children)
            g_array_free(children, TRUE);
    }
    
    /* search the node in the tree */

    /*
	(void) fi;
    flom_vfs_inode_type_t type;
    flom_uid_t uid;


    // retrieve the type associated to the inode and the FLoM unique id
    flom_vfs_inode_to_uid(ino, &type, &uid);

    if (FLOM_VFS_ROOT_DIR != type &&
        FLOM_VFS_STATUS_DIR != type &&
        FLOM_VFS_LOCKERS_DIR != type &&
        FLOM_VFS_LOCKERS_UID_DIR != type) {

        FLOM_TRACE(("flom_vfs_readdir: type=%d is not a dir\n", type));
        
		fuse_reply_err(req, ENOTDIR);
    } else {
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
        
        switch (type) {
            case FLOM_VFS_ROOT_DIR:
                dirbuf_add(req, &b, ".", FLOM_VFS_INO_ROOT_DIR);
                dirbuf_add(req, &b, "..", FLOM_VFS_INO_ROOT_DIR);
                dirbuf_add(req, &b, FLOM_VFS_STATUS_DIR_NAME,
                           FLOM_VFS_INO_STATUS_DIR);
                break;
            case FLOM_VFS_STATUS_DIR:
                dirbuf_add(req, &b, ".", FLOM_VFS_INO_STATUS_DIR);
                dirbuf_add(req, &b, "..", FLOM_VFS_INO_ROOT_DIR);
                dirbuf_add(req, &b, FLOM_VFS_LOCKERS_DIR_NAME,
                           FLOM_VFS_INO_LOCKERS_DIR);
                break;
            default:
                FLOM_TRACE(("flom_vfs_readdir: type=%d is not implemented!\n",
                            type));
                break;
        } // switch (type) 
        
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
    */
}

void flom_vfs_open(fuse_req_t req, fuse_ino_t ino,
                   struct fuse_file_info *fi)
{
    enum Exception { ROOT_DIR
                     , NO_ENTRY
                     , IS_A_DIR
                     , NOT_READONLY
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GNode *node = NULL;
    flom_vfs_ram_node_t *node_in_ram = NULL;
    
    FLOM_TRACE(("flom_vfs_open: ino=" FUSE_INO_T_FORMAT "\n", ino));
    TRY {
        if (FLOM_VFS_INO_ROOT_DIR == ino)
            THROW(ROOT_DIR);
        if (NULL == (node = flom_vfs_ram_tree_find((gpointer)ino)))
            THROW(NO_ENTRY);
        node_in_ram = (flom_vfs_ram_node_t *)node->data;
        if (flom_vfs_ram_node_is_dir(node_in_ram))
            THROW(IS_A_DIR);
        if ((fi->flags & 3) != O_RDONLY)
            THROW(NOT_READONLY);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case ROOT_DIR:
                fuse_reply_err(req, EISDIR);
                break;
            case NO_ENTRY:
                fuse_reply_err(req, ENOENT);        
                break;
            case IS_A_DIR:
                fuse_reply_err(req, EISDIR);
                break;
            case NOT_READONLY:
                fuse_reply_err(req, EACCES);
                break;
            case NONE:
                fuse_reply_open(req, fi);
                ret_cod = FLOM_RC_OK;
                break;
            default:
                fuse_reply_err(req, ENOENT);        
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_vfs_open/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return;
}



void flom_vfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
                   off_t off, struct fuse_file_info *fi)
{
    GNode *node = NULL;
    flom_vfs_ram_node_t *node_in_ram = NULL;
    /* @@@ ???
	(void) fi;
    */
    
    FLOM_TRACE(("flom_vfs_read: ino=" FUSE_INO_T_FORMAT "\n", ino));
    if (NULL == (node = flom_vfs_ram_tree_find((gpointer)ino))) {
        FLOM_TRACE(("flom_vfs_read: unable to find node in ram for ino="
                    FUSE_INO_T_FORMAT "\n", ino));
        reply_buf_limited(req, "", strlen(""), off, size);        
    }
    node_in_ram = (flom_vfs_ram_node_t *)node->data;

	reply_buf_limited(req, flom_vfs_ram_node_get_content(node_in_ram),
                      strlen(flom_vfs_ram_node_get_content(node_in_ram)),
                      off, size);
}



/*
int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;

	if (fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1 &&
	    (ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		se = fuse_lowlevel_new(&args, &hello_ll_oper,
				       sizeof(hello_ll_oper), NULL);
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
	}
	fuse_opt_free_args(&args);

	return err ? 1 : 0;
}
*/




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


/* @@@ just for debug, remove me!!! */
gboolean iter(GNode *n, gpointer data) {
    /*
    FLOM_TRACE(("ITER *** data=%p\n", data));
    */
    FLOM_TRACE(("ITER *** name='%s'\n",
                ((flom_vfs_ram_node_t *)n->data)->name));
    return FALSE;
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
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    flom_vfs_ram_node_t *tmp_root_node = NULL;
    flom_vfs_ram_node_t *tmp_status_node = NULL;
    flom_vfs_ram_node_t *tmp_lockers_node = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_init(activate=%d)\n", activate));
    TRY {
        GNode *tmp;

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
        
        /* create the data block for the third node, lockers dir */
        if (NULL == (tmp_lockers_node = flom_vfs_ram_node_create(
                         FLOM_VFS_LOCKERS_DIR_NAME, NULL)))
            THROW(NODE_CREATE_ERROR3);
        /* append the node to the tree, create a child */
        if (NULL == (tmp = g_node_prepend_data(
                         tmp, tmp_lockers_node)))
            THROW(G_NODE_PREPEND_DATA3);
        
        /* @@@ just for debug, remove me */
        g_node_traverse(flom_vfs_ram_tree.root, G_PRE_ORDER, G_TRAVERSE_ALL,
                        -1, iter, NULL);
        tmp = g_node_find(flom_vfs_ram_tree.root, G_LEVEL_ORDER,
                          G_TRAVERSE_ALL, tmp_status_node);
        FLOM_TRACE(("flom_vfs_ram_tree_init: *** '%s'\n",
                    ((flom_vfs_ram_node_t *)tmp->data)->name));
        tmp = g_node_find(flom_vfs_ram_tree.root, G_LEVEL_ORDER,
                          G_TRAVERSE_ALL, tmp_lockers_node);
        FLOM_TRACE(("flom_vfs_ram_tree_init: *** '%s'\n",
                    ((flom_vfs_ram_node_t *)tmp->data)->name));
        /* ### implement now the real logic of FUSE travelling the tree */

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BYPASS_ACTIVATION:
                ret_cod = FLOM_RC_INACTIVE_FEATURE;
                break;
            case NODE_CREATE_ERROR1:
            case NODE_CREATE_ERROR2:
            case NODE_CREATE_ERROR3:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case G_NODE_NEW_ERROR1:
                ret_cod = FLOM_RC_G_NODE_NEW_ERROR;
                break;
            case G_NODE_PREPEND_DATA2:
            case G_NODE_PREPEND_DATA3:
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



GNode *flom_vfs_ram_tree_find_parent(GNode *node)
{
    enum Exception { INACTIVE_FEATURE
                     , NULL_OBJECT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    GNode *result = NULL;
    int locked = FALSE;
    
    FLOM_TRACE(("flom_vfs_ram_tree_find_parent: node=%p\n", node));
    TRY {
        if (!flom_vfs_ram_tree.active)
            THROW(INACTIVE_FEATURE);
        
        if (NULL == node || NULL == node->data)
            THROW(NULL_OBJECT);
        FLOM_TRACE(("flom_vfs_ram_tree_find_parent: node=%p, name='%s'\n",
                    node, ((flom_vfs_ram_node_t *)node->data)->name));
        
        /* lock the tree to avoid conflicts */
        g_mutex_lock(&flom_vfs_ram_tree.mutex);
        locked = TRUE;
    
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
    if (locked)
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



int flom_vfs_ram_tree_add_locker(flom_uid_t uid, const char *resource_name,
                                 const char *resource_type)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME
                     , RAM_NODE_UID_DIR_ERROR
                     , G_NODE_UID_DIR_APPEND_DATA
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
        /* create the data block for the resource_name file node */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", resource_name);
        if (NULL == (
                tmp_ram_node_resource_name_file = flom_vfs_ram_node_create(
                         FLOM_VFS_LOCKERS_RESNAME_FILE_NAME,
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
                         FLOM_VFS_LOCKERS_RESTYPE_FILE_NAME,
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
    
    FLOM_TRACE(("flom_vfs_ram_tree_del_locker\n"));
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



int flom_vfs_ram_tree_add_locker_holder(flom_uid_t locker_uid,
                                        flom_uid_t conn_uid,
                                        const char *peer_name)
{
    enum Exception { INACTIVE_FEATURE
                     , FIND_NODE_BY_NAME1
                     , FIND_NODE_BY_NAME2
                     , FIND_NODE_BY_NAME3
                     , RAM_NODE_CONN_UID_DIR_ERROR
                     , G_NODE_UID_CONN_DIR_APPEND_DATA
                     , RAM_NODE_RESOURCE_TYPE_FILE_ERROR
                     , G_NODE_PEER_NAME_FILE_APPEND_DATA
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int locked = FALSE;
    flom_vfs_ram_node_t *tmp_ram_node_conn_uid_dir = NULL;
    flom_vfs_ram_node_t *tmp_ram_node_conn_peer_name_file = NULL;
    
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_holder(locker_uid="
                FLOM_UID_T_FORMAT ", conn_uid=" FLOM_UID_T_FORMAT ")\n",
                locker_uid, conn_uid));
    TRY {
        GNode *lockers_node = NULL;
        GNode *specific_locker_node = NULL;
        GNode *holders_node = NULL;
        GNode *tmp_node_conn_uid_dir = NULL;
        GNode *tmp_node_peer_name_file = NULL;
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
        /* locate the holders directory */
        if (FLOM_RC_OK != (ret_cod = flom_vfs_ram_tree_find_node_by_name(
                               specific_locker_node,
                               FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME,
                               TRUE, &holders_node)))
            THROW(FIND_NODE_BY_NAME3);
        /* create the data block for the conn node */
        sprintf(uid_buffer, FLOM_UID_T_FORMAT, conn_uid);
        if (NULL == (tmp_ram_node_conn_uid_dir = flom_vfs_ram_node_create(
                         uid_buffer, NULL)))
            THROW(RAM_NODE_CONN_UID_DIR_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_conn_uid_dir = g_node_append_data(
                         holders_node, tmp_ram_node_conn_uid_dir)))
            THROW(G_NODE_UID_CONN_DIR_APPEND_DATA);
        tmp_ram_node_conn_uid_dir = NULL;        
        /* create the data block for the peer_name file node */
        snprintf(string_buffer, sizeof(string_buffer), "%s\n", peer_name);
        if (NULL == (
                tmp_ram_node_conn_peer_name_file = flom_vfs_ram_node_create(
                    FLOM_VFS_LOCKERS_HOLDERS_PEERNAME_FILE_NAME,
                    string_buffer)))
            THROW(RAM_NODE_RESOURCE_TYPE_FILE_ERROR);
        /* append the node to the tree, create a child */
        if (NULL == (tmp_node_peer_name_file = g_node_append_data(
                         tmp_node_conn_uid_dir,
                         tmp_ram_node_conn_peer_name_file)))
            THROW(G_NODE_PEER_NAME_FILE_APPEND_DATA);
        tmp_ram_node_conn_peer_name_file = NULL;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case FIND_NODE_BY_NAME1:
            case FIND_NODE_BY_NAME2:
            case FIND_NODE_BY_NAME3:
                break;
            case RAM_NODE_CONN_UID_DIR_ERROR:
            case RAM_NODE_RESOURCE_TYPE_FILE_ERROR:
                break;
            case G_NODE_UID_CONN_DIR_APPEND_DATA:
            case G_NODE_PEER_NAME_FILE_APPEND_DATA:
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
            else if (NULL != tmp_ram_node_conn_peer_name_file)
                flom_vfs_ram_node_destroy(tmp_ram_node_conn_peer_name_file);
        } /* if (excp != NONE) */
    } /* TRY-CATCH */
    /* unlock the tree to avoid conflicts */
    if (locked)
        g_mutex_unlock(&flom_vfs_ram_tree.mutex);
    FLOM_TRACE(("flom_vfs_ram_tree_add_locker_holder/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

