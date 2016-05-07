#
# Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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
from flom import *


#
# This example program shows the basic usage of libflom API library from a
# Python program.
# These are the steps:
# 1. create and allocate an object of type flom_handle_t
# 2. initialize the allocated handle using function flom_handle_init()
# 3. acquire a lock using function flom_handle_lock()
# 4. execute the code protected by the acquired lock
# 5. release the lock using function flom_handle_unlock()
# 6. clean-up the allocated handle using function flom_handle_clean()
# 7. deallocate the handle object
#
# Execution command:
#     python basic.py
# 
# Note: this program needs an already started FLoM daemon, for instance:
#     flom -d -1 -- true
#     python basic.py
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#     export FLOM_TRACE_MASK=0x80000
#     python basic.py
#



# step 1: handle allocation
handle = flom_handle_t()
ret_cod = FLOM_RC_OK

# step 2: handle initialization
ret_cod = flom_handle_init(handle)
if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_init() returned " + str(ret_cod) +
        " '"  + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

# step 3: lock acquisition
ret_cod = flom_handle_lock(handle)
if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_lock() returned " + str(ret_cod) +
        " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

# step 4: execute the code that needs lock protection
#         put some code below this comment and before lock release
#         ...

# step 5: lock release
ret_cod = flom_handle_unlock(handle)
if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_unlock() returned " + str(ret_cod) +
        " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

# step 6: handle clean-up (memory release)
ret_cod = flom_handle_clean(handle)
if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_clean() returned " + str(ret_cod) +
        " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

# step 7: handle deallocation
del handle
