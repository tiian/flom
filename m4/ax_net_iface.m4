dnl @synopsis AX_NET_IFACE
dnl
dnl Copyright (c) 2009-2016, Christian Ferrari <tiian@users.sourceforge.net>
dnl All rights reserved.
dnl
dnl This file is part of FLoM, Free Lock Manager
dnl
dnl FLoM is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License version 2 as published
dnl by the Free Software Foundation.
dnl
dnl FLoM is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
dnl
dnl
dnl This macro provides test for first network device that's UP and support
dnl MULTICAST
dnl
dnl This macro calls:
dnl
dnl   AC_SUBST(NET_IFACE)
dnl
AC_DEFUN([AX_NET_IFACE], [
	AC_MSG_CHECKING([Looking for a valid network interface])
NET_IFACE=$(ip link show | grep MULTICAST | grep UP | head -n 1 | cut -d':' -f 2 | tr -d [[:blank:]])
if test -z $NET_IFACE
then
	AC_MSG_ERROR([cannot find the name of a valid network interface])
else
	AC_MSG_RESULT([$NET_IFACE])
fi
AC_SUBST([NET_IFACE])
])
