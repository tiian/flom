/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM and libflom (FLoM API client library)
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * This file is part of libflom too and you can redistribute it and/or modify
 * it under the terms of one of the following licences:
 * - GNU General Public License version 2.0
 * - GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * GNU Lesser General Public License along with FLoM.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>



#ifdef HAVE_STRING_H
/* strcasestr is not POSIX standard and needs GNU extensions... */
# define _GNU_SOURCE
# include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif



#include "flom_config.h"
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
const gchar *FLOM_MSG_PROP_CREATE         = (gchar *)"create";
const gchar *FLOM_MSG_PROP_ELEMENT        = (gchar *)"element";
const gchar *FLOM_MSG_PROP_LEVEL          = (gchar *)"level";
const gchar *FLOM_MSG_PROP_IMMEDIATE      = (gchar *)"immediate";
const gchar *FLOM_MSG_PROP_LIFESPAN       = (gchar *)"lifespan";
const gchar *FLOM_MSG_PROP_MODE           = (gchar *)"mode";
const gchar *FLOM_MSG_PROP_NAME           = (gchar *)"name";
const gchar *FLOM_MSG_PROP_PEERID         = (gchar *)"peerid";
const gchar *FLOM_MSG_PROP_PORT           = (gchar *)"port";
const gchar *FLOM_MSG_PROP_QUANTITY       = (gchar *)"quantity";
const gchar *FLOM_MSG_PROP_RC             = (gchar *)"rc";
const gchar *FLOM_MSG_PROP_STEP           = (gchar *)"step";
const gchar *FLOM_MSG_PROP_VERB           = (gchar *)"verb"; 
const gchar *FLOM_MSG_PROP_WAIT           = (gchar *)"wait";
const gchar *FLOM_MSG_TAG_ANSWER          = (gchar *)"answer";
const gchar *FLOM_MSG_TAG_MSG             = (gchar *)"msg";
const gchar *FLOM_MSG_TAG_NETWORK         = (gchar *)"network";
const gchar *FLOM_MSG_TAG_RESOURCE        = (gchar *)"resource";
const gchar *FLOM_MSG_TAG_SESSION         = (gchar *)"session";
const gchar *FLOM_MSG_TAG_SHUTDOWN        = (gchar *)"shutdown";



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



