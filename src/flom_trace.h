/*
 * Copyright (c) 2013-2018, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_TRACE_H
# define FLOM_TRACE_H



#include <config.h>



#ifdef HAVE_IFADDRS_H
# include <ifaddrs.h>
#endif
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif /* HAVE_STDIO_H */



#include <flom_defines.h>



/**
 * Name of the environment variable must be used to set the trace mask
 */
#define FLOM_TRACE_MASK_ENV_VAR    "FLOM_TRACE_MASK"



/**
 * trace module for files do not need trace feature
 */
#define FLOM_TRACE_MOD_NO_TRACE           0x00000000

/**
 * trace module for generic functions
 */
#define FLOM_TRACE_MOD_GENERIC            0x00000001

/**
 * trace module for config functions
 */
#define FLOM_TRACE_MOD_CONFIG             0x00000002

/**
 * trace module for fork & exec functions
 */
#define FLOM_TRACE_MOD_EXEC               0x00000004

/**
 * trace module for client functions
 */
#define FLOM_TRACE_MOD_CLIENT             0x00000008

/**
 * trace module for daemon functions
 */
#define FLOM_TRACE_MOD_DAEMON             0x00000010

/**
 * trace module for conns (utility) functions
 */
#define FLOM_TRACE_MOD_CONNS              0x00000020

/**
 * trace module for parser functions
 */
#define FLOM_TRACE_MOD_PARSER             0x00000040

/**
 * trace module for messages functions
 */
#define FLOM_TRACE_MOD_MSG                0x00000080

/**
 * trace module for locker functions
 */
#define FLOM_TRACE_MOD_LOCKER             0x00000100

/**
 * trace module for rsrc (resource) functions
 */
#define FLOM_TRACE_MOD_RSRC               0x00000200

/**
 * trace module for simple resource functions
 */
#define FLOM_TRACE_MOD_RESOURCE_SIMPLE    0x00000400

/**
 * trace module for numeric resource functions
 */
#define FLOM_TRACE_MOD_RESOURCE_NUMERIC   0x00000800

/**
 * trace module for resource set functions
 */
#define FLOM_TRACE_MOD_RESOURCE_SET       0x00001000

/**
 * trace module for hierarchical resource functions
 */
#define FLOM_TRACE_MOD_RESOURCE_HIER      0x00002000

/**
 * trace module for sequence resource functions
 */
#define FLOM_TRACE_MOD_RESOURCE_SEQUENCE  0x00004000

/**
 * trace module for timestamp resource functions
 */
#define FLOM_TRACE_MOD_RESOURCE_TIMESTAMP 0x00008000

/**
 * trace module for daemon management functions
 */
#define FLOM_TRACE_MOD_DAEMON_MNGMNT      0x00040000

/**
 * trace module for API (Application Programming Interface) client library
 */
#define FLOM_TRACE_MOD_API                0x00080000

/**
 * trace module for debug features
 */
#define FLOM_TRACE_MOD_DEBUG_FEATURES     0x00100000

/**
 * trace module for TLS/SSL functions
 */
#define FLOM_TRACE_MOD_TLS                0x00200000

/**
 * trace module for TCP functions
 */
#define FLOM_TRACE_MOD_TCP                0x00400000



/**
 * Status of the trace: TRUE = initialized, FALSE = uninitialized
 */
extern int flom_trace_initialized;



/**
 * This is the mask retrieved from environment var FLOM_TRACE_MASK and
 * determines which modules are traced
 */
extern unsigned long flom_trace_mask;



/**
 * FLOM_TRACE_INIT macro is used to compile @ref flom_trace_init function
 * only if _TRACE macro is defined
 */
#ifdef _TRACE
# define FLOM_TRACE_INIT   flom_trace_init()
#else
# define FLOM_TRACE_INIT
#endif



/**
 * FLOM_TRACE_REOPEN macro is used to compile
 * @ref flom_trace_reopen function
 * only if _TRACE macro is defined
 */
#ifdef _TRACE
# define FLOM_TRACE_REOPEN(a,b) flom_trace_reopen(a,b)
#else
# define FLOM_TRACE_REOPEN
#endif



/**
 * FLOM_TRACE macro is used to compile trace messages only if _TRACE macro is
 * defined
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE(a)    (FLOM_TRACE_MODULE & flom_trace_mask ? \
                           flom_trace a : 0)
#else
# define FLOM_TRACE(a)
#endif /* _TRACE */



/**
 * FLOM_TRACE_HEX_DATA macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE_HEX_DATA(a,b,c) (FLOM_TRACE_MODULE & flom_trace_mask ? \
                                     flom_trace_hex_data(a,b,c) : 0)
#else
# define FLOM_TRACE_HEX_DATA(a,b,c)
#endif /* _TRACE */



