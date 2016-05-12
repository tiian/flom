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
from flom import *



#
# Happy path usage with a static handle
#
def happy_path():
    # handle allocation
    handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_init() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # lock acquisition
    ret_cod = flom_handle_lock(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_lock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # lock release
    ret_cod = flom_handle_unlock(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_unlock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)
	 
    # handle deallocation
    del handle



#
# Missing flom_handle_init method
#
def missing_init():
    # handle allocation
    handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # lock acquisition
    ret_cod = flom_handle_lock(handle)
    if FLOM_RC_API_INVALID_SEQUENCE != ret_cod:
        sys.stderr.write("happy_path/flom_handle_lock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # lock release
    ret_cod = flom_handle_unlock(handle)
    if FLOM_RC_API_INVALID_SEQUENCE != ret_cod:
        sys.stderr.write("happy_path/flom_handle_unlock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(handle)
    if FLOM_RC_API_INVALID_SEQUENCE != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)
	 
    # handle deallocation
    del handle



#
# Missing flom_handle_lock method
#
def missing_lock():
    # handle allocation
    handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_init() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # lock release
    ret_cod = flom_handle_unlock(handle)
    if FLOM_RC_API_INVALID_SEQUENCE != ret_cod:
        sys.stderr.write("happy_path/flom_handle_unlock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)
	 
    # handle deallocation
    del handle



#
# Missing flom_handle_unlock method
#
def missing_unlock():
    # handle allocation
    handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_init() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # lock acquisition
    ret_cod = flom_handle_lock(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_lock() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
	sys.exit(1)
	 
    # handle deallocation
    del handle



happy_path()
missing_init()
missing_lock()
missing_unlock()

