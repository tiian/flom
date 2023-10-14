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
 * Experimental stuff: use GLIB N-ary Tree to create the VFS in memory.
 * Move this types to a more convenient file if necessary to avoid strange
 * dependencies in the includes
 */


#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



/**
 * Type used to create an in RAM representation of the information that
 * are exposed by the VFS for every file and directory (node)
 */
typedef struct {
    /**
     * Boolean value, TRUE if the RAM node is related to a directory, FALSE
     * if the RAM node is related to a regular file
     */
    int              is_dir;
    /**
     * Name associated to the inode
     */
    char            *name;
} flom_vfs_ram_node_t;
 


/**
 * This struct is used to generate temporary arrays that are returned to
 * provide the content of a dir. It must be used as a struct, with direct
 * access to the fields
 */
struct flom_vfs_ino_name_pair_s {
    fuse_ino_t  ino;
    char       *name;
};



/**
 * This struct is used to retrieve the parent of a child from an iterator
 */
struct flom_vfs_ram_tree_parent_child_s {
    GNode *parent;
    GNode *child;
};
    


/**
 * Type used to create an in RAM representation of the whole VFS.
 * Working threads update this object, VFS thread read this object
 */
typedef struct {
    /**
     * The ram tree is active: if FALSE, no operation must be done on the
     * object because the VFS feature is disabled. The field is not protected
     * by mutex because it's set only once by @ref flom_vfs_ram_tree_init and
     * read by all other functions
     */
    int              active;
    /**
     * This mutex serializes all and every access to the n-ary tree
     */
    GMutex           mutex;
    /**
     * N-ary tree used to represent the VFS
     */
    GNode           *root;
} flom_vfs_ram_tree_t;



flom_vfs_ram_tree_t flom_vfs_ram_tree;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Create a new node for a file or a directory
     * @param name IN name associated to the inode
     * @param is_dir IN the name is associated to a directory (if FALSE, it's
     *        associated to a regular file)
     * @return a valid pointer or NULL in case of error
     */
    flom_vfs_ram_node_t *flom_vfs_ram_node_create(
        const char *name, int is_dir);



    /**
     * @return if the ram node is associated to a directory (TRUE) or to a
     *         regular file (FALSE)
     */
    static inline int flom_vfs_ram_node_is_dir(
        const flom_vfs_ram_node_t *node) {
        return node->is_dir;
    }
    


    /**
     * Destroy a node for a file or a directory
     * @param node OUT pointer to the node to be destroyed
     */
    void flom_vfs_ram_node_destroy(flom_vfs_ram_node_t *node);
    
        

    /**
     * Initialize the VFS RAM n-ary tree. No parameters are necessary because
     * there's only one static object of this type, shared by all the threads
     * @param activate IN is a boolean value TRUE if the VFS must be activated,
     *        FALSE if the VFS must not be activated and the ram tree must
     *        stay empty
     * @return a reason code
     */
    int flom_vfs_ram_tree_init(int activate);



    /**
     * Cleanup all the memory used by the VFS RAM n-ary tree
     */
    void flom_vfs_ram_tree_cleanup();



    /**
     * Find in the n-ary tree a specific data node;
     * @param data IN is a pointer to a node of data in the tree; if data is
     *        NULL, the root node of the tree will be returned
     * @return NULL if the node is not available, a pointer to the node in
     *         the tree if the data are available in that node
     */
    GNode *flom_vfs_ram_tree_find(gpointer data);



    /**
     * Itarator to traverse a ram tree in search of the parent of a node
     * @param node IN traversed node
     * @param data IN/OUT a pointer to a @ref flom_vfs_ram_tree_parent_child_s
     *        struct that contains both the child to be find and the parent
     *        when found
     */
    gboolean flom_vfs_ram_tree_find_parent_iter(GNode *node, gpointer data);



    /**
     * Find a parent of a node in the ram tree
     * @param node IN is a pointer to a node in the tree
     * @param the parent of the node if it exists; for root node, root itself
     *        is returned
     */
    GNode *flom_vfs_ram_tree_find_parent(GNode *node);
    

    
    /**
     * Find a specific child of a node, searching for the name of the file
     * @param root IN inode of the root of the tree; the search can start at
     *        any point, but the node can not be a leaf (it must have children)
     * @param name IN of the file (or dir) in the node
     * @return the pointer to the node if it exists or NULL if it does not
     *         exists
     */
    GNode *flom_vfs_ram_tree_find_child_by_name(
        GNode *root, const char *name);



    /**
     * Retrieve all the children of a node to build the list of the directory
     * The allocated array, in not NULL, must be freed with usage
     */
    GArray *flom_vfs_ram_tree_retrieve_children(gpointer data);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * END OF:
 * Experimental stuff: use GLIB N-ary Tree to create the VFS in memory.
 * Move this types to a more convenient file if necessary to avoid strange
 * dependencies in the includes
 */




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
#define FLOM_VFS_INO_ROOT_DIR             (fuse_ino_t)1
/**
 * Inode associated to status dir
 */
#define FLOM_VFS_INO_STATUS_DIR           (fuse_ino_t)2
/**
 * Inode associated to lockers dir
 */
#define FLOM_VFS_INO_LOCKERS_DIR          (fuse_ino_t)3
/**
 * First Inode used for locker dir and files
 */
#define FLOM_VFS_LOCKERS_UID_FIRST_INO    (FLOM_VFS_INO_LOCKERS_DIR + 1)
/**
 * Last Inode that can be used for locker dir and files
 */
#define FLOM_VFS_LOCKERS_UID_LAST_INO     (FLOM_VFS_INO_LAST_POSSIBLE)


const char *FLOM_VFS_NAME_ROOT_DIR;




/**
 * Structure with the pointers to all the callback functions invoked by FUSE
 */
struct fuse_lowlevel_ops fuse_callback_functions;



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
    /**
     * VFS activation time, it will be used as the default time
     */
    time_t    time;
} flom_vfs_common_values_t;

flom_vfs_common_values_t flom_vfs_common_values;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    void flom_vfs_lookup(fuse_req_t req, fuse_ino_t parent, const char *name);


    void hello_ll_getattr(fuse_req_t req, fuse_ino_t ino,
                                 struct fuse_file_info *fi);

    void hello_ll_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name,
                              size_t size);

    void flom_vfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
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
