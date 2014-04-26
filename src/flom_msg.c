/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_STRING_H
/* strcasestr is not POSIX standard and needs GNU extensions... */
# define _GNU_SOURCE
# include <string.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_errors.h"
#include "flom_msg.h"
#include "flom_rsrc.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_MSG



const gchar *FLOM_MSG_HEADER              = (gchar *)"<?xml";
const gchar *FLOM_MSG_PROP_ADDRESS        = (gchar *)"address";
const gchar *FLOM_MSG_PROP_LEVEL          = (gchar *)"level";
const gchar *FLOM_MSG_PROP_NAME           = (gchar *)"name";
const gchar *FLOM_MSG_PROP_QUANTITY       = (gchar *)"quantity";
const gchar *FLOM_MSG_PROP_RC             = (gchar *)"rc";
const gchar *FLOM_MSG_PROP_STEP           = (gchar *)"step";
const gchar *FLOM_MSG_PROP_MODE           = (gchar *)"mode";
const gchar *FLOM_MSG_PROP_PORT           = (gchar *)"port";
const gchar *FLOM_MSG_PROP_VERB           = (gchar *)"verb"; 
const gchar *FLOM_MSG_PROP_WAIT           = (gchar *)"wait";
const gchar *FLOM_MSG_TAG_ANSWER          = (gchar *)"answer";
const gchar *FLOM_MSG_TAG_MSG             = (gchar *)"msg";
const gchar *FLOM_MSG_TAG_NETWORK         = (gchar *)"network";
const gchar *FLOM_MSG_TAG_RESOURCE        = (gchar *)"resource";



GMarkupParser flom_msg_parser = {
    flom_msg_deserialize_start_element,
    flom_msg_deserialize_end_element,
    flom_msg_deserialize_text,
    NULL, NULL };



