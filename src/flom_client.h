/*
 * Copyright (c) 2013-2020, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM, Free Lock Manager
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLoM.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CLIENT_H
# define CLIENT_H



#include <config.h>



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_CLIENT



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Try to connect to flom daemon
     * @param config IN configuration object, NULL for global config
     * @param conn OUT connection object
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect(flom_config_t *config,
                            flom_conn_t *conn, int start_daemon);
    


    /**
     * Try to connect to flom daemon using local (AF_LOCAL) socket
     * @param config IN configuration object, NULL for global config
     * @param conn OUT connection object
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect_local(flom_config_t *config,
                                  flom_conn_t *conn,
                                  int start_daemon);
    


    /**
     * Try to connect to flom daemon using network (TCP/IP) socket
     * @param config IN configuration object, NULL for global config
     * @param conn OUT connection object
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect_tcp(flom_config_t *config,
                                flom_conn_t *conn,
                                int start_daemon);



    /**
     * Discover flom daemon address using multicast UDP
     * @param config IN configuration object, NULL for global config
     * @param conn IN/OUT connection object
     * @param start_daemon IN try to start a new daemon if connection fails
     * @param family OUT AF_INET or AF_INET6
     * @return a reason code
     */
    int flom_client_discover_udp(flom_config_t *config,
                                 flom_conn_t *conn,
                                 int start_daemon,
                                 sa_family_t *family);



    /**
     * Connect to a daemon using TCP/IP; daemon was previously discovered
     * using multicast UDP/IP
     * @param conn IN/OUT connection object
     * @param so IN address (IPv4 or IPv6) and port of daemon
     * @param addrlen size of sa struct
     * @return a reason code
     */
    int flom_client_discover_udp_connect(flom_conn_t *conn,
                                         const struct sockaddr *so,
                                         socklen_t addrlen);
    
    

    /**
     * Disconnect from lock daemon
     * @param conn IN/OUT connection object
     * @result a reason code
     */
    int flom_client_disconnect(flom_conn_t *conn);
    


    /**
     * Send lock command to the daemon
     * @param config IN configuration object
     * @param conn IN connection object
     * @param timeout IN maximum wait time for lock acquisition
     * @param element OUT the obtained element if the locked resource is of
     *        type resource set, NULL if the locked resource
     *        does not returns an element. The return name is null terminated.
     *        Note: the string allocated with g_malloc and MUST be freed by the
     *        caller using g_free!
     * @return a reason code
     */
    int flom_client_lock(flom_config_t *config, flom_conn_t *conn,
                         int timeout, char **element);



    /**
     * Wait while the desired resource is busy, then go on
     * @param conn IN connection object
     * @param msg IN/OUT message used to deserialize the replies
     * @param timeout IN maximum wait time for lock acquisition
     * @return a reason code
     */     
    int flom_client_wait_lock(flom_conn_t *conn,
                              struct flom_msg_s *msg, int timeout);



    /**
     * Send unlock command to the daemon
     * @param config IN configuration object
     * @param conn IN connection object
     * @param rollback IN rollback resource value (it is useful only for
     *        transactional resources, ignored for non transactional resources)
     * @return a reason code
     */
    int flom_client_unlock(flom_config_t *config, flom_conn_t *conn,
                           int rollback);



    /**
     * Connect to daemon and send a shutdown message
     * @param config IN configuration object, NULL for global config
     * @param immediate IN is the shutdown immediate (else it's quiesce)
     * @return a reason code
     */
    int flom_client_shutdown(flom_config_t *config, int immediate);



#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* CLIENT_H */
