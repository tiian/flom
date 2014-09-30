#!/bin/sh
#
# Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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

# Options:
# $1 ID
# $2 delay time
# $3 duration time
# $4 flom args

if test $# -lt 4
then
	echo "At least four parameters must be specified"
	exit 1
fi	

# Triple comments are used to distinguish debug rows

# print start message
###echo -n $(date +'%s %N')
###echo " $1 starting and waiting $2 seconds"
# wait some seconds...
sleep $2 
# execution with duration
###echo -n $(date +'%s %N')
echo " $1 locking for $3 seconds"
###echo "flom $4 -- sleep $3"
flom $4 -- sleep_and_echo.sh $3
EXIT_RC=$?
# print end message
###echo -n $(date +'%s %N')
echo " $1 ending"
exit $EXIT_RC
