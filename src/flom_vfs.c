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



static const char *hello_str = "Hello World!\n";
static const char *hello_name = "hello";



struct fuse_lowlevel_ops fuse_callback_functions = {
	.lookup		= hello_ll_lookup,
	.getattr	= hello_ll_getattr,
	.getxattr	= hello_ll_getxattr,
	.readdir	= hello_ll_readdir,
	.open		= hello_ll_open,
	.read		= hello_ll_read,
};



void flom_vfs_inode_to_uid(fuse_ino_t ino,
                           flom_vfs_inode_type_t *type,
                           flom_uid_t *uid)
{
    FLOM_TRACE(("flom_vfs_inode_to_uid/ino=" FLOM_UID_T_FORMAT "\n", ino));

    if (FLOM_VFS_INO_ROOT_DIR == ino) {
        *type = FLOM_VFS_ROOT_DIR;
        *uid = 0;
    } else if (FLOM_VFS_INO_STATUS_DIR == ino) {
        *type = FLOM_VFS_STATUS_DIR;
        *uid = 0;
    } else if (FLOM_VFS_INO_LOCKERS_DIR == ino) {
        *type = FLOM_VFS_LOCKERS_DIR;
        *uid = 0;
    }
    /* @@@ put code here */
    FLOM_TRACE(("flom_vfs_inode_to_uid/type=%d/"
                "uid=" FLOM_UID_T_FORMAT "\n", *type, *uid));
}



fuse_ino_t flom_vfs_uid_to_inode(flom_vfs_inode_type_t type,
                                 flom_uid_t uid)
{
    fuse_ino_t ino = 0;
    
    FLOM_TRACE(("flom_vfs_uid_to_inode/type=%d/"
                "uid=" FLOM_UID_T_FORMAT "\n", type, uid));

    /* check for root dir */
    if (FLOM_VFS_ROOT_DIR == type) {
        ino = FLOM_VFS_INO_ROOT_DIR;
    } else if (FLOM_VFS_STATUS_DIR == type) {
        ino = FLOM_VFS_INO_STATUS_DIR;
    } else if (FLOM_VFS_LOCKERS_DIR == type) {
        ino = FLOM_VFS_INO_LOCKERS_DIR;
    }
    /* @@@ put code here */
    FLOM_TRACE(("flom_vfs_uid_to_inode/ino=" FLOM_UID_T_FORMAT "\n", ino));
    return ino;
}



int flom_vfs_check_uid_inode_integrity(void)
{
    enum Exception { ROOT_DIR_ERROR
                     , STATUS_DIR_ERROR
                     , LOCKERS_DIR_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_vfs_check_uid_inode_integrity\n"));
    TRY {
        flom_vfs_inode_type_t type;
        flom_uid_t uid;
        fuse_ino_t ino;
        
        /* Check root dir */
        ino = FLOM_VFS_INO_ROOT_DIR;
        flom_vfs_inode_to_uid(ino, &type, &uid);
        if (ino != flom_vfs_uid_to_inode(type, uid))
            THROW(ROOT_DIR_ERROR);
        /* Check status dir */
        ino = FLOM_VFS_INO_STATUS_DIR;
        flom_vfs_inode_to_uid(ino, &type, &uid);
        if (ino != flom_vfs_uid_to_inode(type, uid))
            THROW(STATUS_DIR_ERROR);
        /* Check lockers dir */
        ino = FLOM_VFS_INO_LOCKERS_DIR;
        flom_vfs_inode_to_uid(ino, &type, &uid);
        if (ino != flom_vfs_uid_to_inode(type, uid))
            THROW(LOCKERS_DIR_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case ROOT_DIR_ERROR:
            case STATUS_DIR_ERROR:
            case LOCKERS_DIR_ERROR:
                ret_cod = FLOM_RC_VFS_CONSISTENCY_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (FLOM_RC_VFS_CONSISTENCY_ERROR == ret_cod)
        syslog(LOG_ERR, FLOM_SYSLOG_FLM021E);
    FLOM_TRACE(("flom_vfs_check_uid_inode_integrity/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}




int hello_stat(fuse_ino_t ino, struct stat *stbuf)
{
	stbuf->st_ino = ino;
	switch (ino) {
	case 1:
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = 
		stbuf->st_ctime = 
		stbuf->st_mtime = time( NULL );
		break;

	case 2:
		stbuf->st_mode = S_IFREG | 0644;
		stbuf->st_nlink = 1;
		stbuf->st_uid = getuid();
		stbuf->st_gid = getgid();
		stbuf->st_atime = 
		stbuf->st_ctime = 
		stbuf->st_mtime = time( NULL );
		stbuf->st_size = strlen(hello_str);
		break;

	default:
		return -1;
	}
	return 0;
}

void hello_ll_getattr(fuse_req_t req, fuse_ino_t ino,
			     struct fuse_file_info *fi)
{
	struct stat stbuf;

	(void) fi;

	memset(&stbuf, 0, sizeof(stbuf));
	if (hello_stat(ino, &stbuf) == -1)
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



void hello_ll_lookup(fuse_req_t req, fuse_ino_t parent, const char *name)
{
	struct fuse_entry_param e;

	if (parent != 1 || strcmp(name, hello_name) != 0)
		fuse_reply_err(req, ENOENT);
	else {
		memset(&e, 0, sizeof(e));
		e.ino = 2;
		e.attr_timeout = 1.0;
		e.entry_timeout = 1.0;
		hello_stat(e.ino, &e.attr);

		fuse_reply_entry(req, &e);
	}
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

void hello_ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
			     off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	if (ino != 1)
		fuse_reply_err(req, ENOTDIR);
	else {
		struct dirbuf b;

		memset(&b, 0, sizeof(b));
		dirbuf_add(req, &b, ".", 1);
		dirbuf_add(req, &b, "..", 1);
		dirbuf_add(req, &b, hello_name, 2);
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
}

void hello_ll_open(fuse_req_t req, fuse_ino_t ino,
			  struct fuse_file_info *fi)
{
	if (ino != 2)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & 3) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else
		fuse_reply_open(req, fi);
}

void hello_ll_read(fuse_req_t req, fuse_ino_t ino, size_t size,
			  off_t off, struct fuse_file_info *fi)
{
	(void) fi;

	assert(ino == 2);
	reply_buf_limited(req, hello_str, strlen(hello_str), off, size);
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