int flom_msg_free(struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP_LOCK
                     , INVALID_STEP_UNLOCK
                     , INVALID_STEP_PING
                     , INVALID_STEP_DISCOVER
                     , INVALID_STEP_MNGMNT
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
                        if (NULL != msg->body.lock_8.session.peerid) {
                            g_free(msg->body.lock_8.session.peerid);
                            msg->body.lock_8.session.peerid = NULL;
                        }
                        if (NULL != msg->body.lock_8.resource.name) {
                            g_free(msg->body.lock_8.resource.name);
                            msg->body.lock_8.resource.name = NULL;
                        }
                        break;
                    case 2*FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.lock_16.session.peerid) {
                            g_free(msg->body.lock_16.session.peerid);
                            msg->body.lock_16.session.peerid = NULL;
                        }
                        if (NULL != msg->body.lock_16.answer.element) {
                            g_free(msg->body.lock_16.answer.element);
                            msg->body.lock_16.answer.element = NULL;
                        }
                        break;
                    case 3*FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.lock_24.answer.element) {
                            g_free(msg->body.lock_24.answer.element);
                            msg->body.lock_24.answer.element = NULL;
                        }
                        break;
                    case 4*FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.lock_32.answer.element) {
                            g_free(msg->body.lock_32.answer.element);
                            msg->body.lock_32.answer.element = NULL;
                        }
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
            case FLOM_MSG_VERB_MNGMNT:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (NULL != msg->body.mngmnt_8.session.peerid) {
                            g_free(msg->body.mngmnt_8.session.peerid);
                            msg->body.mngmnt_8.session.peerid = NULL;
                        }
                        break;
                    default:
                        THROW(INVALID_STEP_MNGMNT);
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
            case INVALID_STEP_MNGMNT:
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
                case 4*FLOM_MSG_STEP_INCR:
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
        case FLOM_MSG_VERB_MNGMNT:
            switch(msg->header.pvs.step) {
                case FLOM_MSG_STEP_INCR:
                    ret_cod = client ? TRUE : FALSE;
                    break;
                default:
                    break;
            } /* switch(msg->header.pvs.step) */
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
                     , SERIALIZE_LOCK_32_ERROR
                     , INVALID_LOCK_STEP
                     , SERIALIZE_UNLOCK_8_ERROR
                     , INVALID_UNLOCK_STEP
                     , SERIALIZE_PING_8_ERROR
                     , SERIALIZE_PING_16_ERROR
                     , INVALID_PING_STEP
                     , SERIALIZE_DISCOVER_8_ERROR
                     , SERIALIZE_DISCOVER_16_ERROR
                     , INVALID_DISCOVER_STEP
                     , SERIALIZE_MNGMNT_8_ERROR
                     , INVALID_MNGMNT_STEP
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
                    case FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_lock_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_8_ERROR);
                        break;
                    case 2*FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_lock_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_16_ERROR);
                        break;
                    case 3*FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_lock_24(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_24_ERROR);
                        break;
                    case 4*FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_lock_32(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_LOCK_32_ERROR);
                        break;
                    default:
                        THROW(INVALID_LOCK_STEP);
                }
                break;
            case FLOM_MSG_VERB_UNLOCK:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_unlock_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_UNLOCK_8_ERROR);
                        break;
                    default:
                        THROW(INVALID_UNLOCK_STEP);
                }
                break;
            case FLOM_MSG_VERB_PING:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_ping_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_PING_8_ERROR);
                        break;
                    case 2*FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_ping_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_PING_16_ERROR);
                        break;
                    default:
                        THROW(INVALID_PING_STEP);
                }
                break;
            case FLOM_MSG_VERB_DISCOVER:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_discover_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_DISCOVER_8_ERROR);
                        break;
                    case 2*FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_discover_16(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_DISCOVER_16_ERROR);
                        break;
                    default:
                        THROW(INVALID_DISCOVER_STEP);
                }
                break;
            case FLOM_MSG_VERB_MNGMNT:
                switch (msg->header.pvs.step) {
                    case FLOM_MSG_STEP_INCR:
                        if (FLOM_RC_OK != (
                                ret_cod = flom_msg_serialize_mngmnt_8(
                                    msg, buffer, &offset, &free_chars)))
                            THROW(SERIALIZE_MNGMNT_8_ERROR);
                        break;
                    default:
                        THROW(INVALID_MNGMNT_STEP);
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
            case SERIALIZE_LOCK_32_ERROR:
            case SERIALIZE_UNLOCK_8_ERROR:
            case SERIALIZE_PING_8_ERROR:
            case SERIALIZE_PING_16_ERROR:
            case SERIALIZE_DISCOVER_8_ERROR:
            case SERIALIZE_DISCOVER_16_ERROR:
            case SERIALIZE_MNGMNT_8_ERROR:
                break;
            case INVALID_LOCK_STEP:
            case INVALID_UNLOCK_STEP:
            case INVALID_PING_STEP:
            case INVALID_DISCOVER_STEP:
            case INVALID_MNGMNT_STEP:
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
    enum Exception { G_BASE64_ENCODE_ERROR
                     , BUFFER_TOO_SHORT1
                     , INVALID_RESOURCE_TYPE
                     , BUFFER_TOO_SHORT2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    gchar *base64_resource_name = NULL;
    
    FLOM_TRACE(("flom_msg_serialize_lock_8\n"));
    TRY {
        int used_chars;
        flom_rsrc_type_t frt;

        /* encode resource name using base64 encoding */
        if (NULL == (base64_resource_name =
                     g_base64_encode((guchar *)msg->body.lock_8.resource.name,
                                     strlen(msg->body.lock_8.resource.name))))
            THROW(G_BASE64_ENCODE_ERROR);
        /* <session> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\"/>",
                              FLOM_MSG_TAG_SESSION,
                              FLOM_MSG_PROP_PEERID,
                              NULL != msg->body.lock_8.session.peerid ?
                              msg->body.lock_8.session.peerid :
                              FLOM_EMPTY_STRING);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT1);
        *free_chars -= used_chars;
        *offset += used_chars;
        /* <resource> */
        frt = flom_rsrc_get_type(msg->body.lock_8.resource.name);
        switch (frt) {
            case FLOM_RSRC_TYPE_SIMPLE:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\" "
                                      "%s=\"%d\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      base64_resource_name,
                                      FLOM_MSG_PROP_MODE,
                                      msg->body.lock_8.resource.mode,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait,
                                      FLOM_MSG_PROP_CREATE,
                                      msg->body.lock_8.resource.create,
                                      FLOM_MSG_PROP_LIFESPAN,
                                      msg->body.lock_8.resource.lifespan);
                break;
            case FLOM_RSRC_TYPE_NUMERIC:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\" "
                                      "%s=\"%d\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      base64_resource_name,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait,
                                      FLOM_MSG_PROP_QUANTITY,
                                      msg->body.lock_8.resource.quantity,
                                      FLOM_MSG_PROP_CREATE,
                                      msg->body.lock_8.resource.create,
                                      FLOM_MSG_PROP_LIFESPAN,
                                      msg->body.lock_8.resource.lifespan);
                break;
            case FLOM_RSRC_TYPE_SET:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\" "
                                      "%s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      base64_resource_name,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait,
                                      FLOM_MSG_PROP_CREATE,
                                      msg->body.lock_8.resource.create,
                                      FLOM_MSG_PROP_LIFESPAN,
                                      msg->body.lock_8.resource.lifespan);
                break;
            case FLOM_RSRC_TYPE_HIER:
                used_chars = snprintf(buffer + *offset, *free_chars,
                                      "<%s %s=\"%s\" %s=\"%d\" %s=\"%d\" "
                                      "%s=\"%d\" %s=\"%d\"/>",
                                      FLOM_MSG_TAG_RESOURCE,
                                      FLOM_MSG_PROP_NAME,
                                      base64_resource_name,
                                      FLOM_MSG_PROP_MODE,
                                      msg->body.lock_8.resource.mode,
                                      FLOM_MSG_PROP_WAIT,
                                      msg->body.lock_8.resource.wait,
                                      FLOM_MSG_PROP_CREATE,
                                      msg->body.lock_8.resource.create,
                                      FLOM_MSG_PROP_LIFESPAN,
                                      msg->body.lock_8.resource.lifespan);
                break;
            default:
                THROW(INVALID_RESOURCE_TYPE);
        } /* switch (frt) */
        
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT2);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_BASE64_ENCODE_ERROR:
                ret_cod = FLOM_RC_G_BASE64_ENCODE_ERROR;
                break;
            case BUFFER_TOO_SHORT1:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case INVALID_RESOURCE_TYPE:
                ret_cod = FLOM_RC_INVALID_RESOURCE_NAME;
                break;
            case BUFFER_TOO_SHORT2:
                ret_cod = FLOM_RC_CONTAINER_FULL;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release memory */
    if (NULL != base64_resource_name) {
        g_free(base64_resource_name);
        base64_resource_name = NULL;
    }
    FLOM_TRACE(("flom_msg_serialize_lock_8/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_lock_16(const struct flom_msg_s *msg,
                               char *buffer,
                               size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT1
                     , BUFFER_TOO_SHORT2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_16\n"));
    TRY {
        int used_chars;
        
        /* <session> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\"/>",
                              FLOM_MSG_TAG_SESSION,
                              FLOM_MSG_PROP_PEERID,
                              NULL != msg->body.lock_16.session.peerid ?
                              msg->body.lock_16.session.peerid :
                              FLOM_EMPTY_STRING);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT1);
        *free_chars -= used_chars;
        *offset += used_chars;
        /* <answer> */
        if (NULL == msg->body.lock_16.answer.element) {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_16.answer.rc);
        } else {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\" %s=\"%s\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_16.answer.rc,
                                  FLOM_MSG_PROP_ELEMENT,
                                  msg->body.lock_16.answer.element);
        } /* if (NULL == msg->body.lock_16.answer.element) */
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT2);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT1:
            case BUFFER_TOO_SHORT2:
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
        if (NULL == msg->body.lock_24.answer.element) {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_24.answer.rc);
        } else {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\" %s=\"%s\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_24.answer.rc,
                                  FLOM_MSG_PROP_ELEMENT,
                                  msg->body.lock_24.answer.element);
        } /* if (NULL == msg->body.lock_16.answer.element) */
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



