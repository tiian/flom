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
# -----------------------------------------------------------------------
#
# This example program shows the usage of libflom API library from a
# Perl5 program; it uses a resource set instead of the default resource and
# displays the name of the obtained element.
#
# These are the steps:
# 1. create and allocate an object of type flom_handle_t
# 2. initialize the allocated handle using function Flom::handle_init()
# 3a. set a non default resource name (a name valid for a trasactional
#     sequence resource)
# 3b. set a non default resource idle lifespan
# 4. acquire a lock using method Flom::handle_lock()
# 5. release the lock using method Flom::handle_unlock_rollback()
# 6. acquire a new lock using method Floma::handle_lock() and verifying the
#    FLoM daemon returns the same value
# 7. release the lock using method Flom::handle_unlock()
# 8. acquire a new lock using method Flom::handle_lock() and verifying the
#    FLoM daemon returnes a different value
# 9. sleep 5 seconds to allow program killing
# 10. release the lock using method Flom::handle_unlock()
# 11. clean-up the allocated handle using function Flom::handle_clean()
# 12. deallocate the handle object
#
# Execution command:
#
#     perl transactional.pl
# 
# Note: this program needs an already started FLoM daemon, for instance:
#
#     flom -d -1 -- true
#     perl transactional.pl
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#
#     export FLOM_TRACE_MASK=0x80000
#     perl transactional.pl
#

use strict;
use warnings;

use Flom;

my $rc;
my $handle;

# step 2: new handle creation

unless ($handle = Flom::handle_new()) {

    printf("Flom::handle_new() returned %p\n", $handle);
    exit 1;

}

# step 3a: set a different (non default) resource name to lock

if (($rc = Flom::handle_set_resource_name($handle, '_S_transact[1]')) != Flom::RC_OK) {

    printf("Flom::handle_set_resource_name() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 3b: set a different (non default) resource idle lifespan 

if (($rc = Flom::handle_set_resource_idle_lifespan($handle, 60000)) != Flom::RC_OK) {

    printf("Flom::handle_set_resource_idle_lifespan() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}
    
# step 4: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::_handle_get_locked_element(): '%s' (first lock)\n",
        Flom::handle_get_locked_element($handle));

}    

# step 5: lock release 

if (($rc = Flom::handle_unlock_rollback($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock_rollback() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 6: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {
    
    printf("Flom::_handle_get_locked_element(): '%s' (second lock)\n",
        Flom::handle_get_locked_element($handle));

}    

# step 7: lock release 

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 8: lock acquisition 

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

} elsif ($rc = Flom::handle_get_locked_element($handle)) {

    printf("Flom::_handle_get_locked_element(): '%s' (third lock)\n",
        Flom::handle_get_locked_element($handle));

}

# step 9: sleep 5 seconds to allow program killing

printf("The program is waiting 5 seconds: kill it with the [control]+[c] " .
    "keystroke and restart it to verify resource rollback...\n");
sleep(5);

# step 10: lock release 

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, '%s'\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 11: delete the handle 

fini:
  Flom::handle_delete($handle);

