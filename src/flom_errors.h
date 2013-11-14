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
#ifndef FLOM_ERRORS_H
# define FLOM_ERRORS_H

#include <config.h>



#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif /* HAVE_ERRNO_H */



#include <flom_defines.h>



/**
 * Default exit status when FLOM is unable to execute the command
 */
#define FLOM_ES_UNABLE_TO_EXECUTE_COMMAND   3



/*********************************************************
 *                                                       *
 * REASON / RETURN CODES                                 *
 *                                                       *
 *********************************************************/



/**
 * Peer has closed socket while expecting data
 */
#define FLOM_RC_CONNECTION_CLOSED               +1



/**
 * Successfully completion
 */
#define FLOM_RC_OK                               0



/**
 * Internal error: unrecoverable status!
 */
#define FLOM_RC_INTERNAL_ERROR                  -1
/**
 * Flom was not able to start a new daemon and/or connect to it
 */
#define FLOM_RC_DAEMON_NOT_STARTED              -2
/**
 * Unespected network event
 */
#define FLOM_RC_NETWORK_EVENT_ERROR             -3
/**
 * A passed object/option/arg is NULL and it can NOT be inferred from a default
 * value
 */
#define FLOM_RC_NULL_OBJECT                     -4
/**
 * A specified option is not valid for method and/or object status
 */
#define FLOM_RC_INVALID_OPTION                  -5
/**
 * A corrupted object has been discovered
 */
#define FLOM_RC_OBJ_CORRUPTED                   -6
/**
 * A parameter passed to a function is OUT OF RANGE
 */
#define FLOM_RC_OUT_OF_RANGE                    -7
/**
 * The number of chars of the prefix of the message 
 */
#define FLOM_RC_INVALID_PREFIX_SIZE             -8
/**
 * The process has been stopped to avoid a buffer overflow
 */
#define FLOM_RC_BUFFER_OVERFLOW                 -9
/**
 * The length of the message differs from prefix 
 */
#define FLOM_RC_INVALID_MSG_LENGTH             -10
/**
 * The XML message is malformed and can not be processed
 */
#define FLOM_RC_INVALID_PROPERTY_VALUE         -11
/**
 * The container is full and can NOT store more elements
 */
#define FLOM_RC_CONTAINER_FULL                 -12
/**
 * A routine has been invoked in an improper context
 */
#define FLOM_RC_PROTOCOL_ERROR                 -13



/**
 * "accept" function error
 */
#define FLOM_RC_ACCEPT_ERROR                        -100
/**
 * "bind" function error
 */
#define FLOM_RC_BIND_ERROR                          -101
/**
 * "chdir" function error
 */
#define FLOM_RC_CHDIR_ERROR                         -102
/**
 * "close" function error
 */
#define FLOM_RC_CLOSE_ERROR                         -103
/**
 * "connect" function error
 */
#define FLOM_RC_CONNECT_ERROR                       -104
/**
 * "execvp" function error
 */
#define FLOM_RC_EXECVP_ERROR                        -105
/**
 * "fork" function error
 */
#define FLOM_RC_FORK_ERROR                          -106
/**
 * "getsockopt" function error
 */
#define FLOM_RC_GETSOCKOPT_ERROR                    -107
/**
 * "g_markup_parse_context_new_error" function error
 */
#define FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR    -108
/**
 * "g_markup_parse_context_parse" function error
 */
#define FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR  -109
/**
 * "g_strdup" function error
 */
#define FLOM_RC_G_STRDUP_ERROR                      -110
/**
 * "g_thread_create" function error
 */
#define FLOM_RC_G_THREAD_CREATE_ERROR               -111
/**
 * "listen" function error
 */
#define FLOM_RC_LISTEN_ERROR                        -112
/**
 * "malloc"/"g_malloc" function error
 */
#define FLOM_RC_MALLOC_ERROR                        -113
/**
 * "pipe" function error
 */
#define FLOM_RC_PIPE_ERROR                          -114
/**
 * "poll" function error
 */
#define FLOM_RC_POLL_ERROR                          -115
/**
 * "read" function error
 */
#define FLOM_RC_READ_ERROR                          -116
/**
 * "recv" function error
 */
#define FLOM_RC_RECV_ERROR                          -117
/**
 * "signal" function error
 */
#define FLOM_RC_SIGNAL_ERROR                        -118
/**
 * "send" function error
 */
#define FLOM_RC_SEND_ERROR                          -119
/**
 * "setsid" function error
 */
#define FLOM_RC_SETSID_ERROR                        -120
/**
 * "socket" function error
 */
#define FLOM_RC_SOCKET_ERROR                        -121
/**
 * "unlink" function error
 */
#define FLOM_RC_UNLINK_ERROR                        -122
/**
 * "wait" function error
 */
#define FLOM_RC_WAIT_ERROR                          -123
/**
 * "write" function error
 */
#define FLOM_RC_WRITE_ERROR                         -124



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



      /**
       * <B>PUBLIC METHOD</B><BR>
       * Retrieve the description associated to a return/reason code
       * @param ret_cod IN return/reason code of the desired description
       * @return a const string containing a description of reason code
       */
      const char *flom_strerror(int ret_cod);
      
      

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_ERRORS_H */


