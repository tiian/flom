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
#ifndef FLOM_VFS_H
# define FLOM_VFS_H



#include <config.h>



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DAEMON_MNGMNT



#ifdef HAVE_FUSE_LOWLEVEL_H
# define FUSE_USE_VERSION 26
# include <fuse_lowlevel.h>
#endif



#include "flom_defines.h"
#include "flom_types.h"



/*
  Virtual Filesystem Structure:

  /status/lockers/<uid>/resource_name
  /status/lockers/<uid>/resource_type

*/



/**
 * Number of files inside a lockers dir:
 */ 
#define FLOM_VFS_LOCKERS_DIR_NOF          2



/**
 * Type of inode in the Virtual File System: every inode type has its own
 + value
*/
typedef enum flom_vfs_inode_type_e {
    /**
     * Root directory /
     */
    FLOM_VFS_ROOT_DIR,
    /**
     * Status directory /status
     */
    FLOM_VFS_STATUS_DIR,
    /**
     * Lockers directory /status/lockers
     */
    FLOM_VFS_LOCKERS_DIR,
    /**
     * Locker specific directory /status/lockers/<uid>
     */
    FLOM_VFS_LOCKERS_UID_DIR,
    /**
     * Locker resource name file /status/lockers/<uid>/resource_name
     */
    FLOM_VFS_LOCKERS_UID_RESOURCE_NAME_FILE,
    /**
     * Locker resource name file /status/lockers/<uid>/resource_type
     */
    FLOM_VFS_LOCKERS_UID_RESOURCE_TYPE_FILE,
} flom_vfs_inode_type_t;



/**
 * Last possible Inode
 */
#define FLOM_VFS_INO_LAST_POSSIBLE        (fuse_ino_t)-1
/**
 * Inode associated to root dir
 */
#define FLOM_VFS_INO_ROOT_DIR             (fuse_ino_t)0
/**
 * Inode associated to status dir
 */
#define FLOM_VFS_INO_STATUS_DIR           (fuse_ino_t)1
/**
 * Inode associated to lockers dir
 */
#define FLOM_VFS_INO_LOCKERS_DIR          (fuse_ino_t)2
/**
 * First Inode used for locker dir and files
 */
#define FLOM_VFS_LOCKERS_UID_FIRST_INO    (FLOM_VFS_INO_LOCKERS_DIR + 1)
/**
 * Last Inode that can be used for locker dir and files
 */
#define FLOM_VFS_LOCKERS_UID_LAST_INO     (FLOM_VFS_INO_LAST_POSSIBLE)




/**
 * Structure with the pointers to all the callback functions invoked by FUSE
 */
struct fuse_lowlevel_ops fuse_callback_functions;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Conversion from a FUSE inode to a FLoM uid
     * @param ino IN inode in the FUSE filesystem
     * @param type OUT associated to the inode in the FLoM internal structure
     * @param uid OUT of the object represented by the inode
     */
    void flom_vfs_inode_to_uid(fuse_ino_t ino,
                               flom_vfs_inode_type_t *type,
                               flom_uid_t *uid);



    /**
     * Conversion from a FLoM uid to a FUSE inode
     * @param type IN associated to the inode in the FLoM internal structure
     * @param uid IN of the object represented by the inode
     * @return inode in the FUSE filesystem
     */     
    fuse_ino_t flom_vfs_uid_to_inode(flom_vfs_inode_type_t type,
                                     flom_uid_t uid);



    /**
     * Check the system is able to manage the transformation between uid and
     * inode; in case of error, it can be a bug or a compile mistake
     * @return a reason code
     */     
    int flom_vfs_check_uid_inode_integrity(void);

    

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



#endif /* FLOM_VFS_H */
