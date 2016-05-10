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
# 3a. set a non default resource name (a name valid for a trasactional
#     sequence resource)
# 3b. set a non default resource idle lifespan
# 4. acquire a lock using method lock()
# 5. release the lock using method unlock_rollback()
# 6. acquire a new lock using method lock() and verifying the
#    FLoM daemon returnes the same value
# 7. release the lock using method unlock()
# 8. acquire a new lock using method lock() and verifying the
#    FLoM daemon returnes a different value
# 9. sleep 5 seconds to allow program killing
# 10. release the lock using method unlock()
# 11. clean-up the allocated handle using function flom_handle_clean()
# 12. deallocate the handle object
#
# Execution command:
#     python transactional.py
# 
# Note: this program needs an already started FLoM daemon, for instance:
#     flom -d -1 -- true
#     python transactional.py
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#     export FLOM_TRACE_MASK=0x80000
#     python transactional.py
#


import time
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

# step 3a: set a different (non default) resource name to lock
ret_cod = flom_handle_set_resource_name(my_handle, "_S_transact[1]")
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_set_resource_name() returned " +
                     str(ret_cod) + ", '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 3b: set a different (non default) resource idle lifespan
ret_cod = flom_handle_set_resource_idle_lifespan(my_handle, 60000)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_set_resource_idle_lifespan() returned " +
                     ret_cod + ", '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 4: lock acquisition
ret_cod = flom_handle_lock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_lock() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)
else:
    sys.stdout.write("flom_handle_lock(): locked element is '" +
                     flom_handle_get_locked_element(my_handle) + 
                     "' (first lock)\n")

# step 5: lock release
ret_cod = flom_handle_unlock_rollback(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_unlock_rollback() returned " +
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 6: lock acquisition
ret_cod = flom_handle_lock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_lock() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)
else:
    sys.stdout.write("flom_handle_lock(): locked element is '" +
                     flom_handle_get_locked_element(my_handle) + 
                     "' (second lock)\n")

# step 7: lock release
ret_cod = flom_handle_unlock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_unlock() returned " +
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 8: lock acquisition
ret_cod = flom_handle_lock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_lock() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)
else:
    sys.stdout.write("flom_handle_lock(): locked element is '" +
                     flom_handle_get_locked_element(my_handle) + 
                     "' (second lock)\n")

# step 9: sleep 5 seconds to allow program killing
sys.stdout.write("The program is waiting 5 seconds: kill it with the " +
                 "[control]+[c] keystroke and restart it to verify resource " +
                 "rollback...\n")
time.sleep(5)

# step 10: lock release
ret_cod = flom_handle_unlock(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("flom_handle_unlock() returned " +
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 11: handle clean-up (memory release)
ret_cod = flom_handle_clean(my_handle)
if FLOM_RC_OK != ret_cod:
    sys.stderr.write("happy_path/flom_handle_clean() returned " + 
                     str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
    sys.exit(1)

# step 12: handle deallocation
del my_handle

