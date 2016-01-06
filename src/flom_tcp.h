/*
 * Copyright (c) 2013-2016, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_TCP_H
# define FLOM_TCP_H



#include <config.h>



#include "flom_config.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_TCP



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_TCP_H */



/**
 * Prepare an incoming TCP/IP connection ready to accept new clients
 * @param config IN configuration object, NULL for global config
 * @param domain IN communication domain; this selects the protocol family
 *                  which will be used for communication (AF_INET for IPv4,
 *                  AF_INET6 for IPv6)
 * @param sockfd OUT file descriptor associated to the opened socket
 * @param addrlen OUT size of the returned address
 * @param address OUT address associated with the connection, it must be
 *                    pre-allocated with the size of struct sockaddr_storage
 * @return a reason code
 */
int flom_tcp_listen(flom_config_t *config, int domain,
                    int *sockfd, size_t *addrlen, struct sockaddr *address);
