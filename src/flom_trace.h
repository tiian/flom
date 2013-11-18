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
#ifndef FLOM_TRACE_H
# define FLOM_TRACE_H



#include <config.h>



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
 * trace module for connect functions
 */
#define FLOM_TRACE_MOD_CONNECT            0x00000008

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
 * trace module for rsrc (resoruce) functions
 */
#define FLOM_TRACE_MOD_RSRC               0x00000200



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
# define FLOM_TRACE_REOPEN(a) flom_trace_reopen(a)
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
                                     flom_trace_hex_data(a,b,c,stderr) : 0)
#else
# define FLOM_TRACE_HEX_DATA(a,b,c)
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
     */     
    void flom_trace_reopen(const char *file_name);

    
    
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
     * @param out_stream IN destination standard I/O stream
     */
    void flom_trace_hex_data(const char *prefix, const byte_t *data,
                             size_t size, FILE *out_stream);


      
    /**
     * Dump the content of a piece of memory to a stream (text format)
     * @param prefix IN trace prefix to print before dump (it is a fixed
     *               prefix, not a format with values)
     * @param data IN pointer to base memory
     * @param size IN number of bytes to dump
     * @param out_stream IN destination standard I/O stream
     */
    void flom_trace_text_data(const char *prefix, const byte_t *data,
                              size_t size, FILE *out_stream);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_TRACE_H */
