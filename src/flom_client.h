/*
 * Copyright (c) 2013-2014, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of FLoM.
 *
 * FLoM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * FLoM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
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
     * @param cd OUT connection data
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect(struct flom_conn_data_s *cd, int start_daemon);
    


    /**
     * Try to connect to flom daemon using local (AF_LOCAL) socket
     * @param cd OUT connection data
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect_local(struct flom_conn_data_s *cd,
                                  int start_daemon);
    


    /**
     * Try to connect to flom daemon using network (TCP/IP) socket
     * @param cd OUT connection data
     * @param start_daemon IN try to start a new daemon if connection fails
     * @result a reason code
     */
    int flom_client_connect_tcp(struct flom_conn_data_s *cd,
                                int start_daemon);



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
     * @param start_daemon IN try to start a new daemon if connection fails
     * @return a reason code
     */
    int flom_client_discover_udp(struct flom_conn_data_s *cd,
                                 int start_daemon);



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
     * @param element OUT the obtained element if the locked resource is of
     *        type resource set, a null string "\0" if the locked resource
     *        does not returns an element. The return name is null terminated.
     * @param element_size IN size of the buffer that will be used by the
     *        function to store the element name; the trailing null
     *        character decrease to element_size-1 the max lenght of the
     *        returned element
     * @return a reason code
     */
    int flom_client_lock(struct flom_conn_data_s *cd, int timeout,
                         char *element, size_t element_size);



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



    /**
     * Connect to daemon and send a shutdown message
     * @param immediate IN is the shutdown immediate (else it's quiesce)
     * @return a reason code
     */
    int flom_client_shutdown(int immediate);


    
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
