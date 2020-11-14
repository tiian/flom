#!/bin/bash
#
# Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
# All rights reserved.
#
# This file is part of FLOM.
#
# FLOM is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# FLOM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
#

# Options:
# $1  Signal: SIGTERM or another string like ABC
#
# Exit code:
# 0 - the flom monitor has been terminated 
# 1 - the flom monitor completed normally (signal ignored)

if test $# -lt 1
then
        echo "At least one parameter must be specified"
        exit 1
fi

kill_flom() {
	echo "KKK>>> Killer function started and sleeping a little bit..."
	# wait 1 second to be sure the monitor is up and running
	sleep 1
	# get PGID
	PGID=$(ps j -A | grep 'sleep' | grep -v 'grep\|flom' | awk '{print $3}')
	echo "KKK>>> Processes with PGID=$PGID"
	ps j -A | grep "PGID\|$PGID" | grep -v 'grep'
	# send SIGTERM to the group of processes
	/bin/kill -SIGTERM -$PGID
	echo "KKK>>> kill return code: " $?
}

# start the killer function in background
kill_flom &

# activate monitoring to start the child process in a group different than 
# shell's one
set -m
# execute in background
flom --ignore-signal=$1 -- sleep 1d
RC=$?
echo "flom return code: " $RC
set +m

# returning FLoM RC
exit $RC
