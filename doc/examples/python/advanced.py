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



#
# This example program shows the usage of libflom API library from a
# Python program; it uses a resource set instead of the default resource and
# displays the name of the obtained element.
# These are the steps:
# 1. create and allocate an object of type flom_handle_t
# 2. initialize the allocated handle using function flom_handle_init()
# 3. set custom properties different from default values:
#    3a. use a different AF_UNIX/PF_LOCAL socket to reach FLoM daemon
#    3b. specify a resource name to lock
# 4. acquire a lock using function flom_handle_lock()
# 5. execute the code protected by the acquired lock
# 6. release the lock using function flom_handle_unlock()
# 7. clean-up the allocated handle using function flom_handle_clean()
# 8. deallocate the handle object
#
# Execution command:
#     python advanced.py
# 
# Note: this program needs an already started FLoM daemon, for instance:
#     flom -s /tmp/my_socket_name -d -1 -- true
#     python advanced.py
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#     export FLOM_TRACE_MASK=0x8000
#     python advanced.php
#


import sys
from flom import *


# step 1: handle allocation
my_handle = flom_handle_t()
ret_cod = FLOM_RC_OK

# step 2: handle initialization
ret_cod = flom_handle_init(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_init() returned " +
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 3a: set a different AF_UNIX/PF_LOCAL socket to connect to FLoM  daemon
ret_cod = flom_handle_set_socket_name(my_handle, "/tmp/my_socket_name")
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_set_socket_name() returned " +
                     ret_cod + ", '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 3b: set a different (non default) resource name to lock
ret_cod = flom_handle_set_resource_name(my_handle, "Red.Blue.Green")
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_set_resource_name() returned " +
                     str(ret_cod) + ", '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 4: lock acquisition
ret_cod = flom_handle_lock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("happy_path/flom_handle_lock() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)
else:
    sys.stdout.write("flom_handle_lock(): locked element is '" +
                     flom_handle_get_locked_element(my_handle) + "'\n")

# step 5: execute the code that needs lock protection
#         put some code below this comment and before lock release
#         ...

# step 6: lock release
ret_cod = flom_handle_unlock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("happy_path/flom_handle_unlock() returned " +
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)


# step 7: handle clean-up (memory release)
ret_cod = flom_handle_clean(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("happy_path/flom_handle_clean() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 8: handle deallocation
del my_handle

