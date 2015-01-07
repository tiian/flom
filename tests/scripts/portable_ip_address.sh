#!/bin/sh
#
# Copyright (c) 2013-2015, Christian Ferrari <tiian@users.sourceforge.net>
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

IPADDR=""
# check if hostname -I is available
hostname -I >/dev/null 2>&1
if test $? -eq 0
then
	IPADDR=$(hostname -I | cut -f 1 -d ' ')
else
	# check if ip -o -4 addr is available
	ip -o -4 addr >/dev/null 2>&1
	if test $? -eq 0
	then
		IPADDR=$(ip -o -4 addr | tr '/' ' ' | awk '!/^[0-9]*: ?lo|link\/ether/ {print $4}' | head -1)
	fi
fi

if test "$IPADDR"z = z
then
	echo "Unable to retrieve non loopback IP address"
	exit 1
else
	echo $IPADDR
fi
