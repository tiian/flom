/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
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
#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_TIME_H
# include <time.h>
#endif



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_rsrc.h"
#include "flom_resource_timestamp.h"
#include "flom_tcp.h"
#include "flom_trace.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_RESOURCE_TIMESTAMP



const gchar MICRO_FORMAT[] = "#ffffff";



int flom_resource_timestamp_can_lock(flom_resource_t *resource)
{
    struct timeval tv;
    FLOM_TRACE(("flom_resource_timestamp_can_lock: "
                "total_quantity=%d, locked_quantity=%d\n",
                resource->data.timestamp.total_quantity,
                resource->data.timestamp.locked_quantity));
    FLOM_TRACE(("flom_resource_timestamp_can_lock: "
                "interval.tv_sec=%d, interval.tv_usec=%d\n",
                resource->data.timestamp.interval.tv_sec,
                resource->data.timestamp.interval.tv_usec));
    /* all the avaialable slots are already used */
    if (resource->data.timestamp.total_quantity -
        resource->data.timestamp.locked_quantity <= 0) {
        FLOM_TRACE(("flom_resource_timestamp_can_lock: FALSE, no available "
                    "slots\n"));
        return FALSE;
    }
    /* check last released timestamp and current time */
    gettimeofday(&tv, NULL);
    FLOM_TRACE(("flom_resource_timestamp_can_lock: "
                "tv.tv_sec=%d, tv.tv_usec=%d\n", tv.tv_sec, tv.tv_usec));
    FLOM_TRACE(("flom_resource_timestamp_can_lock: "
                "last_timestamp.tv_sec=%d, last_timestamp.tv_usec=%d\n",
                resource->data.timestamp.last_timestamp.tv_sec,
                resource->data.timestamp.last_timestamp.tv_usec));
    /* check the minimum interval grain */
    if (0 < resource->data.timestamp.interval.tv_usec) {
        /* if seconds changed, return TRUE */
        if (tv.tv_sec > resource->data.timestamp.last_timestamp.tv_sec) {
            FLOM_TRACE(("flom_resource_timestamp_can_lock: TRUE, different "
                        "second\n"));
            return TRUE;
        }
        /* this point is reached ONLY if the current time is in the same
           second of the last timestamp */
        if (tv.tv_usec/resource->data.timestamp.interval.tv_usec >
            resource->data.timestamp.last_timestamp.tv_usec/
            resource->data.timestamp.interval.tv_usec) {
            FLOM_TRACE(("flom_resource_timestamp_can_lock: TRUE, enought time "
                        "interval (micro seconds)\n"));
            return TRUE;
        }
        FLOM_TRACE(("flom_resource_timestamp_can_lock: FALSE, not enought "
                    "time interval (micro seconds)\n"));
        return FALSE;
    }
    /* this point is reached ONLY if the format does not contain fractions
       of second */
    if (0 < resource->data.timestamp.interval.tv_sec) {
        /* timestamp format contains at least 1 second */
        if (tv.tv_sec/resource->data.timestamp.interval.tv_sec >
            resource->data.timestamp.last_timestamp.tv_sec/
            resource->data.timestamp.interval.tv_sec) {
            FLOM_TRACE(("flom_resource_timestamp_can_lock: TRUE, enought time "
                    "interval (seconds)\n"));
            return TRUE;
        }
        FLOM_TRACE(("flom_resource_timestamp_can_lock: FALSE, not enought "
                    "time interval (seconds)\n"));
        return FALSE;
    }
    FLOM_TRACE(("flom_resource_timestamp_can_lock: FALSE, "
                "time interval is 0!\n"));
    return FALSE;
}



