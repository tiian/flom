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
 * This example program shows the usage of libflom API library with
 * a transactional resource: a unique transactional sequence.
 * These are the steps:
 * 1. declare a pointer for type flom_handle_t
 * 2. create (allocate and initialize) a new handle using function
 *    flom_handle_new()
 * 3a. set a non default resource name (a name valid for a trasactional
 *     sequence resource)
 * 3b. set a non default resource idle lifespan
 * 4. acquire a lock using function flom_handle_lock()
 * 5. release the lock using function flom_handle_unlock_rollback()
 * 6. acquire a new lock using function flom_handle_lock() and verifying the
 *    FLoM daemon returnes the same value
 * 7. release the lock using function flom_handle_unlock_rollback()
 * 8. acquire a new lock using function flom_handle_lock() and verifying the
 *    FLoM daemon returnes a different value
 * 9. sleep 5 seconds to allow program killing
 * 10. release the lock using function flom_handle_unlock_rollback()
 * 11. delete (clean-up and deallocate) the handle using function
 *     flom_handle_delete()
 *
 * Compilation command:
 *     make -f example_makefile transactional
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./transactional
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x80000
 * ./transactional
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
    
    /* step 3a: set a different (non default) resource name to lock */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           my_handle, "_S_transact[1]"))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* step 3b: set a different (non default) resource idle lifespan */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_idle_lifespan(
                           my_handle, 60000))) {
        fprintf(stderr, "flom_handle_set_resource_idle_lifespan() returned "
                "%d, '%s'\n", ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 4: lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle)) {
        printf("flom_handle_get_locked_element(): '%s' (first lock)\n",
               flom_handle_get_locked_element(my_handle));
    } 
    
    /* step 5: lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock_rollback(my_handle))) {
        fprintf(stderr, "flom_handle_unlock_rollback() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 6: lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle)) {
        printf("flom_handle_get_locked_element(): '%s' (second lock)\n",
               flom_handle_get_locked_element(my_handle));
    } 
    
    /* step 7: lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 8: lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle)) {
        printf("flom_handle_get_locked_element(): '%s' (third lock)\n",
               flom_handle_get_locked_element(my_handle));
    } 

    /* step 9: sleep 5 seconds to allow program killing */
    printf("The program is waiting 5 seconds: kill it with the [control]+[c] "
           "keystroke and restart it to verify resource rollback...\n");
    sleep(5);
    
    /* step 10: lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    
    /* step 11: delete the handle */
    flom_handle_delete(my_handle);
    
    /* exit */
    return 0;
}
