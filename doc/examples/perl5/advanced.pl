#!/usr/bin/perl
#
# Copyright (c) 2016, Kevin L. Esteb <kevin@kesteb.us>
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
# ------------------------------------------------------------------------
#
# This example program shows the usage of libflom API library from a
# Python program; it uses a resource set instead of the default resource and
# displays the name of the obtained element.
#
# These are the steps:
# 1. create and allocate an object of type flom_handle_t
# 2. initialize the allocated handle using function Flom::handle_init()
# 3. set custom properties different from default values:
#    3a. use a different AF_UNIX/PF_LOCAL socket to reach FLoM daemon
#    3b. specify a resource name to lock
# 4. acquire a lock using function Flom::handle_lock()
# 5. execute the code protected by the acquired lock
# 6. release the lock using function Flom::handle_unlock()
# 7. clean-up the allocated handle using function Flom::handle_clean()
# 8. deallocate the handle object
#
# Execution command:
#
#     perl advanced.pl
# 
# Note: this program needs an already started FLoM daemon, for instance:
#
#     flom -s /tmp/my_socket_name -d -1 -- true
#     perl advanced.pl
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#
#     export FLOM_TRACE_MASK=0x80000
#     perl advanced.pl
#

use strict;
use warnings;

use Flom;

my $rc;
my $handle;

# step 2: new handle creation

unless ($handle = Flom::handle_new) {

    printf("Flom::handle_new() returned %p\n", $handle);
    exit 1;

}

# step 3a: set a different AF_UNIX/PF_LOCAL socket to connect to FLoM

if (($rc = Flom::handle_set_socket_name($handle, '/tmp/my_socket_name')) != Flom::RC_OK) {

    printf("flom_handle_set_socket_name() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 3b: set a different (non default) resource name to lock

if (($rc = Flom::handle_set_resource_name($handle, 'Red.Blue.Green')) != Flom::RC_OK) {
    
    printf("Flom::handle_set_resource_name() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;
    
}

# step 4: lock acquistion

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {
    
    printf("Flom::handle_lock() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::handle_get_locked_element(): %s\n", 
        Flom::handle_get_locked_element($handle));

}

# step 5: execute the code that needs lock protection

sleep(10);

# step 6: lock release

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {
    
    printf("Flom::handle_unlock() returned %s, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

fini:
  Flom::handle_delete($handle);

