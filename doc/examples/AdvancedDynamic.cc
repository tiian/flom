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
#include <iostream>

#include "flom.hh"
using namespace flom;



/*
 * This example program shows the basic usage of libflom API library with
 * a dynamically allocated handle; it uses a resource set instead of the
 * default resource and displays the name of the element obtained.
 * These are the steps:
 * 1. declare a pointer for type FlomHandle
 * 2. create a new handle
 * 3. set custom properties different from default values:
 *    3a. use a different AF_UNIX/PF_LOCAL socket to reach FLoM daemon
 *    3b. specifies a resource name to lock
 * 4. acquire a lock using method lock()
 * 5. execute the code protected by the acquired lock
 * 6. release the lock using method unlock()
 * 7. delete the handle
 *
 * Compilation command:
 *     make -f example_makefile AdvancedDynamic
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -s /tmp/my_socket_name -d -1 -- true
 * ./AdvancedDynamic
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x8000
 * ./AdvancedDynamic
 */



int main(int argc, char *argv[]) {
    int retCod;
    /* step 1: handle declaration */
    FlomHandle *myHandle = NULL;

    /* step 2: new handle creation */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "FlomHandle() returned NULL" << endl;
        exit(1);
    }    
    
    /* step 3a: set a different AF_UNIX/PF_LOCAL socket to connect to FLoM
       daemon */
    if (FLOM_RC_OK != (retCod = myHandle->setSocketName(
                           "/tmp/my_socket_name"))) {
        cerr << "FlomHandle->setSocketName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* step 3b: set a different (non default) resource name to lock */
    if (FLOM_RC_OK != (retCod = myHandle->setResourceName("Red.Blue.Green"))) {
        cerr << "FlomHandle->setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 4: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "FlomHandle->getLockedElement(): '" <<
            myHandle->getLockedElement() << "'" << endl;
    }
    
    /* step 5: execute the code that needs lock protection */
    
    /* step 6: lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle->unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* step 7: delete the handle */
    delete myHandle;
    /* exit */
    return 0;
}
