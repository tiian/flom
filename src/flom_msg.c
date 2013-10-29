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
#include "flom_msg.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_MSG



const gchar *FLOM_MSG_HEADER              = (gchar *)"<?xml";
const gchar *FLOM_MSG_PROP_LEVEL          = (gchar *)"level";
const gchar *FLOM_MSG_PROP_NAME           = (gchar *)"name";
const gchar *FLOM_MSG_PROP_RC             = (gchar *)"rc";
const gchar *FLOM_MSG_PROP_STEP           = (gchar *)"step";
const gchar *FLOM_MSG_PROP_TYPE           = (gchar *)"type";
const gchar *FLOM_MSG_PROP_VERB           = (gchar *)"verb"; 
const gchar *FLOM_MSG_PROP_WAIT           = (gchar *)"wait";
const gchar *FLOM_MSG_TAG_ANSWER          = (gchar *)"answer";
const gchar *FLOM_MSG_TAG_MSG             = (gchar *)"msg";
const gchar *FLOM_MSG_TAG_RESOURCE        = (gchar *)"resource";



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



int flom_msg_send(int fd, const char *buf, size_t buf_size)
{
    enum Exception { GETSOCKOPT_ERROR
                     , CONNECTION_CLOSED
                     , SEND_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_send\n"));
    TRY {
        ssize_t wrote_bytes;
        int optval;
        socklen_t optlen = sizeof(optval);

        if (0 != getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen))
            THROW(GETSOCKOPT_ERROR);
        FLOM_TRACE(("flom_msg_send: so_error=%d (EPIPE=%d, ECONNRESET=%d)\n",
                    optval, EPIPE, ECONNRESET));
        if (EPIPE == optval || ECONNRESET == optval) {
            int rc = 0;
            rc = shutdown(fd, SHUT_RDWR);
            FLOM_TRACE(("flom_msg_send: socket with fd=%d was shutdown "
                        "(rc=%d,errno=%d)\n", fd, rc, errno));
            THROW(CONNECTION_CLOSED);
        }
        FLOM_TRACE(("flom_msg_send: sending " SIZE_T_FORMAT
                    " bytes to the server (fd=%d)...\n", buf_size, fd));
        wrote_bytes = send(fd, buf, buf_size, 0);
        if (buf_size != wrote_bytes) {
            FLOM_TRACE(("flom_msg_send: sent " SSIZE_T_FORMAT
                        " bytes instead of " SIZE_T_FORMAT " to the server\n",
                        wrote_bytes, buf_size));
            THROW(SEND_ERROR);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_GETSOCKOPT_ERROR;
                break;
            case CONNECTION_CLOSED:
                ret_cod = FLOM_RC_CONNECTION_CLOSED;
                break;
            case SEND_ERROR:
                ret_cod = FLOM_RC_SEND_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_send/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_free(struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP_LOCK
                     , INVALID_STEP_UNLOCK
                     , INVALID_STEP_PING
                     , INVALID_VERB
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_free\n"));
    TRY {
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_NULL: /* nothing to release */
                break;
            case FLOM_MSG_VERB_LOCK:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (NULL != msg->body.lock_8.resource.name) {
                            g_free(msg->body.lock_8.resource.name);
                            msg->body.lock_8.resource.name = NULL;
                        }
                        break;
                    case 16: /* nothing to release */
                    case 24:
                        break;
                    default:
                        THROW(INVALID_STEP_LOCK);
                }
                break;
            case FLOM_MSG_VERB_UNLOCK:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (NULL != msg->body.unlock_8.resource.name) {
                            g_free(msg->body.unlock_8.resource.name);
                            msg->body.unlock_8.resource.name = NULL;
                        }
                        break;
                    case 16: /* nothing to release */
                        break;
                    default:
                        THROW(INVALID_STEP_UNLOCK);
                }
                break;
            case FLOM_MSG_VERB_PING: /* nothing to release */
                switch (msg->header.pvs.step) {
                    case 8: /* nothing to release */
                    case 16: 
                        break;
                    default:
                        THROW(INVALID_STEP_PING);
                }
                break;
            default:
                THROW(INVALID_VERB);
        } /* switch (msg->header.pvs.verb) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_STEP_LOCK:
            case INVALID_STEP_UNLOCK:
            case INVALID_STEP_PING:
            case INVALID_VERB:
                FLOM_TRACE(("flom_msg_free: verb=%d, step=%d\n",
                            msg->header.pvs.verb, msg->header.pvs.step));
                ret_cod = FLOM_RC_INVALID_PROPERTY_VALUE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_free/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize(const struct flom_msg_s *msg,
                       char *buffer, size_t buffer_len,
                       size_t *msg_len)
{
    enum Exception { BUFFER_TOO_SHORT1
                     , BUFFER_TOO_SHORT2
                     , SERIALIZE_LOCK_8_ERROR
                     , SERIALIZE_LOCK_16_ERROR
                     , SERIALIZE_LOCK_24_ERROR
                     , INVALID_LOCK_STEP
                     , SERIALIZE_UNLOCK_8_ERROR
                     , INVALID_UNLOCK_STEP
                     , SERIALIZE_PING_8_ERROR
                     , SERIALIZE_PING_16_ERROR
                     , INVALID_PING_STEP
                     , INVALID_VERB
                     , BUFFER_TOO_SHORT3
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int used_chars = 0;
    size_t free_chars = buffer_len, offset = 0;
    char prefix[FLOM_MSG_PREFIX_DIGITS + 1];
    
    FLOM_TRACE(("flom_msg_serialize\n"));
    TRY {
        /* reserving space for prefix size */
        free_chars -= FLOM_MSG_PREFIX_DIGITS;
        offset += FLOM_MSG_PREFIX_DIGITS;
        /* <xml ... > */
        used_chars = snprintf(buffer + offset, free_chars,
                              "%s version=\"1.0\" encoding=\"UTF-8\" ?>",
                              FLOM_MSG_HEADER);
        if (used_chars >= free_chars)
            THROW(BUFFER_TOO_SHORT1);
        /* <msg ... > */
        free_chars -= used_chars;
        offset += used_chars;
        used_chars = snprintf(buffer + offset, free_chars,
                              "<%s %s=\"%d\" %s=\"%d\" %s=\"%d\">",
                              FLOM_MSG_TAG_MSG,
                              FLOM_MSG_PROP_LEVEL,
                              msg->header.level,
                              FLOM_MSG_PROP_VERB,
                              msg->header.pvs.verb,
                              FLOM_MSG_PROP_STEP,
                              msg->header.pvs.step);
        if (used_chars >= free_chars)
            THROW(BUFFER_TOO_SHORT2);
        free_chars -= used_chars;
        offset += used_chars;

        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_lock_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_8_ERROR);
                        break;
                    case 16:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_lock_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_16_ERROR);
                        break;
                    case 24:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_lock_24(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_24_ERROR);
                        break;
                    default:
                        THROW(INVALID_LOCK_STEP);
                }
                break;
            case FLOM_MSG_VERB_UNLOCK:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_unlock_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_UNLOCK_8_ERROR);
                        break;
                    default:
                        THROW(INVALID_UNLOCK_STEP);
                }
                break;
            case FLOM_MSG_VERB_PING:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_ping_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_PING_8_ERROR);
                        break;
                    case 16:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_ping_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_PING_16_ERROR);
                        break;
                    default:
                        THROW(INVALID_PING_STEP);
                }
                break;
            default:
                THROW(INVALID_VERB);
        }
        /* </msg> */
        used_chars = snprintf(buffer + offset, free_chars,
                              "</%s>", FLOM_MSG_TAG_MSG);
        if (used_chars >= free_chars)
            THROW(BUFFER_TOO_SHORT3);
        free_chars -= used_chars;
        offset += used_chars;

        /* writing prefix size at buffer head */
        snprintf(prefix, sizeof(prefix), "%*.*d",
                 FLOM_MSG_PREFIX_DIGITS,
                 FLOM_MSG_PREFIX_DIGITS,
                 (int)(offset - FLOM_MSG_PREFIX_DIGITS));
        strncpy(buffer, prefix, FLOM_MSG_PREFIX_DIGITS);
        
        *msg_len = offset;
        
        FLOM_TRACE(("flom_msg_serialize: serialized message is |%*.*s|\n",
                    *msg_len, *msg_len, buffer));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT1:
            case BUFFER_TOO_SHORT2:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case SERIALIZE_LOCK_8_ERROR:
            case SERIALIZE_LOCK_16_ERROR:
            case SERIALIZE_LOCK_24_ERROR:
            case SERIALIZE_UNLOCK_8_ERROR:
            case SERIALIZE_PING_8_ERROR:
            case SERIALIZE_PING_16_ERROR:
                break;
            case INVALID_LOCK_STEP:
            case INVALID_UNLOCK_STEP:
            case INVALID_PING_STEP:
            case INVALID_VERB:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case BUFFER_TOO_SHORT3:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_lock_8(const struct flom_msg_s *msg,
                              char *buffer,
                              size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_8\n"));
    TRY {
        int used_chars;
        
        /* <resource> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\"/>",
                              FLOM_MSG_TAG_RESOURCE,
                              FLOM_MSG_PROP_NAME,
                              msg->body.lock_8.resource.name,
                              FLOM_MSG_PROP_TYPE,
                              msg->body.lock_8.resource.type,
                              FLOM_MSG_PROP_WAIT,
                              msg->body.lock_8.resource.wait);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_lock_8/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_lock_16(const struct flom_msg_s *msg,
                               char *buffer,
                               size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_16\n"));
    TRY {
        int used_chars;
        
        /* <answer> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%d\"/>",
                              FLOM_MSG_TAG_ANSWER,
                              FLOM_MSG_PROP_RC,
                              msg->body.lock_16.answer.rc);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_lock_16/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_lock_24(const struct flom_msg_s *msg,
                               char *buffer,
                               size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_24\n"));
    TRY {
        int used_chars;
        
        /* <answer> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%d\"/>",
                              FLOM_MSG_TAG_ANSWER,
                              FLOM_MSG_PROP_RC,
                              msg->body.lock_24.answer.rc);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_lock_24/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_unlock_8(const struct flom_msg_s *msg,
                                char *buffer,
                                size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_unlock_8\n"));
    TRY {
        int used_chars;
        
        /* <resource> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\"/>",
                              FLOM_MSG_TAG_RESOURCE,
                              FLOM_MSG_PROP_NAME,
                              msg->body.unlock_8.resource.name);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_unlock_8/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_ping_8(const struct flom_msg_s *msg,
                              char *buffer,
                              size_t *offset, size_t *free_chars)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_ping_8\n"));
    TRY {
        /* nothing to add */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_ping_8/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_ping_16(const struct flom_msg_s *msg,
                               char *buffer,
                               size_t *offset, size_t *free_chars)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_ping_16\n"));
    TRY {
        /* nothing to add */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_serialize_ping_16/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_trace(const struct flom_msg_s *msg)
{
    enum Exception { TRACE_LOCK_ERROR
                     , TRACE_UNLOCK_ERROR
                     , TRACE_PING_ERROR
                     , INVALID_VERB
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace\n"));
    TRY {
        FLOM_TRACE(("flom_msg_trace: header[level=%d,pvs.verb=%d,"
                    "pvs.step=%d]\n",
                    msg->header.level, msg->header.pvs.verb,
                    msg->header.pvs.step));
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK: /* lock */
                if (FLOM_RC_OK != (ret_cod = flom_msg_trace_lock(msg)))
                    THROW(TRACE_LOCK_ERROR);
                break;
            case FLOM_MSG_VERB_UNLOCK: /* unlock */
                if (FLOM_RC_OK != (ret_cod = flom_msg_trace_unlock(msg)))
                    THROW(TRACE_UNLOCK_ERROR);
                break;
            case FLOM_MSG_VERB_PING: /* ping */
                if (FLOM_RC_OK != (ret_cod = flom_msg_trace_ping(msg)))
                    THROW(TRACE_PING_ERROR);
                break;
            default:
                THROW(INVALID_VERB);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TRACE_LOCK_ERROR:
            case TRACE_UNLOCK_ERROR:
            case TRACE_PING_ERROR:
                break;
            case INVALID_VERB:
                ret_cod = FLOM_RC_INVALID_PROPERTY_VALUE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_trace/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
    


int flom_msg_trace_lock(const struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace_lock\n"));
    TRY {
        switch (msg->header.pvs.step) {
            case 8:
                FLOM_TRACE(("flom_msg_trace_lock: body[resource["
                            "name='%s',type=%d,wait=%d]]\n",
                            msg->body.lock_8.resource.name,
                            msg->body.lock_8.resource.type,
                            msg->body.lock_8.resource.wait));
            case 16:
                FLOM_TRACE(("flom_msg_trace_lock: body[answer["
                            "rc=%d]]\n",
                            msg->body.lock_16.answer.rc));
                break;
            case 24:
                FLOM_TRACE(("flom_msg_trace_lock: body[answer["
                            "rc=%d]]\n",
                            msg->body.lock_16.answer.rc));
                break;
            default:
                THROW(INVALID_STEP);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_STEP:
                ret_cod = FLOM_RC_INVALID_PROPERTY_VALUE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_trace_lock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}


    
int flom_msg_trace_unlock(const struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace_unlock\n"));
    TRY {
        switch (msg->header.pvs.step) {
            case 8:
                FLOM_TRACE(("flom_msg_trace_unlock: body[resource["
                            "name='%s',type=%d,wait=%d]]\n",
                            msg->body.unlock_8.resource.name));
            default:
                THROW(INVALID_STEP);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_STEP:
                ret_cod = FLOM_RC_INVALID_PROPERTY_VALUE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_trace_unlock/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}


    
int flom_msg_trace_ping(const struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace_ping\n"));
    TRY {
        switch (msg->header.pvs.step) {
            case 8:
            case 16:
                FLOM_TRACE(("flom_msg_trace_ping: body[null]\n"));
                FLOM_TRACE(("flom_msg_trace_ping: body[null]\n"));
                break;
            default:
                THROW(INVALID_STEP);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_STEP:
                ret_cod = FLOM_RC_INVALID_PROPERTY_VALUE;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_trace_ping/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}


    
int flom_msg_deserialize(char *buffer, size_t buffer_len,
                         struct flom_msg_s *msg)
{
    enum Exception { G_MARKUP_PARSE_CONTEXT_PARSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_deserialize\n"));
    TRY {
        GMarkupParser parser = {
            flom_msg_deserialize_start_element,
            flom_msg_deserialize_end_element,
            flom_msg_deserialize_text,
            NULL, NULL };
        GMarkupParseContext *context = g_markup_parse_context_new (
            &parser, 0, (gpointer)msg, NULL);
        
        FLOM_TRACE(("flom_msg_deserialize: deserializing message |%*.*s|\n",
                    buffer_len, buffer_len, buffer));
        
        if (FALSE == g_markup_parse_context_parse(
                context, buffer, buffer_len, NULL))
            THROW(G_MARKUP_PARSE_CONTEXT_PARSE_ERROR);
        g_markup_parse_context_free (context);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_MARKUP_PARSE_CONTEXT_PARSE_ERROR:
                ret_cod = FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_deserialize/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_msg_deserialize_start_element(
    GMarkupParseContext *context,
    const gchar         *element_name,
    const gchar        **attribute_names,
    const gchar        **attribute_values,
    gpointer             user_data,
    GError             **error)
{
    enum {dummy_tag, msg_tag, resource_tag} tag_type = dummy_tag;
    /* deserialized message */
    struct flom_msg_s *msg = (struct flom_msg_s *)user_data;
    
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;

    FLOM_TRACE(("flom_msg_deserialize_start_element: element_name='%s'\n",
                element_name));
    if (!strcmp(element_name, FLOM_MSG_TAG_MSG))
        tag_type = msg_tag;
    else if (!strcmp(element_name, FLOM_MSG_TAG_RESOURCE))
        tag_type = resource_tag;
    while (*name_cursor) {
        FLOM_TRACE(("flom_msg_deserialize_start_element: name_cursor='%s' "
                    "value_cursor='%s'\n", *name_cursor, *value_cursor));
        switch (tag_type) {
            case msg_tag:
                if (!strcmp(*name_cursor, FLOM_MSG_PROP_LEVEL))
                    msg->header.level = strtol(*value_cursor, NULL, 10);
                else if (!strcmp(*name_cursor, FLOM_MSG_PROP_VERB))
                    msg->header.pvs.verb = strtol(*value_cursor, NULL, 10);
                else if (!strcmp(*name_cursor, FLOM_MSG_PROP_STEP))
                    msg->header.pvs.step = strtol(*value_cursor, NULL, 10);
                break;
            case resource_tag:
                if (!strcmp(*name_cursor, FLOM_MSG_PROP_NAME))
                    ; /* @@@ implement me!!! */
                break;
            default:
                FLOM_TRACE(("flom_msg_deserialize_start_element: ERROR, "
                            "tag_type=%d\n", tag_type));
        }
        name_cursor++;
        value_cursor++;
    }
}



void flom_msg_deserialize_text(GMarkupParseContext *context,
                               const gchar         *text,
                               gsize                text_len,
                               gpointer             user_data,
                               GError             **error)
{
    FLOM_TRACE(("flom_msg_deserialize_text: text='%*s'\n",
                text_len, text));
}



void flom_msg_deserialize_end_element(GMarkupParseContext *context,
                                      const gchar         *element_name,
                                      gpointer             user_data,
                                      GError             **error)
{
    FLOM_TRACE(("flom_msg_deserialize_end_element: element_name='%s'\n",
                element_name));
}
