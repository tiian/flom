/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_DAEMON_MNGMNT_VFS_H
# define FLOM_DAEMON_MNGMNT_VFS_H



#include <config.h>



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DAEMON_MNGMNT



/**
 * Structure with the pointers to all the callback functions invoked by FUSE
 */
struct fuse_lowlevel_ops fuse_callback_functions;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    void hello_ll_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);


    void hello_ll_getattr(fuse_req_t req, fuse_ino_t ino,
                                 struct fuse_file_info *fi);

    void hello_ll_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              size_t size);

    void hello_ll_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                                 off_t off, struct fuse_file_info *fi);

    void hello_ll_open(fuse_req_t req, fuse_ino_t ino,
                              struct fuse_file_info *fi);

    void hello_ll_read(fuse_req_t req, fuse_ino_t ino, size_t size,
                              off_t off, struct fuse_file_info *fi);



#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_DAEMON_MNGMNT_VFS_H */
