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



#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include "flom_trace.h"



int flom_trace_initialized = FALSE;

unsigned long flom_trace_mask = 0;

FILE *trace_file = NULL;


/**
 * This mutex is used to avoid contention (bad output) on trace file
 */
GStaticMutex flom_trace_mutex = G_STATIC_MUTEX_INIT;



/**
 * Initialize the library when the library is loaded.
 */
void flom_trace_init(void)
{
    /* initialize thread system if necessary */
    if (!g_thread_supported ()) g_thread_init(NULL);

    /* lock the mutex */
    g_static_mutex_lock(&flom_trace_mutex);
    if (!flom_trace_initialized) {
        /* retrieve environemnt variable */
        if (getenv(FLOM_TRACE_MASK_ENV_VAR) != NULL)
            flom_trace_mask = strtoul(
                getenv(FLOM_TRACE_MASK_ENV_VAR), NULL, 0);
        else
            flom_trace_mask = 0x0;
        trace_file = stderr;
        flom_trace_initialized = TRUE;
    }
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}



void flom_trace_reopen(const char *file_name)
{
    FILE *tmp_trace_file;
    if (NULL != file_name) {
        tmp_trace_file = fopen(file_name, "w");
        if (NULL != tmp_trace_file) {
            fclose(trace_file);
            trace_file = tmp_trace_file;
        }
    } else {
        /* close trace! */
        fclose(trace_file);
        trace_file = NULL;
    }
}



void flom_trace(const char *fmt, ...)
{
    va_list args;
    struct tm broken_time;
    struct timeval tv;
    char buffer[2000];
    int nw1;

    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;
    
    va_start(args, fmt);
#ifdef HAVE_VSNPRINTF
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &broken_time);
    /* default header */
    nw1 = snprintf(
        buffer, sizeof(buffer), 
        "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%6.6d [" PID_T_FORMAT "/%p] ",
        broken_time.tm_year + 1900, broken_time.tm_mon + 1,
        broken_time.tm_mday, broken_time.tm_hour,
        broken_time.tm_min, broken_time.tm_sec, (int)tv.tv_usec,
        getpid(), g_thread_self());
    if (nw1 < sizeof(buffer))
        /* custom message */
        vsnprintf(buffer+nw1, sizeof(buffer)-nw1, fmt, args);
    fputs(buffer, trace_file);
    fflush(trace_file);
#else
# error "vsnprintf is necessary for flom_trace function!"
#endif
    va_end(args);
}



void flom_trace_hex_data(const char *prefix, const byte_t *data, size_t size)
{
    size_t i;
    struct tm broken_time;
    struct timeval tv;
    
    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;
    
    /* lock the mutex */
    g_static_mutex_lock(&flom_trace_mutex);
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &broken_time);
    /* default header */
    fprintf(trace_file,
            "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%6.6d [" PID_T_FORMAT
            "/%p] %s",
            broken_time.tm_year + 1900, broken_time.tm_mon + 1,
            broken_time.tm_mday, broken_time.tm_hour,
            broken_time.tm_min, broken_time.tm_sec, (int)tv.tv_usec,
            getpid(), g_thread_self(), prefix);
    /* dump data */
    for (i = 0; i < size; ++i) {
        fprintf(trace_file, "%02x ", (data[i] & 0xff));
    } /* for (i = 0; i < size; ++i) */
    /* close trace record */
    fprintf(trace_file, "\n");
    fflush(trace_file);
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}



void flom_trace_text_data(const char *prefix, const byte_t *data, size_t size)
{
    size_t i;
    struct tm broken_time;
    struct timeval tv;
    
    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;
    
    /* lock the mutex */
    g_static_mutex_lock(&flom_trace_mutex);
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &broken_time);
    /* default header */
    fprintf(trace_file,
            "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%6.6d ["
            PID_T_FORMAT "/%p] %s",
            broken_time.tm_year + 1900, broken_time.tm_mon + 1,
            broken_time.tm_mday, broken_time.tm_hour,
            broken_time.tm_min, broken_time.tm_sec, (int)tv.tv_usec,
            getpid(), g_thread_self(), prefix);
    /* dump data */
    for (i = 0; i < size; ++i) {
        if (data[i] >= (byte_t)' ' && data[i] < (byte_t)0x80)
            putc((int)(data[i] & 0xff), trace_file);
        else
            putc((int)' ', trace_file);
    } /* for (i = 0; i < size; ++i) */
    /* close trace record */
    fprintf(trace_file, "\n");
    fflush(trace_file);
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}



void flom_trace_addrinfo(const char *prefix, const struct addrinfo *p)
{
    struct tm broken_time;
    struct timeval tv;
    
    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;
    
    /* lock the mutex */
    g_static_mutex_lock(&flom_trace_mutex);
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &broken_time);
    /* default header */
    fprintf(trace_file,
            "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d.%6.6d ["
            PID_T_FORMAT "/%p] %s",
            broken_time.tm_year + 1900, broken_time.tm_mon + 1,
            broken_time.tm_mday, broken_time.tm_hour,
            broken_time.tm_min, broken_time.tm_sec, (int)tv.tv_usec,
            getpid(), g_thread_self(), prefix);
    /* dump data */
    while (NULL != p) {
        fprintf(trace_file, "[ai_flags=%d,ai_family=%d,ai_socktype=%d,"
                "ai_protocol=%d,ai_addrlen=%u,ai_canonname='%s'] ",
                p->ai_flags, p->ai_family, p->ai_socktype, p->ai_protocol,
                p->ai_addrlen,
                NULL != p->ai_canonname ? p->ai_canonname : "");
        p = p->ai_next;
    }
    /* close trace record */
    fprintf(trace_file, "\n");
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}
