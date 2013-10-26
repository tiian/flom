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
#include <config.h>



#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_errors.h"
#include "flom_parser.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_PARSER



int flom_msg_retrieve(int fd,
                      char *buf, size_t buf_size,
                      ssize_t *read_bytes)
{
    enum Exception { RECV_ERROR1
                     , CONNECTION_CLOSED
                     , INVALID_PREFIX_SIZE
                     , BUFFER_OVERFLOW
                     , RECV_ERROR2
                     , INVALID_MSG_LENGTH
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_retrieve\n"));
    TRY {
        char prefix[FLOM_MSG_PREFIX_DIGITS+1];
        ssize_t to_read = 0;

        /* read the prefix to determine message size */
        if (0 > (*read_bytes = recv(
                     fd, prefix, FLOM_MSG_PREFIX_DIGITS, 0))) {
            THROW(RECV_ERROR1);
        } else if (*read_bytes == 0) {
            THROW(CONNECTION_CLOSED);
        } else if (*read_bytes != FLOM_MSG_PREFIX_DIGITS) {
            /* retrieve message size */
            FLOM_TRACE(("flom_msg_retrieve: peer sent "
                        SSIZE_T_FORMAT " bytes, expected %d bytes for "
                        "message prefix\n", read_bytes,
                        FLOM_MSG_PREFIX_DIGITS));
            THROW(INVALID_PREFIX_SIZE);
        } else {
            prefix[FLOM_MSG_PREFIX_DIGITS] = '\0';
            to_read = strtol(prefix, NULL, 10);
            FLOM_TRACE(("flom_msg_retrieve: message prefix "
                        "is '%s' (" SSIZE_T_FORMAT ")\n", prefix, to_read));
        }

        if (to_read > buf_size)
            THROW(BUFFER_OVERFLOW);
        
        if (0 > (*read_bytes = recv(fd, buf, to_read, 0)))
            THROW(RECV_ERROR2);
        
        FLOM_TRACE(("flom_msg_retrieve: fd = %d returned "
                    SSIZE_T_FORMAT " bytes\n", fd, *read_bytes));
        if (to_read != *read_bytes) {
            FLOM_TRACE(("flom_msg_retrieve: expected " SSIZE_T_FORMAT
                        " bytes, received " SSIZE_T_FORMAT " bytes\n",
                        to_read, *read_bytes));
            THROW(INVALID_MSG_LENGTH);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case RECV_ERROR1:
                ret_cod = FLOM_RC_RECV_ERROR;
                break;
            case CONNECTION_CLOSED:
                ret_cod = FLOM_RC_CONNECTION_CLOSED;
                break;
            case INVALID_PREFIX_SIZE:
                ret_cod = FLOM_RC_INVALID_PREFIX_SIZE;
                break;
            case BUFFER_OVERFLOW:
                ret_cod = FLOM_RC_BUFFER_OVERFLOW;
                break;
            case RECV_ERROR2:
                ret_cod = FLOM_RC_RECV_ERROR;
                break;
            case INVALID_MSG_LENGTH:
                ret_cod = FLOM_RC_INVALID_MSG_LENGTH;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_retrieve/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
