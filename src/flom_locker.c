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



#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_locker.h"
#include "flom_trace.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_LOCKER



void flom_locker_destroy(struct flom_locker_s *locker)
{
    if (NULL != locker) {
        g_free(locker->resource_name);
        close(locker->write_pipe);
        close(locker->read_pipe);
        g_free(locker);
    }
}



void flom_locker_array_init(flom_locker_array_t *lockers)
{
    lockers->n = 0;
    lockers->array = g_ptr_array_new_with_free_func(
        (GDestroyNotify)flom_locker_destroy);
}



void flom_locker_array_add(flom_locker_array_t *lockers,
                           struct flom_locker_s *locker)
{
    g_ptr_array_add(lockers->array, (gpointer)locker);
    lockers->n++;
}



gpointer flom_locker_loop(gpointer data)
{
    FLOM_TRACE(("flom_locker_loop: new thread in progress...\n"));
    /* @@@ */
    FLOM_TRACE(("flom_locker_loop: this thread completed service\n"));
    return data;
}
