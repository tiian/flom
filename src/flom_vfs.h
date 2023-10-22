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
 * Buffer size used for standard purposes, like for examples adding a \n
 * at the end of strings when necessary
 */
#define FLOM_VFS_STD_BUFFER_SIZE 1024



/**
 * Type used to create an in RAM representation of the information that
 * are exposed by the VFS for every file and directory (node)
 */
typedef struct {
    /**
     * Name associated to the inode (filename / dirname)
     */
    char            *name;
    /**
     * Content of the file; in case of a dir, content == NULL
     */
    char            *content;
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
     * @param content IN of the file when the node is associated to a file;
     *        use NULL when the node is used to create a dir
     * @return a valid pointer or NULL in case of error
     */
    flom_vfs_ram_node_t *flom_vfs_ram_node_create(
        const char *name, const char *content);



    /**
     * @return if the ram node is associated to a directory (TRUE) or to a
     *         regular file (FALSE)
     */
    static inline int flom_vfs_ram_node_is_dir(
        const flom_vfs_ram_node_t *node) {
        return node->content == NULL;
    }
    


    /**
     * @return the content associated to a regular file; in case of dir an
     *         empty string is returned
     */
    static inline const char *flom_vfs_ram_node_get_content(
        const flom_vfs_ram_node_t *node) {
        return node->content != NULL ? node->content : "";
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
     * @param node IN to start cleanup: if NULL, root will be used
     * @param locked IN TRUE if the global mutex is already locked by the
     *        caller, FALSE if the global mutex is not already locked and it
     *        must be locked/unlocked by this function
     */
    void flom_vfs_ram_tree_cleanup(GNode *node, int locked);



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
     * Localize a node in the tree searching for it for the name; in case of
     * duplicated names, the first occurrence will be localized
     * @param start IN node in the tree where the search must start
     * @param name IN of the file (or dir) in the node
     * @param locked IN TRUE if the global mutex is already locked by the
     *        caller, FALSE if the global mutex is not already locked and it
     *        must be locked/unlocked by this function
     * @param result OUT the pointer to the node if it exists or NULL if it
     *        does not exists
     * @return a reason code
     */     
    int flom_vfs_ram_tree_find_node_by_name(GNode *start, const char *name,
                                            int locked, GNode **result);

    

    /**
     * Retrieve all the children of a node to build the list of the directory
     * The allocated array, in not NULL, must be freed with usage
     * @param data IN associated to a node in the tree
     * @return an array of structs, one for every children
     */
    GArray *flom_vfs_ram_tree_retrieve_children(gpointer data);



    /**
     * Add in the ram tree a node associated to a locker
     * @param uid IN unique identified of the locker, it will the name of the
     *        directory in the VFS
     * @param resource_name IN the name of the resource managed by the locker
     * @param resource_type IN the type (human readable) of the resource
     *        managed by the locker
     * @return a reason code
     */
    int flom_vfs_ram_tree_add_locker(flom_uid_t uid,
                                     const char *resource_name,
                                     const char *resource_type);



    /**
     * Delete from the ram tree the node associated to a locker
     * @param uid IN unique identified of the locker, it is the name of the
     *        directory in the VFS
     * @return a reason code
     */     
    int flom_vfs_ram_tree_del_locker(flom_uid_t uid);


    
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
 * Inode associated to root dir
 */
#define FLOM_VFS_INO_ROOT_DIR             (fuse_ino_t)1



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

    void flom_vfs_open(fuse_req_t req, fuse_ino_t ino,
                       struct fuse_file_info *fi);

    void flom_vfs_read(fuse_req_t req, fuse_ino_t ino, size_t size,
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
