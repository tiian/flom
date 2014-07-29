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
#ifndef FLOM_DAEMON_H
# define FLOM_DAEMON_H



#include <config.h>



#include "flom_config.h"
#include "flom_conns.h"
#include "flom_locker.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DAEMON



/**
 * Requested shutdown type
 */
enum shutdown_e {
    /**
     * Do not start shutdown procedure
     */
    FLOM_SHUTDOWN_NOSHUT,
    /**
     * Quiesce shutdown: do not accept new connections, keep active ones
     */
    FLOM_SHUTDOWN_QUIESCE,
    /**
     * Immediate shutdown: close all current connections and exit
     */
    FLOM_SHUTDOWN_IMMEDIATE,
    /**
     * Forced shutdown: leaving immediately using exit function
     */
    FLOM_SHUTDOWN_FORCE
};



/**
 * Normally set to value @ref FLOM_SHUTDOWN_NOSHUT, it is changed by signal
 * handler to a different value if shutdown is requested by the user
 */
extern enum shutdown_e asked_shutdown;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Create a lock daemon
     * @param family IN socket family, socket domain (AF_LOCAL, AF_INET, ...)
     * @result a reason code
     */
    int flom_daemon(int family);
    

    
    /**
     * Create a listen socket to serve the clients
     * @param conns OUT connections object
     * @result a reason code
     */
    int flom_listen(flom_conns_t *conns);



    /**
     * Create a listen local (AF_LOCAL) socket to serve the clients
     * @param conns OUT connections object
     * @result a reason code
     */
    int flom_listen_local(flom_conns_t *conns);



    /**
     * Create a listen network (AF_INET, TCP/IP) socket to serve the clients
     * @param conns OUT connections object
     * @result a reason code
     */
    int flom_listen_tcp(flom_conns_t *conns);



    /**
     * Create a listen network (AF_INET, TCP/IP) socket to serve the clients;
     * it uses the configured TCP/IP unicast address
     * @param conns OUT connections object
     * @result a reason code
     */
    int flom_listen_tcp_configured(flom_conns_t *conns);



    /**
     * Create a listen network (AF_INET, TCP/IP) socket to serve the clients;
     * it uses an automatic TCP/IP unicast address using INADDR_ANY and an
     * ephemeral port
     * @param conns OUT connections object
     * @result a reason code
     */
    int flom_listen_tcp_automatic(flom_conns_t *conns);



    /**
     * Create a listen network (AF_INET, UDP/IP, multicast) to answer
     * location inquiry from other flom commands
     * @param conns OUT connections object
     * @result a reason code
     */     
    int flom_listen_udp(flom_conns_t *conns);

    

    /**
     * Clean-up the listen socket before daemon termination
     * @param conns IN/OUT connections object
     * @result a reason code     
     */
    int flom_listen_clean(flom_conns_t *conns);


    
    /**
     * Possible infinite accept loop: every incoming connection will be
     * processed; after idle time, it will leave
     * @param conns IN/OUT connections object
     * @result a reason code
     */
    int flom_accept_loop(flom_conns_t *conns);



    /**
     * Manage server shutdown
     * @param conns IN/OUT connections object
     * @return a reason code
     */
    int flom_accept_shutdown(flom_conns_t *conns);

    

    /**
     * Manager POLLIN event received from listener daemon
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @param lockers IN/OUT array of lockers serving the connected clients
     * @param moved OUT boolean value: TRUE the connection was passed to
     *                  a locker thread, FALSE the connection is still valid
     * @return a reason code
     */
    int flom_accept_loop_pollin(flom_conns_t *conns, guint id,
                                flom_locker_array_t *lockers,
                                int *moved);



    /**
     * Transfer the arrived message to a slave thread (locker)
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @param lockers IN/OUT array of lockers serving the connected clients
     * @return a reason code
     */     
    int flom_accept_loop_transfer(flom_conns_t *conns, guint id,
                                  flom_locker_array_t *lockers);


    /**
     * Helper function for flom_accept_loop_transfer: it manages the
     * details related to connection movement
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @param locker IN/OUT locker object associated to locker thread that
     *                      will host the connection
     * @param cd IN connection that must be transferred to the locker
     * @return a reason code
     */
    int flom_accept_loop_transfer_conn(flom_conns_t *conns, guint id,
                                       struct flom_locker_s *locker,
                                       struct flom_conn_data_s *cd);

    

    /**
     * Start a new locker and add it to lockers array
     * @param lockers IN/OUT array of lockers
     * @param msg IN message carrying the request
     * @param flrt IN type of resource will be assigned to the new locker
     * @param new_locker OUT new locker allocated for the resource
     * @param new_thread OUT new thread started for the new locker
     * @return a reason code
     */
    int flom_accept_loop_start_locker(flom_locker_array_t *lockers,
                                      struct flom_msg_s *msg,
                                      flom_rsrc_type_t flrt,
                                      struct flom_locker_s **new_locker,
                                      GThread **new_thread);

    

    /**
     * Check lockers state, start locker termination if idle time exceeded
     * @param lockers IN/OUT array of lockers serving the connected clients
     * @param again OUT boolean value, "call me again": a new polling loop must
     *              be performed as soon as possible, there's probably another
     *              locker to destroy
     * @return a reason code
     */
    int flom_accept_loop_chklockers(flom_locker_array_t *lockers, int *again);



    /**
     * Send a reply message to the to client
     * @param cd IN/OUT client connection data
     * @param rc IN answer return code
     * @return a reason code
     */
    int flom_accept_loop_reply(struct flom_conn_data_s *cd, int rc);

    

    /**
     * Reply to a discover multicast message: I'm here!
     * @param fd IN file descriptor that must be used to reply to client
     * @param src_addr IN address extracted from discover packet
     * @param addrlen IN size of src_addr structure
     * @return a reason code
     */
    int flom_accept_discover_reply(int fd, const struct sockaddr *src_addr,
                                   socklen_t addrlen);



    /**
     * Set signal handler
     * @return a reason code
     */
    int flom_daemon_signal(void);



    /**
     * Signal action associated to intercepted signals
     * @param signo IN signal number received by the process
     */
    void flom_daemon_signal_action(int signo);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_DAEMON_H */
