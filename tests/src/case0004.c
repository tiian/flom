/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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



int main(int argc, char *argv[]) {
    int ret_cod;
    flom_handle_t *my_handle1 = NULL; /* used for non transactional resource */
    flom_handle_t *my_handle2 = NULL; /* used for transactional resource */

    /* First step: non transactional resource */
    /* create a new handle */
    if (NULL == (my_handle1 = flom_handle_new())) {
        fprintf(stderr, "flom_handle_init() returned %p\n", my_handle1);
        exit(1);
    }
    /* setting the resource name: non transactional sequence */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           my_handle1, "_s_nontransactional[1]"))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* set a new value for resource idle lifespan */
    flom_handle_set_resource_idle_lifespan(my_handle1, 60000);
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle1))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle1)) {
        printf("locked element is %s\n",
               flom_handle_get_locked_element(my_handle1));
    } 
    /* lock release & rollback: the resource is not transactional, the
     * function must return a warning condition */
    if (FLOM_RC_RESOURCE_IS_NOT_TRANSACTIONAL != (
            ret_cod = flom_handle_unlock_rollback(my_handle1))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle1))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle1)) {
        printf("locked element is %s\n",
               flom_handle_get_locked_element(my_handle1));
    } 
    /* the resource associated to my_handle1 is intentionally not unlocked
     * to check the behavior in case of abort */

    /* Second step: transactional resource */
    /* create a new handle */
    if (NULL == (my_handle2 = flom_handle_new())) {
        fprintf(stderr, "flom_handle_init() returned %p\n", my_handle2);
        exit(1);
    }
    /* setting the resource name: non transactional sequence */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           my_handle2, "_S_transactional[1]"))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* set a new value for resource idle lifespan */
    flom_handle_set_resource_idle_lifespan(my_handle2, 60000);
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle2))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle2)) {
        printf("locked element is %s\n",
               flom_handle_get_locked_element(my_handle2));
    } 
    /* lock release & rollback: the resource is transactional, the
     * function must NOT return a warning condition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock_rollback(my_handle2))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle2))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle2)) {
        printf("locked element is %s\n",
               flom_handle_get_locked_element(my_handle2));
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle2))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle2))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle2)) {
        printf("locked element is %s\n",
               flom_handle_get_locked_element(my_handle2));
    }
    /* interrupt execution to verify transactionality (the program must be
     * restarted */
    abort();
    /* this point will be never reached! */
}
