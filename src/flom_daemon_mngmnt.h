/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_DAEMON_MNGMNT_H
# define FLOM_DAEMON_MNGMNT_H



#include <config.h>



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Main daemon management function (entry point)
     * @param config IN configuration object, NULL for global config
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @return a reason code
     */
    int flom_daemon_mngmnt(flom_config_t *config,
                           flom_conns_t *conns, guint id);

    

    /**
     * Shutdown management function
     * @param config IN configuration object, NULL for global config
     * @param conns IN/OUT connections object
     * @param id IN connection id
     * @return a reason code
     */
    int flom_daemon_mngmnt_shutdown(flom_config_t *config,
                                    flom_conns_t *conns, guint id);

    

#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_DAEMON_MNGMNT_H */
