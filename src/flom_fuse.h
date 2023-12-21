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



/*
 * CREDITS:
 * some excerpts of source code are copies or adaptations of pieces of code
 * available in
 * libfuse/example/hello_ll.c
 * Both "FLoM" and "libfuse" are distributed under the terms of GPLv2, so
 * there's no license infringement
 */



#ifndef FLOM_FUSE_H
# define FLOM_FUSE_H



#include <config.h>



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DAEMON_MNGMNT



#include "flom_defines.h"
#include "flom_types.h"



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



/**
 * Structure with the pointers to all the callback functions invoked by FUSE
 */
extern struct fuse_lowlevel_ops fuse_callback_functions;



/**
 * Structure used to store common values that does not require to be
 * retrieved by system call every time they are needed
 */
typedef struct {
    /**
     * user id that will be associated to all dirs and files
     */
    uid_t     uid;
    /**
     * group id that will be associated to all dirs and files
     */
    gid_t     gid;
} flom_fuse_common_values_t;

extern flom_fuse_common_values_t flom_fuse_common_values;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * FUSE "lookup" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     */
    void flom_fuse_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);


    
    /**
     * FUSE "getattr" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     */
    void flom_fuse_getattr(fuse_req_t req, fuse_ino_t ino,
                          struct fuse_file_info *fi);


    
    /**
     * FUSE "getxattr" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     * Note: currently not really implemented/used by FLoM
     */
    void flom_fuse_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                           size_t size);


    
    /**
     * FUSE "readdir" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     */
    void flom_fuse_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                          off_t off, struct fuse_file_info *fi);


    
    /**
     * FUSE "open" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     */
    void flom_fuse_open(fuse_req_t req, fuse_ino_t ino,
                       struct fuse_file_info *fi);


    
    /**
     * FUSE "read" callback function, see
     * https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html
     * for original libfuse documentation
     */
    void flom_fuse_read(fuse_req_t req, fuse_ino_t ino, size_t size,
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



#endif /* FLOM_FUSE_H */