flom_lock_mode_t flom_lock_mode_retrieve(const gchar *text)
{
    /* parsing is case sensitive only on GNU systems */
    char *p = NULL;
    
    FLOM_TRACE(("flom_bool_value_retrieve: '%s'\n", text));
    /* check if 'NullLock', 'NL' - any case - is in the text */
    if (NULL != (p = STRCASESTR(text, "NullLock")) ||
        NULL != (p = STRCASESTR(text, "NL"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Null Lock' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_NL;
    /* check if 'ConcurrentRead', 'CR' - any case - is in the text */
    } else if (NULL != (p = STRCASESTR(text, "ConcurrentRead")) ||
               NULL != (p = STRCASESTR(text, "CR"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Concurrent Read' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_CR;
    /* check if 'ConcurrentWrite', 'CW' - any case - is in the text */
    } else if (NULL != (p = STRCASESTR(text, "ConcurrentWrite")) ||
               NULL != (p = STRCASESTR(text, "CW"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Concurrent Write' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_CW;
    /* check if 'ProtectedRead', 'PR' - any case - is in the text */
    } else if (NULL != (p = STRCASESTR(text, "ProtectedRead")) ||
               NULL != (p = STRCASESTR(text, "PR"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Protected Read' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_PR;
    /* check if 'ProtectedWrite', 'PW' - any case - is in the text */
    } else if (NULL != (p = STRCASESTR(text, "ProtectedWrite")) ||
               NULL != (p = STRCASESTR(text, "PW"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Protected Write' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_PW;
    /* check if 'Exclusive', 'EX' - any case - is in the text */
    } else if (NULL != (p = STRCASESTR(text, "Exclusive")) ||
               NULL != (p = STRCASESTR(text, "EX"))) {
        FLOM_TRACE(("flom_bool_value_retrieve: found 'Exclusive' mode "
                    "here: '%s'\n", p));
        return FLOM_LOCK_MODE_EX;
    }
    return FLOM_LOCK_MODE_INVALID;
}



int flom_msg_retrieve(int fd, int type,
                      char *buf, size_t buf_size,
                      ssize_t *read_bytes,
                      int timeout,
                      struct sockaddr *src_addr, socklen_t *addrlen)
{
    enum Exception { POLL_ERROR
                     , NETWORK_TIMEOUT
                     , INTERNAL_ERROR
                     , INVALID_SOCKET_TYPE
                     , RECV_ERROR
                     , RECVFROM_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_retrieve\n"));
    TRY {
        if (timeout >= 0) {
            struct pollfd fds[1];
            int rc;
            /* use poll to check the filedescriptor for a limited amount of
               time */
            fds[0].fd = fd;
            fds[0].events = POLLIN;
            fds[0].revents = 0;
            rc = poll(fds, 1, timeout);
            switch (rc) {
                case -1: /* error in poll function */
                    THROW(POLL_ERROR);
                    break;
                case 0: /* timeout, return! */
                    THROW(NETWORK_TIMEOUT);
                    break;
                case 1: /* data arrived, go on... */
                    break;
                default: /* unexpected result, internal error! */
                    THROW(INTERNAL_ERROR);
            } /* switch (rc) */
        } /* if (timeout >= 0) */

        switch (type) {
            case SOCK_STREAM:
                if (0 > (*read_bytes = recv(fd, buf, buf_size, 0)))
                    THROW(RECV_ERROR);
                break;
            case SOCK_DGRAM:
                if (0 > (*read_bytes = recvfrom(
                             fd, buf, buf_size, 0,
                             (struct sockaddr *)src_addr, addrlen)))
                    THROW(RECVFROM_ERROR);
                FLOM_TRACE_HEX_DATA("flom_msg_retrieve: from ",
                                    (void *)src_addr, *addrlen);        
                break;
            default:
                THROW(INVALID_SOCKET_TYPE);
        } /* switch (type) */
        
        FLOM_TRACE(("flom_msg_retrieve: fd=%d returned "
                    SSIZE_T_FORMAT " bytes '%*.*s'\n", fd, *read_bytes,
                    *read_bytes, *read_bytes, buf));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case NETWORK_TIMEOUT:
                ret_cod = FLOM_RC_NETWORK_TIMEOUT;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case INVALID_SOCKET_TYPE:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case RECV_ERROR:
                ret_cod = FLOM_RC_RECV_ERROR;
                break;
            case RECVFROM_ERROR:
                ret_cod = FLOM_RC_RECVFROM_ERROR;
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
    
    FLOM_TRACE(("flom_msg_send: fd=%d\n", fd));
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
                    " bytes (fd=%d) '%*.*s'...\n", buf_size, fd,
                    buf_size, buf_size, buf));
        wrote_bytes = send(fd, buf, buf_size, MSG_NOSIGNAL);
        if (buf_size != wrote_bytes) {
            FLOM_TRACE(("flom_msg_send: sent " SSIZE_T_FORMAT
                        " bytes instead of " SIZE_T_FORMAT "\n",
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
                     , INVALID_STEP_DISCOVER
                     , INVALID_VERB
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_free\n"));
    TRY {
        msg->state = FLOM_MSG_STATE_INVALID;
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
            case FLOM_MSG_VERB_DISCOVER: /* nothing to release */
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR: /* nothing to release */
                        break;
                    case 2*FLOM_MSG_STEP_INCR: /* release address */
                        if (NULL != msg->body.discover_16.network.address) {
                            g_free(msg->body.discover_16.network.address);
                            msg->body.discover_16.network.address = NULL;
                        }
                        break;
                    default:
                        THROW(INVALID_STEP_DISCOVER);
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
            case INVALID_STEP_DISCOVER:
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



int flom_msg_check_protocol(const struct flom_msg_s *msg, int client)
{
    int ret_cod = FALSE;
    FLOM_TRACE(("flom_msg_check_protocol\n"));
    switch (msg->header.pvs.verb) {
        case FLOM_MSG_VERB_LOCK:
            switch (msg->header.pvs.step) {
                case FLOM_MSG_STEP_INCR:
                    ret_cod = client ? TRUE : FALSE;
                    break;
                case 2*FLOM_MSG_STEP_INCR:
                case 3*FLOM_MSG_STEP_INCR:
                    ret_cod = client ? FALSE : TRUE;
                    break;
                default:
                    break;
            } /* switch (msg->header.pvs.step) */
            break;
        case FLOM_MSG_VERB_UNLOCK:
            switch (msg->header.pvs.step) {
                case FLOM_MSG_STEP_INCR:
                    ret_cod = client ? TRUE : FALSE;
                    break;
                default:
                    break;
            } /* switch (msg->header.pvs.step) */                
            break;
        case FLOM_MSG_VERB_PING:
            switch (msg->header.pvs.step) {
                case FLOM_MSG_STEP_INCR:
                    ret_cod = client ? FALSE : TRUE;
                    break;
                case 2*FLOM_MSG_STEP_INCR:
                    ret_cod = client ? TRUE : FALSE;
                    break;
                default:
                    break;
            } /* switch (msg->header.pvs.step) */                
            break;
        case FLOM_MSG_VERB_DISCOVER:
            switch (msg->header.pvs.step) {
                case FLOM_MSG_STEP_INCR:
                    ret_cod = client ? TRUE : FALSE;
                    break;
                case 2*FLOM_MSG_STEP_INCR:
                    ret_cod = client ? FALSE : TRUE;
                    break;
                default:
                    break;
            } /* switch (msg->header.pvs.step) */                
            break;
        default:
            break;
    } /* switch (msg->header.pvs.verb) */
    FLOM_TRACE(("flom_msg_check_protocol/ret_cod=%d\n", ret_cod));
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
                     , SERIALIZE_DISCOVER_8_ERROR
                     , SERIALIZE_DISCOVER_16_ERROR
                     , INVALID_DISCOVER_STEP
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
            case FLOM_MSG_VERB_DISCOVER:
                switch (msg->header.pvs.step) {
                    case 8:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_discover_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_DISCOVER_8_ERROR);
                        break;
                    case 16:
                        if (FLOM_RC_OK != (
                                ret_cod =
                                flom_msg_serialize_discover_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_DISCOVER_16_ERROR);
                        break;
                    default:
                        THROW(INVALID_DISCOVER_STEP);
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
            case SERIALIZE_DISCOVER_8_ERROR:
            case SERIALIZE_DISCOVER_16_ERROR:
                break;
            case INVALID_LOCK_STEP:
            case INVALID_UNLOCK_STEP:
            case INVALID_PING_STEP:
            case INVALID_DISCOVER_STEP:
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
    enum Exception { INVALID_RESOURCE_TYPE
                     , BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_8\n"));
    TRY {
        int used_chars;
        flom_rsrc_type_t frt;
        
        /* <resource> */
        frt = flom_rsrc_get_type(msg->body.lock_8.resource.name);
        switch (frt) {
            case FLOM_RSRC_TYPE_SIMPLE:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      msg->body.lock_8.resource.name,
                                      FLOM_MSG_PROP_MODE,
                                      msg->body.lock_8.resource.mode,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait);
                break;
            case FLOM_RSRC_TYPE_NUMERIC:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      msg->body.lock_8.resource.name,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait,
                                      FLOM_MSG_PROP_QUANTITY,
                                      msg->body.lock_8.resource.quantity);
                break;
            case FLOM_RSRC_TYPE_SET:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      msg->body.lock_8.resource.name,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait);
                break;
            default:
                THROW(INVALID_RESOURCE_TYPE);
        } /* switch (frt) */
        
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INVALID_RESOURCE_TYPE:
                ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
                break;
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



int flom_msg_serialize_discover_8(const struct flom_msg_s *msg,
                                  char *buffer,
                                  size_t *offset, size_t *free_chars)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_discover_8\n"));
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
    FLOM_TRACE(("flom_msg_serialize_discover_8/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_discover_16(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_discover_16\n"));
    TRY {
        int used_chars;
        
        /* <network> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\" %s=\"%hu\"/>",
                              FLOM_MSG_TAG_NETWORK,
                              FLOM_MSG_PROP_ADDRESS,
                              NULL != msg->body.discover_16.network.address ?
                              msg->body.discover_16.network.address : "",
                              FLOM_MSG_PROP_PORT,
                              msg->body.discover_16.network.port);
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
    FLOM_TRACE(("flom_msg_serialize_discover_16/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_trace(const struct flom_msg_s *msg)
{
    enum Exception { TRACE_LOCK_ERROR
                     , TRACE_UNLOCK_ERROR
                     , TRACE_PING_ERROR
                     , TRACE_DISCOVER_ERROR
                     , INVALID_VERB
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace: object=%p\n", msg));
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
            case FLOM_MSG_VERB_DISCOVER: /* discover */
                if (FLOM_RC_OK != (ret_cod = flom_msg_trace_discover(msg)))
                    THROW(TRACE_DISCOVER_ERROR);
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
            case TRACE_DISCOVER_ERROR:
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
                            "name='%s',mode=%d,wait=%d,quantity=%d]]\n",
                            msg->body.lock_8.resource.name != NULL ?
                            msg->body.lock_8.resource.name : "",
                            msg->body.lock_8.resource.mode,
                            msg->body.lock_8.resource.wait,
                            msg->body.lock_8.resource.quantity));
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


    
int flom_msg_trace_discover(const struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace_discover\n"));
    TRY {
        switch (msg->header.pvs.step) {
            case 8:
                FLOM_TRACE(("flom_msg_trace_discover: body[null]\n"));
                break;
            case 16:
                FLOM_TRACE(("flom_msg_trace_discover: body[%s["
                            "%s='%s',%s=%hu]]\n",
                            FLOM_MSG_TAG_NETWORK,
                            FLOM_MSG_PROP_ADDRESS,
                            NULL != msg->body.discover_16.network.address ?
                            msg->body.discover_16.network.address : "",
                            FLOM_MSG_PROP_PORT,
                            msg->body.discover_16.network.port));
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
    FLOM_TRACE(("flom_msg_trace_discover/excp=%d/"
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
        GError *error = NULL;
        
        FLOM_TRACE(("flom_msg_deserialize: deserializing message |%*.*s|\n",
                    buffer_len, buffer_len, buffer));
        
        if (FALSE == g_markup_parse_context_parse(
                gmpc, buffer, buffer_len, &error)) {
            if (NULL != error) {
                FLOM_TRACE(("flom_msg_deserialize: code=%d, message='%s'\n",
                            error->code, error->message));
                g_error_free(error);
                error = NULL;
            }
            THROW(G_MARKUP_PARSE_CONTEXT_PARSE_ERROR);
        }
        
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
                     , G_STRDUP_ERROR1
                     , INVALID_PROPERTY1
                     , INVALID_PROPERTY2
                     , INVALID_PROPERTY3
                     , INVALID_PROPERTY4
                     , G_STRDUP_ERROR2
                     , INVALID_PROPERTY5
                     , TAG_TYPE_ERROR
                     , NONE } excp;
    
    enum {dummy_tag, msg_tag, resource_tag,
          answer_tag, network_tag } tag_type = dummy_tag;
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
        else if (!strcmp(element_name, FLOM_MSG_TAG_ANSWER))
            tag_type = answer_tag;
        else if (!strcmp(element_name, FLOM_MSG_TAG_NETWORK))
            tag_type = network_tag;
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
                                THROW(G_STRDUP_ERROR1);
                            }
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.name = tmp;
                            else
                                msg->body.unlock_8.resource.name = tmp;
                        } else if (!strcmp(*name_cursor, FLOM_MSG_PROP_MODE)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.mode =
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
                        } else if (!strcmp(*name_cursor,
                                           FLOM_MSG_PROP_QUANTITY)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.quantity =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY3);
                            }
                        }
                    }
                    break;
                case answer_tag:
                    /* check if this tag is OK for the current message */
                    if ((FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         2*FLOM_MSG_STEP_INCR == msg->header.pvs.step) ||
                        (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         3*FLOM_MSG_STEP_INCR == msg->header.pvs.step)) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_RC)) {
                            if (2*FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                                msg->body.lock_16.answer.rc =
                                    strtol(*value_cursor, NULL, 10);
                            else if (3*FLOM_MSG_STEP_INCR ==
                                     msg->header.pvs.step)
                                msg->body.lock_24.answer.rc =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY4);
                            }
                        }
                    }
                    break;
                case network_tag:
                    /* check if this tag is OK for the current message */
                    if (FLOM_MSG_VERB_DISCOVER == msg->header.pvs.verb &&
                        2*FLOM_MSG_STEP_INCR == msg->header.pvs.step) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_PORT))
                            msg->body.discover_16.network.port =
                                strtol(*value_cursor, NULL, 10);
                        else if (!strcmp(*name_cursor,
                                         FLOM_MSG_PROP_ADDRESS)) {
                            gchar *tmp = g_strdup(*value_cursor);
                            if (NULL == tmp) {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: unable to duplicate "
                                            "*value_cursor\n"));
                                THROW(G_STRDUP_ERROR2);
                            }
                            msg->body.discover_16.network.address = tmp;
                        } else {
                            FLOM_TRACE(("flom_msg_deserialize_start_"
                                        "element: property '%s' is not "
                                        "valid for verb '%s'\n",
                                        *name_cursor, element_name));
                            THROW(INVALID_PROPERTY5);
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
            case G_STRDUP_ERROR1:
            case INVALID_PROPERTY1:
            case INVALID_PROPERTY2:
            case INVALID_PROPERTY3:
            case INVALID_PROPERTY4:
            case G_STRDUP_ERROR2:
            case INVALID_PROPERTY5:
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



int flom_msg_build_answer(struct flom_msg_s *msg,
                          int verb, int step, int rc)
{
    enum Exception { NULL_OBJECT
                     , INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_build_answer\n"));
    TRY {
        if (NULL == msg)
            THROW(NULL_OBJECT);
        msg->state = FLOM_MSG_STATE_PARSING;
        msg->header.level = FLOM_MSG_LEVEL;
        msg->header.pvs.verb = verb;
        msg->header.pvs.step = step;
        switch (step) {
            case 2*FLOM_MSG_STEP_INCR:
                msg->body.lock_16.answer.rc = rc;
                break;
            case 3*FLOM_MSG_STEP_INCR:
                msg->body.lock_24.answer.rc = rc;
                break;
            default:
                THROW(INVALID_STEP);
                break;
        } /*  switch (step) */
        msg->state = FLOM_MSG_STATE_READY;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INVALID_STEP:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_msg_build_answer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