struct timeval flom_resource_timestamp_next_deadline(
    flom_resource_t *resource)
{
    struct timeval ret;
    
    FLOM_TRACE(("flom_resource_timestamp_next_deadline: "
                "last_timestamp.tv_sec=%d, last_timestamp.tv_usec=%d\n",
                resource->data.timestamp.last_timestamp.tv_sec,
                resource->data.timestamp.last_timestamp.tv_usec));
    ret.tv_sec = 0;
    ret.tv_usec = 0;
    if (0 < resource->data.timestamp.interval.tv_sec)
        ret.tv_sec = (resource->data.timestamp.last_timestamp.tv_sec /
                      resource->data.timestamp.interval.tv_sec + 1) *
            resource->data.timestamp.interval.tv_sec;
    else
        ret.tv_sec = resource->data.timestamp.last_timestamp.tv_sec;
    if (0 < resource->data.timestamp.interval.tv_usec)
        ret.tv_usec = (resource->data.timestamp.last_timestamp.tv_usec /
                       resource->data.timestamp.interval.tv_usec + 1) *
            resource->data.timestamp.interval.tv_usec;
    if (999999 < ret.tv_usec) {
        ret.tv_sec++;
        ret.tv_usec -= 1000000;
    }
    FLOM_TRACE(("flom_resource_timestamp_next_deadline: "
                "ret.tv_sec=%d, ret.tv_usec=%d\n", ret.tv_sec, ret.tv_usec));
    return ret;
}



