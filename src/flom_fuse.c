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



/*
 * CREDITS:
 * some excerpts of source code are copies or adaptations of pieces of code
 * available in
 * libfuse/example/hello_ll.c
 * Both "FLoM" and "libfuse" are distributed under the terms of GPLv2, so
 * there's no license infringement
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



#include "flom_fuse.h"
#include "flom_errors.h"
#include "flom_trace.h"
#include "flom_vfs.h"



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



flom_fuse_common_values_t flom_fuse_common_values;


/**
 * FUSE callback functions as documented here:
 * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
 */
struct fuse_lowlevel_ops fuse_callback_functions = {
	.lookup		= flom_fuse_lookup,
	.getattr	= flom_fuse_getattr,
	.getxattr	= flom_fuse_getxattr,
	.readdir	= flom_fuse_readdir,
	.open		= flom_fuse_open,
	.read		= flom_fuse_read,
};



int flom_fuse_stat(fuse_ino_t ino, struct stat *stbuf)
{
    GNode *node_in_ram;
    
    FLOM_TRACE(("flom_fuse_stat: ino=" FLOM_UID_T_FORMAT "\n", ino));
    
    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    if (NULL == (node_in_ram = flom_vfs_ram_tree_find(
                     FLOM_VFS_INO_ROOT_DIR == ino ?
                     NULL : (gpointer)ino))) {
        FLOM_TRACE(("flom_fuse_stat: ino=" FLOM_UID_T_FORMAT
                    ", not found\n", ino));
    } else {
        flom_vfs_ram_node_t *data = (flom_vfs_ram_node_t *)node_in_ram->data;
        if (flom_vfs_ram_node_is_dir(data)) {
            stbuf->st_ino = ino;
            stbuf->st_mode = S_IFDIR | 0550;
            stbuf->st_nlink = 2;
            stbuf->st_uid = flom_fuse_common_values.uid;
            stbuf->st_gid = flom_fuse_common_values.gid;
            stbuf->st_ctime = flom_vfs_ram_node_get_ctime(data);
            stbuf->st_mtime = flom_vfs_ram_node_get_mtime(data);
        } else {
            stbuf->st_ino = ino;
            stbuf->st_mode = S_IFREG | 0440;
            stbuf->st_nlink = 1;
            stbuf->st_uid = flom_fuse_common_values.uid;
            stbuf->st_gid = flom_fuse_common_values.gid;
            stbuf->st_ctime = flom_vfs_ram_node_get_ctime(data);
            stbuf->st_mtime = flom_vfs_ram_node_get_mtime(data);
            stbuf->st_size = strlen(flom_vfs_ram_node_get_content(data));
        }
    }
    
	return 0;
}



void flom_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
                      struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

    FLOM_TRACE(("flom_fuse_getattr: ino=" FLOM_UID_T_FORMAT "\n", ino));
    
	memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
	if (flom_fuse_stat(ino, &stbuf) == -1)
		fuse_reply_err(req, ENOENT);
	else
		fuse_reply_attr(req, &stbuf, 1.0);
}



void flom_fuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                       size_t size)
{
    FLOM_TRACE(("flom_fuse_getxattr: ino=" FLOM_UID_T_FORMAT ", "
                "name='%s', size=" SIZE_T_FORMAT "\n",
                ino, name, size));
    
	fuse_reply_buf(req, NULL, 0);
}



void flom_fuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;
    GNode *parent_node = NULL;
    GNode *child_node = NULL;

    FLOM_TRACE(("flom_fuse_lookup: parent=" FLOM_UID_T_FORMAT ", name='%s'\n",
                parent, name));

    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    parent_node = flom_vfs_ram_tree_find(FLOM_VFS_INO_ROOT_DIR == parent ?
                                         NULL : (gpointer)parent);
    if (NULL == parent_node) {
        FLOM_TRACE(("flom_fuse_lookup: parent=" FLOM_UID_T_FORMAT
                    ", not found\n", parent));
		fuse_reply_err(req, ENOENT);
    } else {
        /* search what node below parent has name "name" */
        if (NULL == (child_node =
                     flom_vfs_ram_tree_find_child_by_name(
                         parent_node, name))) {
            FLOM_TRACE(("flom_fuse_lookup: child with name '%s' not found\n",
                        name));
            fuse_reply_err(req, ENOENT);
        } else {
            flom_vfs_ram_node_t *child_in_ram =
                (flom_vfs_ram_node_t *)child_node->data;
            memset(&e, 0, sizeof(e));
            e.ino = (fuse_ino_t)child_in_ram;
            e.attr_timeout = 1.0;
            e.entry_timeout = 1.0;
            flom_fuse_stat(e.ino, &e.attr);
            
            fuse_reply_entry(req, &e);
        }
    }    
}



/*
 * BEGIN
 *
 * Source code imported from libfuse/example/hello_ll.c
 */
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
/*
 * END
 *
 * Source code imported from libfuse/example/hello_ll.c
 */



void flom_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                      off_t off, struct fuse_file_info *fi)
{
    GNode *node, *parent;
    flom_vfs_ram_node_t *node_in_ram;
    GArray *children = NULL;
    FLOM_TRACE(("flom_fuse_readdir: ino=" FUSE_INO_T_FORMAT "\n", ino));

    /* ino is mapped to the pointer of the node in RAM, but
       root dir is special case, ino==1 every time */
    node = flom_vfs_ram_tree_find(FLOM_VFS_INO_ROOT_DIR == ino ?
                                  NULL : (gpointer)ino);
    parent = flom_vfs_ram_tree_find_parent(node, FALSE);
    if (NULL == node) {
        /* the inode does not exists! */
		fuse_reply_err(req, ENOENT);        
    } else {
        node_in_ram = (flom_vfs_ram_node_t *)node->data;
        if (!flom_vfs_ram_node_is_dir(node_in_ram)) {
            FLOM_TRACE(("flom_fuse_readdir: inode=" FUSE_INO_T_FORMAT
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
                FLOM_TRACE(("flom_fuse_readdir: name='%s', ino="
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
}



void flom_fuse_open(fuse_req_t req, fuse_ino_t ino,
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
    
    FLOM_TRACE(("flom_fuse_open: ino=" FUSE_INO_T_FORMAT "\n", ino));
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
    FLOM_TRACE(("flom_fuse_open/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return;
}



void flom_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
                   off_t off, struct fuse_file_info *fi)
{
    GNode *node = NULL;
    flom_vfs_ram_node_t *node_in_ram = NULL;
    
    FLOM_TRACE(("flom_fuse_read: ino=" FUSE_INO_T_FORMAT "\n", ino));
    if (NULL == (node = flom_vfs_ram_tree_find((gpointer)ino))) {
        FLOM_TRACE(("flom_fuse_read: unable to find node in ram for ino="
                    FUSE_INO_T_FORMAT "\n", ino));
        reply_buf_limited(req, "", strlen(""), off, size);        
    }
    node_in_ram = (flom_vfs_ram_node_t *)node->data;

	reply_buf_limited(req, flom_vfs_ram_node_get_content(node_in_ram),
                      strlen(flom_vfs_ram_node_get_content(node_in_ram)),
                      off, size);
}
