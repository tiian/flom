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
 * Happy path usage with a static handle
 */
void staticHandleHappyPath(void) {
    int retCod;
    FlomHandle myHandle;
    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle.lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    } 
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle.unlock())) {
        cerr << "FlomHandle.unlock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
}



/*
 * Happy path usage with a dynamic handle
 */
void dynamicHandleHappyPath(void) {
    int retCod;
    FlomHandle *myHandle = NULL;

    /* create a new handle */
    if (NULL == (myHandle = new FlomHandle())) {
        cerr << "FlomHandle() returned " << myHandle << endl;
        exit(1);
    }    
    /* lock acquisition */
    if (FLOM_RC_OK != (retCod = myHandle->lock())) {
        cerr << "FlomHandle.lock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
    } 
    /* lock release */
    if (FLOM_RC_OK != (retCod = myHandle->unlock())) {
        cerr << "FlomHandle.unlock() returned " << retCod
             << ", '" << flom_strerror(retCod) << "'" << endl;
        exit(1);
    }
    /* delete the handle */
    delete myHandle;
}



int main(int argc, char *argv[]) {
    /* static handle tests */
    staticHandleHappyPath();
    /* dynamic handle test */
    dynamicHandleHappyPath();
    /* exit */
    return 0;
}
