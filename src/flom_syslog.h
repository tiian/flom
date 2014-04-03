/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_SYSLOG_H
# define FLOM_SYSLOG_H



#include <config.h>



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


    
/* This file contains only syslog messages: they are specified as macros
   because every message must be used only once inside the source code;
   used messages code:
   D: LOG_DEBUG
   I: LOG_INFO
   N: LOG_NOTICE
   W: LOG_WARNING
   E: LOG_ERR
   C: LOG_CRIT
*/
#define FLOM_SYSLOG_FLM000I "FLM000I this process is activating a local " \
    "FLOM daemon listening UNIX socket '%s'"
#define FLOM_SYSLOG_FLM001I "FLM001I this process is activating a network " \
    "FLOM daemon listening TCP/IP socket %s/%u"
#define FLOM_SYSLOG_FLM002I "FLM002I joined multicast group %s/%u to " \
    "answer auto-discovery queries"
#define FLOM_SYSLOG_FLM003N "FLM003N FLOM daemon ready to serve incoming " \
    "requests"
#define FLOM_SYSLOG_FLM004N "FLM004N FLOM daemon has terminated to serve " \
    "incoming requests"
#define FLOM_SYSLOG_FLM005I "FLM005I auto-discovery query received from " \
    "%s/%s"



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_SYSLOG_H */
