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
#include <iostream>

#include "flom.hh"

using namespace flom;

/*
 * Happy path usage with a static handle
 */
void static_handle_happy_path(void) {
    int ret_cod;
    FlomHandle my_handle;
    char locked_element[100];
    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = my_handle.lock(locked_element,
                                                sizeof(locked_element)))) {
        cerr << "FlomHandle.lock() returned " << ret_cod
             << ", '" << flom_strerror(ret_cod) << "'" << endl;
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = my_handle.unlock())) {
        cerr << "FlomHandle.unlock() returned " << ret_cod
             << ", '" << flom_strerror(ret_cod) << "'" << endl;
        exit(1);
    }
}



/*
 * Happy path usage with a dynamic handle
 */
void dynamic_handle_happy_path(void) {
    int ret_cod;
    FlomHandle *my_handle = NULL;
    char locked_element[100];

    /* create a new handle */
    if (NULL == (my_handle = new FlomHandle())) {
        cerr << "FlomHandle() returned " << my_handle << endl;
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (ret_cod = my_handle->lock(locked_element,
                                                 sizeof(locked_element)))) {
        cerr << "FlomHandle.lock() returned " << ret_cod
             << ", '" << flom_strerror(ret_cod) << "'" << endl;
    } 
    /* lock release */
    if (FLOM_RC_OK != (ret_cod = my_handle->unlock())) {
        cerr << "FlomHandle.unlock() returned " << ret_cod
             << ", '" << flom_strerror(ret_cod) << "'" << endl;
        exit(1);
    }
    /* delete the handle */
    delete my_handle;
}



int main(int argc, char *argv[]) {
    /* static handle tests */
    static_handle_happy_path();
    /* dynamic handle test */
    dynamic_handle_happy_path();
    /* exit */
    return 0;
}
