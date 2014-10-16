/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FLOM_H
# define FLOM_H



#include <glib.h>



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Lock a resource
     */
    int flom_lock();



    /**
     * Unlock a resource
     */
    int flom_unlock();


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_H */
