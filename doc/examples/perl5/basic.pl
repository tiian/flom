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
# This example program shows the basic usage of libflom API library from a
# Perl5 program.
#
# These are the steps:
# 1. create and allocate an object of type flom_handle_t
# 2. initialize the allocated handle using function Flom::handle_init()
# 3. acquire a lock using function Flom::handle_lock()
# 4. execute the code protected by the acquired lock
# 5. release the lock using function Flom::handle_unlock()
# 6. clean-up the allocated handle using function Flom::handle_clean()
# 7. deallocate the handle object
#
# Execution command:
#
#     perl basic.pl
# 
# Note: this program needs an already started FLoM daemon, for instance:
#
#     flom -d -1 -- true
#     perl basic.pl
#
# The program itself is not verbose, but you might activate tracing if you
# were interested to understand what's happen:
#
#     export FLOM_TRACE_MASK=0x80000
#     perl basic.pl
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

# step 3: lock acquisition

if (($rc = Flom::handle_lock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_lock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 4: execute the code that needs lock protection

sleep(10);

# step 5: lock release

if (($rc = Flom::handle_unlock($handle)) != Flom::RC_OK) {

    printf("Flom::handle_unlock() returned %d, %s\n", $rc, Flom::strerror($rc));
    goto fini;

}

# step 6: delete the lock

fini:
  Flom::handle_delete($handle);

