/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
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
void staticHandleHappyPath(void) {
    int retCod;
    FlomHandle myHandle;
    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle.lock())) {
        cerr << "/FlomHandle.lock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle.unlock())) {
        cerr << "staticHandleHappyPath/FlomHandle.unlock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
}



/*
 * Stress test with a static handle, missing flom_handle_lock method
 */
void staticHandleMissingLock(void) {
    int retCod;
    FlomHandle myHandle;

    /* lock release */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            retCod = myHandle.unlock())) {
        cerr << "staticHandleMissingLock/FlomHandle.unlock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
}



/*
 * Stress test with a static handle, missing unlock method
 */
void staticHandleMissingUnlock(void) {
    int retCod;
    FlomHandle myHandle;

    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle.lock())) {
        cerr << "staticHandleMissingUnlock/FlomHandle.lock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    } 
    /* the "out of scope" state will force unlock! */
}



/*
 * Happy path usage with a dynamic handle
 */
void dynamicHandleHappyPath(void) {
    int retCod;
    FlomHandle *myHandle = NULL;

    /* create a new handle */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "dynamicHandleHappyPath/FlomHandle() returned " <<
            myHandle << endl;
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "dynamicHandleHappyPath/FlomHandle.lock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "dynamicHandleHappyPath/FlomHandle.unlock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* delete the handle */
    delete myHandle;
}



/*
 * Stress test with a dynamic handle, missing flom_handle_lock method
 */
void dynamicHandleMissingLock(void) {
    int retCod;
    FlomHandle *myHandle = NULL;

    /* create a new handle */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "dynamicHandleMissingLock/FlomHandle() returned " <<
            myHandle << endl;
        exit(1);
    }    
    /* lock release */
    if (FLOM_RC_API_INVALID_SEQUENCE != (
            retCod = myHandle->unlock())) {
        cerr << "dynamicHandleMissingLock/FlomHandle.unlock() returned " <<
            retCod << " '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* delete the handle */
    delete myHandle;
}



void dynamicHandleMissingUnlock(void) {
    int retCod;
    FlomHandle *myHandle = NULL;

    /* create a new handle */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "dynamicHandleMissingLock/FlomHandle() returned " <<
            myHandle << endl;
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "dynamicHandleMissingLock/FlomHandle.lock() returned " <<
            retCod << "'" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* delete will force unlock! */
    delete myHandle;
}



int main(int argc, char *argv[]) {
    /* static handle tests */
    staticHandleHappyPath();
    staticHandleMissingLock();
    staticHandleMissingUnlock();
    /* dynamic handle test */
    dynamicHandleHappyPath();
    dynamicHandleMissingLock();
    dynamicHandleMissingUnlock();
    /* exit */
    return 0;
}