/**
 * FLOM_TRACE_TEXT_DATA macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE_TEXT_DATA(a,b,c) (FLOM_TRACE_MODULE & flom_trace_mask ? \
                                     flom_trace_text_data(a,b,c) : 0)
#else
# define FLOM_TRACE_TEXT_DATA(a,b,c)
#endif /* _TRACE */



/**
 * FLOM_TRACE_ADDRINFO macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE_ADDRINFO(a,b) (FLOM_TRACE_MODULE & flom_trace_mask ? \
                                   flom_trace_addrinfo(a,b) : 0)
#else
# define FLOM_TRACE_ADDRINFO(a,b)
#endif /* _TRACE */



/**
 * FLOM_TRACE_IPADDRS macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef HAVE_GETIFADDRS
    /* getifaddrs is not POSIX and we can not be sure it's available */
# ifdef _TRACE
#  define FLOM_TRACE_IFADDRS(a,b) (FLOM_TRACE_MODULE & flom_trace_mask ? \
                                   flom_trace_ifaddrs(a,b) : 0)
# else /* _TRACE */
#  define FLOM_TRACE_IFADDRS(a,b)
# endif /* _TRACE */
#else /* HAVE_GETIFADDRS */
# define FLOM_TRACE_IFADDRS(a,b)
#endif /* HAVE_GETIFADDRS */



/**
 * FLOM_TRACE_SOCKADDR macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE_SOCKADDR(p,a,l)                                     \
    (FLOM_TRACE_MODULE & flom_trace_mask ? flom_trace_sockaddr(p,a,l) : 0)
#else
# define FLOM_TRACE_SOCKADDR(p,a,l)
#endif /* _TRACE */
    


/**
 * FLOM_TRACE_SSLERR macro is used to compile trace messages only if _TRACE
 * macro is defined;
 * trace message is printed only for modules (FLOM_TRACE_MODULE) covered by
 * trace mask (FLOM_TRACE_MASK) specified as environment variable
 */
#ifdef _TRACE
# define FLOM_TRACE_SSLERR(p,e)    (FLOM_TRACE_MODULE & flom_trace_mask ? \
                                    flom_trace_sslerr(p,e) : 0)
#else
# define FLOM_TRACE_SSLERR(p,e)
#endif /* _TRACE */



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


    
    /**
     * This method MUST be called BEFORE first log call to avoid lock
     * contention in multithread environments
     */
    void flom_trace_init(void);
    


    /**
     * Open a new stream for trace
     * @param file_name IN name of the file must be used for trace or NULL
     *                     for stderr
     * @param append IN value boolean, if TRUE the trace file will be appended,
     *               otherwise the trace file will be truncated
     */     
    void flom_trace_reopen(const char *file_name, int append);

    
    
    /**
     * Send trace record to stderr
     * @param fmt IN record format
     * @param ... IN record data
     */
    void flom_trace(const char *fmt, ...);

        

    /**
     * Dump the content of a piece of memory to a stream (hex format)
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param data IN pointer to base memory
     * @param size IN number of bytes to dump
     */
    void flom_trace_hex_data(const char *prefix, const byte_t *data,
                             size_t size);


      
    /**
     * Dump the content of a piece of memory to a stream (text format)
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param data IN pointer to base memory
     * @param size IN number of bytes to dump
     */
    void flom_trace_text_data(const char *prefix, const byte_t *data,
                              size_t size);



    /**
     * Dump the content of an addrinfo list typically returned by getaddrinfo
     * POSIX function
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param ai IN pointer to list head
     */
    void flom_trace_addrinfo(const char *prefix, const struct addrinfo *ai);



#ifdef HAVE_IFADDRS_H
    /**
     * Dump the content of an ifaddrs list typically returned by getifaddrs
     * non POSIX function
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param ifaddr IN pointer to the first element of the list
     */
    void flom_trace_ifaddrs(const char *prefix, const struct ifaddrs *ifaddr);
#endif


    
    /**
     * Dump the content of a sockaddr struct
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param addr IN the address to be dumped
     * @param addrlen IN specifies the size, in bytes, of the address
     *                   structure pointed to by addr
     */
    void flom_trace_sockaddr(const char *prefix, const struct sockaddr *addr,
                             socklen_t addrlen);
    


    /**
     * Dump the content of the OpenSSL error queue
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param err IN error retrieved with function ERR_get_error
     */
    void flom_trace_sslerr(const char *prefix, unsigned long err);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_TRACE_H */