int flom_resource_timestamp_get(flom_resource_t *resource,
                                struct timeval *tv,
                                gchar *str_timestamp, size_t max)
{
    enum Exception { GETTIMEOFDAY_ERROR
                     , G_STRDUP_ERROR
                     , LOCALTIME_R_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    gchar *tmp_format = NULL;

    FLOM_TRACE(("flom_resource_timestamp_get\n"));
    TRY {
        struct timeval tv;
        struct tm broken_time;
        gchar micro_format[sizeof(MICRO_FORMAT)];
        gchar micro_seconds[7];
        size_t digits;

        strcpy(micro_format, MICRO_FORMAT);
        /* retrieve time from the system */
        if (0 != gettimeofday(&tv, NULL))
            THROW(GETTIMEOFDAY_ERROR);
        resource->data.timestamp.last_timestamp = tv;
        /* serialize microseconds to a string */
        snprintf(micro_seconds, sizeof(micro_seconds), "%06d",
                 (int)tv.tv_usec);
        /* create a temporary format and substitute fraction of second
         * formats with the current value: see below loop */
        if (NULL == (tmp_format = g_strdup(resource->data.timestamp.format)))
            THROW(G_STRDUP_ERROR);
        
        /* look for second fraction formats (it's not provided by strftime)
         * #f : tenths of a second
         * #ff : hundredths of a second
         * #fff : milliseconds
         * #ffff : tenths of a millisecond
         * #fffff : hundredths of a millisecond (tens of microseconds)
         * #ffffff : microseconds
         */
        while (0 < (digits = strlen(micro_format)-1)) {
            gchar *next_fraction = tmp_format;
            /* search the fraction format inside the whole format */
            while (NULL != (next_fraction = strstr(
                                next_fraction, micro_format))) {
                *next_fraction = '.';
                strncpy(++next_fraction, micro_seconds, digits);
                next_fraction += digits;
            } /* while (NULL != (next_fraction = strstr( */
            /* remove one digit */
            micro_format[digits] = '\0';
        } /* while (strlen(micro_format) > 1) */
        
        /* break & serialize time using the format required by the user */
        if (NULL == localtime_r(&tv.tv_sec, &broken_time))
            THROW(LOCALTIME_R_ERROR);
        strftime(str_timestamp, max, tmp_format, &broken_time);
        FLOM_TRACE(("flom_resource_timestamp_get: '%s'\n", str_timestamp));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETTIMEOFDAY_ERROR:
                ret_cod = FLOM_RC_GETTIMEOFDAY_ERROR;
                break;
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case LOCALTIME_R_ERROR:
                ret_cod = FLOM_RC_LOCALTIME_R_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != tmp_format)
        g_free(tmp_format);
    FLOM_TRACE(("flom_resource_timestamp_get/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_timestamp_init(flom_resource_t *resource,
                                 const gchar *name)
{
    enum Exception { G_STRDUP_ERROR
                     , RSRC_GET_INFIX_ERROR
                     , RSRC_GET_NUMBER_ERROR
                     , G_QUEUE_NEW_ERROR1
                     , G_QUEUE_NEW_ERROR2
                     , INVALID_TIMESTAMP_FORMAT
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_timestamp_init\n"));
    TRY {
        unsigned int microsec = 1;
        gchar micro_format[sizeof(MICRO_FORMAT)];
        size_t digits;
        
        strcpy(micro_format, MICRO_FORMAT);
        if (NULL == (resource->name = g_strdup(name)))
            THROW(G_STRDUP_ERROR);
        FLOM_TRACE(("flom_resource_timestamp_init: initialized resource "
                    "('%s')\n", resource->name));

        if (FLOM_RC_OK != (ret_cod = flom_rsrc_get_infix(
                               name, FLOM_RSRC_TYPE_TIMESTAMP,
                               &(resource->data.timestamp.format))))
            THROW(RSRC_GET_INFIX_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_rsrc_get_number(
                               name, FLOM_RSRC_TYPE_TIMESTAMP,
                               &(resource->data.timestamp.total_quantity))))
            THROW(RSRC_GET_NUMBER_ERROR);
        resource->data.timestamp.locked_quantity = 0;
        resource->data.timestamp.last_timestamp.tv_usec = 0;
        resource->data.timestamp.last_timestamp.tv_sec = 0;
        resource->data.timestamp.interval.tv_usec = 0;
        resource->data.timestamp.interval.tv_sec = 0;
        resource->data.timestamp.holders = NULL;
        if (NULL == (resource->data.timestamp.waitings = g_queue_new()))
            THROW(G_QUEUE_NEW_ERROR2);
        /* minimum interval of time from the format */
        /* fraction of a second formats */
        while (0 < (digits = strlen(micro_format)-1)) {
            if (NULL != strstr(resource->data.timestamp.format,
                               micro_format)) {
                resource->data.timestamp.interval.tv_usec = microsec;
                break;
            }
            microsec *= 10;
            micro_format[digits] = '\0';
        } /* while (0 < (digits = strlen(micro_format)-1)) */
        if (0 == resource->data.timestamp.interval.tv_usec) {
            if (NULL != strstr(resource->data.timestamp.format, "%c") ||
                NULL != strstr(resource->data.timestamp.format, "%r") ||
                NULL != strstr(resource->data.timestamp.format, "%s") ||
                NULL != strstr(resource->data.timestamp.format, "%S") ||
                NULL != strstr(resource->data.timestamp.format, "%T") ||
                NULL != strstr(resource->data.timestamp.format, "%X")) {
                /* the above specifiers change once per second */
                resource->data.timestamp.interval.tv_sec = 1;
            } else if (NULL != strstr(resource->data.timestamp.format, "%M") ||
                       NULL != strstr(resource->data.timestamp.format, "%R")) {
                /* the above specifiers change once per minute */
                resource->data.timestamp.interval.tv_sec = 60;
            } else if (NULL != strstr(resource->data.timestamp.format, "%H") ||
                       NULL != strstr(resource->data.timestamp.format, "%I")) {
                /* the above specifiers change once per hour */
                resource->data.timestamp.interval.tv_sec = 3600;
            }
        } /* if (0 == microsec_resolution) */
        FLOM_TRACE(("flom_resource_timestamp_init: interval.tv_sec=%d, "
                    "interval.tv_usec=%d\n",
                    resource->data.timestamp.interval.tv_sec,
                    resource->data.timestamp.interval.tv_usec));
        /* check that at least one specifier has been found */
        if (0 == resource->data.timestamp.interval.tv_usec &&
            0 == resource->data.timestamp.interval.tv_sec) {
            FLOM_TRACE(("flom_resource_timestamp_init: format '%s' does not "
                        "contain a specifier that changes at least once per "
                        "hour\n", resource->data.timestamp.format));
            THROW(INVALID_TIMESTAMP_FORMAT);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_STRDUP_ERROR:
                ret_cod = FLOM_RC_G_STRDUP_ERROR;
                break;
            case RSRC_GET_INFIX_ERROR:
            case RSRC_GET_NUMBER_ERROR:
                break;
            case G_QUEUE_NEW_ERROR1:
            case G_QUEUE_NEW_ERROR2:
                ret_cod = FLOM_RC_G_QUEUE_NEW_ERROR;
                break;
            case INVALID_TIMESTAMP_FORMAT:
                ret_cod = FLOM_RC_INVALID_TIMESTAMP_FORMAT;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_timestamp_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_timestamp_inmsg(flom_resource_t *resource,
                                  flom_conn_t *conn,
                                  struct flom_msg_s *msg,
                                  struct timeval *next_deadline)
{
    enum Exception { MSG_FREE_ERROR1
                     , G_TRY_MALLOC_ERROR1
                     , RESOURCE_TIMESTAMP_GET_ERROR
                     , MSG_BUILD_ANSWER_ERROR1
                     , G_TRY_MALLOC_ERROR2
                     , MSG_BUILD_ANSWER_ERROR2
                     , MSG_BUILD_ANSWER_ERROR3
                     , INVALID_OPTION
                     , OBJ_CORRUPTED
                     , RESOURCE_TIMESTAMP_CLEAN_ERROR
                     , MSG_FREE_ERROR2
                     , PROTOCOL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_resource_timestamp_inmsg\n"));
    TRY {
        int can_lock = TRUE;
        int can_wait = TRUE;
        int impossible_lock = FALSE;
        gchar element[1000]; /* it must contain a guint */
        struct flom_rsrc_conn_lock_s *cl = NULL;
        
        flom_msg_trace(msg);
        switch (msg->header.pvs.verb) {
            case FLOM_MSG_VERB_LOCK:
                can_lock = flom_resource_timestamp_can_lock(resource);
                can_wait = msg->body.lock_8.resource.wait;
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR1);
                flom_msg_init(msg);
                if (can_lock) {
                    /* get the lock */
                    /* put this connection in holders list */
                    FLOM_TRACE(("flom_resource_timestamp_inmsg: asked lock "
                                "can be assigned to connection %p\n", conn));
                    if (NULL == (cl = flom_rsrc_conn_lock_new()))
                        THROW(G_TRY_MALLOC_ERROR1);
                    if (FLOM_RC_OK != (ret_cod = flom_resource_timestamp_get(
                                           resource, &cl->info.timestamp_value,
                                           element, sizeof(element))))
                        THROW(RESOURCE_TIMESTAMP_GET_ERROR);
                    cl->conn = conn;
                    resource->data.timestamp.holders = g_slist_prepend(
                        resource->data.timestamp.holders,
                        (gpointer)cl);
                    resource->data.timestamp.locked_quantity++;
                    if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                           msg, FLOM_MSG_VERB_LOCK,
                                           flom_conn_get_last_step(conn) +
                                           FLOM_MSG_STEP_INCR,
                                           FLOM_RC_OK, element)))
                        THROW(MSG_BUILD_ANSWER_ERROR1);
                } else {
                    /* can't lock, enqueue */
                    if (can_wait) {
                        /* compute next deadline */
                        *next_deadline = flom_resource_timestamp_next_deadline(
                            resource);
                        /* put this connection in waitings queue */
                        FLOM_TRACE(("flom_resource_timestamp_inmsg: "
                                    "lock can not be assigned to "
                                    "connection %p, queing...\n", conn));
                        if (NULL == (cl = flom_rsrc_conn_lock_new()))
                            THROW(G_TRY_MALLOC_ERROR2);
                        cl->conn = conn;
                        g_queue_push_tail(
                            resource->data.timestamp.waitings,
                            (gpointer)cl);
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               FLOM_RC_LOCK_ENQUEUED, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR2);
                    } else {
                        FLOM_TRACE(("flom_resource_timestamp_inmsg: asked "
                                    "lock can not be assigned to "
                                    "connection %p, rejecting...\n", conn));
                        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                               msg, FLOM_MSG_VERB_LOCK,
                                               flom_conn_get_last_step(conn) +
                                               FLOM_MSG_STEP_INCR,
                                               impossible_lock ?
                                               FLOM_RC_LOCK_IMPOSSIBLE :
                                               FLOM_RC_LOCK_BUSY, NULL)))
                            THROW(MSG_BUILD_ANSWER_ERROR3);
                    } /* if (msg->body.lock_8.resource.wait) */
                } /* if (can_lock) */
                break;
            case FLOM_MSG_VERB_UNLOCK:
                /* check lock is managed by this locker (this check will
                   trigger some issue if a client obtained more locks...) */
                if (g_strcmp0(flom_resource_get_name(resource),
                              msg->body.unlock_8.resource.name)) {
                    FLOM_TRACE(("flom_resource_timestamp_inmsg: client wants "
                                "to unlock resource '%s' while it's locking "
                                "resource '%s'\n",
                                msg->body.unlock_8.resource.name,
                                flom_resource_get_name(resource)));
                    syslog(LOG_WARNING, FLOM_SYSLOG_FLM009W,
                           msg->body.unlock_8.resource.name,
                           flom_resource_get_name(resource));
                    THROW(INVALID_OPTION);
                }
                /* clean lock */
                if (FLOM_RC_OK != (ret_cod = flom_resource_timestamp_clean(
                                       resource, conn)))
                    THROW(RESOURCE_TIMESTAMP_CLEAN_ERROR);
                /* free the input message */
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(msg)))
                    THROW(MSG_FREE_ERROR2);
                flom_msg_init(msg);
                break;
            default:
                THROW(PROTOCOL_ERROR);
        } /* switch (msg->header.pvs.verb) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_FREE_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR1:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case RESOURCE_TIMESTAMP_GET_ERROR:
                break;
            case MSG_BUILD_ANSWER_ERROR1:
                break;
            case G_TRY_MALLOC_ERROR2:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case MSG_BUILD_ANSWER_ERROR2:
            case MSG_BUILD_ANSWER_ERROR3:
                break;
            case INVALID_OPTION:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case OBJ_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case RESOURCE_TIMESTAMP_CLEAN_ERROR:
            case MSG_FREE_ERROR2:
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_timestamp_inmsg/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_timestamp_clean(flom_resource_t *resource,
                                  flom_conn_t *conn)
{
    enum Exception { NULL_OBJECT
                     , TIMESTAMP_WAITINGS_ERROR
                     , INTERNAL_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_timestamp_clean\n"));
    TRY {
        GSList *p = NULL;

        if (NULL == resource)
            THROW(NULL_OBJECT);
        /* check if the connection keeps a lock */
        if (NULL != (p = flom_rsrc_conn_find(
                         resource->data.timestamp.holders,conn))) {
            struct flom_rsrc_conn_lock_s *cl =
                (struct flom_rsrc_conn_lock_s *)p->data;
            FLOM_TRACE(("flom_resource_timestamp_clean: the client is holding "
                        "a lock with timestamp %u, removing it...\n",
                        cl->info.timestamp_value));
            FLOM_TRACE(("flom_resource_timestamp_clean: cl=%p\n", cl));
            resource->data.timestamp.holders = g_slist_remove(
                resource->data.timestamp.holders, cl);
            resource->data.timestamp.locked_quantity--;
            /* free the now useless connection lock record */
            flom_rsrc_conn_lock_delete(cl);
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_timestamp_waitings(
                                   resource)))
                THROW(TIMESTAMP_WAITINGS_ERROR);
        } else {
            guint i = 0;
            /* check if the connection was waiting a lock */
            do {
                struct flom_rsrc_conn_lock_s *cl =
                    (struct flom_rsrc_conn_lock_s *)
                    g_queue_peek_nth(resource->data.timestamp.waitings, i);
                if (NULL == cl)
                    break;
                if (cl->conn == conn) {
                    /* remove from waitings */
                    FLOM_TRACE(("flom_resource_timestamp_clean: the client is "
                                "waiting for a lock, removing it...\n"));
                    cl = g_queue_pop_nth(resource->data.timestamp.waitings, i);
                    if (NULL == cl) {
                        /* this should be impossibile because peek was ok
                           some rows above */
                        THROW(INTERNAL_ERROR);
                    } else {
                        /* free the now useless connection lock record */
                        flom_rsrc_conn_lock_delete(cl);
                    }
                    break;
                } else
                    ++i;
            } while (TRUE);
        } /* if (NULL != p) */
                
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case TIMESTAMP_WAITINGS_ERROR:
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_timestamp_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



