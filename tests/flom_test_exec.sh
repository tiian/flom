#!/bin/sh
#
# Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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
# $1 ID
# $2 sleep time

if test $# -lt 2
then
	echo "At least two parameters must be specified"
	exit 1
fi	

# print start message
echo "$1 starting"
# wait some seconds...
sleep $2 
# print end message
echo "$1 ending"
