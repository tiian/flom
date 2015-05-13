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
#/



import sys
sys.path.append('../../../src/python')
from flom import *



#
# Happy path usage
#
def happy_path() :
    #
    # Non default values used for tests
    #
    nd_socket_name = "/tmp/flom_socket_name"
    nd_trace_filename = "/tmp/flom.trc"
    nd_resource_name = "red.green.blue"
    nd_unicast_address = "127.0.0.1"
    nd_multicast_address = "224.0.0.1"
    # handle allocation
    handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_init() returned " +
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # get current AF_UNIX/PF_LOCAL socket_name
    sys.stdout.write("flom_handle_get_socket_name() = '" +
           flom_handle_get_socket_name(handle) + "'\n")
    # set a new AF_UNIX/PF_LOCAL socket_name
    ret_cod = flom_handle_set_socket_name(handle, nd_socket_name)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_socket_name() returned " +
		ret_cod + ", '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # get new AF_UNIX/PF_LOCAL socket_name
    sys.stdout.write("flom_handle_get_socket_name() = '" +
           flom_handle_get_socket_name(handle) +  "'\n")
    # check socket name
    if nd_socket_name != flom_handle_get_socket_name(handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
		"get_socket_name\n")
        sys.exit(1)

    # we don't get current trace filename because it can be altered by a
    # global config file
    # set a new trace filename
    flom_handle_set_trace_filename(handle, nd_trace_filename)
    # get new trace filename
    sys.stdout.write("flom_handle_get_trace_filename() = '" +
           flom_handle_get_trace_filename(handle) + "'\n")
    # check trace filename
    if nd_trace_filename != flom_handle_get_trace_filename(handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_trace_filename\n")
        sys.exit(1)

    # get current resource name
    sys.stdout.write("flom_handle_get_resource_name() = '" +
           flom_handle_get_resource_name(handle) + "'\n")
    # set a new resource name
    ret_cod = flom_handle_set_resource_name(handle, nd_resource_name)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_resource_name() returned " +
                         str(ret_cod) + ", '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # get new resource name
    sys.stdout.write("flom_handle_get_resource_name() = '" +
                     flom_handle_get_resource_name(handle) + "'\n")
    # get current value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(handle)) + "\n")
    # set a new value for resource create property
    flom_handle_set_resource_create(handle, FALSE)
    # get new value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(handle)) + "\n")
    # check resource create 1/2
    if flom_handle_get_resource_create(handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_create\n")
        sys.exit(1);

    # set a new value for resource create property
    flom_handle_set_resource_create(handle, TRUE)
    # get new value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(handle)) + "\n")
    # check resource create 2/2
    if not flom_handle_get_resource_create(handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_create\n")
        sys.exit(1)

    # get current value for resource timeout property
    sys.stdout.write("flom_handle_get_resource_timeout() = " +
                     str(flom_handle_get_resource_timeout(handle)) + "\n");
    # set a new value for resource timeout property
    flom_handle_set_resource_timeout(handle, -1);
    # get new value for resource timeout property
    sys.stdout.write("flom_handle_get_resource_timeout() = " +
                     str(flom_handle_get_resource_timeout(handle)) + "\n");
    # check resource timeout
    ret_cod = flom_handle_get_resource_timeout(handle)
    if -1 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_timeout\n");
        sys.exit(1);

    # lock acquisition
    ret_cod = flom_handle_lock(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # retrieve locked element
    sys.stdout.write("happy_path locked element is '" +
                     flom_handle_get_locked_element(handle) + "'\n")
    # lock release
    ret_cod = flom_handle_unlock(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_unlock() returned " +
                         str(ret_cod) + " '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1);

    # handle deallocation
    del handle



happy_path()

