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

my $ret_cod = Flom::RC_OK;
my $handle1;
my $handle2;

# create first handle
unless ($handle1 = Flom::handle_new()) {
    printf(STDERR "Flom::handle_new() returned %p ($handle1)\n", $handle1);
    exit 1;
}

# setting the resource name: non transactional sequence
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_resource_name(
			$handle1, "_s_nontransactional[1]"))) {
    printf(STDERR "Flom::handle_set_resource_name() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# set a new value for resource idle lifespan
Flom::handle_set_resource_idle_lifespan($handle1, 60000);
# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle1))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
} else {
    # retrieve locked element
    printf(STDOUT "locked element is %s\n",
	   Flom::handle_get_locked_element($handle1));
}
# lock release & rollback: the resource is not transactional, the
# function must return a warning condition
if (Flom::RC_RESOURCE_IS_NOT_TRANSACTIONAL != (
		$ret_cod = Flom::handle_unlock_rollback($handle1))) {
    printf(STDERR "Flom::handle_unlock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle1))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
} else {
    # retrieve locked element
    printf(STDOUT "locked element is %s\n",
	   Flom::handle_get_locked_element($handle1));
}
# the resource associated to handle1 is intentionally not unlocked
# to check the behavior in case of abort

# Second step: transactional resource
# create a new handle
unless ($handle2 = Flom::handle_new()) {
    printf(STDERR "Flom::handle_new() returned %p ($handle2)\n", $handle2);
    exit 1;
}

# setting the resource name: transactional sequence
if (Flom::RC_OK != ($ret_cod = Flom::handle_set_resource_name(
		$handle2, "_S_nontransactional[1]"))) {
    printf(STDERR "Flom::handle_set_resource_name() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# set a new value for resource idle lifespan
Flom::handle_set_resource_idle_lifespan($handle2, 60000);
# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle2))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
} else {
    # retrieve locked element
    printf(STDOUT "locked element is %s\n",
	   Flom::handle_get_locked_element($handle2));
}
# lock release & rollback
if (Flom::RC_OK != ($ret_cod = Flom::handle_unlock_rollback($handle2))) {
    printf(STDERR "Flom::handle_unlock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle2))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
} else {
    # retrieve locked element
    printf(STDOUT "locked element is %s\n",
	   Flom::handle_get_locked_element($handle2));
}
# lock release & rollback
if (Flom::RC_OK != ($ret_cod = Flom::handle_unlock($handle2))) {
    printf(STDERR "Flom::handle_unlock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
}
# lock acquisition
if (Flom::RC_OK != ($ret_cod = Flom::handle_lock($handle2))) {
    printf(STDERR "Flom::handle_lock() returned %d, %s\n",
	   $ret_cod, Flom::strerror($ret_cod));
    exit 1;
} else {
    # retrieve locked element
    printf(STDOUT "locked element is %s\n",
	   Flom::handle_get_locked_element($handle2));
}
# interrupt execution to verify transactionality (the program must be
# restarted
exit 0;
# this point will be never reached!

