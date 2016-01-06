#!/bin/sh
#
# Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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
# $1 IP family: inet or inet6
# $2 number of the interface 1, 2, 3, 4, ...

if test $# -lt 2
then
	echo "At least two parameters must be specified"
	exit 1
fi

ip -f $1 addr show | grep -E "^$2" | sed -re 's/^[[:digit:]]+\:[[:space:]]*([[:alnum:]]+)\:.*$/\1/'