int flom_msg_serialize_lock_32(const struct flom_msg_s *msg,
                               char *buffer,
                               size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_lock_32\n"));
    TRY {
        int used_chars;
        
        /* <answer> */
        if (NULL == msg->body.lock_32.answer.element) {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_32.answer.rc);
        } else {
            used_chars = snprintf(buffer + *offset, *free_chars,
                                  "<%s %s=\"%d\" %s=\"%s\"/>",
                                  FLOM_MSG_TAG_ANSWER,
                                  FLOM_MSG_PROP_RC,
                                  msg->body.lock_32.answer.rc,
                                  FLOM_MSG_PROP_ELEMENT,
                                  msg->body.lock_32.answer.element);
        } /* if (NULL == msg->body.lock_16.answer.element) */
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
    FLOM_TRACE(("flom_msg_serialize_lock_32/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_msg_serialize_unlock_8(const struct flom_msg_s *msg,
                                char *buffer,
                                size_t *offset, size_t *free_chars)
{
    enum Exception { G_BASE64_ENCODE_ERROR
                     , BUFFER_TOO_SHORT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    gchar *base64_resource_name = NULL;
    
    FLOM_TRACE(("flom_msg_serialize_unlock_8\n"));
    TRY {
        int used_chars;
        
        /* encode resource name using base64 encoding */
        if (NULL == (base64_resource_name =
                     g_base64_encode(
                         (guchar *)msg->body.unlock_8.resource.name,
                         strlen(msg->body.unlock_8.resource.name))))
            THROW(G_BASE64_ENCODE_ERROR);
        /* <resource> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\"/>",
                              FLOM_MSG_TAG_RESOURCE,
                              FLOM_MSG_PROP_NAME,
                              base64_resource_name);
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_BASE64_ENCODE_ERROR:
                ret_cod = FLOM_RC_G_BASE64_ENCODE_ERROR;
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
    /* release memory */
    if (NULL != base64_resource_name) {
        g_free(base64_resource_name);
        base64_resource_name = NULL;
    }
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
                              msg->body.discover_16.network.address :
                              FLOM_EMPTY_STRING,
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



int flom_msg_serialize_mngmnt_8(const struct flom_msg_s *msg,
                                char *buffer,
                                size_t *offset, size_t *free_chars)
{
    enum Exception { BUFFER_TOO_SHORT1
                     , BUFFER_TOO_SHORT2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_serialize_mngmnt_8\n"));
    TRY {
        int used_chars = 0;
        
        /* <session> */
        used_chars = snprintf(buffer + *offset, *free_chars,
                              "<%s %s=\"%s\"/>",
                              FLOM_MSG_TAG_SESSION,
                              FLOM_MSG_PROP_PEERID,
                              STROREMPTY(msg->body.mngmnt_8.session.peerid));
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT1);
        *free_chars -= used_chars;
        *offset += used_chars;
        /* shutdown action */
        if (FLOM_MSG_MNGMNT_ACTION_SHUTDOWN == msg->body.mngmnt_8.action) {
            used_chars = snprintf(
                buffer + *offset, *free_chars, "<%s %s=\"%d\"/>",
                FLOM_MSG_TAG_SHUTDOWN, FLOM_MSG_PROP_IMMEDIATE,
                msg->body.mngmnt_8.action_data.shutdown.immediate);
        }
        if (used_chars >= *free_chars)
            THROW(BUFFER_TOO_SHORT2);
        *free_chars -= used_chars;
        *offset += used_chars;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case BUFFER_TOO_SHORT1:
            case BUFFER_TOO_SHORT2:
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
                     , TRACE_MNGMNT_ERROR
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
            case FLOM_MSG_VERB_MNGMNT: /* mngmnt */
                if (FLOM_RC_OK != (ret_cod = flom_msg_trace_mngmnt(msg)))
                    THROW(TRACE_MNGMNT_ERROR);
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
            case TRACE_MNGMNT_ERROR:
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
            case FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_lock: body["
                            "%s[%s='%s'], "
                            "%s[%s='%s',%s=%d,%s=%d,%s=%d,%s=%d,%s=%d]"
                            "]\n",
                            FLOM_MSG_TAG_SESSION,
                            FLOM_MSG_PROP_PEERID,
                            STROREMPTY(msg->body.lock_8.session.peerid),
                            FLOM_MSG_TAG_RESOURCE,
                            FLOM_MSG_PROP_NAME,
                            STROREMPTY(msg->body.lock_8.resource.name),
                            FLOM_MSG_PROP_MODE,
                            msg->body.lock_8.resource.mode,
                            FLOM_MSG_PROP_WAIT,
                            msg->body.lock_8.resource.wait,
                            FLOM_MSG_PROP_QUANTITY,
                            msg->body.lock_8.resource.quantity,
                            FLOM_MSG_PROP_CREATE,
                            msg->body.lock_8.resource.create,
                            FLOM_MSG_PROP_LIFESPAN,
                            msg->body.lock_8.resource.lifespan));
                break;
            case 2*FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_lock: body["
                            "%s[%s='%s'], "
                            "%s[%s=%d,%s='%s']]\n",
                            FLOM_MSG_TAG_SESSION,
                            FLOM_MSG_PROP_PEERID,
                            STROREMPTY(msg->body.lock_16.session.peerid),
                            FLOM_MSG_TAG_RESOURCE,
                            FLOM_MSG_PROP_RC,
                            msg->body.lock_16.answer.rc,
                            FLOM_MSG_PROP_ELEMENT,
                            msg->body.lock_16.answer.element != NULL ?
                            msg->body.lock_16.answer.element :
                            FLOM_EMPTY_STRING));
                break;
            case 3*FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_lock: body[%s["
                            "%s=%d,%s='%s']]\n",
                            FLOM_MSG_TAG_RESOURCE,
                            FLOM_MSG_PROP_RC,
                            msg->body.lock_24.answer.rc,
                            FLOM_MSG_PROP_ELEMENT,
                            msg->body.lock_24.answer.element != NULL ?
                            msg->body.lock_24.answer.element :
                            FLOM_EMPTY_STRING));
                break;
            case 4*FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_lock: body[%s["
                            "%s=%d,%s='%s']]\n",
                            FLOM_MSG_TAG_RESOURCE,
                            FLOM_MSG_PROP_RC,
                            msg->body.lock_32.answer.rc,
                            FLOM_MSG_PROP_ELEMENT,
                            msg->body.lock_32.answer.element != NULL ?
                            msg->body.lock_32.answer.element :
                            FLOM_EMPTY_STRING));
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
            case FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_unlock: body[%s["
                            "%s='%s']]\n",
                            FLOM_MSG_TAG_RESOURCE,
                            FLOM_MSG_PROP_NAME,
                            msg->body.unlock_8.resource.name != NULL ?
                            msg->body.unlock_8.resource.name :
                            FLOM_NULL_STRING));
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
            case FLOM_MSG_STEP_INCR:
            case 2*FLOM_MSG_STEP_INCR:
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
            case FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_discover: body[null]\n"));
                break;
            case 2*FLOM_MSG_STEP_INCR:
                FLOM_TRACE(("flom_msg_trace_discover: body[%s["
                            "%s='%s',%s=%hu]]\n",
                            FLOM_MSG_TAG_NETWORK,
                            FLOM_MSG_PROP_ADDRESS,
                            NULL != msg->body.discover_16.network.address ?
                            msg->body.discover_16.network.address :
                            FLOM_NULL_STRING, FLOM_MSG_PROP_PORT,
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


    
int flom_msg_trace_mngmnt(const struct flom_msg_s *msg)
{
    enum Exception { INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_msg_trace_mngmnt\n"));
    TRY {
        switch (msg->header.pvs.step) {
            case FLOM_MSG_STEP_INCR:
                if (FLOM_MSG_MNGMNT_ACTION_SHUTDOWN ==
                    msg->body.mngmnt_8.action) {
                    FLOM_TRACE(
                        ("flom_msg_trace_mngmnt: body["
                         "%s[%s='%s'], "
                         "%s[%s=%d]"
                         "]\n",
                         FLOM_MSG_TAG_SESSION,
                         FLOM_MSG_PROP_PEERID,
                         STROREMPTY(msg->body.mngmnt_8.session.peerid),
                         FLOM_MSG_TAG_SHUTDOWN, FLOM_MSG_PROP_IMMEDIATE,
                         msg->body.mngmnt_8.action_data.shutdown.immediate));
                }
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
    FLOM_TRACE(("flom_msg_trace_mngmnt/excp=%d/"
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



int flom_msg_deserialize_resource_name(const gchar *base64,
                                       gchar **resource_name)
{
    enum Exception { G_BASE64_DECODE_ERROR
                     , G_TRY_REALLOC_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    guchar *buffer = NULL;
    gchar *res_name = NULL;
    
    FLOM_TRACE(("flom_msg_deserialize_resource_name\n"));
    TRY {
        gsize out_len;
        
        if (NULL == (buffer = g_base64_decode(base64, &out_len)))
            THROW(G_BASE64_DECODE_ERROR);
        /* add 1 char to append the string terminator */
        if (NULL == (res_name = g_try_realloc((gpointer)buffer, out_len+1)))
            THROW(G_TRY_REALLOC_ERROR);
        res_name[out_len] = '\0';
        FLOM_TRACE(("flom_msg_deserialize_resource_name: base64='%s', "
                    "res_name='%s'\n", base64, res_name));
        *resource_name = res_name;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_BASE64_DECODE_ERROR:
                ret_cod = FLOM_RC_G_BASE64_DECODE_ERROR;
                break;
            case G_TRY_REALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_REALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* garbage collector */
    if (NULL != buffer && NONE != excp) {
        g_free(buffer);
        buffer = NULL;
    }
    FLOM_TRACE(("flom_msg_deserialize_resource_name/excp=%d/"
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
                     , PROTOCOL_LEVEL_MISMATCH
                     , DESERIALIZE_RESOURCE_NAME_ERROR
                     , INVALID_PROPERTY1
                     , INVALID_PROPERTY2
                     , INVALID_PROPERTY3
                     , INVALID_PROPERTY4
                     , INVALID_PROPERTY5
                     , INVALID_PROPERTY6
                     , G_STRDUP_ERROR1
                     , G_STRDUP_ERROR2
                     , INVALID_PROPERTY7
                     , G_STRDUP_ERROR3
                     , INVALID_PROPERTY8
                     , TAG_TYPE_ERROR
                     , NONE } excp;
    
    enum {
        dummy_tag, msg_tag, resource_tag, answer_tag, network_tag,
        session_tag, shutdown_tag
    } tag_type = dummy_tag;
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
        else if (!strcmp(element_name, FLOM_MSG_TAG_SESSION))
            tag_type = session_tag;
        else if (!strcmp(element_name, FLOM_MSG_TAG_SHUTDOWN))
            tag_type = shutdown_tag;
        while (*name_cursor) {
            FLOM_TRACE(("flom_msg_deserialize_start_element: name_cursor='%s' "
                        "value_cursor='%s'\n", *name_cursor, *value_cursor));
            switch (tag_type) {
                case msg_tag:
                    if (!strcmp(*name_cursor, FLOM_MSG_PROP_LEVEL)) {
                        msg->header.level = strtol(*value_cursor, NULL, 10);
                        if (FLOM_MSG_LEVEL != msg->header.level)
                            THROW(PROTOCOL_LEVEL_MISMATCH);
                    } else if (!strcmp(*name_cursor, FLOM_MSG_PROP_VERB))
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
                            gchar *tmp;
                            if (FLOM_RC_OK !=
                                flom_msg_deserialize_resource_name(
                                    *value_cursor, &tmp))
                                THROW(DESERIALIZE_RESOURCE_NAME_ERROR);
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
                        } else if (!strcmp(*name_cursor,
                                           FLOM_MSG_PROP_CREATE)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.create =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY4);
                            }
                        } else if (!strcmp(*name_cursor,
                                           FLOM_MSG_PROP_LIFESPAN)) {
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb)
                                msg->body.lock_8.resource.lifespan =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY5);
                            }
                        }
                    }
                    break;
                case answer_tag:
                    /* check if this tag is OK for the current message */
                    if ((FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         2*FLOM_MSG_STEP_INCR == msg->header.pvs.step) ||
                        (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         3*FLOM_MSG_STEP_INCR == msg->header.pvs.step) ||
                        (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                         4*FLOM_MSG_STEP_INCR == msg->header.pvs.step)) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_RC)) {
                            if (2*FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                                msg->body.lock_16.answer.rc =
                                    strtol(*value_cursor, NULL, 10);
                            else if (3*FLOM_MSG_STEP_INCR ==
                                     msg->header.pvs.step)
                                msg->body.lock_24.answer.rc =
                                    strtol(*value_cursor, NULL, 10);
                            else if (4*FLOM_MSG_STEP_INCR ==
                                     msg->header.pvs.step)
                                msg->body.lock_32.answer.rc =
                                    strtol(*value_cursor, NULL, 10);
                            else {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: property '%s' is not "
                                            "valid for verb '%s'\n",
                                            *name_cursor, element_name));
                                THROW(INVALID_PROPERTY6);
                            }
                        } else if (!strcmp(*name_cursor,
                                           FLOM_MSG_PROP_ELEMENT)) {
                            gchar *tmp = g_strdup(*value_cursor);
                            if (NULL == tmp) {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: unable to duplicate "
                                            "*name_cursor\n"));
                                THROW(G_STRDUP_ERROR1);
                            }
                            if (2*FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                                msg->body.lock_16.answer.element = tmp;
                            else
                                msg->body.lock_24.answer.element = tmp;
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
                            THROW(INVALID_PROPERTY7);
                        }
                    }
                    break;
                case session_tag:
                    /* check if this tag is OK for the current message */
                    if (
                        (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb &&
                        ((FLOM_MSG_STEP_INCR == msg->header.pvs.step) ||
                         (2*FLOM_MSG_STEP_INCR == msg->header.pvs.step))) ||
                        (FLOM_MSG_VERB_MNGMNT == msg->header.pvs.verb &&
                         FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                        ) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_PEERID)) {
                            gchar *tmp = g_strdup(*value_cursor);
                            if (NULL == tmp) {
                                FLOM_TRACE(("flom_msg_deserialize_start_"
                                            "element: unable to duplicate "
                                            "*value_cursor\n"));
                                THROW(G_STRDUP_ERROR3);
                            }
                            if (FLOM_MSG_VERB_LOCK == msg->header.pvs.verb) {
                                /* lock verb message */
                                if (FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                                    msg->body.lock_8.session.peerid = tmp;
                                else
                                    msg->body.lock_16.session.peerid = tmp;
                            } else {
                                /* mngmnt verb message */
                                msg->body.mngmnt_8.session.peerid = tmp;
                            }
                        }
                    } 
                    break;
                case shutdown_tag:
                    /* check if this tag is OK for the current message */
                    if (FLOM_MSG_VERB_MNGMNT == msg->header.pvs.verb &&
                        FLOM_MSG_STEP_INCR == msg->header.pvs.step) {
                        if (!strcmp(*name_cursor, FLOM_MSG_PROP_IMMEDIATE)) {
                            msg->body.mngmnt_8.action =
                                FLOM_MSG_MNGMNT_ACTION_SHUTDOWN;
                            msg->body.mngmnt_8.action_data.shutdown.immediate =
                                strtol(*value_cursor, NULL, 10);
                        } else {
                            FLOM_TRACE(("flom_msg_deserialize_start_"
                                        "element: property '%s' is not "
                                        "valid for verb '%s'\n",
                                        *name_cursor, element_name));
                            THROW(INVALID_PROPERTY8);
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
            case PROTOCOL_LEVEL_MISMATCH:
            case DESERIALIZE_RESOURCE_NAME_ERROR:
            case INVALID_PROPERTY1:
            case INVALID_PROPERTY2:
            case INVALID_PROPERTY3:
            case INVALID_PROPERTY4:
            case INVALID_PROPERTY5:
            case INVALID_PROPERTY6:
            case G_STRDUP_ERROR1:
            case G_STRDUP_ERROR2:
            case INVALID_PROPERTY7:
            case G_STRDUP_ERROR3:
            case INVALID_PROPERTY8:
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
                          int verb, int step, int rc, const gchar *element)
{
    enum Exception { NULL_OBJECT
                     , G_STRDUP_ERROR1
                     , G_STRDUP_ERROR2
                     , INVALID_STEP
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    gchar *tmp_element = NULL;
    
    FLOM_TRACE(("flom_msg_build_answer: verb=%d, step=%d\n", verb, step));
    TRY {    
        if (NULL == msg)
            THROW(NULL_OBJECT);
        if (NULL != element &&
            NULL == (tmp_element = g_strdup(element)))
            THROW(G_STRDUP_ERROR1);
            
        msg->state = FLOM_MSG_STATE_PARSING;
        msg->header.level = FLOM_MSG_LEVEL;
        msg->header.pvs.verb = verb;
        msg->header.pvs.step = step;
        switch (step) {
            case 2*FLOM_MSG_STEP_INCR:
                if (NULL != msg->body.lock_16.session.peerid)
                    g_free(msg->body.lock_16.session.peerid);
                if (NULL == (msg->body.lock_16.session.peerid =
                             g_strdup(flom_tls_get_unique_id())))
                    THROW(G_STRDUP_ERROR2);

                msg->body.lock_16.answer.rc = rc;
                msg->body.lock_16.answer.element = tmp_element;
                tmp_element = NULL;
                break;
            case 3*FLOM_MSG_STEP_INCR:
                msg->body.lock_24.answer.rc = rc;
                msg->body.lock_24.answer.element = tmp_element;
                tmp_element = NULL;
                break;
            case 4*FLOM_MSG_STEP_INCR:
                msg->body.lock_32.answer.rc = rc;
                msg->body.lock_32.answer.element = tmp_element;
                tmp_element = NULL;
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
            case G_STRDUP_ERROR1:
            case G_STRDUP_ERROR2:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
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
    /* release memory if an error occurred */
    if (NULL != tmp_element)
        g_free(tmp_element);
    tmp_element = NULL;
    FLOM_TRACE(("flom_msg_build_answer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



gchar *flom_msg_get_peerid(const struct flom_msg_s *msg)
{
    gchar *ret = NULL;
    FLOM_TRACE(("flom_msg_get_peerid\n"));
    if (NULL == msg) {
        FLOM_TRACE(("flom_msg_get_peerid: passed message is NULL!\n"));
    } else {
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                if (FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                    ret = msg->body.lock_8.session.peerid;
                else if (2*FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                    ret = msg->body.lock_16.session.peerid;
                break;
            case FLOM_MSG_VERB_MNGMNT:
                if (FLOM_MSG_STEP_INCR == msg->header.pvs.step)
                    ret = msg->body.mngmnt_8.session.peerid;
                break;
            default:
                break;
        } /* switch (msg->header.pvs.verb) */
    } /* if (NULL == msg) */
    return ret;
}

