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
#ifndef FLOM_DAEMON_H
# define FLOM_DAEMON_H



#include <config.h>



#include "flom_config.h"
#include "flom_trace.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_DAEMON



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Create a lock daemon
     * @param config IN configuration object
     * @result a reason code
     */
    int flom_daemon(const flom_config_t *config);
    

    
    /**
     * Create a listen socket to serve the clients
     * @param config IN configuration object
     * @param listenfd OUT listening socket file descriptor
     * @result a reason code
     */
    int flom_listen(const flom_config_t *config,
                    int *listenfd);



    /**
     * Clean-up the listen socket before daemon termination
     * @param config IN configuration object
     * @param listenfd IN listening socket file descriptor
     * @result a reason code     
     */
    int flom_listen_clean(const flom_config_t *config, int listenfd);


    
    /**
     * Possible infinite accept loop: every incoming connection will be
     * processed; after idle time, it will leave
     * @param config IN configuration object
     * @param listenfd IN listening socket file descriptor
     * @result a reason code
     */
    int flom_accept_loop(const flom_config_t *config, int listenfd);

    

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
