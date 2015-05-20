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
    my_handle = flom_handle_t()
    ret_cod = FLOM_RC_OK

    # handle initialization
    ret_cod = flom_handle_init(my_handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_init() returned " +
		str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # get current AF_UNIX/PF_LOCAL socket_name
    sys.stdout.write("flom_handle_get_socket_name() = '" +
           flom_handle_get_socket_name(my_handle) + "'\n")
    # set a new AF_UNIX/PF_LOCAL socket_name
    ret_cod = flom_handle_set_socket_name(my_handle, nd_socket_name)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_socket_name() returned " +
		ret_cod + ", '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1)

    # get new AF_UNIX/PF_LOCAL socket_name
    sys.stdout.write("flom_handle_get_socket_name() = '" +
           flom_handle_get_socket_name(my_handle) +  "'\n")
    # check socket name
    if nd_socket_name != flom_handle_get_socket_name(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
		"get_socket_name\n")
        sys.exit(1)

    # we don't get current trace filename because it can be altered by a
    # global config file
    # set a new trace filename
    flom_handle_set_trace_filename(my_handle, nd_trace_filename)
    # get new trace filename
    sys.stdout.write("flom_handle_get_trace_filename() = '" +
           flom_handle_get_trace_filename(my_handle) + "'\n")
    # check trace filename
    if nd_trace_filename != flom_handle_get_trace_filename(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_trace_filename\n")
        sys.exit(1)

    # get current resource name
    sys.stdout.write("flom_handle_get_resource_name() = '" +
           flom_handle_get_resource_name(my_handle) + "'\n")
    # set a new resource name
    ret_cod = flom_handle_set_resource_name(my_handle, nd_resource_name)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_resource_name() returned " +
                         str(ret_cod) + ", '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # get new resource name
    sys.stdout.write("flom_handle_get_resource_name() = '" +
                     flom_handle_get_resource_name(my_handle) + "'\n")
    # get current value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(my_handle)) + "\n")
    # set a new value for resource create property
    flom_handle_set_resource_create(my_handle, FALSE)
    # get new value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(my_handle)) + "\n")
    # check resource create 1/2
    if flom_handle_get_resource_create(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_create\n")
        sys.exit(1);

    # set a new value for resource create property
    flom_handle_set_resource_create(my_handle, TRUE)
    # get new value for resource create property
    sys.stdout.write("flom_handle_get_resource_create() = " +
                     str(flom_handle_get_resource_create(my_handle)) + "\n")
    # check resource create 2/2
    if not flom_handle_get_resource_create(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_create\n")
        sys.exit(1)

    # get current value for resource timeout property
    sys.stdout.write("flom_handle_get_resource_timeout() = " +
                     str(flom_handle_get_resource_timeout(my_handle)) + "\n");
    # set a new value for resource timeout property
    flom_handle_set_resource_timeout(my_handle, -1);
    # get new value for resource timeout property
    sys.stdout.write("flom_handle_get_resource_timeout() = " +
                     str(flom_handle_get_resource_timeout(my_handle)) + "\n");
    # check resource timeout
    ret_cod = flom_handle_get_resource_timeout(my_handle)
    if -1 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_timeout\n");
        sys.exit(1);

    # get current value for resource quantity property
    sys.stdout.write("flom_handle_get_resource_quantity() = " + 
                     str(flom_handle_get_resource_quantity(my_handle)) + "\n");
    # set a new value for resource quantity property
    flom_handle_set_resource_quantity(my_handle, 3);
    # get new value for resource quantity property
    sys.stdout.write("flom_handle_get_resource_quantity() = " + 
                     str(flom_handle_get_resource_quantity(my_handle)) + "\n");
    # check resource quantity
    ret_cod = flom_handle_get_resource_quantity(my_handle)
    if 3 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_quantity\n");
        sys.exit(1);

    # get current value for resource lock mode property
    sys.stdout.write("flom_handle_get_lock_mode() = " +
                     str(flom_handle_get_lock_mode(my_handle)) + "\n");
    # set a new value for resource lock mode property
    flom_handle_set_lock_mode(my_handle, FLOM_LOCK_MODE_PW);
    # get new value for resource lock mode property
    sys.stdout.write("flom_handle_get_lock_mode() = " +
                     str(flom_handle_get_lock_mode(my_handle)) + "\n");
    # check resource lock mode
    ret_cod = flom_handle_get_lock_mode(my_handle)
    if FLOM_LOCK_MODE_PW != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set" + 
                         "/get_lock_mode\n");
        sys.exit(1);
    
    # get current value for resource idle lifespan
    sys.stdout.write("flom_handle_get_resource_idle_lifespan() = " +
                     str(flom_handle_get_resource_idle_lifespan(my_handle)) +
                     "\n");
    # set a new value for resource idle lifespan
    flom_handle_set_resource_idle_lifespan(my_handle, 10000);
    # get new value for resource idle lifespan
    sys.stdout.write("flom_handle_get_resource_idle_lifespan() = " +
                     str(flom_handle_get_resource_idle_lifespan(my_handle)) +
                     "\n");
    # check resource idle lifespan
    ret_cod = flom_handle_get_resource_idle_lifespan(my_handle)
    if 10000 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_resource_idle_lifespan\n");
        sys.exit(1);
    
    # get current unicast address
    sys.stdout.write("flom_handle_get_unicast_address() = '" + 
           str(flom_handle_get_unicast_address(my_handle)) + "'\n");
    # set a new unicast_address
    flom_handle_set_unicast_address(my_handle, nd_unicast_address);
    # get new unicast address
    sys.stdout.write("flom_handle_get_unicast_address() = '" +
           str(flom_handle_get_unicast_address(my_handle)) + "'\n");
    # check unicast address
    if nd_unicast_address != flom_handle_get_unicast_address(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                	   "get_unicast_address\n");
        sys.exit(1);
    
    # get current multicast address */
    sys.stdout.write("flom_handle_get_multicast_address() = '" +
           str(flom_handle_get_multicast_address(my_handle)) + "'\n");
    # set a new multicast_address
    flom_handle_set_multicast_address(my_handle, nd_multicast_address);
    # get new multicast address
    sys.stdout.write("flom_handle_get_multicast_address() = '" +
           str(flom_handle_get_multicast_address(my_handle)) + "'\n");
    # check multicast address
    if nd_multicast_address != flom_handle_get_multicast_address(my_handle):
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_multicast_address\n");
        sys.exit(1);
    
    # set AF_UNIX/PF_LOCAL socket_name again
    ret_cod = flom_handle_set_socket_name(my_handle, nd_socket_name);
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("flom_handle_set_socket_name() returned " +
	    		 str(ret_cod) + ", '" + flom_strerror(ret_cod) + 
                         "'\n");
        sys.exit(1);
    
    # get current value for unicast port
    sys.stdout.write("flom_handle_get_unicast_port() = " +
           str(flom_handle_get_unicast_port(my_handle)) + "\n");
    # set a new value for unicast_port
    flom_handle_set_unicast_port(my_handle, 7777);
    # get new value for unicast port
    sys.stdout.write("flom_handle_get_unicast_port() = " +
           str(flom_handle_get_unicast_port(my_handle)) + "\n");
    # check unicast port
    ret_cod = flom_handle_get_unicast_port(my_handle);
    if 7777 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_unicast_port\n");
        sys.exit(1);
    
    # get current value for multicast port
    sys.stdout.write("flom_handle_get_multicast_port() = " +
           str(flom_handle_get_multicast_port(my_handle)) + "\n");
    # set a new value for multicast_port
    flom_handle_set_multicast_port(my_handle, 8888);
    # get new value for multicast port
    sys.stdout.write("flom_handle_get_multicast_port() = " +
           str(flom_handle_get_multicast_port(my_handle)) + "\n");
    # check multicast port
    ret_cod = flom_handle_get_multicast_port(my_handle);
    if 8888 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_multicast_port\n");
        sys.exit(1);
    
    # get current value for discovery attempts property
    sys.stdout.write("flom_handle_get_discovery_attempts() = " +
           str(flom_handle_get_discovery_attempts(my_handle)) + "\n");
    # set a new value for discovery attempts property
    flom_handle_set_discovery_attempts(my_handle, 5);
    # get new value for discovery attempts
    sys.stdout.write("flom_handle_get_discovery_attempts() = " +
           str(flom_handle_get_discovery_attempts(my_handle)) + "\n");
    # check discovery attempts
    ret_cod = flom_handle_get_discovery_attempts(my_handle);
    if 5 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_discovery_attempts\n");
        sys.exit(1);
    
    # get current value for discovery timeout property
    sys.stdout.write("flom_handle_get_discovery_timeout() = " +
                     str(flom_handle_get_discovery_timeout(my_handle)) + "\n");
    # set a new value for discovery timeout property
    flom_handle_set_discovery_timeout(my_handle, 750);
    # get new value for discovery timeout
    sys.stdout.write("flom_handle_get_discovery_timeout() = " +
                     str(flom_handle_get_discovery_timeout(my_handle)) + "\n");
    # check discovery timeout
    ret_cod = flom_handle_get_discovery_timeout(my_handle);
    if 750 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_discovery_timeout\n");
        sys.exit(1);
    
    # get current value for discovery ttl property
    sys.stdout.write("flom_handle_get_discovery_ttl() = " +
                     str(flom_handle_get_discovery_ttl(my_handle)) + "\n");
    # set a new value for discovery ttl property
    flom_handle_set_discovery_ttl(my_handle, 2);
    # get new value for discovery ttl
    sys.stdout.write("flom_handle_get_discovery_ttl() = " +
                     str(flom_handle_get_discovery_ttl(my_handle)) + "\n");
    # check discovery ttl
    ret_cod = flom_handle_get_discovery_ttl(my_handle);
    if 2 != ret_cod:
        sys.stderr.write("Unexpected result from flom_handle_set/" +
                         "get_discovery_ttl\n");
        sys.exit(1);
    
    # lock acquisition
    ret_cod = flom_handle_lock(my_handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_lock() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # retrieve locked element
    sys.stdout.write("happy_path locked element is '" +
                     flom_handle_get_locked_element(my_handle) + "'\n")
    # lock release
    ret_cod = flom_handle_unlock(my_handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_unlock() returned " +
                         str(ret_cod) + " '" + flom_strerror(ret_cod) +
                         "'\n")
        sys.exit(1);

    # handle clean-up (memory release)
    ret_cod = flom_handle_clean(my_handle)
    if FLOM_RC_OK != ret_cod:
        sys.stderr.write("happy_path/flom_handle_clean() returned " + 
                         str(ret_cod) + " '" + flom_strerror(ret_cod) + "'\n")
        sys.exit(1);

    # handle deallocation
    del my_handle



happy_path()

