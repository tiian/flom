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
#include <iostream>

#include "flom.hh"

using namespace flom;



int main(int argc, char *argv[]) {
    int retCod;
    FlomHandle *myHandle1 = NULL; /* used for non transactional resource */
    FlomHandle *myHandle2 = NULL; /* used for transactional resource */

    /* First step: non transactional resource */
    /* create a new handle */
    if (NULL == (myHandle1 = new FlomHandle())) {
        cerr << "FlomHandle() returned " << myHandle1 << endl;
        exit(1);
    }    
    /* set a new resource name */
    if (FLOM_RC_OK != (retCod = myHandle1->setResourceName(
                           "_s_nontransactional[1]"))) {
        cerr << "FlomHandle->setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* set a new value for resource idle lifespan */
    myHandle1->setResourceIdleLifespan(60000);
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle1->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } else {
        cout << "locked element is " << myHandle1->getLockedElement() << endl;
    } 
    /* lock release & rollback: the resource is not transactional, the
     * function must return a warning condition */
    if  (FLOM_RC_RESOURCE_IS_NOT_TRANSACTIONAL != (
             retCod = myHandle1->unlockRollback())) {
        cerr << "FlomHandle.unlock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle1->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } else {
        cout << "locked element is " << myHandle1->getLockedElement() << endl;
    } 
    /* the resource associated to myHandle1 is intentionally not unlocked
     * to check the behavior in case of abort */

    /* Second step: transactional resource */
    /* create a new handle */
    if (NULL == (myHandle2 = new FlomHandle())) {
        cerr << "FlomHandle() returned " << myHandle2 << endl;
        exit(1);
    }    
    /* set a new resource name */
    if (FLOM_RC_OK != (retCod = myHandle2->setResourceName(
                           "_S_transactional[1]"))) {
        cerr << "FlomHandle->setResourceName() returned " << retCod << " '" <<
            flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* set a new value for resource idle lifespan */
    myHandle2->setResourceIdleLifespan(60000);
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle2->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } else {
        cout << "locked element is " << myHandle2->getLockedElement() << endl;
    } 
    /* lock release & rollback: the resource is transactional, the
     * function must NOT return a warning condition */
    if  (FLOM_RC_OK != (retCod = myHandle2->unlockRollback())) {
        cerr << "FlomHandle.unlock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle2->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } else {
        cout << "locked element is " << myHandle2->getLockedElement() << endl;
    } 
    /* lock release */
    if  (FLOM_RC_OK != (retCod = myHandle2->unlock())) {
        cerr << "FlomHandle.unlock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle2->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } else {
        cout << "locked element is " << myHandle2->getLockedElement() << endl;
    } 
    /* interrupt execution to verify transactionality (the program must be
     * restarted */
    abort();
    /* this point will be never reached! */
}
