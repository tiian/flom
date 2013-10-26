/*
 * Copyright (c) 2013, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef FLOM_PARSER_H
# define FLOM_PARSER_H



#include <config.h>



#include "flom_msg.h"



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_PARSER



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    
    /**
     * Retrieve the first message from a socket (file descriptor)
     * @param fd IN file descriptor associated to the socket
     * @param buf OUT buffer will be used to store the message
     * @param buf_size IN size of buf
     * @param read_bytes OUT number of bytes read, message length
     * @return a reason code
     */    
    int flom_msg_retrieve(int fd, char *buf, size_t buf_size,
                          ssize_t *read_bytes);



    /**
     * Compose a message that must be sent over the socket
     * @param buf IN/OUT buffer will be used to store the message
     * @param buf_size IN size of the buffer
     * @param write_bytes OUT number of bytes used inside buffer to store the
     *                        message
     * @return a reason code
     */
    int flom_msg_compose(char *buf, size_t buf_size, ssize_t *write_bytes);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of FLOM_TRACE_MODULE */
#ifdef FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE FLOM_TRACE_MODULE_SAVE
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE_SAVE */



#endif /* FLOM_PARSER_H */