void flom_resource_timestamp_free(flom_resource_t *resource)
{
    /* clean-up format string */
    g_free(resource->data.timestamp.format);
    /* clean-up holders list... */
    FLOM_TRACE(("flom_resource_timestamp_free: "
                "cleaning-up holders list...\n"));
    while (NULL != resource->data.timestamp.holders) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)
            resource->data.timestamp.holders->data;
        resource->data.timestamp.holders = g_slist_remove(
            resource->data.timestamp.holders, cl);
        flom_rsrc_conn_lock_delete(cl);
    }
    resource->data.timestamp.holders = NULL;
    /* clean-up waitings queue... */
    FLOM_TRACE(("flom_resource_timestamp_free: cleaning-up waitings "
                "queue...\n"));
    while (!g_queue_is_empty(resource->data.timestamp.waitings)) {
        struct flom_rsrc_conn_lock_s *cl =
            (struct flom_rsrc_conn_lock_s *)g_queue_pop_head(
                resource->data.timestamp.waitings);
        flom_rsrc_conn_lock_delete(cl);
    }
    g_queue_free(resource->data.timestamp.waitings);
    
    resource->data.timestamp.waitings = NULL;
    resource->data.timestamp.total_quantity =
        resource->data.timestamp.locked_quantity = 0;
    /* releasing resource name */
    if (NULL != resource->name)
        g_free(resource->name);
    resource->name = NULL;
}



