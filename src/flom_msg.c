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



GMarkupParser flom_msg_parser = {
    flom_msg_deserialize_start_element,
    flom_msg_deserialize_end_element,
    flom_msg_deserialize_text,
    NULL, NULL };



int flom_msg_retrieve(int fd,
                      char *buf, size_t buf_size,
                      ssize_t *read_bytes)
{
    enum Exception { RECV_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_retrieve\n"));
    TRY {
        if (0 > (*read_bytes = recv(fd, buf, buf_size, 0)))
            THROW(RECV_ERROR);
        
        FLOM_TRACE(("flom_msg_retrieve: fd=%d returned "
                    SSIZE_T_FORMAT " bytes\n", fd, *read_bytes));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case RECV_ERROR:
                ret_cod = FLOM_RC_RECV_ERROR;
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
                    case FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.lock_8.resource.name) {
                            g_free(msg->body.lock_8.resource.name);
                            msg->body.lock_8.resource.name = NULL;
                        }
                        break;
                    case 2*FLOM_MSG_STEP_INCR: /* nothing to release */
                    case 3*FLOM_MSG_STEP_INCR:
                        break;
                    default:
                        THROW(INVALID_STEP_LOCK);
                }
                break;
            case FLOM_MSG_VERB_UNLOCK:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.unlock_8.resource.name) {
                            g_free(msg->body.unlock_8.resource.name);
                            msg->body.unlock_8.resource.name = NULL;
                        }
                        break;
                    case 2*FLOM_MSG_STEP_INCR: /* nothing to release */
                        break;
                    default:
                        THROW(INVALID_STEP_UNLOCK);
                }
                break;
            case FLOM_MSG_VERB_PING: /* nothing to release */
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR: /* nothing to release */
                    case 2*FLOM_MSG_STEP_INCR:
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
    
    FLOM_TRACE(("flom_msg_serialize\n"));
    TRY {
        /* <xml ... > */
        used_chars = snprintf(buffer, free_chars,
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
        FLOM_TRACE(("flom_msg_trace: state=%d,header[level=%d,pvs.verb=%d,"
                    "pvs.step=%d]\n", msg->state,
                    msg->header.level, msg->header.pvs.verb,
                    msg->header.pvs.step));
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_NULL: /* null verb, skipping... */
                break;
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
                            msg->body.lock_8.resource.name != NULL ?
                            msg->body.lock_8.resource.name : "",
                            msg->body.lock_8.resource.type,
                            msg->body.lock_8.resource.wait));
                break;
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
                            "name='%s']]\n",
                            msg->body.unlock_8.resource.name != NULL ?
                            msg->body.unlock_8.resource.name : ""));
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
                         struct flom_msg_s *msg,
                         GMarkupParseContext *gmpc)
{
    enum Exception { G_MARKUP_PARSE_CONTEXT_PARSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_deserialize\n"));
    TRY {
        FLOM_TRACE(("flom_msg_deserialize: deserializing message |%*.*s|\n",
                    buffer_len, buffer_len, buffer));
        
        if (FALSE == g_markup_parse_context_parse(
                gmpc, buffer, buffer_len, NULL))
            THROW(G_MARKUP_PARSE_CONTEXT_PARSE_ERROR);
        
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
    enum Exception { ALREADY_INVALID
                     , G_STRDUP_ERROR
                     , INVALID_PROPERTY1
                     , INVALID_PROPERTY2
                     , TAG_TYPE_ERROR
                     , NONE } excp;
    
    enum {dummy_tag, msg_tag, resource_tag} tag_type = dummy_tag;
    /* deserialized message */
    struct flom_msg_s *msg = (struct flom_msg_s *)user_data;
    
    const gchar **name_cursor = attribute_names;
    const gchar **value_cursor = attribute_values;

    FLOM_TRACE(("flom_msg_deserialize_start_element\n"));
    TRY {
        if (FLOM_MSG_STATE_INVALID == msg->state) {
            FLOM_TRACE(("flom_msg_deserialize_start_element: message already "
                        "marked as invalid, leaving...\n"));
            THROW(ALREADY_INVALID);
        } else if (FLOM_MSG_STATE_INITIALIZED == msg->state)
            msg->state = FLOM_MSG_STATE_PARSING;
    
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
                    /* check if this tag is OK for the current message */
                    if ((FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         FLOM_MSG_STEP_INCR == msg->header.pvs.step) ||
                        (FLOM_MSG_VERB_UNLOCK == msg->header.pvs.verb &&
                         FLOM_MSG_STEP_INCR == msg->header.pvs.step)) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_NAME)) {
                            gchar *tmp = g_strdup(*value_cursor);
                            if (NULL == tmp) {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: unable to duplicate "
                                            "*name_cursor\n"));
                                THROW(G_STRDUP_ERROR);
                            }
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.name = tmp;
                            else
                                msg->body.unlock_8.resource.name = tmp;
                        } else if (!strcmp(*name_cursor, FLOM_MSG_PROP_TYPE)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.type =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY1);
                            }
                        } else if (!strcmp(*name_cursor, FLOM_MSG_PROP_WAIT)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.wait =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY2);
                            }
                        }
                    }
                    break;
                default:
                    FLOM_TRACE(("flom_msg_deserialize_start_element: ERROR, "
                                "tag_type=%d\n", tag_type));
                    THROW(TAG_TYPE_ERROR);
            }
            name_cursor++;
            value_cursor++;
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case ALREADY_INVALID:
                break;
            case G_STRDUP_ERROR:
            case INVALID_PROPERTY1:
            case INVALID_PROPERTY2:
            case TAG_TYPE_ERROR:
                msg->state = FLOM_MSG_STATE_INVALID;
                break;
            case NONE:
                break;
            default:
                msg->state = FLOM_MSG_STATE_INVALID;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_deserialize_start_element/excp=%d/"
                "msg->state=%d\n", excp, msg->state));
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
    enum Exception { ALREADY_INVALID
                     , ALREADY_READY
                     , ONLY_INITIALIZED
                     , INTERNAL_ERROR
                     , NONE } excp;
    /* deserialized message */
    struct flom_msg_s *msg = (struct flom_msg_s *)user_data;
    
    FLOM_TRACE(("flom_msg_deserialize_end_element\n"));
    TRY {
        FLOM_TRACE(("flom_msg_deserialize_end_element: element_name='%s'\n",
                    element_name));

        switch (msg->state) {
            case FLOM_MSG_STATE_INVALID:
                FLOM_TRACE(("flom_msg_deserialize_start_element: message "
                            "already marked as invalid, leaving...\n"));
                THROW(ALREADY_INVALID);
                break;
            case FLOM_MSG_STATE_READY:
                FLOM_TRACE(("flom_msg_deserialize_start_element: message is "
                            "already in ready state, skipping...\n"));
                THROW(ALREADY_READY);
                break;
            case FLOM_MSG_STATE_INITIALIZED:
                FLOM_TRACE(("flom_msg_deserialize_start_element: message is "
                            "in only initialized, this is an error\n"));
                THROW(ONLY_INITIALIZED);
                break;
            case FLOM_MSG_STATE_PARSING: /* OK, this is the right state */
                break;
            default: /* this is an internal code error */
                FLOM_TRACE(("flom_msg_deserialize_start_element: internal "
                            "error, this point should never be reached\n"));
                THROW(INTERNAL_ERROR);
                break;
        } /* switch (msg->state) */
            
        if (!strcmp(element_name, FLOM_MSG_TAG_MSG)) {
            msg->state = FLOM_MSG_STATE_READY;
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case ALREADY_INVALID:
            case ALREADY_READY:
                break;
            case ONLY_INITIALIZED:
            case INTERNAL_ERROR:
                msg->state = FLOM_MSG_STATE_INVALID;
                break;
            case NONE:
                break;
            default:
                msg->state = FLOM_MSG_STATE_INVALID;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_deserialize_end_element/excp=%d/"
                "msg->state=%d\n", excp, msg->state));
}
