/*
 * Copyright (c) 2013-2024, Christian Ferrari <tiian@users.sourceforge.net>
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



#include "flom_defines.h"
#include "flom_types.h"



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
    /**
     * Creation time of the ram node (of the virtual file)
     */
    time_t           ctime;
    /**
     * Modification time of the ram node (of the virtual file)
     */
    time_t           mtime;
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



/**
 * In memory mapping of the Virtual File System that FUSE makes accessible
 * to the user
 */
extern flom_vfs_ram_tree_t flom_vfs_ram_tree;



/**
 * Filename of root dir
 */
extern const char *FLOM_VFS_ROOT_DIR_NAME;
/**
 * Filename of status dir
 */
extern const char *FLOM_VFS_STATUS_DIR_NAME;
/**
 * Filename of incubator dir
 */
extern const char *FLOM_VFS_INCUBATOR_DIR_NAME;
/**
 * Filename of lockers dir
 */
extern const char *FLOM_VFS_LOCKERS_DIR_NAME;
/**
 * Filename of resource_name file
 */
extern const char *FLOM_VFS_RESNAME_FILE_NAME;
/**
 * Filename of resource_type file
 */
extern const char *FLOM_VFS_RESTYPE_FILE_NAME;
/**
 * Filename of holders dir
 */
extern const char *FLOM_VFS_LOCKERS_HOLDERS_DIR_NAME;
/**
 * Filename of waitings dir
 */
extern const char *FLOM_VFS_LOCKERS_WAITINGS_DIR_NAME;
/**
 * Filename of peer_name file
 */
extern const char *FLOM_VFS_PEERNAME_FILE_NAME;
/**
 * Filename of lock_mode file
 */
extern const char *FLOM_VFS_LOCKERS_LOCKMODE_FILE_NAME;
/**
 * Filename of quantity file
 */
extern const char *FLOM_VFS_LOCKERS_QUANTITY_FILE_NAME;
/**
 * Filename of sequence_value file
 */
extern const char *FLOM_VFS_LOCKERS_SEQUENCE_VALUE_FILE_NAME;
/**
 * Filename of timestamp_value file
 */
extern const char *FLOM_VFS_LOCKERS_TIMESTAMP_VALUE_FILE_NAME;



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
     * @return creation time of the ram node / virtual file
     */
    static inline time_t flom_vfs_ram_node_get_ctime(
        const flom_vfs_ram_node_t *node) {
        return node->ctime;
    }


    
    /**
     * @return modifcation time of the ram node / virtual file
     */
    static inline time_t flom_vfs_ram_node_get_mtime(
        const flom_vfs_ram_node_t *node) {
        return node->mtime;
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
     * @param locked IN TRUE if the global mutex is already locked by the
     *        caller, FALSE if the global mutex is not already locked and it
     *        must be locked/unlocked by this function
     * @param the parent of the node if it exists; for root node, root itself
     *        is returned
     */
    GNode *flom_vfs_ram_tree_find_parent(GNode *node, int locked);
    

    
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
     * Update the modification time of a node in the ram tree
     * @param node IN is the node with the modification time that must be
     *        updated
     * @return a reason code
     */
    int flom_vfs_ram_tree_update_mtime(GNode *node);


    
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
     * @param uid IN unique identifier of the locker, it is the name of the
     *        directory in the VFS
     * @return a reason code
     */     
    int flom_vfs_ram_tree_del_locker(flom_uid_t uid);



    /**
     * Add in the ram tree a node associated to a connection below a specific
     * locker; connection can be added to the list of "holders" or to the list
     * of "waitings"
     * @param locker_uid IN unique identifier of the locker
     * @param conn_uid IN unique identifier of the conn (connection)
     * @param is_holder IN boolean value, if TRUE the connection will be added
     *        in the "holders" list, otherwise in the "waitings" list
     * @param peer_name IN IP address and port in human readable format
     * @param lock_mode IN asked by the client
     * @param quantity IN requested quantity, when applicable (otherwise NULL
     *        is passed)
     * @param sequence_value IN assigned to the requester, when applicable
     *        (otherwise NULL is passed)
     * @param timestamp_value IN assigned to the requester, when applicable
     *        (otherwise NULL is passed)
     * @return a reason code
     */
    int flom_vfs_ram_tree_add_locker_conn(flom_uid_t locker_uid,
                                          flom_uid_t conn_uid,
                                          int is_holder,
                                          const char *peer_name,
                                          flom_lock_mode_t lock_mode,
                                          const gint *quantity,
                                          const gchar *sequence_value,
                                          const gchar *timestamp_value);



    /**
     * Add a file inside the directory assigned to a connection
     * @param locker_uid IN unique identifier of the locker
     * @param conn_uid IN unique identifier of the conn (connection)
     * @param already_locked IN TRUE if the global mutex is already locked by
     *        the caller, FALSE if the global mutex is not already locked and
     *        it must be locked/unlocked by this function
     * @param file_name IN name of the file to be added
     * @param file_content IN content of the file to be added
     * @return a reason code
     */
    int flom_vfs_ram_tree_add_locker_conn_file(flom_uid_t locker_uid,
                                               flom_uid_t conn_uid,
                                               int already_locked,
                                               const char *file_name,
                                               const char *file_content);


    
    /**
     * Delete from the ram tree the node associated to a connection below a
     * locker; connection can be removed from the list of "holders"
     * or from the list of "waitings"
     * @param conn_uid IN unique identifier of the conn (connection)
     * @param incubating IN TRUE if the connection is still incubating and the
     *        connection must be removed from the incubator; FALSE if the
     *        connection is associated to a locker
     * @return a reason code
     */     
    int flom_vfs_ram_tree_del_conn(flom_uid_t conn_uid,
                                   int incubating);



    /**
     * Move a connection in the ram tree from the waitings list to the holders
     * list
     * @param conn_uid IN unique identifier of the conn (connection)
     * @return a reason code
     */
    int flom_vfs_ram_tree_move_locker_conn(flom_uid_t conn_uid);

    

    /**
     * Add a folder and the files inside it for a new connection entered in
     * the incubator
     * @param conn_uid IN unique identifier of the conn (connection)
     * @param peer_name IN IP address and port in human readable format
     * @param resource_name IN the name of the resource managed by the locker
     * @param resource_type IN the type (human readable) of the resource
     *        managed by the locker
     * @return a reason code
     */
    int flom_vfs_ram_tree_add_incubator_conn(flom_uid_t conn_uid,
                                             const char *peer_name,
                                             const char *resource_name,
                                             const char *resource_type);


    
    /**
     * Add a file inside a directory in the VFS ram tree.
     * Note: this function can be called only by a function that already
     *       locked the VFS ram tree and that guarantees the reference to dir
     *       is valid (the lock of the VFS ram tree is the condition that
     *       proof the constraint)
     * @param dir IN/OUT reference to the directory in the VFS ram tree
     * @param file_name IN name of the file to be added
     * @param file_content IN content of the file to be added
     * @return a reason code
     */
    int flom_vfs_ram_tree_add_file(GNode *dir,
                                   const char *file_name,
                                   const char *file_content);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



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
extern struct fuse_lowlevel_ops fuse_callback_functions;



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_VFS_H */
