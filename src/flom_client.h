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
     * Try to connect to lock daemon
     * @param cd OUT connection data
     * @result a reason code
     */
    int flom_client_connect(struct flom_conn_data_s *cd);
    


    /**
     * Disconnect from lock daemon
     * @param cd IN/OUT connection data
     * @result a reason code
     */
    int flom_client_disconnect(struct flom_conn_data_s *cd);
    


    /**
     * Send lock command to the daemon
     * @param cd IN connection data
     * @return a reason code
     */
    int flom_client_lock(struct flom_conn_data_s *cd);



    /**
     * Wait while the desired resource is busy, then go on
     * @param cd IN connection data
     * @param msg IN/OUT message used to deserialize the replies
     * @return a reason code
     */     
    int flom_client_wait_lock(struct flom_conn_data_s *cd,
                               struct flom_msg_s *msg);
    

    
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
