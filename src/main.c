/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLOM.
 *
 * FLOM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * FLOM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLOM.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>



#include "flom_trace.h"



#define FLOM_TRACE_MODULE FLOM_TRACE_MOD_GENERIC



int main (int argc, char *argv[])
{
    FLOM_TRACE_INIT;
        
	printf("Hello world\n");
    FLOM_TRACE(("this is a trace\n"));
	return 0;
}
