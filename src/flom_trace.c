/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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



#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_OPENSSL_SSL_H
# include <openssl/ssl.h>
#endif
#ifdef HAVE_OPENSSL_ERR_H
# include <openssl/err.h>
#endif
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



#include "flom_config.h"
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



void flom_trace_reopen(const char *file_name, int append)
{
    FILE *tmp_trace_file;
    if (NULL != file_name) {
        tmp_trace_file = fopen(file_name, append ? "a" : "w");
        if (NULL != tmp_trace_file) {
            /* only if different than stderr, the file must be closed */
            if (stderr != trace_file)
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
    if (nw1 < sizeof(buffer)) {
        /* custom message */
        vsnprintf(buffer+nw1, sizeof(buffer)-nw1, fmt, args);
        buffer[sizeof(buffer)-1] = '\0';
    }
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
                NULL != p->ai_canonname ? p->ai_canonname : FLOM_NULL_STRING);
        p = p->ai_next;
    }
    /* close trace record */
    fprintf(trace_file, "\n");
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}



#ifdef HAVE_GETIFADDRS
/* getifaddrs is not POSIX and we can not be sure it's available */
void flom_trace_ifaddrs(const char *prefix, const struct ifaddrs *ifaddr)
{
    const struct ifaddrs *ifa;
    int n;
    char *new_prefix = NULL;
    gsize new_prefix_size;
    
    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;    
    /* lock the mutex */
    for (ifa = ifaddr, n=0; NULL != ifa; ifa=ifa->ifa_next,n++) {
        socklen_t addrlen;
        
        if (NULL == ifa->ifa_addr)
            continue;
        switch (ifa->ifa_addr->sa_family) {
            case AF_INET:
                addrlen = sizeof(struct sockaddr_in);
                break;
            case AF_INET6:
                addrlen = sizeof(struct sockaddr_in6);
                break;
            default:
                continue;
        } /* switch (ifa->ifa_addr->sa_family) */
        new_prefix_size = strlen(prefix) + strlen(ifa->ifa_name) + 100;
        new_prefix = g_malloc(new_prefix_size);
        snprintf(new_prefix, new_prefix_size, "%s ifa_name='%s'; ",
                 prefix, ifa->ifa_name);
        /* dump sockaddr structure */
        flom_trace_sockaddr(new_prefix, ifa->ifa_addr, addrlen);
        g_free(new_prefix);
        new_prefix = NULL;
    } /* for (ifa = ifaddrs, n=0; */
    if (NULL != new_prefix)
        g_free(new_prefix);
}
#endif /* HAVE_GETIFADDRS */



void flom_trace_sockaddr(const char *prefix, const struct sockaddr *addr,
                         socklen_t addrlen)
{
    int trace_hex = FALSE;
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
    if (NULL == addr) {
        fprintf(trace_file, " addr is NULL\n");
    } else {
        struct sockaddr_in sin;
        struct sockaddr_in6 sin6;
        char sin_addr[INET6_ADDRSTRLEN];
        fprintf(trace_file, "addrlen=" SOCKLEN_T_FORMAT ";", addrlen);
        switch (addr->sa_family) {
            case AF_INET:
                if (sizeof(sin) != addrlen) {
                    fprintf(trace_file, " IPv4, invalid length structure "
                            "(" SOCKLEN_T_FORMAT "/" SIZE_T_FORMAT ")",
                            addrlen, sizeof(sin));
                    trace_hex = TRUE;
                } else {
                    fprintf(trace_file, " IPv4 address");
                    memcpy(&sin, addr, addrlen);
                    fprintf(trace_file, ", sin_port=" IN_PORT_T_FORMAT,
                            htons(sin.sin_port));
                    inet_ntop(AF_INET, &sin.sin_addr, sin_addr,
                              sizeof(sin_addr));
                    fprintf(trace_file, ", sin_addr='%s'", sin_addr);
                }
                break;
            case AF_INET6:
                if (sizeof(sin6) != addrlen) {
                    fprintf(trace_file, "IPv6, invalid length structure "
                            "(" SOCKLEN_T_FORMAT "/" SIZE_T_FORMAT ")",
                            addrlen, sizeof(sin6));
                    trace_hex = TRUE;
                } else {
                    fprintf(trace_file, " IPv6 address");
                    memcpy(&sin6, addr, addrlen);
                    fprintf(trace_file, ", sin6_port=" IN_PORT_T_FORMAT,
                            htons(sin6.sin6_port));
                    fprintf(trace_file, ", sin6_flowinfo=0x%x",
                            sin6.sin6_flowinfo);
                    inet_ntop(AF_INET6, &sin6.sin6_addr, sin_addr,
                              sizeof(sin_addr));
                    fprintf(trace_file, ", sin6_addr='%s'", sin_addr);
                    fprintf(trace_file, ", sin6_scope_id=%u",
                            sin6.sin6_scope_id);
                }
                break;
            default:
                fprintf(trace_file, " unknown family, sa_family=%d, "
                        "addrlen=%d", addr->sa_family, addrlen);
                trace_hex = TRUE;
                break;
        } /* switch (addr->sa_family) */
    } /* if (NULL == addr) */
    /* close trace record */
    fprintf(trace_file, "\n");
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
    /* hex dump if necessary */
    if (trace_hex)
        flom_trace_hex_data(prefix, (byte_t *)addr, addrlen);
}



void flom_trace_sslerr(const char *prefix, unsigned long err)
{
    struct tm broken_time;
    struct timeval tv;
    
    /* trace is closed, skipping it! */
    if (NULL == trace_file)
        return;    
    /* lock the mutex */
    g_static_mutex_lock(&flom_trace_mutex);
    /* loop on errors */
    if (SSL_ERROR_NONE != err) {
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
        char buf[1024];
        ERR_error_string_n(err, buf, sizeof(buf));
        fprintf(trace_file, " %s\n", buf);
    } /* if (SSL_ERROR_NONE != err) */
    /* remove the lock from mutex */
    g_static_mutex_unlock(&flom_trace_mutex);
}
