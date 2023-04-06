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

#include <iostream>

#include "flom.hh"
using namespace flom;


/*
 * This example program shows the basic usage of libflom API library with
 * a statically allocated handle.
 * These are the steps:
 * 1. declare an object of type FlomHandle inside process stack
 * 2. acquire a lock using method lock()
 * 3. execute the code protected by the acquired lock
 * 4. release the lock using method unlock()
 *
 * Compilation command:
 *     make -f example_makefile BasicStatic
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./BasicStatic
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x80000
 * ./BasicStatic
 */



int main(int argc, char *argv[]) {
    int retCod;
    /* step 1: handle declaration */
    FlomHandle myHandle;
    
    /* step 2: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle.lock())) {
        cerr << "FlomHandle.lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 3: execute the code that needs lock protection */
    
    /* step 4: lock release */
    if (FLOM_RC_OK != (retCod = myHandle.unlock())) {
        cerr << "FlomHandle.unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* exit */
    return 0;
}
