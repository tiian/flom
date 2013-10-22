/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
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



#include "flom_conns.h"
#include "flom_errors.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_CONNS



int flom_conns_init(flom_conns_t *conns, int domain)
{
    enum Exception { MALLOC_ERROR1
                     , MALLOC_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_init\n"));
    TRY {
        void *tmp;
        nfds_t i;
        
        /* reset */
        conns->allocated = 0;
        conns->used = 0;
        conns->fds = NULL;
        conns->domain = domain;
        conns->addr = NULL;
        /* allocate with default size */
        if (NULL == (tmp = malloc(sizeof(struct pollfd) *
                                  FLOM_CONNS_DEFAULT_ALLOCATION)))
            THROW(MALLOC_ERROR1);
        /* reset the content of the array */
        memset(tmp, 0, sizeof(struct pollfd) *
               FLOM_CONNS_DEFAULT_ALLOCATION);
        conns->fds = (struct pollfd *)tmp;
        for (i=0; i<FLOM_CONNS_DEFAULT_ALLOCATION; ++i)
            conns->fds[i].fd = NULL_FD;
        if (NULL == (tmp = malloc(sizeof(struct flom_addr_s) *
                                  FLOM_CONNS_DEFAULT_ALLOCATION)))
            THROW(MALLOC_ERROR2);
        /* reset the content of the array */
        memset(tmp, 0, sizeof(struct flom_addr_s) *
               FLOM_CONNS_DEFAULT_ALLOCATION);
        conns->addr = (struct flom_addr_s *)tmp;
        conns->allocated = FLOM_CONNS_DEFAULT_ALLOCATION;
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MALLOC_ERROR1:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case MALLOC_ERROR2:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_init/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_add(flom_conns_t *conns, int fd,
                   socklen_t addr_len, const struct sockaddr *sa)
{
    enum Exception { CONNS_EXPAND_ERROR
                     , INVALID_DOMAIN
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_add\n"));
    TRY {
        if (conns->used == conns->allocated) {
            if (FLOM_RC_OK != (ret_cod = flom_conns_expand(conns)))
                THROW(CONNS_EXPAND_ERROR);
        }
        conns->fds[conns->used].fd = fd;
        conns->fds[conns->used].events = 0;
        conns->fds[conns->used].revents = 0;
        conns->addr[conns->used].addr_len = addr_len;
        switch (conns->domain) {
            case AF_UNIX:
                conns->addr[conns->used].saun = *((struct sockaddr_un *)sa);
                break;
            case AF_INET:
                conns->addr[conns->used].sain = *((struct sockaddr_in *)sa);
                break;
            default:
                THROW(INVALID_DOMAIN);
        }
        conns->used++;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_EXPAND_ERROR:
                break;
            case INVALID_DOMAIN:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_add/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_set_events(flom_conns_t *conns, short events)
{
    enum Exception { OBJECT_CORRUPTED
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_set_events\n"));
    TRY {
        nfds_t i;
        for (i=0; i<conns->used; ++i) {
            if (NULL_FD != conns->fds[i].fd)
                conns->fds[i].events = events;
            else {
                FLOM_TRACE(("flom_conns_set_events: i=%d, "
                            "conns->fds[i].fd=%d\n", i, conns->fds[i].fd));
                THROW(OBJECT_CORRUPTED);
            }
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OBJECT_CORRUPTED:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_set_events/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_expand(flom_conns_t *conns)
{
    enum Exception { INTERNAL_ERROR
                     , MALLOC_ERROR1
                     , MALLOC_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_expand\n"));
    TRY {
        void *tmp;
        nfds_t i;
        nfds_t new_allocated =
            conns->allocated * FLOM_CONNS_STEP_ALLOCATION + 1;
        if (conns->allocated >= new_allocated) {
            FLOM_TRACE(("flom_conns_expand: conns->allocated=%d, "
                        "new_allocated=%d\n", conns->allocated, new_allocated));
            THROW(INTERNAL_ERROR);
        }
        FLOM_TRACE(("flom_conns_expand: expanding from %d to %d connections\n",
                    conns->allocated, new_allocated));
        if (NULL == (tmp = realloc(conns->fds, sizeof(struct pollfd) *
                                   new_allocated)))
            THROW(MALLOC_ERROR1);
        /* reset the content of the new allocated memory */
        memset(tmp + conns->allocated * sizeof(struct pollfd), 0,
               (new_allocated - conns->allocated) * sizeof(struct pollfd));
        conns->fds = (struct pollfd *)tmp;
        for (i=conns->allocated; i<new_allocated; ++i)
            conns->fds[i].fd = NULL_FD;
        if (NULL == (tmp = realloc(conns->addr, sizeof(struct flom_addr_s) *
                                   new_allocated)))
            THROW(MALLOC_ERROR2);
        /* reset the content of the new allocated memory */
        memset(tmp + conns->allocated * sizeof(struct flom_addr_s), 0,
               (new_allocated - conns->allocated) * sizeof(struct flom_addr_s));
        conns->addr = (struct flom_addr_s *)tmp;
        conns->allocated = new_allocated;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case MALLOC_ERROR1:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case MALLOC_ERROR2:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_expand/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_close_fd(flom_conns_t *conns, nfds_t id)
{
    enum Exception { OUT_OF_RANGE
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_close_fd\n"));
    TRY {
        FLOM_TRACE(("flom_conns_close: closing connection id=%d with fd=%d\n",
                    id, conns->fds[id].fd));
        if (id < 0 || id >= conns->used)
            THROW(OUT_OF_RANGE);
        if (0 != close(conns->fds[id].fd))
            THROW(CLOSE_ERROR);
        conns->fds[id].fd = NULL_FD;

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OUT_OF_RANGE:
                ret_cod = FLOM_RC_OUT_OF_RANGE;
                break;
            case CLOSE_ERROR:
                ret_cod = FLOM_RC_CLOSE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_conns_close_fd/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_conns_clean(flom_conns_t *conns)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_clean\n"));
    TRY {
        nfds_t i=0;
        while (i<conns->used) {
            nfds_t last = conns->used-1;
            FLOM_TRACE(("flom_conns_clean: i=%d, fd=%d %s\n",
                        i, conns->fds[i].fd,
                       NULL_FD == conns->fds[i].fd ? "(removing...)" : ""));
            if (NULL_FD == conns->fds[i].fd) {
                if (i != last) {
                    /* moving last connection to this position */
                    conns->fds[i] = conns->fds[last];
                    conns->addr[i] = conns->addr[last];
                }
                conns->used--;
            } else i++;
        }
        
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
    FLOM_TRACE(("flom_conns_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}