int flom_resource_timestamp_timeout(flom_resource_t *resource,
                                    struct timeval *next_deadline)
{
    enum Exception { QUEUE_IS_EMPTY
                     , TIMESTAMP_WAITINGS_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_resource_timestamp_timeout\n"));
    TRY {
        /* check if the queue is empty */
        if (NULL == g_queue_peek_head(resource->data.timestamp.waitings)) {
            FLOM_TRACE(("flom_resource_timestamp_timeout: waiting "
                        "connection queue is empty, leaving...\n"));
            THROW(QUEUE_IS_EMPTY);
        }
        if (flom_resource_timestamp_can_lock(resource)) {
            /* check if some other clients can get a lock now */
            if (FLOM_RC_OK != (ret_cod = flom_resource_timestamp_waitings(
                                   resource)))
                THROW(TIMESTAMP_WAITINGS_ERROR);
            *next_deadline = flom_resource_timestamp_next_deadline(resource);
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case QUEUE_IS_EMPTY:
                ret_cod = FLOM_RC_OK;
                break;
            case TIMESTAMP_WAITINGS_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_resource_timestamp_timeout/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_resource_timestamp_waitings(flom_resource_t *resource)
{
    enum Exception { INTERNAL_ERROR
                     , RESOURCE_TIMESTAMP_GET_ERROR
                     , MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    struct flom_rsrc_conn_lock_s *cl = NULL;
    
    FLOM_TRACE(("flom_resource_timestamp_waitings\n"));
    TRY {
        guint i = 0;
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        gchar element[1000]; /* it must contain a guint */
        
        /* check if there is any connection waiting for a lock */
        do {
            cl = (struct flom_rsrc_conn_lock_s *)
                g_queue_peek_nth(resource->data.timestamp.waitings, i);
            if (NULL == cl)
                break;
            /* try to apply this lock... */
            if (flom_resource_timestamp_can_lock(resource)) {
                /* remove from waitings */
                cl = g_queue_pop_nth(resource->data.timestamp.waitings, i);
                if (NULL == cl)
                    /* this should be impossibile because peek was ok
                       some rows above */
                    THROW(INTERNAL_ERROR);
                FLOM_TRACE(("flom_resource_timestamp_waitings: asked lock "
                            "can be assigned to connection %p\n",
                            cl->conn));
                if (FLOM_RC_OK != (ret_cod = flom_resource_timestamp_get(
                                       resource, &cl->info.timestamp_value,
                                       element, sizeof(element))))
                    THROW(RESOURCE_TIMESTAMP_GET_ERROR);
                /* send a message to the client that's waiting the lock */
                flom_msg_init(&msg);
                if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                                       &msg, FLOM_MSG_VERB_LOCK,
                                       3*FLOM_MSG_STEP_INCR,
                                       FLOM_RC_OK, element)))
                    THROW(MSG_BUILD_ANSWER_ERROR);
                if (FLOM_RC_OK != (
                        ret_cod = flom_msg_serialize(
                            &msg, buffer, sizeof(buffer), &to_send)))
                    THROW(MSG_SERIALIZE_ERROR);
                if (FLOM_RC_OK != (ret_cod = flom_conn_send(
                                       cl->conn, buffer, to_send)))
                    THROW(MSG_SEND_ERROR);
                flom_conn_set_last_step(cl->conn, msg.header.pvs.step);
                if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
                    THROW(MSG_FREE_ERROR);                
                /* insert into holders */
                resource->data.timestamp.holders = g_slist_prepend(
                    resource->data.timestamp.holders,
                    (gpointer)cl);
                resource->data.timestamp.locked_quantity++;
                cl = NULL;
            } else
                ++i;
        } while (TRUE);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case RESOURCE_TIMESTAMP_GET_ERROR:
            case MSG_BUILD_ANSWER_ERROR:
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != cl) {
        flom_rsrc_conn_lock_delete(cl);
    }
    FLOM_TRACE(("flom_resource_timestamp_waitings/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

