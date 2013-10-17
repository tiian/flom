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



int flom_conns_init(flom_conns_t *fc)
{
    enum Exception { MALLOC_ERROR1
                     , MALLOC_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_conns_init\n"));
    TRY {
        void *tmp;
        
        /* reset */
        fc->allocated = 0;
        fc->used = 0;
        fc->fds = NULL;
        fc->domain = 0;
        fc->addr = NULL;
        /* allocate with default size */
        if (NULL == (tmp = malloc(sizeof(struct pollfd) *
                                  FLOM_CONNS_DEFAULT_ALLOCATION)))
            THROW(MALLOC_ERROR1);
        fc->fds = (struct pollfd *)tmp;
        if (NULL == (tmp = malloc(sizeof(struct flom_addr_s) *
                                  FLOM_CONNS_DEFAULT_ALLOCATION)))
            THROW(MALLOC_ERROR2);
        fc->addr = (struct flom_addr_s *)tmp;
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

