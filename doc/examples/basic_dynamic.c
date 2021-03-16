/*
 * Copyright (c) 2013-2021, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>

#include "flom.h"



/*
 * This example program shows the basic usage of libflom API library with
 * a dynamically allocated handle.
 * These are the steps:
 * 1. declare a pointer for type flom_handle_t
 * 2. create (allocate and initialize) a new handle using function
 *    flom_handle_new()
 * 3. acquire a lock using function flom_handle_lock()
 * 4. execute the code protected by the acquired lock
 * 5. release the lock using function flom_handle_unlock()
 * 6. delete (clean-up and deallocate) the handle using function
 *    flom_handle_delete()
 *
 * Compilation command:
 *     make -f example_makefile basic_dynamic
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./basic_dynamic
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x80000
 * ./basic_dynamic
 */



int main(int argc, char *argv[]) {
    int ret_cod;
    /* step 1: handle declaration */
    flom_handle_t *my_handle = NULL;

    /* step 2: new handle creation */
    if (NULL == (my_handle = flom_handle_new())) {
        fprintf(stderr, "flom_handle_init() returned %p\n", my_handle);
        exit(1);
    }    
    /* step 3: lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 4: execute the code that needs lock protection */
    
    /* step 5: lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* step 6: delete the handle */
    flom_handle_delete(my_handle);
    /* exit */
    return 0;
}
