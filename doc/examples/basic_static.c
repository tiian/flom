/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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
 * a statically allocated handle.
 * These are the steps:
 * 1. declare an object of type flom_handle_t inside process stack
 * 2. initialize the allocated handle using function flom_handle_init()
 * 3. acquire a lock using function flom_handle_lock()
 * 4. execute the code protected by the acquired lock
 * 5. release the lock using function flom_handle_unlock()
 * 6. clean-up the allocated handle using function flom_handle_clean()
 *
 * Compilation command:
 *     make -f example_makefile basic_static
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./basic_static
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x8000
 * ./basic_static
 */



int main(int argc, char *argv[]) {
    int ret_cod;
    /* step 1: handle declaration */
    flom_handle_t my_handle;
    
    /* step 2: handle initialization */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "flom_handle_init() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }    
    /* step 3: lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(&my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 4: execute the code that needs lock protection */
    
    /* step 5: lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(&my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* step 6: handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "flom_handle_clean() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* exit */
    return 0;
}
