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
 * This example program shows the usage of libflom API library with
 * a transactional resource: a unique transactional sequence.
 * These are the steps:
 * 1. declare a pointer for type FlomHandle
 * 2. create a new handle
 * 3a. set a non default resource name (a name valid for a trasactional
 *     sequence resource)
 * 3b. set a non default resource idle lifespan
 * 4. acquire a lock using method lock()
 * 5. release the lock using method unlock_rollback()
 * 6. acquire a new lock using method lock() and verifying the
 *    FLoM daemon returnes the same value
 * 7. release the lock using method unlock()
 * 8. acquire a new lock using method lock() and verifying the
 *    FLoM daemon returnes a different value
 * 9. sleep 5 seconds to allow program killing
 * 10. release the lock using method unlock()
 * 11. delete the handle
 *
 * Compilation command:
 *     make -f example_makefile Transactional
 *
 * Note: this program needs an already started FLoM daemon, for instance:
 * flom -d -1 -- true
 * ./Transactional
 *
 * The program itself is not verbose, but you might activate tracing if you
 * were interested to understand what's happen:
 * export FLOM_TRACE_MASK=0x80000
 * ./Transactional
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
    
    /* step 3a: set a different (non default) resource name to lock */
    if (FLOM_RC_OK != (retCod = myHandle->setResourceName("_S_transact[1]"))) {
        cerr << "FlomHandle->setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* step 3b: set a different (non default) resource idle lifespan */
    if (FLOM_RC_OK != (retCod = myHandle->setResourceIdleLifespan(60000))) {
        cerr << "FlomHandle->setResourceIdleLifespan() returned " << retCod <<
            " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 4: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "FlomHandle->getLockedElement(): '" <<
            myHandle->getLockedElement() << "' (first lock)" << endl;
    }
    
    /* step 5: lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlockRollback())) {
        cerr << "FlomHandle->unlockRollback() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 6: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "FlomHandle->getLockedElement(): '" <<
            myHandle->getLockedElement() << "' (second lock)" << endl;
    }
    
    /* step 7: lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle->unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 8: lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle->lock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    } else {
        cout << "FlomHandle->getLockedElement(): '" <<
            myHandle->getLockedElement() << "' (third lock)" << endl;
    }
    
    /* step 9: sleep 5 seconds to allow program killing */
    cout << "The program is waiting 5 seconds: kill it with the [control]+[c] "
        "keystroke and restart it to verify resource rollback..." << endl;
    sleep(5);
    
    /* step 10: lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle->unlock() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    
    /* step 11: delete the handle */
    delete myHandle;
    /* exit */
    return 0;
}
