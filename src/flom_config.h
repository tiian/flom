/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_CONFIG_H
# define FLOM_CONFIG_H



#include <config.h>



#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_CONFIG



#define LOCAL_SOCKET_SIZE sizeof(((struct sockaddr_un *)NULL)->sun_path)



/* configure dependent constant values */
/**
 * E-mail address as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_BUGREPORT;
/**
 * Name of the package as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_NAME;
/**
 * Version of the package as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_VERSION;
/**
 * Date of package release as set inside configure.ac
 */
extern const char *FLOM_PACKAGE_DATE;
/**
 * Installation configuration dir (./configure output)
 */
extern const char FLOM_INSTALL_SYSCONFDIR[];



/**
 * Default name for a simple resource
 */
extern const char *DEFAULT_RESOURCE_NAME;
/**
 * Filename of system wide configuration file
 */
extern const char FLOM_SYSTEM_CONFIG_FILENAME[];
/**
 * Filename of user default configuration file
 */
extern const char FLOM_USER_CONFIG_FILENAME[];
/**
 * Separator used between directory and file names
 */
extern const char FLOM_DIR_FILE_SEPARATOR[];



/**
 * This struct contains all the values necessary for configuration
 */
typedef struct flom_config {
    /**
     * Path of UNIX socket using for local connection
     */
    char        local_socket_path_name[LOCAL_SOCKET_SIZE];
    /**
     * Name of the file must be used to write trace messages from the daemon
     */
    char const *daemon_trace_file;
    /**
     * Name of the file must be used to write trace messages from the command
     */
    char const *command_trace_file;
    /**
     * After idle_time milliseconds without new incoming requests, the daemon
     * will terminate activity
     */
    int         idle_time;
    /**
     * Name of the resource that must be locked
     */
    char const *resource_name;
} flom_config_t;



/**
 * This is a global static object shared by all the application
 */
extern flom_config_t global_config;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Set config to system default
     */
    void flom_config_reset();
    


    /**
     * Initialize configuration (global) object retrieving data from
     * configuration files
     * @param user_config_file_name IN filename of user configuration file
     */
    void flom_config_init(const char *user_config_file_name);



    /**
     * Load a configuration file, parse it and initialize global configuration
     * as described in the config file
     * @param config_file_name IN configuration file to open and parse
     */
    void flom_config_init_load(const char *config_file_name);


    
    /**
     * Set trace_file in config object
     * @param daemon_trace_file IN set the new value for trace_file properties
     */
    static inline void flom_config_set_daemon_trace_file(
        const char *daemon_trace_file) {
        global_config.daemon_trace_file = daemon_trace_file; }



    /**
     * Retrieve the trace file specified for daemon process
     * @return trace file name
     */
    static inline const char *flom_config_get_daemon_trace_file(void) {
        return global_config.daemon_trace_file; }


    
    /**
     * Set trace_file in config object
     * @param command_trace_file IN set the new value for trace_file properties
     */
    static inline void flom_config_set_command_trace_file(
        const char *command_trace_file) {
        global_config.command_trace_file = command_trace_file; }



    /**
     * Retrieve the trace file specified for command process
     * @return trace file name
     */
    static inline const char *flom_config_get_command_trace_file(void) {
        return global_config.command_trace_file; }


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_CONFIG_H */
