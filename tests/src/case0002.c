/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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
    char locked_element[100];

    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "flom_handle_init() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get current AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(&my_handle));
    /* set a new AF_UNIX/PF_LOCAL socket_name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           &my_handle, "/tmp/flom_socket_name"))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(&my_handle));
    /* get current trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(&my_handle));
    /* set a new trace filename */
    flom_handle_set_trace_filename(&my_handle, "/tmp/flom.trc");
    /* get new trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(&my_handle));
    /* get current resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* set a new resource name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           &my_handle, "myResource"))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* get current value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(&my_handle, FALSE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(&my_handle, TRUE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* get current value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(&my_handle));
    /* set a new value for resource timeout property */
    flom_handle_set_resource_timeout(&my_handle, -1);
    /* get new value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(&my_handle));
    /* get current value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(&my_handle));
    /* set a new value for resource quantity property */
    flom_handle_set_resource_quantity(&my_handle, 3);
    /* get new value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(&my_handle));
    /* get current value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(&my_handle));
    /* set a new value for resource lock mode property */
    flom_handle_set_lock_mode(&my_handle, FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(&my_handle));
    /* get current value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(&my_handle));
    /* set a new value for resource idle lifespan */
    flom_handle_set_resource_idle_lifespan(&my_handle, 10000);
    /* get new value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(&my_handle));
    /* get current unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(&my_handle));
    /* set a new unicast_address */
    flom_handle_set_unicast_address(&my_handle, "127.0.0.1");
    /* get new unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(&my_handle));
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           &my_handle, "/tmp/flom_socket_name"))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }    
    /* get current value for unicast port */
    printf("flom_handle_get_unicast_port() = %d\n",
           flom_handle_get_unicast_port(&my_handle));
    /* set a new value for unicast_port */
    flom_handle_set_unicast_port(&my_handle, 7777);
    /* get new value for unicast port */
    printf("flom_handle_get_unicast_port() = %d\n",
           flom_handle_get_unicast_port(&my_handle));
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(&my_handle, locked_element,
                                           sizeof(locked_element)))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(&my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "flom_handle_clean() returned %d, '%s'\n",
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
    char locked_element[100];

    /* create a new handle */
    if (NULL == (my_handle = flom_handle_new())) {
        fprintf(stderr, "flom_handle_init() returned %p\n", my_handle);
        exit(1);
    }    
    /* get current AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(my_handle));
    /* set a new AF_UNIX/PF_LOCAL socket_name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           my_handle, "/tmp/flom_socket_name"))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(my_handle));
    /* get current trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(my_handle));
    /* set a new trace filename */
    flom_handle_set_trace_filename(my_handle, "/tmp/flom.trc");
    /* get new trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(my_handle));
    /* get current resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(my_handle));
    /* set a new resource name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           my_handle, "myResource"))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(my_handle));
    /* get current value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(my_handle, FALSE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(my_handle, TRUE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* get current value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(my_handle));
    /* set a new value for resource timeout property */
    flom_handle_set_resource_timeout(my_handle, -1);
    /* get new value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(my_handle));
    /* get current value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(my_handle));
    /* set a new value for resource quantity property */
    flom_handle_set_resource_quantity(my_handle, 3);
    /* get new value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(my_handle));
    /* get current value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(my_handle));
    /* set a new value for resource lock mode property */
    flom_handle_set_lock_mode(my_handle, FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(my_handle));
    /* get current value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(my_handle));
    /* set a new value for resource idle lifespan */
    flom_handle_set_resource_idle_lifespan(my_handle, 10000);
    /* get new value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(my_handle));
    /* get current unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(my_handle));
    /* set a new unicast_address */
    flom_handle_set_unicast_address(my_handle, "127.0.0.1");
    /* get new unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(my_handle));
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           my_handle, "/tmp/flom_socket_name"))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }    
    /* get current value for unicast port */
    printf("flom_handle_get_unicast_port() = %d\n",
           flom_handle_get_unicast_port(my_handle));
    /* set a new value for unicast_port */
    flom_handle_set_unicast_port(my_handle, 7777);
    /* get new value for unicast port */
    printf("flom_handle_get_unicast_port() = %d\n",
           flom_handle_get_unicast_port(my_handle));
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle, locked_element,
                                                  sizeof(locked_element)))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_handle_unlock(my_handle))) {
        fprintf(stderr, "flom_handle_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* delete the handle */
    flom_handle_delete(my_handle);
}



int main(int argc, char *argv[]) {
    /* static handle tests */
    static_handle_happy_path();
    /* dynamic handle test */
    dynamic_handle_happy_path();
    /* exit */
    return 0;
}
