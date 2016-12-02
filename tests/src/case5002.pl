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

use strict;
use warnings;

use Flom;

#
# Non default values used for tests
#
my $nd_socket_name = "/tmp/flom_socket_name";
my $nd_trace_filename = "/tmp/flom.trc";
my $nd_resource_name = "red.green.blue";
my $nd_unicast_address = "127.0.0.1";
my $nd_multicast_address = "224.0.0.1";
my $nd_tls_certificate = "CA1/peer1_CA1_cert.pem";
my $nd_tls_private_key = "CA1/peer1_CA1_key.pem";
my $nd_tls_ca_certificate = "CA1/cacert.pem";

my $ret_cod = Flom::RC_OK;
my $handle;

# step 1: handle creation
unless ($handle = Flom::handle_new()) {
    printf(STDERR "Flom::handle_new() returned %p\n", $handle);
    exit 1;
}

# get current AF_UNIX/PF_LOCAL socket_name
printf(STDOUT "Flom::handle_get_socket_name() = '%s'\n",
       Flom::handle_get_socket_name($handle));
# set a new AF_UNIX/PF_LOCAL socket_name
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_socket_name(
	    		$handle, $nd_socket_name))) {
    printf(STDERR "Flom::handle_set_socket_name() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# get new AF_UNIX/PF_LOCAL socket_name
printf(STDOUT "Flom::handle_get_socket_name() = '%s'\n",
       Flom::handle_get_socket_name($handle));
# check socket name
if ($nd_socket_name ne Flom::handle_get_socket_name($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_socket_name\n");
    exit 1;
}

# we don't get current trace filename because it can be altered by a
# global config file
# set a new trace filename
Flom::handle_set_trace_filename($handle, $nd_trace_filename);
# get new trace filename
printf(STDOUT "Flom::handle_get_trace_filename() = '%s'\n",
       Flom::handle_get_trace_filename($handle));
# check trace filename
if ($nd_trace_filename ne Flom::handle_get_trace_filename($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_trace_filename\n");
    exit 1;
}

# get current resource name
printf(STDOUT "Flom::handle_get_resource_name() = '%s'\n",
       Flom::handle_get_resource_name($handle));
# set a new resource name
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_resource_name(
	    		$handle, $nd_resource_name))) {
    printf(STDERR "Flom::handle_set_resource_name() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# get new resource name
printf(STDOUT "Flom::handle_get_resource_name() = '%s'\n",
       Flom::handle_get_resource_name($handle));

# get current value for resource create property
printf(STDOUT "Flom::handle_get_resource_create() = %d\n",
       Flom::handle_get_resource_create($handle));
# set a new value for resource create property
Flom::handle_set_resource_create($handle, Flom::FALSE);
# get new value for resource create property
printf(STDOUT "Flom::handle_get_resource_create() = %d\n",
       Flom::handle_get_resource_create($handle));
# check resource create 1/2
if (Flom::handle_get_resource_create($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_resource_create\n");
    exit 1;
}
# set a new value for resource create property
Flom::handle_set_resource_create($handle, Flom::TRUE);
# get new value for resource create property
printf(STDOUT "Flom::handle_get_resource_create() = %d\n",
       Flom::handle_get_resource_create($handle));
# check resource create 2/2
if (!Flom::handle_get_resource_create($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_resource_create\n");
    exit 1;
}
	 
# get current value for resource timeout property
printf(STDOUT "Flom::handle_get_resource_timeout() = %d\n",
       Flom::handle_get_resource_timeout($handle));
# set a new value for resource timeout property
Flom::handle_set_resource_timeout($handle, -1);
# get new value for resource timeout property
printf(STDOUT "Flom::handle_get_resource_timeout() = %d\n",
       Flom::handle_get_resource_timeout($handle));
# check resource timeout
if (-1 != Flom::handle_get_resource_timeout($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_resource_timeout\n");
    exit 1;
}
    
# get current value for resource quantity property
printf(STDOUT "Flom::handle_get_resource_quantity() = %d\n",
       Flom::handle_get_resource_quantity($handle));
# set a new value for resource quantity property
Flom::handle_set_resource_quantity($handle, 3);
# get new value for resource quantity property
printf(STDOUT "Flom::handle_get_resource_quantity() = %d\n",
       Flom::handle_get_resource_quantity($handle));
# check resource quantity
if (3 != Flom::handle_get_resource_quantity($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_resource_quantity\n");
    exit 1;
}

# get current value for resource lock mode property
printf(STDOUT "Flom::handle_get_lock_mode() = %d\n",
       Flom::handle_get_lock_mode($handle));
# set a new value for resource lock mode property
Flom::handle_set_lock_mode($handle, Flom::LOCK_MODE_PW);
# get new value for resource lock mode property
printf(STDOUT "Flom::handle_get_lock_mode() = %d\n",
       Flom::handle_get_lock_mode($handle));
# check resource lock mode
if (Flom::LOCK_MODE_PW != Flom::handle_get_lock_mode($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_lock_mode\n");
    exit 1;
}
    
# get current value for resource idle lifespan
printf(STDOUT "Flom::handle_get_resource_idle_lifespan() = %d\n",
       Flom::handle_get_resource_idle_lifespan($handle));
# set a new value for resource idle lifespan
Flom::handle_set_resource_idle_lifespan($handle, 10000);
# get new value for resource idle lifespan
printf(STDOUT "Flom::handle_get_resource_idle_lifespan() = %d\n",
       Flom::handle_get_resource_idle_lifespan($handle));
# check resource idle lifespan
if (10000 != Flom::handle_get_resource_idle_lifespan($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_resource_idle_lifespan\n");
    exit 1;
}
    
# get current unicast address
printf(STDOUT "Flom::handle_get_unicast_address() = '%s'\n",
       Flom::handle_get_unicast_address($handle));
# set a new unicast_address
Flom::handle_set_unicast_address($handle, $nd_unicast_address);
# get new unicast address
printf(STDOUT "Flom::handle_get_unicast_address() = '%s'\n",
       Flom::handle_get_unicast_address($handle));
# check unicast address
if ($nd_unicast_address ne Flom::handle_get_unicast_address($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_unicast_address\n");
    exit 1;
}
    
# get current multicast address
printf(STDOUT "Flom::handle_get_multicast_address() = '%s'\n",
       Flom::handle_get_multicast_address($handle));
# set a new multicast_address
Flom::handle_set_multicast_address($handle, $nd_multicast_address);
# get new multicast address
printf(STDOUT "Flom::handle_get_multicast_address() = '%s'\n",
       Flom::handle_get_multicast_address($handle));
# check multicast address
if ($nd_multicast_address ne Flom::handle_get_multicast_address($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_multicast_address\n");
    exit 1;
}
    
# set AF_UNIX/PF_LOCAL socket_name again
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_socket_name(
			$handle, $nd_socket_name))) {
    printf(STDERR "Flom::handle_set_socket_name() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
    
# get current value for unicast port
printf(STDOUT "Flom::handle_get_unicast_port() = %d\n", 
       Flom::handle_get_unicast_port($handle));
# set a new value for unicast_port
Flom::handle_set_unicast_port($handle, 7777);
# get new value for unicast port
printf(STDOUT "Flom::handle_get_unicast_port() = %d\n",
       Flom::handle_get_unicast_port($handle));
# check unicast port
if (7777 != Flom::handle_get_unicast_port($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_unicast_port\n");
    exit 1;
}
    
# get current value for multicast port
printf(STDOUT "Flom::handle_get_multicast_port() = %d\n",
       Flom::handle_get_multicast_port($handle));
# set a new value for multicast_port
Flom::handle_set_multicast_port($handle, 8888);
# get new value for multicast port
printf(STDOUT "Flom::handle_get_multicast_port() = %d\n",
       Flom::handle_get_multicast_port($handle));
# check multicast port
if (8888 != Flom::handle_get_multicast_port($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_multicast_port\n");
    exit 1;
}
    
# get current value for discovery attempts property
printf(STDOUT "Flom::handle_get_discovery_attempts() = %d\n",
       Flom::handle_get_discovery_attempts($handle));
# set a new value for discovery attempts property
Flom::handle_set_discovery_attempts($handle, 5);
# get new value for discovery attempts
printf(STDOUT "Flom::handle_get_discovery_attempts() = %d\n",
       Flom::handle_get_discovery_attempts($handle));
# check discovery attempts
if (5 != Flom::handle_get_discovery_attempts($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_discovery_attempts\n");
    exit 1;
}
    
# get current value for discovery timeout property
printf(STDOUT "Flom::handle_get_discovery_timeout() = %d\n",
       Flom::handle_get_discovery_timeout($handle));
# set a new value for discovery timeout property
Flom::handle_set_discovery_timeout($handle, 750);
# get new value for discovery timeout
printf(STDOUT "Flom::handle_get_discovery_timeout() = %d\n",
       Flom::handle_get_discovery_timeout($handle));
# check discovery timeout
if (750 != Flom::handle_get_discovery_timeout($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_discovery_timeout\n");
    exit 1;
}
    
# get current value for discovery ttl property
printf(STDOUT "Flom::handle_get_discovery_ttl() = %d\n",
       Flom::handle_get_discovery_ttl($handle));
# set a new value for discovery ttl property
Flom::handle_set_discovery_ttl($handle, 2);
# get new value for discovery ttl
printf(STDOUT "Flom::handle_get_discovery_ttl() = %d\n",
       Flom::handle_get_discovery_ttl($handle));
# check discovery ttl
if (2 != Flom::handle_get_discovery_ttl($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_discovery_ttl\n");
    exit 1;
}
    
# get current value for TLS certificate
printf(STDERR "Flom::handle_get_tls_certificate() = %d\n",
       Flom::handle_get_tls_certificate($handle));
# set a new TLS certificate
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_tls_certificate(
			$handle, $nd_tls_certificate))) {
    printf(STDERR "Flom::handle_set_tls_certificate() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}

# get new TLS certificate
printf(STDERR "Flom::handle_get_tls_certificate() = '%s'\n",
       Flom::handle_get_tls_certificate($handle));

# get current value for TLS private key
printf(STDERR "Flom::handle_get_tls_private_key() = '%s'\n",
       Flom::handle_get_tls_private_key($handle));
# set a new TLS private key
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_tls_private_key(
			$handle, $nd_tls_private_key))) {
    printf(STDERR "Flom::handle_set_tls_private_key() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# get new TLS private key
printf(STDERR "Flom::handle_get_tls_private_key() = '%s'\n",
       Flom::handle_get_tls_private_key($handle));

# get current value for TLS CA certificate
printf(STDERR "Flom::handle_get_tls_ca_certificate() = '%s'\n",
       Flom::handle_get_tls_ca_certificate($handle));
# set a new TLS CA certificate
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_tls_ca_certificate(
			$handle, $nd_tls_ca_certificate))) {
    printf(STDERR "Flom::handle_set_tls_ca_certificate() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# get new TLS CA certificate
printf(STDERR "Flom::handle_get_tls_private_key() = '%s'\n",
       Flom::handle_get_tls_private_key($handle));

# get current value for TLS check peer ID property
printf(STDOUT "Flom::handle_get_tls_check_peer_id() = %d\n",
       Flom::handle_get_tls_check_peer_id($handle));
# set a new value for TLS check peer ID property
Flom::handle_set_tls_check_peer_id($handle, Flom::FALSE);
# get new value for TLS check peer ID property
printf(STDOUT "Flom::handle_get_tls_check_peer_id() = %d\n",
       Flom::handle_get_tls_check_peer_id($handle));
# check TLS check peer ID 1/2
if (Flom::handle_get_tls_check_peer_id($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_tls_check_peer_id\n");
    exit 1;
}
# set a new value for TLS check peer ID property
Flom::handle_set_tls_check_peer_id($handle, Flom::TRUE);
# get new value for TLS check peer ID property
printf(STDOUT "Flom::handle_get_tls_check_peer_id() = %d\n",
       Flom::handle_get_tls_check_peer_id($handle));
# check TLS check peer ID 2/2
if (!Flom::handle_get_tls_check_peer_id($handle)) {
    printf(STDERR "Unexpected result from Flom::handle_set/get_tls_check_peer_id\n");
    exit 1;
}

# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# retrieve locked element
printf(STDOUT "locked element is '%s'\n",
       Flom::handle_get_locked_element($handle));
# lock release
if (Flom::RC_OK != ($ret_cod = Flom::handle_unlock($handle))) {
    printf(STDERR "Flom::handle_unlock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# handle delete
Flom::handle_delete($handle);

exit 0;
