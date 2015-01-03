<?php 
/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */

include("flom.php");

$handle = new flom_handle_t();
/*
$element = "XXXXXXXXXXXXXXXXXXXXXXXXXXXX";
*/
$ret_cod = FLOM_RC_OK;
if (FLOM_RC_OK != ($ret_cod = flom_handle_init($handle))) {
	fwrite(STDERR, "flom_handle_init() returned " . $ret_cod . " '" . 
		flom_strerror($ret_cod) . "'\n");
	exit(1);
}
if (FLOM_RC_OK != ($ret_cod = flom_handle_lock($handle, NULL, 0))) {
	fwrite(STDERR, "flom_handle_lock() returned " . $ret_cod . " '" . 
		flom_strerror($ret_cod) . "'\n");
	exit(1);
}
if (FLOM_RC_OK != ($ret_cod = flom_handle_unlock($handle))) {
	fwrite(STDERR, "flom_handle_unlock() returned " . $ret_cod . " '" . 
		flom_strerror($ret_cod) . "'\n");
	exit(1);
}
if (FLOM_RC_OK != ($ret_cod = flom_handle_clean($handle))) {
	fwrite(STDERR, "flom_handle_clean() returned " . $ret_cod . " '" . 
		flom_strerror($ret_cod) . "'\n");
	exit(1);
}
$handle = NULL;

/*
flom_handle_set_resource_name($handle, "red.green.blue");
flom_handle_lock($handle, $element, strlen($element));
echo "Element is " . $element . "\n";
flom_handle_unlock($handle);
*/
?>