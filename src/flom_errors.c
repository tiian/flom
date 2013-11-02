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
#include <config.h>



#include <flom_errors.h>



const char *flom_strerror(int ret_cod)
{
    switch (ret_cod) {
/*
        case FLOM_RC_MAINTENANCE_MODE:
            return "WARNING: maintenance mode execution only";
        case FLOM_RC_ASKED_SHUTDOWN:
            return "WARNING: shutdown must be performed";
        case FLOM_RC_THREAD_SWITCH:
            return "WARNING: the thread is serving the client must be "
                "switched to a different one";
        case FLOM_RC_FLOMC_CONF_CHANGED:
            return "WARNING: the digest of the flomc config file changed -> "
                "the client config file changed";
        case FLOM_RC_RECOVERY_PENDING_TX:
            return "WARNING: this thread of control should recover some "
                "recovery pending transactions";
        case FLOM_RC_TRUNCATION_OCCURRED:
            return "WARNING: a truncation occurred because the destination "
                "is smaller then the source";
        case FLOM_RC_BYPASSED_OPERATION:
            return "WARNING: operation was not performed because it can "
                "not be requested";
        case FLOM_RC_EMPTY_CONTAINER:
            return "WARNING: the container is empty";
        case FLOM_RC_OBJ_NOT_FOUND:
            return "WARNING: object not found";
*/
        case FLOM_RC_CONNECTION_CLOSED:
            return "WARNING: peer has closed socket connection";
        case FLOM_RC_OK:
            return "OK: no error";
        case FLOM_RC_INTERNAL_ERROR:
            return "ERROR: internal error / unexpected condition / code bug";
        case FLOM_RC_DAEMON_NOT_STARTED:
            return "ERROR: flom command was not able to start a new daemon "
                "and/or connect to it";
        case FLOM_RC_NULL_OBJECT:
            return "ERROR: an argument is null";
        case FLOM_RC_NETWORK_EVENT_ERROR:
            return "ERROR: an unespected network event raised";
        case FLOM_RC_INVALID_PREFIX_SIZE:
            return "ERROR: the number of chars of the prefix of the "
                "message is wrong";
        case FLOM_RC_BUFFER_OVERFLOW:
            return "ERROR: the process has been stopped to avoid a buffer "
                "overflow";
        case FLOM_RC_INVALID_MSG_LENGTH:
            return "ERROR: the length of the message differs from prefix";
        case FLOM_RC_INVALID_PROPERTY_VALUE:
            return "ERROR: a value of a property is invalid";
        case FLOM_RC_CONTAINER_FULL:
            return "ERROR: the container is full and cannot store more "
                "elements";
        case FLOM_RC_PROTOCOL_ERROR:
            return "ERROR: a routine has been invoked in an improper context";
            /*
        case FLOM_RC_OUT_OF_RANGE:
            return "ERROR: an argument is out of range";
        case FLOM_RC_CONFIG_ERROR:
            return "ERROR: configuration file is broken";
        case FLOM_RC_OBJ_NOT_INITIALIZED:
            return "ERROR: object is not initialized";
        case  FLOM_RC_OBJ_CORRUPTED:
            return "ERROR: object is corrupted";
        case FLOM_RC_CORRUPTED_STATUS_FILE:
            return "ERROR: the status file is corrupted and can not be used";
        case FLOM_RC_INVALID_OPTION:
            return "ERROR: a specified option is not valid";
        case FLOM_RC_INVALID_STATUS:
                  return "ERROR: invalid object status";
        case FLOM_RC_TOO_MANY_RSRMGRS:
            return "ERROR: too many resource managers";
        case FLOM_RC_EMPTY_XML_MSG:
            return "ERROR: the XML message is empty";
        case FLOM_RC_MALFORMED_XML_MSG:
            return "ERROR: the XML message is malformed and cannot be "
                "interpreted";
        case FLOM_RC_XML_UNRECOGNIZED_TAG:
            return "ERROR: the XML contains a tag is not known or is "
                "in the wrong place";
        case FLOM_RC_ASYNC_NOT_IMPLEMENTED:
            return "ERROR: an operation is referring to asynchronous mode "
                "that is not yet implemented";
        case FLOM_RC_UNSUPPORTED_OPTION:
            return "ERROR: the specified option might be valid, but it's not "
                "(yet) supported by FLOM";
        case FLOM_RC_FILE_NOT_EXISTS:
            return "ERROR: a specified file can not be opened because it does "
                "not exist";
        case FLOM_RC_ABORTED_RECOVERY:
            return "ERROR: a transaction can not be recovered";
        case FLOM_RC_RECOVERY_INFO_MISMATCH:
            return "ERROR: client/server recovery configuration do not match";
        case FLOM_RC_MALFORMED_XID:
            return "ERROR: a malformed XID has been discovered";
        case FLOM_RC_TX_FAIL:
            return "ERROR: the client status is unknown due to a "
                "previous TX_FAIL";
        case FLOM_RC_TX_ERROR:
            return "ERROR: generic error for a TX error (a TX return code "
                "not equal TX_OK)";
        case FLOM_RC_XA_ERROR:
            return "ERROR: an XA function returned an unexpcted return code";
            */
        case FLOM_RC_BIND_ERROR:
            return "ERROR: 'bind' function returned an error condition";
        case FLOM_RC_CHDIR_ERROR:
            return "ERROR: 'chdir' function returned an error condition";
        case FLOM_RC_CLOSE_ERROR:
            return "ERROR: 'close' function returned an error condition";
        case FLOM_RC_CONNECT_ERROR:
            return "ERROR: 'connect' function returned an error condition";
        case FLOM_RC_FORK_ERROR:
            return "ERROR: 'fork' function returned an error condition";
        case FLOM_RC_EXECVP_ERROR:
            return "ERROR: 'execvp' function returned an error condition";
        case FLOM_RC_GETSOCKOPT_ERROR:
            return "ERROR: 'getsockopt' function returned an error condition";
        case FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR:
            return "ERROR: 'g_markup_parse_context_new' function returned "
                "an error condition";
        case FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR:
            return "ERROR: 'g_markup_parse_context_parse' function returned "
                "an error condition";
        case FLOM_RC_G_STRDUP_ERROR:
            return "ERROR: 'g_strdup' function returned an error condition";
        case FLOM_RC_G_THREAD_CREATE_ERROR:
            return "ERROR: 'g_thread_create' function returned an error "
                "condition";
        case FLOM_RC_LISTEN_ERROR:
            return "ERROR: 'listen' function returned an error condition";
        case FLOM_RC_MALLOC_ERROR:
            return "ERROR: 'malloc'/'g_malloc' function returned an error "
                "condition";
        case FLOM_RC_PIPE_ERROR:
            return "ERROR: 'pipe' function returned an error condition";
        case FLOM_RC_POLL_ERROR:
            return "ERROR: 'poll' function returned an error condition";
        case FLOM_RC_READ_ERROR:
            return "ERROR: 'read' function returned an error condition";
        case FLOM_RC_RECV_ERROR:
            return "ERROR: 'recv' function returned an error condition";
        case FLOM_RC_SEND_ERROR:
            return "ERROR: 'send' function returned an error condition";
        case FLOM_RC_SETSID_ERROR:
            return "ERROR: 'setsid' function returned an error condition";
        case FLOM_RC_SIGNAL_ERROR:
            return "ERROR: 'signal' function returned an error condition";
        case FLOM_RC_SOCKET_ERROR:
            return "ERROR: 'socket' function returned an error condition";
        case FLOM_RC_UNLINK_ERROR:
            return "ERROR: 'unlink' function returned an error condition";
        case FLOM_RC_WAIT_ERROR:
            return "ERROR: 'wait' function returned an error condition";
        case FLOM_RC_WRITE_ERROR:
            return "ERROR: 'write' function returned an error condition";
            /*
        case FLOM_RC_REALLOC_ERROR:
            return "ERROR: 'realloc' function returned an error condition";
        case FLOM_RC_OPEN_ERROR:
            return "ERROR: 'open' function returned an error condition";
        case FLOM_RC_TRUNCATE_ERROR:
            return "ERROR: 'truncate' function returned an error condition";
        case FLOM_RC_PATHCONF_ERROR:
            return "ERROR: 'pathconf' function returned an error condition";
        case FLOM_RC_REALPATH_ERROR:
            return "ERROR: 'realpath' function returned an error condition";
        case FLOM_RC_FOPEN_ERROR:
            return "ERROR: 'fopen' function returned an error condition";
        case FLOM_RC_FCLOSE_ERROR:
            return "ERROR: 'fclose' function returned an error condition";
        case FLOM_RC_FGETS_ERROR:
            return "ERROR: 'fgets' function returned an error condition";
            */
            /*
              case FLOM_RC_FDATASYNC_ERROR:
              return "ERROR: 'fdatasync' function returned an error "
              "condition";
              case FLOM_RC_FPUTC_ERROR:
              return "ERROR: 'fputc' function (or 'putc' macro) returned an "
              "error condition";
              case FLOM_RC_FTRUNCATE_ERROR:
              return "ERROR: 'ftruncate' function returned an error "
              "condition";
              case FLOM_RC_FILENO_ERROR:
              return "ERROR: 'fileno' function returned an error "
              "condition";
              case FLOM_RC_RENAME_ERROR:
              return "ERROR: 'rename' function returned an error "
              "condition";
            */
            /*
        case FLOM_RC_STAT_ERROR:
            return "ERROR: 'stat' function returned an error condition";
        case FLOM_RC_FSTAT_ERROR:
            return "ERROR: 'fstat' function returned an error condition";
        case FLOM_RC_MMAP_ERROR:
            return "ERROR: 'mmap' function returned an error condition";
        case FLOM_RC_MUNMAP_ERROR:
            return "ERROR: 'munmap' function returned an error condition";
        case FLOM_RC_MSYNC_ERROR:
            return "ERROR: 'msync' function returned an error condition";
            */
            /*
              case FLOM_RC_VSNPRINTF_ERROR:
              return "ERROR: 'vsnprintf' function returned an error "
              "condition";
              case FLOM_RC_TIMES_ERROR:
              return "ERROR: 'times' function returned an error condition";
            */
            /*
        case FLOM_RC_UUID_PARSE_ERROR:
            return "ERROR: 'uuid_parse' function returned an error condition";
        case FLOM_RC_LOCALTIME_ERROR:
            return "ERROR: 'localtime/localtime_r' function returned an error "
                "condition";
        case FLOM_RC_GETTIMEOFDAY_ERROR:
            return "ERROR: 'gettimeofday' function returned an error "
                "condition";
            */
            /*
            */
            /*
        case FLOM_RC_SETSOCKOPT_ERROR:
            return "ERROR: 'setsockopt' function returned an error condition";
        case FLOM_RC_ACCEPT_ERROR:
            return "ERROR: 'accept' function returned an error condition";
        case FLOM_RC_SHUTDOWN_ERROR:
            return "ERROR: 'shutdown' function returned an error condition";
        case FLOM_RC_GETADDRINFO_ERROR:
            return "ERROR: 'getaddrinfo' function returned an error condition";
        case FLOM_RC_GETSOCKNAME_ERROR:
            return "ERROR: 'getsockname' function returned an error condition";
        case FLOM_RC_GETPEERNAME_ERROR:
            return "ERROR: 'getpeername' function returned an error condition";
        case FLOM_RC_PTHREAD_CREATE_ERROR:
            return "ERROR: 'pthread_create' function returned an error "
                "condition";
        case FLOM_RC_PTHREAD_MUTEX_LOCK_ERROR:
            return "ERROR: 'pthread_mutex_lock' function returned an "
                "error condition";
        case FLOM_RC_PTHREAD_MUTEX_UNLOCK_ERROR:
            return "ERROR: 'pthread_mutex_unlock' function returned an "
                "error condition";
        case FLOM_RC_PTHREAD_RWLOCK_WRLOCK_ERROR:
            return "ERROR: 'pthread_rwlock_wrlock' function returned an "
                "error condition";
        case FLOM_RC_PTHREAD_RWLOCK_RDLOCK_ERROR:
            return "ERROR: 'pthread_rwlock_rdlock' function returned an "
                "error condition";
        case FLOM_RC_PTHREAD_RWLOCK_UNLOCK_ERROR:
            return "ERROR: 'pthread_rwlock_unlock' function returned an "
                "error condition";
        case FLOM_RC_XML_READ_FILE_ERROR:
            return "ERROR: 'xmlReadFile' function returned an error condition";
        case FLOM_RC_XML_READ_DOC_ERROR:
            return "ERROR: 'xmlReadDoc' function returned an error "
                "condition";
        case FLOM_RC_XML_READ_MEMORY_ERROR:
            return "ERROR: 'xmlReadMemory' function returned an error "
                "condition";
        case FLOM_RC_XML_DOC_GET_ROOT_ELEMENT_ERROR:
            return "ERROR: 'xmlDocGetRootElement' function returned an "
                "error condition";
        case FLOM_RC_XML_CHAR_STRDUP_ERROR:
            return "ERROR: 'xmlCharStrdup' function returned a NULL pointer";
        case FLOM_RC_XML_STRDUP_ERROR:
            return "ERROR: 'xmlStrdup' function returned a NULL pointer";
        case FLOM_RC_G_RETURNED_NULL:
            return "ERROR:  a glib function returned a NULL pointer; the "
                "function is not documented as returnig NULL. This is "
                "an internal error";
        case FLOM_RC_G_MODULE_OPEN_ERROR:
            return "ERROR: 'g_module_open' function returned an "
                "error condition";
        case FLOM_RC_G_MODULE_CLOSE_ERROR:
            return "ERROR: 'g_module_close' function returned an "
                "error condition";
        case FLOM_RC_G_MODULE_SYMBOL_ERROR:
            return "ERROR: 'g_module_symbol' function returned an "
                "error condition";
        case FLOM_RC_G_CHECKSUM_NEW_ERROR:
            return "ERROR: 'g_checksum_new' function returned an "
                "error condition";
        case FLOM_RC_G_CHECKSUM_GET_STRING_ERROR:
            return "ERROR: 'g_checksum_get_string' function returned an "
                "error condition";
           */
        default:
            return "ERROR: unknown error";
    } /* switch (ret_cod) */
}
