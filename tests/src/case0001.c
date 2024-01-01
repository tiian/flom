/*
 * Copyright (c) 2013-2024, Christian Ferrari <tiian@users.sourceforge.net>
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
 * Happy path usage with a static handle
 */
void static_handle_happy_path(void) {
    int ret_cod;
    flom_handle_t my_handle;
    
    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "static_handle_happy_path/flom_handle_init() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(&my_handle))) {
        fprintf(stderr, "static_handle_happy_path/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(&my_handle))) {
        fprintf(stderr, "static_handle_happy_path/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "static_handle_happy_path/flom_handle_clean() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
}



/*
 * Stress test with a static handle, missing flom_handle_init method
 */
void static_handle_missing_init(void) {
    int ret_cod;
    flom_handle_t my_handle;

    /* lock acquisition */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            ret_cod = flom_handle_lock(&my_handle))) {
        fprintf(stderr, "static_handle_missing_init/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }     
    /* lock release */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            ret_cod = flom_handle_unlock(&my_handle))) {
        fprintf(stderr, "static_handle_missing_init/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* handle clean-up (memory release) */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "static_handle_missing_init/flom_handle_clean() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
}



/*
 * Stress test with a static handle, missing flom_handle_lock method
 */
void static_handle_missing_lock(void) {
    int ret_cod;
    flom_handle_t my_handle;

    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "static_handle_missing_lock/flom_handle_init() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* lock release */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            ret_cod = flom_handle_unlock(&my_handle))) {
        fprintf(stderr, "static_handle_missing_lock/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "static_handle_missing_lock/flom_handle_clean() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
}



/*
 * Stress test with a static handle, missing flom_handle_unlock method
 */
void static_handle_missing_unlock(void) {
    int ret_cod;
    flom_handle_t my_handle;

    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "static_handle_missing_unlock/flom_handle_init() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(&my_handle))) {
        fprintf(stderr, "static_handle_missing_unlock/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "static_handle_missing_unlock/flom_handle_clean() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
}



/*
 * Happy path usage with a dynamic handle
 */
void dynamic_handle_happy_path(void) {
    int ret_cod;
    flom_handle_t *my_handle = NULL;

    /* create a new handle */
    if (NULL == (my_handle = flom_handle_new())) {
        fprintf(stderr, "dynamic_handle_happy_path/flom_handle_init() "
                "returned %p\n", my_handle);
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "dynamic_handle_happy_path/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "dynamic_handle_happy_path/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* delete the handle */
    flom_handle_delete(my_handle);
}



/*
 * Stress test with a dynamic handle, missing flom_handle_new method
 */
void dynamic_handle_missing_new(void) {
    int ret_cod;
    flom_handle_t *my_handle = NULL;

    /* lock acquisition */
    if (FLOM_RC_NULL_OBJECT != (
            ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "dynamic_handle_missing_new/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_NULL_OBJECT != (
            ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "dynamic_handle_missing_new/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* delete the handle */
    flom_handle_delete(my_handle);
}



/*
 * Stress test with a dynamic handle, missing flom_handle_lock method
 */
void dynamic_handle_missing_lock(void) {
    int ret_cod;
    flom_handle_t *my_handle = NULL;

    /* create a new handle */
    if (NULL == (my_handle = flom_handle_new())) {
        fprintf(stderr, "dynamic_handle_missing_lock/flom_handle_init() "
                "returned %p\n", my_handle);
        exit(1);
    }    
    /* lock release */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "dynamic_handle_missing_lock/flom_handle_unlock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* delete the handle */
    flom_handle_delete(my_handle);
}



void dynamic_handle_missing_unlock(void) {
    int ret_cod;
    flom_handle_t *my_handle = NULL;

    /* create a new handle */
    if (NULL == (my_handle = flom_handle_new())) {
        fprintf(stderr, "dynamic_handle_missing_unlock/flom_handle_init() "
                "returned %p\n", my_handle);
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "dynamic_handle_missing_unlock/flom_handle_lock() "
                "returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* delete the handle */
    flom_handle_delete(my_handle);
}



int main(int argc, char *argv[]) {
    /* static handle tests */
    static_handle_happy_path();
    static_handle_missing_init();
    static_handle_missing_lock();
    static_handle_missing_unlock();
    /* dynamic handle test */
    dynamic_handle_happy_path();
    dynamic_handle_missing_new();
    dynamic_handle_missing_lock();
    dynamic_handle_missing_unlock();
    /* exit */
    return 0;
}
