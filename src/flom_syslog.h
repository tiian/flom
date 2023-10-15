/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_SYSLOG_H
# define FLOM_SYSLOG_H



#include <config.h>



#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif



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
    "FLoM daemon listening UNIX socket '%s'"
#define FLOM_SYSLOG_FLM001I "FLM001I this process is activating a network " \
    "FLoM daemon listening TCP/IP socket %s/%u"
#define FLOM_SYSLOG_FLM002I "FLM002I joined multicast group %s/%u to " \
    "answer auto-discovery queries"
#define FLOM_SYSLOG_FLM003N "FLM003N FLoM daemon is ready to serve incoming " \
    "requests"
#define FLOM_SYSLOG_FLM004N "FLM004N FLoM daemon has terminated to serve " \
    "incoming requests"
#define FLOM_SYSLOG_FLM005I "FLM005I auto-discovery query received from " \
    "%s/%s"
#define FLOM_SYSLOG_FLM006W "FLM006W FLoM daemon is using comm. level %d, " \
    "FLOM client is using comm. level %d; communication can not be performed"
#define FLOM_SYSLOG_FLM007N "FLM007N FLoM daemon is exiting due to " \
    "immediate shutdown request"
#define FLOM_SYSLOG_FLM008N "FLM008N FLoM daemon is starting quiesce " \
    "shutdown"
#define FLOM_SYSLOG_FLM009W "FLM009W a client wants to unlock resource '%s' " \
    "but it has locked resource '%s'"
#define FLOM_SYSLOG_FLM010W "FLM010W the peer has not presented an X.509 certificate"
#define FLOM_SYSLOG_FLM011I "FLM011I X.509 CA certificate fields are %s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s"
#define FLOM_SYSLOG_FLM012I "FLM012I X.509 peer certificate fields are %s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s/%s=%s"
#define FLOM_SYSLOG_FLM013E "FLM013E peer '%s' sent unique ID '%s' that does not match che CN field '%s' inside the presented X.509 certificate"
#define FLOM_SYSLOG_FLM014I "FLM014I peer '%s' with unique ID '%s' was authenticated using CN field '%s' inside the presented X.509 certificate"
#define FLOM_SYSLOG_FLM015I "FLM015I a connecting peer sent id '%s' for verb %d and step %d"
#define FLOM_SYSLOG_FLM016I "FLM016I the serving peer sent id '%s' for verb %d and step %d"
#define FLOM_SYSLOG_FLM017E "FLM017E X.509 certificate error, SSL_get_verify_result returned %ld"
#define FLOM_SYSLOG_FLM018E "FLM018E X.509 certificate error related to private key: %s"
#define FLOM_SYSLOG_FLM019E "FLM019E 'bind' error %d ('%s') in function '%s'"
#define FLOM_SYSLOG_FLM020N "FLM020N flom_conn_recv returned %d ('%s') " \
    "instead of FLOM_RC_RECV_ERROR during flom_client_shutdown"
#define FLOM_SYSLOG_FLM021E "FLM021E consistency error in VFS inode mapping, VFS can not be activated"
#define FLOM_SYSLOG_FLM022I "FLM022I mounting FUSE file system '%s'"
#define FLOM_SYSLOG_FLM023I "FLM023I unmounting FUSE file system with command '%s'"
#define FLOM_SYSLOG_FLM024N "FLM024N command '%s' exited with status %d"
    
    

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* FLOM_SYSLOG_H */
