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
if test $# -lt 1
then
	echo "$0 requires at least one parameter: IPv4 or IPv6"
	exit 1
fi

IPADDR=""
if test "$1"z = IPv4z
then
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
elif test "$1"z = IPv6z
then
	# check if ip -o -6 addr is available
	ip -o -6 addr >/dev/null 2>&1
	if test $? -eq 0
	then
		IPADDR=$(ip -o -6 addr | tr '/' ' ' | awk '!/^[0-9]*: ?lo|link\/ether/ {print $4}' | head -1)
	fi
else
	echo "First parameter must be IPv4 or IPv6"
	exit 1
fi

if test "$IPADDR"z = z
then
	echo "Unable to retrieve non loopback IP address"
	exit 1
else
	echo $IPADDR
fi
