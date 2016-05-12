#
# Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
# All rights reserved.
#
# This file is part of FLoM.
#
# FLoM is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# FLoM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
#



import sys
sys.path.append('../../../src/python')
import os
from flom import *



def sequence_resource_test() :
    # First step: non transactional resource
    # handle allocation
    my_handle1 = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(my_handle1)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_init() returned " +
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # set a new resource name
    ret_cod = flom_handle_set_resource_name(
        my_handle1, "_s_nontransactional[1]")
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_resource_name() returned " +
                         str(ret_cod) + ", '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1)
    # set a new value for resource idle lifespan
    flom_handle_set_resource_idle_lifespan(my_handle1, 60000)
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle1)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # retrieve locked element
    sys.stdout.write("locked element is " +
                     flom_handle_get_locked_element(my_handle1) + "\n")
    # lock release & rollback: the resource is not transactional, the
    # function must return a warning condition
    ret_cod = flom_handle_unlock_rollback(my_handle1)
    if FLOM_RC_RESOURCE_IS_NOT_TRANSACTIONAL != ret_cod:
        sys.stderr.write("flom_handle_unlock() returned " +
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle1)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # retrieve locked element
    sys.stdout.write("locked element is " +
                     flom_handle_get_locked_element(my_handle1) + "\n")
    # the resource associated to my_handle1 is intentionally not unlocked
    # to check the behavior in case of abort

    # Second step: transactional resource
    # handle allocation
    my_handle2 = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_init() returned " +
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # set a new resource name
    ret_cod = flom_handle_set_resource_name(
        my_handle2, "_S_nontransactional[1]")
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_resource_name() returned " +
                         str(ret_cod) + ", '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1)
    # set a new value for resource idle lifespan
    flom_handle_set_resource_idle_lifespan(my_handle2, 60000)
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # retrieve locked element
    sys.stdout.write("locked element is " +
                     flom_handle_get_locked_element(my_handle2) + "\n")
    # lock release & rollback: the resource is transactional, the
    # function must NOT return a warning condition */
    ret_cod = flom_handle_unlock_rollback(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_unlock() returned " +
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # retrieve locked element
    sys.stdout.write("locked element is " +
                     flom_handle_get_locked_element(my_handle2) + "\n")
    # lock release
    ret_cod = flom_handle_unlock(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_unlock() returned " +
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle2)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)
    # retrieve locked element
    sys.stdout.write("locked element is " +
                     flom_handle_get_locked_element(my_handle2) + "\n")
    # interrupt execution to verify transactionality (the program must be
    # restarted
    os.abort();
    # this point will be never reached!

sequence_resource_test()

