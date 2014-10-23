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



int main(int argc, char *argv[]) {
    int ret_cod;
    flom_handle_t my_handle;
    char locked_element[100];

    /* initialize a new handle */
    if (FLOM_RC_OK != (ret_cod = flom_handle_init(&my_handle))) {
        fprintf(stderr, "flom_handle_init() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = flom_lock(&my_handle, locked_element,
                                           sizeof(locked_element)))) {
        fprintf(stderr, "flom_lock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = flom_unlock(&my_handle))) {
        fprintf(stderr, "flom_unlock() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* handle clean-up (memory release) */
    if (FLOM_RC_OK != (ret_cod = flom_handle_clean(&my_handle))) {
        fprintf(stderr, "flom_handle_clean() returned %d, '%s'\n",
                ret_cod, flom_strerror(ret_cod));
        exit(1);
    }
    /* exit */
    return 0;
}
