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
     * @param cd OUT connection data
     * @result a reason code
     */
    int flom_client_connect(struct flom_conn_data_s *cd);
    


    /**
     * Try to connect to flom daemon using local (AF_LOCAL) socket
     * @param cd OUT connection data
     * @result a reason code
     */
    int flom_client_connect_local(struct flom_conn_data_s *cd);
    


    /**
     * Try to connect to flom daemon using network (TCP/IP) socket
     * @param cd OUT connection data
     * @result a reason code
     */
    int flom_client_connect_tcp(struct flom_conn_data_s *cd);



    /**
     * A TCP/IP connection chance: it can be reapeted after daemon start-up
     * @param gai IN result obtained by getaddrinfo function
     * @param fd OUT file descriptor associated to the connected socket
     * @return the pointer to the element successfully connected, NULL if no
     *         element is available
     */
    const struct addrinfo *flom_client_connect_tcp_try(
        const struct addrinfo *gai, int *fd);
    


    /**
     * Discover flom daemon address using multicast UDP
     * @param cd IN/OUT connection data
     * @return a reason code
     */
    int flom_client_discover_udp(struct flom_conn_data_s *cd);



    /**
     * Connect to a daemon using TCP/IP; daemon was previously discovered
     * using multicast UDP/IP
     * @param cd IN/OUT connection data
     * @param soin IN address and port of daemon
     * @return a reason code
     */
    int flom_client_discover_udp_connect(struct flom_conn_data_s *cd,
                                         const struct sockaddr_in *soin);
    
    

    /**
     * Disconnect from lock daemon
     * @param cd IN/OUT connection data
     * @result a reason code
     */
    int flom_client_disconnect(struct flom_conn_data_s *cd);
    


    /**
     * Send lock command to the daemon
     * @param cd IN connection data
     * @param timeout IN maximum wait time for lock acquisition
     * @return a reason code
     */
    int flom_client_lock(struct flom_conn_data_s *cd, int timeout);



    /**
     * Wait while the desired resource is busy, then go on
     * @param cd IN connection data
     * @param msg IN/OUT message used to deserialize the replies
     * @param timeout IN maximum wait time for lock acquisition
     * @return a reason code
     */     
    int flom_client_wait_lock(struct flom_conn_data_s *cd,
                               struct flom_msg_s *msg, int timeout);
    

    
    /**
     * Send unlock command to the daemon
     * @param cd IN connection data
     * @return a reason code
     */
    int flom_client_unlock(struct flom_conn_data_s *cd);



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
