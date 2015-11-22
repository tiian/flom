/*
 * Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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
#include <string.h>

#include "flom.h"



/*
 * Non default values used for tests
 */
const char *nd_socket_name = "/tmp/flom_socket_name";
const char *nd_trace_filename = "/tmp/flom.trc";
const char *nd_resource_name = "red.green.blue";
const char *nd_unicast_address = "127.0.0.1";
const char *nd_multicast_address = "224.0.0.1";



/*
 * Happy path usage with a static handle
 */
void static_handle_happy_path(const char *nd_network_interface) {
    int ret_cod;
    flom_handle_t my_handle;

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
                           &my_handle, nd_socket_name))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(&my_handle));
    /* check socket name */
    if (strcmp(nd_socket_name, flom_handle_get_socket_name(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_socket_name\n");
        exit(1);
    }

    /* we don't get current trace filename because it can be altered by a
       global config file */
    /* set a new trace filename */
    flom_handle_set_trace_filename(&my_handle, nd_trace_filename);
    /* get new trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(&my_handle));
    /* check trace filename */
    if (strcmp(nd_trace_filename,
               flom_handle_get_trace_filename(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_trace_filename\n");
        exit(1);
    }
    
    /* get current resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* set a new resource name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           &my_handle, nd_resource_name))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* check resource name */
    if (strcmp(nd_resource_name,
               flom_handle_get_resource_name(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_resource_name\n");
        exit(1);
    }
    
    /* get current value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(&my_handle, FALSE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* check resource create 1/2 */
    if (flom_handle_get_resource_create(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_create\n");
        exit(1);
    }
    /* set a new value for resource create property */
    flom_handle_set_resource_create(&my_handle, TRUE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(&my_handle));
    /* check resource create 2/2 */
    if (!flom_handle_get_resource_create(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
		"get_resource_create\n");
        exit(1);
    }
    
    /* get current value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(&my_handle));
    /* set a new value for resource timeout property */
    flom_handle_set_resource_timeout(&my_handle, -1);
    /* get new value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(&my_handle));
    /* check resource timeout */
    if (-1 != flom_handle_get_resource_timeout(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_timeout\n");
        exit(1);
    }
    
    /* get current value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(&my_handle));
    /* set a new value for resource quantity property */
    flom_handle_set_resource_quantity(&my_handle, 3);
    /* get new value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(&my_handle));
    /* check resource quantity */
    if (3 != flom_handle_get_resource_quantity(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_quantity\n");
        exit(1);
    }
    
    /* get current value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(&my_handle));
    /* set a new value for resource lock mode property */
    flom_handle_set_lock_mode(&my_handle, FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(&my_handle));
    /* check resource lock mode */
    if (FLOM_LOCK_MODE_PW != flom_handle_get_lock_mode(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_lock_mode\n");
        exit(1);
    }
    
    /* get current value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(&my_handle));
    /* set a new value for resource idle lifespan */
    flom_handle_set_resource_idle_lifespan(&my_handle, 10000);
    /* get new value for resource idle lifespan */
    printf("flom_handle_get_resource_idle_lifespan() = %d\n",
           flom_handle_get_resource_idle_lifespan(&my_handle));
    /* check resource idle lifespan */
    if (10000 != flom_handle_get_resource_idle_lifespan(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_idle_lifespan\n");
        exit(1);
    }
    
    /* get current unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(&my_handle));
    /* set a new unicast_address */
    flom_handle_set_unicast_address(&my_handle, nd_unicast_address);
    /* get new unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(&my_handle));
    /* check unicast address */
    if (strcmp(nd_unicast_address,
               flom_handle_get_unicast_address(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_unicast_address\n");
        exit(1);
    }
    
    /* get current multicast address */
    printf("flom_handle_get_multicast_address() = '%s'\n",
           flom_handle_get_multicast_address(&my_handle));
    /* set a new multicast address */
    flom_handle_set_multicast_address(&my_handle, nd_multicast_address);
    /* get new multicast address */
    printf("flom_handle_get_multicast_address() = '%s'\n",
           flom_handle_get_multicast_address(&my_handle));
    /* check multicast address */
    if (strcmp(nd_multicast_address,
               flom_handle_get_multicast_address(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_multicast_address\n");
        exit(1);
    }
    
    /* get current network interface */
    fprintf(stderr, "flom_handle_get_network_interface() = '%s'\n",
           flom_handle_get_network_interface(&my_handle));
    /* set a new network interface */
    if (FLOM_RC_OK == flom_handle_set_network_interface(
            &my_handle, nd_network_interface)) {
        /* get new network interface */
        fprintf(stderr, "flom_handle_get_network_interface() = '%s'\n",
               flom_handle_get_network_interface(&my_handle));
        /* check network interface */
        if (strcmp(nd_network_interface,
                   flom_handle_get_network_interface(&my_handle))) {
            fprintf(stderr,
                    "Unexpected result from flom_handle_set/"
                    "get_network_interface\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "'%s' is not a valid IPv6 network interface for "
                "this system\n", nd_network_interface);
        exit(1);
    }
    
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           &my_handle, nd_socket_name))) {
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
    /* check unicast port */
    if (7777 != flom_handle_get_unicast_port(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_unicast_port\n");
        exit(1);
    }
    
    /* get current value for multicast port */
    printf("flom_handle_get_multicast_port() = %d\n",
           flom_handle_get_multicast_port(&my_handle));
    /* set a new value for multicast_port */
    flom_handle_set_multicast_port(&my_handle, 8888);
    /* get new value for multicast port */
    printf("flom_handle_get_multicast_port() = %d\n",
           flom_handle_get_multicast_port(&my_handle));
    /* check multicast port */
    if (8888 != flom_handle_get_multicast_port(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_multicast_port\n");
        exit(1);
    }
    
    /* get current value for discovery attempts property */
    printf("flom_handle_get_discovery_attempts() = %d\n",
           flom_handle_get_discovery_attempts(&my_handle));
    /* set a new value for discovery attempts property */
    flom_handle_set_discovery_attempts(&my_handle, 5);
    /* get new value for discovery attempts */
    printf("flom_handle_get_discovery_attempts() = %d\n",
           flom_handle_get_discovery_attempts(&my_handle));
    /* check discovery attempts */
    if (5 != flom_handle_get_discovery_attempts(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_discovery_attempts\n");
        exit(1);
    }
    
    /* get current value for discovery timeout property */
    printf("flom_handle_get_discovery_timeout() = %d\n",
           flom_handle_get_discovery_timeout(&my_handle));
    /* set a new value for discovery timeout property */
    flom_handle_set_discovery_timeout(&my_handle, 750);
    /* get new value for discovery timeout */
    printf("flom_handle_get_discovery_timeout() = %d\n",
           flom_handle_get_discovery_timeout(&my_handle));
    /* check discovery timeout */
    if (750 != flom_handle_get_discovery_timeout(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_discovery_timeout\n");
        exit(1);
    }
    
    /* get current value for discovery ttl property */
    printf("flom_handle_get_discovery_ttl() = %d\n",
           flom_handle_get_discovery_ttl(&my_handle));
    /* set a new value for discovery ttl property */
    flom_handle_set_discovery_ttl(&my_handle, 2);
    /* get new value for discovery ttl */
    printf("flom_handle_get_discovery_ttl() = %d\n",
           flom_handle_get_discovery_ttl(&my_handle));
    /* check discovery ttl */
    if (2 != flom_handle_get_discovery_ttl(&my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_discovery_ttl\n");
        exit(1);
    }
    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(&my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(&my_handle)) {
        printf("staticHandleHappyPath locked element is %s\n",
               flom_handle_get_locked_element(&my_handle));
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
void dynamic_handle_happy_path(const char *nd_network_interface) {
    int ret_cod;
    flom_handle_t *my_handle = NULL;

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
                           my_handle, nd_socket_name))) {
        fprintf(stderr, "flom_handle_set_socket_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new AF_UNIX/PF_LOCAL socket_name */
    printf("flom_handle_get_socket_name() = '%s'\n",
           flom_handle_get_socket_name(my_handle));
    /* check socket name */
    if (strcmp(nd_socket_name, flom_handle_get_socket_name(my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_socket_name\n");
        exit(1);
    }

    /* we don't get current trace filename because it can be altered by a
       global config file */
    /* set a new trace filename */
    flom_handle_set_trace_filename(my_handle, nd_trace_filename);
    /* get new trace filename */
    printf("flom_handle_get_trace_filename() = '%s'\n",
           flom_handle_get_trace_filename(my_handle));
    /* check trace filename */
    if (strcmp(nd_trace_filename,
               flom_handle_get_trace_filename(my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_trace_filename\n");
        exit(1);
    }
    
    /* get current resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(my_handle));
    /* set a new resource name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           my_handle, nd_resource_name))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(my_handle));
    /* check resource name */
    if (strcmp(nd_resource_name,
               flom_handle_get_resource_name(my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_resource_name\n");
        exit(1);
    }
    
    /* get current value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* set a new value for resource create property */
    flom_handle_set_resource_create(my_handle, FALSE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* check resource create 1/2 */
    if (flom_handle_get_resource_create(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_create\n");
        exit(1);
    }    
    /* set a new value for resource create property */
    flom_handle_set_resource_create(my_handle, TRUE);
    /* get new value for resource create property */
    printf("flom_handle_get_resource_create() = %d\n",
           flom_handle_get_resource_create(my_handle));
    /* check resource create 2/2 */
    if (!flom_handle_get_resource_create(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_create\n");
        exit(1);
    }
        
    /* get current value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(my_handle));
    /* set a new value for resource timeout property */
    flom_handle_set_resource_timeout(my_handle, -1);
    /* get new value for resource timeout property */
    printf("flom_handle_get_resource_timeout() = %d\n",
           flom_handle_get_resource_timeout(my_handle));
    /* check resource timeout */
    if (-1 != flom_handle_get_resource_timeout(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_timeout\n");
        exit(1);
    }
    
    /* get current value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(my_handle));
    /* set a new value for resource quantity property */
    flom_handle_set_resource_quantity(my_handle, 3);
    /* get new value for resource quantity property */
    printf("flom_handle_get_resource_quantity() = %d\n",
           flom_handle_get_resource_quantity(my_handle));
    /* check resource quantity */
    if (3 != flom_handle_get_resource_quantity(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_quantity\n");
        exit(1);
    }
    
    /* get current value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(my_handle));
    /* set a new value for resource lock mode property */
    flom_handle_set_lock_mode(my_handle, FLOM_LOCK_MODE_PW);
    /* get new value for resource lock mode property */
    printf("flom_handle_get_lock_mode() = %d\n",
           flom_handle_get_lock_mode(my_handle));
    /* check resource lock mode */
    if (FLOM_LOCK_MODE_PW != flom_handle_get_lock_mode(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_lock_mode\n");
        exit(1);
    }
    
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
    /* check resource idle lifespan */
    if (10000 != flom_handle_get_resource_idle_lifespan(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_resource_idle_lifespan\n");
        exit(1);
    }
    
    /* set a new unicast_address */
    flom_handle_set_unicast_address(my_handle, nd_unicast_address);
    /* get new unicast address */
    printf("flom_handle_get_unicast_address() = '%s'\n",
           flom_handle_get_unicast_address(my_handle));
    /* check unicast address */
    if (strcmp(nd_unicast_address,
               flom_handle_get_unicast_address(my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_unicast_address\n");
        exit(1);
    }
    
    /* get current multicast address */
    printf("flom_handle_get_multicast_address() = '%s'\n",
           flom_handle_get_multicast_address(my_handle));
    /* set a new multicast_address */
    flom_handle_set_multicast_address(my_handle, nd_multicast_address);
    /* get new multicast address */
    printf("flom_handle_get_multicast_address() = '%s'\n",
           flom_handle_get_multicast_address(my_handle));
    /* check multicast address */
    if (strcmp(nd_multicast_address,
               flom_handle_get_multicast_address(my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_multicast_address\n");
        exit(1);
    }
    
    /* get current network interface */
    fprintf(stderr, "flom_handle_get_network_interface() = '%s'\n",
           flom_handle_get_network_interface(my_handle));
    /* set a new network interface */
    if (FLOM_RC_OK == flom_handle_set_network_interface(
            my_handle, nd_network_interface)) {
        /* get new network interface */
        fprintf(stderr, "flom_handle_get_network_interface() = '%s'\n",
               flom_handle_get_network_interface(my_handle));
        /* check network interface */
        if (strcmp(nd_network_interface,
                   flom_handle_get_network_interface(my_handle))) {
            fprintf(stderr,
                    "Unexpected result from flom_handle_set/"
                    "get_network_interface\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "'%s' is not a valid IPv6 network interface for "
                "this system\n", nd_network_interface);
        exit(1);
    }
    
    /* set AF_UNIX/PF_LOCAL socket_name again */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_socket_name(
                           my_handle, nd_socket_name))) {
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
    /* check unicast port */
    if (7777 != flom_handle_get_unicast_port(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_unicast_port\n");
        exit(1);
    }
    
    /* get current value for multicast port */
    printf("flom_handle_get_multicast_port() = %d\n",
           flom_handle_get_multicast_port(my_handle));
    /* set a new value for multicast_port */
    flom_handle_set_multicast_port(my_handle, 8888);
    /* get new value for multicast port */
    printf("flom_handle_get_multicast_port() = %d\n",
           flom_handle_get_multicast_port(my_handle));
    /* check multicast port */
    if (8888 != flom_handle_get_multicast_port(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_multicast_port\n");
        exit(1);
    }
    
    /* get current value for discovery attempts property */
    printf("flom_handle_get_discovery_attempts() = %d\n",
           flom_handle_get_discovery_attempts(my_handle));
    /* set a new value for discovery attempts property */
    flom_handle_set_discovery_attempts(my_handle, 5);
    /* get new value for discovery attempts */
    printf("flom_handle_get_discovery_attempts() = %d\n",
           flom_handle_get_discovery_attempts(my_handle));
    /* check discovery attempts */
    if (5 != flom_handle_get_discovery_attempts(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_discovery_attempts\n");
        exit(1);
    }
    
    /* get current value for discovery timeout property */
    printf("flom_handle_get_discovery_timeout() = %d\n",
           flom_handle_get_discovery_timeout(my_handle));
    /* set a new value for discovery timeout property */
    flom_handle_set_discovery_timeout(my_handle, 750);
    /* get new value for discovery timeout */
    printf("flom_handle_get_discovery_timeout() = %d\n",
           flom_handle_get_discovery_timeout(my_handle));
    /* check discovery timeout */
    if (750 != flom_handle_get_discovery_timeout(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/"
                "get_discovery_timeout\n");
        exit(1);
    }
    
    /* get current value for discovery ttl property */
    printf("flom_handle_get_discovery_ttl() = %d\n",
           flom_handle_get_discovery_ttl(my_handle));
    /* set a new value for discovery ttl property */
    flom_handle_set_discovery_ttl(my_handle, 2);
    /* get new value for discovery ttl */
    printf("flom_handle_get_discovery_ttl() = %d\n",
           flom_handle_get_discovery_ttl(my_handle));
    /* check discovery ttl */
    if (2 != flom_handle_get_discovery_ttl(my_handle)) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_discovery_ttl\n");
        exit(1);
    }
    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_handle_lock(my_handle))) {
        fprintf(stderr, "flom_handle_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } else if (NULL != flom_handle_get_locked_element(my_handle)) {
        printf("dynamicHandleHappyPath locked element is %s\n",
               flom_handle_get_locked_element(my_handle));
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
    if (argc < 2) {
        fprintf(stderr, "First argument must be a valid IPv6 network "
                "interface\n");
        exit(1);
    }
    /* static handle tests */
    static_handle_happy_path(argv[1]);
    /* dynamic handle test */
    dynamic_handle_happy_path(argv[1]);
    /* exit */
    return 0;
}
