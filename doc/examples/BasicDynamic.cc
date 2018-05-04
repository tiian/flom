/*
 * Copyright (c) 2013-2018, Christian Ferrari <tiian@users.sourceforge.net>
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
 * a dynamically allocated handle.
 * These are the steps:
 * 1. declare a pointer for type FlomHandle
 * 2. create (allocate) a new handle
 * 3. acquire a lock using method lock()
 * 4. execute the code protected by the acquired lock
 * 5. release the lock using method unlock()
 * 6. delete the handle
 *
 * Compilation command:
 *     make -f example_makefile BasicDynamic
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./BasicDynamic
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x80000
 * ./BasicDynamic
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
    
    /* step 3: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 4: execute the code that needs lock protection */
    
    /* step 5: lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle->unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* step 6: delete the handle */
    delete myHandle;
    /* exit */
    return 0;
}
