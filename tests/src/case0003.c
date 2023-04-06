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
#include <string.h>

#include "flom.h"



/*
 * Non default values used for tests
 */
const char *nd_resource_name1 = "resource1";
const char *nd_resource_name2 = "resource2";



/*
 * Happy path usage with a static handle
 */
void unlock_different_resource(void) {
    int ret_cod;
    flom_handle_t my_handle;

    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "flom_handle_init() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get current resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* set a new resource name */
    if (FLOM_RC_OK != (ret_cod = flom_handle_set_resource_name(
                           &my_handle, nd_resource_name1))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* check resource name */
    if (strcmp(nd_resource_name1,
               flom_handle_get_resource_name(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_resource_name\n");
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
    
    /* set a different resource name */
    if (FLOM_RC_API_IMMUTABLE_HANDLE != (
            ret_cod = flom_handle_set_resource_name(
                &my_handle, nd_resource_name2))) {
        fprintf(stderr, "flom_handle_set_resource_name() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* get new resource name */
    printf("flom_handle_get_resource_name() = '%s'\n",
           flom_handle_get_resource_name(&my_handle));
    /* check resource name: it should not be changed because the resource
     is already locked */
    if (strcmp(nd_resource_name1,
               flom_handle_get_resource_name(&my_handle))) {
        fprintf(stderr,
                "Unexpected result from flom_handle_set/get_resource_name\n");
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



int main(int argc, char *argv[]) {
    int bug = -1;
    if (argc != 2) {
        fprintf(stderr, "The first parameter must be the bug to exploit\n");
        exit(1);
    }
    bug = strtol(argv[1], NULL, 10);
    switch (bug) {
        case 1:
            /* try to unlock a resource different from the locked one! */
            unlock_different_resource();
            break;
        default:
            fprintf(stderr, "The bug # %d is not known, exiting...\n", bug);
            exit(1);
    } /* switch */
    /* exit */
    return 0;
}
