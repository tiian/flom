/*
 * Copyright (c) 2013-2023, Christian Ferrari <tiian@users.sourceforge.net>
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
#include <config.h>



#include <stdio.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif



#include "flom_conns.h"
#include "flom_daemon.h"
#include "flom_daemon_mngmnt.h"
#include "flom_errors.h"
#include "flom_locker.h"
#include "flom_msg.h"
#include "flom_tcp.h"
#include "flom_vfs.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON



/* global read only variables (constants initialized by flom_daemon) */
/**
 * String equivalent to IPv4 INADDR_ANY constant
 */
static char FLOM_INADDR_ANY_STRING[INET_ADDRSTRLEN];
/**
 * String equivalent to IPv6 in6addr_any constant
 */
static char FLOM_INADDR6_ANY_STRING[INET6_ADDRSTRLEN];



int flom_daemon(flom_config_t *config, int family)
{
    enum Exception {
        INET_NTOP_ERROR1,
        INET_NTOP_ERROR2,
        PIPE_ERROR,
        FORK_ERROR,
        WAIT_ERROR,
        CHILD_PID_ERROR,
        READ_ERROR,
        DAEMON_NOT_OK,
        FORK_ERROR2,
        SETSID_ERROR,
        SIGNAL_ERROR1,
        SIGNAL_ERROR2,
        FORK_ERROR3,
        CHDIR_ERROR,
        FLOM_DAEMON_SIGNAL_ERROR,
        WRITE_ERROR,
        FLOM_LISTEN_ERROR,
        FLOM_ACCEPT_LOOP_ERROR,
        MALLOC_ERROR,
        NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int pipefd[2];
    int daemon = FALSE;
    int daemon_rc = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon\n"));
    TRY {
        pid_t pid;
        struct sockaddr_in sa_in;
        struct sockaddr_in6 sa_in6;

        /* initializing INxADDR ANY strings */
        sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
        sa_in6.sin6_addr = in6addr_any;
        if (FLOM_INADDR_ANY_STRING != inet_ntop(
                AF_INET, &(sa_in.sin_addr), FLOM_INADDR_ANY_STRING,
                sizeof(FLOM_INADDR_ANY_STRING)))
            THROW(INET_NTOP_ERROR1);
        if (FLOM_INADDR6_ANY_STRING != inet_ntop(
                AF_INET6, &(sa_in6.sin6_addr), FLOM_INADDR6_ANY_STRING,
                sizeof(FLOM_INADDR6_ANY_STRING)))
            THROW(INET_NTOP_ERROR2);
        
        /* creating a communication pipe to understand when the daemon is
           ready to run */
        if (-1 == pipe(pipefd))
            THROW(PIPE_ERROR);
        FLOM_TRACE(("flom_daemon: pipefd[0]=%d, pipefd[1]=%d\n",
                    pipefd[0], pipefd[1]));
        
        /* creating a child process */
        if (-1 == (pid = fork())) {
            THROW(FORK_ERROR);
        } else if (0 != pid) {
            int status;
            pid_t child_pid;
            /* father process */
            FLOM_TRACE(("flom_daemon: started child process pid=" PID_T_FORMAT
                        "\n", pid));
            /* waiting termination: it's only a synchronization because
               while daemonizing the child will fork twice */
            if (-1 == (child_pid = wait(&status)))
                THROW(WAIT_ERROR);
            if (pid != child_pid) {
                FLOM_TRACE(("flom_daemon: something really wrong is happening;"
                            " pid=" PID_T_FORMAT ", child_pid=" PID_T_FORMAT
                            "\n", pid, child_pid));
                THROW(CHILD_PID_ERROR);
            }
            /* waiting the daemon is up and running */
            FLOM_TRACE(("flom_daemon: waiting daemon reason code...\n"));
            if (sizeof(daemon_rc) != read(pipefd[0], &daemon_rc,
                                          sizeof(daemon_rc)))
                THROW(READ_ERROR);
            FLOM_TRACE(("flom_daemon: daemon_rc=%d\n", daemon_rc));
            /* closing pipe (father) */
            close(pipefd[0]);
            close(pipefd[1]);
            if (FLOM_RC_OK != daemon_rc)
                THROW(DAEMON_NOT_OK);
        } else {
            pid_t pid;
            int i;
            int daemon_rc = FLOM_RC_OK;
            flom_conns_t conns;

            /* switch state to daemonized */
            daemon = TRUE;
            /* child process, now daemonizing... */
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR2);
            } else if (0 != pid)
                exit(FLOM_ES_OK);
            FLOM_TRACE(("flom_daemon: daemonizing... setsid()\n"));
            if (-1 == setsid())
                THROW(SETSID_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... "
                        "signal(SIGHUP, SIG_IGN)\n"));
            if (SIG_ERR == signal(SIGHUP, SIG_IGN))
                THROW(SIGNAL_ERROR1);
            /* ignore SIGPIPE to avoid server crash with TLS 1.3 */
            FLOM_TRACE(("flom_daemon: daemonizing... "
                        "signal(SIGPIPE, SIG_IGN)\n"));
            if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
                THROW(SIGNAL_ERROR2);
            FLOM_TRACE(("flom_daemon: daemonizing... fork()\n"));
            if (-1 == (pid = fork())) {
                THROW(FORK_ERROR3);
            } else if (0 != pid)
                exit(FLOM_ES_OK);
            FLOM_TRACE(("flom_daemon: daemonizing... chdir()\n"));
            if (-1 == chdir("/"))
                THROW(CHDIR_ERROR);
            FLOM_TRACE(("flom_daemon: daemonizing... umask()\n"));
            umask(0);
            FLOM_TRACE(("flom_daemon: daemonizing... close()\n"));
            /* close all but communication pipe */
            for (i = 0; i < 64; ++i)
                if (i != pipefd[0] && i != pipefd[1])
                    close(i);

            FLOM_TRACE_REOPEN(flom_config_get_daemon_trace_file(config),
                              flom_config_get_append_trace_file(config));
            FLOM_TRACE(("flom_daemon: now daemonized!\n"));

            /* activate service */
            openlog("flom", LOG_PID, LOG_DAEMON);
            flom_conns_init(&conns, family);
            daemon_rc = flom_listen(config, &conns);
            
            /* sending reason code to father process */
            if (sizeof(daemon_rc) != write(pipefd[1], &daemon_rc,
                                           sizeof(daemon_rc)))
                THROW(WRITE_ERROR);
            /* closing pipe (child) */
            close(pipefd[0]);
            close(pipefd[1]);
            /* checking flom_listed reason code */
            if (FLOM_RC_OK != daemon_rc)
                THROW(FLOM_LISTEN_ERROR);
            
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM003N);
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop(config, &conns)))
                THROW(FLOM_ACCEPT_LOOP_ERROR);
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM004N);

            /* unmounting FUSE filesystem */
            if (NULL != flom_config_get_mount_point_vfs(config)) {
                size_t size;
                char *system_command = NULL;
                int exitstatus;
                
                FLOM_TRACE(("flom_daemon: unmounting VFS file system '%s'\n",
                            flom_config_get_mount_point_vfs(config)));
                size = 20 + strlen(flom_config_get_mount_point_vfs(config));
                if (NULL == (system_command = malloc(size))) {
                    FLOM_TRACE(("flom_daemon: unable to malloc "
                                "system_command\n"));
                    THROW(MALLOC_ERROR);
                }
                snprintf(system_command, size, "fusermount -u %s",
                         flom_config_get_mount_point_vfs(config));
                syslog(LOG_INFO, FLOM_SYSLOG_FLM023I, system_command);
                exitstatus = WEXITSTATUS(system(system_command));
                if (exitstatus != 0)
                    syslog(LOG_NOTICE, FLOM_SYSLOG_FLM024N, system_command,
                           exitstatus);
                free(system_command);
            }
            
            flom_listen_clean(config, &conns);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case INET_NTOP_ERROR1:
            case INET_NTOP_ERROR2:
                ret_cod = FLOM_RC_INET_NTOP_ERROR;
                break;
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case FORK_ERROR:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case WAIT_ERROR:
                ret_cod = FLOM_RC_WAIT_ERROR;
                break;
            case CHILD_PID_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case READ_ERROR:
                ret_cod = FLOM_RC_READ_ERROR;
                break;
            case DAEMON_NOT_OK:
                ret_cod = daemon_rc;
                break;
            case FORK_ERROR2:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case SETSID_ERROR:
                ret_cod = FLOM_RC_SETSID_ERROR;
                break;
            case SIGNAL_ERROR1:
            case SIGNAL_ERROR2:
                ret_cod = FLOM_RC_SIGNAL_ERROR;
                break;
            case FORK_ERROR3:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case CHDIR_ERROR:
                ret_cod = FLOM_RC_CHDIR_ERROR;
                break;
            case FLOM_DAEMON_SIGNAL_ERROR:
                break;
            case WRITE_ERROR:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case FLOM_LISTEN_ERROR:
                ret_cod = daemon_rc;
                break;
            case FLOM_ACCEPT_LOOP_ERROR:
                break;
            case MALLOC_ERROR:
                ret_cod = FLOM_RC_MALLOC_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_daemon/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    
    /* the function must not return control if in daemonized state */
    if (daemon)
        exit(FLOM_ES_OK);
    return ret_cod;
}



int flom_listen(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { LISTEN_LOCAL_ERROR
                     , LISTEN_TCP_ERROR
                     , LISTEN_UDP_ERROR
                     , INVALID_DOMAIN
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_listen\n"));
    TRY {
        int domain = flom_conns_get_domain(conns);
        switch (domain) {
            case AF_LOCAL:
                if (FLOM_RC_OK != (ret_cod = flom_listen_local(
                                       config, conns)))
                    THROW(LISTEN_LOCAL_ERROR);
                break;
            case AF_INET:
            case AF_INET6:
                /* create TCP/IP unicast listener */
                if (FLOM_RC_OK != (ret_cod = flom_listen_tcp(config, conns)))
                    THROW(LISTEN_TCP_ERROR);
                /* create UDP/IP multicast listener (resolver) */
                if (FLOM_RC_OK != (ret_cod = flom_listen_udp(config, conns)))
                    THROW(LISTEN_UDP_ERROR);
                break;
            default:
                FLOM_TRACE(("flom_listen: domain=%d\n", domain));
                THROW(INVALID_DOMAIN);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case LISTEN_LOCAL_ERROR:
            case LISTEN_TCP_ERROR:
            case LISTEN_UDP_ERROR:
                break;
            case INVALID_DOMAIN:
                ret_cod = FLOM_RC_INVALID_OPTION;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_listen/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_local(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { G_TRY_MALLOC_ERROR
                     , OPEN_ERROR
                     , FLOCK_ERROR
                     , SOCKET_ERROR
                     , UNLINK_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , NEW_OBJ
                     , CONN_INIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int fd = 0;
    int lock_fd; /* lock file descriptor */
    flom_conn_t *conn = NULL;
    gchar *lock_filename = NULL;
    const char *lock_filename_suffix = ".lock";
    
    FLOM_TRACE(("flom_listen_local\n"));
    TRY {
        struct sockaddr_un servaddr;

        if (NULL == (lock_filename = g_try_malloc0(
                         strlen(flom_config_get_socket_name(config)) +
                         strlen("lock_filename_suffix") + 1)))
            THROW(G_TRY_MALLOC_ERROR);
        strcpy(lock_filename, flom_config_get_socket_name(config));
        strcat(lock_filename, lock_filename_suffix);
        
        /* exclusive lock must be obtained to a semaphore file */
        if (-1 == (lock_fd = open(lock_filename, O_CREAT,
                                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                  S_IROTH | S_IWOTH)))
            THROW(OPEN_ERROR);
        if (-1 == flock(lock_fd, LOCK_EX | LOCK_NB))
            THROW(FLOCK_ERROR);
        /* lock file is never closed: daemon termination will close the file
           and unlock it */
        
        if (-1 == (fd = socket(flom_conns_get_domain(conns), SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        if (-1 == unlink(flom_config_get_socket_name(config)) &&
            ENOENT != errno)
            THROW(UNLINK_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = flom_conns_get_domain(conns);
        strncpy(servaddr.sun_path, flom_config_get_socket_name(config),
                sizeof(servaddr.sun_path));
        if (-1 == bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr))) {
            syslog(LOG_ERR, FLOM_SYSLOG_FLM019E, errno, strerror(errno),
                   "flom_listen_local");
            THROW(BIND_ERROR);
        }
        if (-1 == listen(fd, LISTEN_BACKLOG))
            THROW(LISTEN_ERROR);
        
        /* create a new connection object */
        if (NULL == (conn = flom_conn_new(NULL)))
            THROW(NEW_OBJ);
        FLOM_TRACE(("flom_listen_local: allocated a new connection "
                    "(%p)\n", conn));
        /* initialize the connection */
        if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                               conn,
                               flom_conns_get_domain(conns),
                               fd, SOCK_STREAM, sizeof(servaddr),
                               (struct sockaddr *)&servaddr, TRUE)))
            THROW(CONN_INIT_ERROR);
        /* add connection */
        flom_conns_add_conn(conns, conn);
        conn = NULL; /* avoid connection delete from this function */
        
        syslog(LOG_NOTICE, FLOM_SYSLOG_FLM000I,
               flom_config_get_socket_name(config));
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case G_TRY_MALLOC_ERROR:
                ret_cod = FLOM_RC_G_TRY_MALLOC_ERROR;
                break;
            case OPEN_ERROR:
                ret_cod = FLOM_RC_OPEN_ERROR;
                break;
            case FLOCK_ERROR:
                ret_cod = FLOM_RC_FLOCK_ERROR;
                break;
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case UNLINK_ERROR:
                ret_cod = FLOM_RC_UNLINK_ERROR;
                break;
            case BIND_ERROR:
                ret_cod = FLOM_RC_BIND_ERROR;
                break;
            case LISTEN_ERROR:
                ret_cod = FLOM_RC_LISTEN_ERROR;
                break;
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release lock filename */
    if (NULL != lock_filename)
        g_free(lock_filename);
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    FLOM_TRACE(("flom_listen_local/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp(flom_config_t *config, flom_conns_t *conns)
{
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    FLOM_TRACE(("flom_listen_tcp\n"));
    if (NULL != flom_config_get_unicast_address(config))
        ret_cod = flom_listen_tcp_configured(config, conns);
    else
        ret_cod = flom_listen_tcp_automatic(config, conns);
    syslog(LOG_NOTICE, FLOM_SYSLOG_FLM001I,
           flom_config_get_unicast_address(config),
           flom_config_get_unicast_port(config));
    FLOM_TRACE(("flom_listen_tcp/ret_cod=%d/errno=%d\n", ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp_configured(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { LISTEN_ERROR
                     , NEW_OBJ
                     , CONN_INIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    flom_conn_t *conn = NULL;

    FLOM_TRACE(("flom_listen_tcp_configured\n"));
    TRY {
        flom_tcp_t tcp;

        flom_tcp_init(&tcp, config);
        
        if (FLOM_RC_OK != (ret_cod = flom_tcp_listen(&tcp)))
            THROW(LISTEN_ERROR);
        
        /* create a new connection object */
        if (NULL == (conn = flom_conn_new(NULL)))
            THROW(NEW_OBJ);
        FLOM_TRACE(("flom_listen_tcp_configured: allocated a new connection "
                    "(%p)\n", conn));
        /* initialize the connection */
        if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                               conn,
                               flom_conns_get_domain(conns),
                               flom_tcp_get_sockfd(&tcp),
                               SOCK_STREAM, flom_tcp_get_addrlen(&tcp),
                               flom_tcp_get_sa(&tcp), TRUE)))
            THROW(CONN_INIT_ERROR);
        /* add connection */
        flom_conns_add_conn(conns, conn);
        conn = NULL; /* avoid connection delete from this function */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case LISTEN_ERROR:
                break;
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    FLOM_TRACE(("flom_listen_tcp_configured/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp_automatic(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { SOCKET_ERROR
                     , SETSOCKOPT_ERROR
                     , INVALID_AI_FAMILY1
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , GETSOCKNAME_ERROR
                     , INVALID_AI_FAMILY2
                     , NEW_OBJ
                     , CONN_INIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int fd = FLOM_NULL_FD;
    flom_conn_t *conn = NULL;
    
    FLOM_TRACE(("flom_listen_tcp_automatic\n"));
    TRY {
        struct sockaddr_in soin4;
        struct sockaddr_in6 soin6;
        struct sockaddr *sa;
        struct sockaddr_storage addr;
        socklen_t sa_len, addrlen;
        in_port_t sa_port;
        int sock_opt = 1;
        gint unicast_port;
        sa_family_t family = flom_conns_get_domain(conns);
        if (-1 == (fd = socket(family, SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                             (void *)&sock_opt, sizeof(sock_opt)))
            THROW(SETSOCKOPT_ERROR);
        /* binding local address, ephemeral port */
        switch (family) {
            case AF_INET:
                sa_len = sizeof(soin4);
                memset(&soin4, 0, sa_len);
                soin4.sin_family = family;
                soin4.sin_addr.s_addr = htonl(INADDR_ANY);
                soin4.sin_port = 0;
                sa = (struct sockaddr *)&soin4;
                break;
            case AF_INET6:
                sa_len = sizeof(soin6);
                memset(&soin6, 0, sa_len);
                soin6.sin6_family = family;
                soin6.sin6_addr = in6addr_any;
                soin6.sin6_port = 0;
                sa = (struct sockaddr *)&soin6;
                break;
            default:
                FLOM_TRACE(("flom_listen_tcp_automatic: family=%d\n",
                            family));
                THROW(INVALID_AI_FAMILY1);
        } /* switch (family) */
        FLOM_TRACE_SOCKADDR("flom_listen_tcp_automatic: binding address ",
                            sa, sa_len);
        if (-1 == bind(fd, sa, sa_len)) {
            syslog(LOG_ERR, FLOM_SYSLOG_FLM019E, errno, strerror(errno),
                   "flom_listen_tcp_automatic");
            THROW(BIND_ERROR);
        }
        if (-1 ==listen(fd, 100))
            THROW(LISTEN_ERROR);
        /* retrieve address and port */
        addrlen = sizeof(addr);
        memset(&addr, 0, addrlen);
        if (-1 == getsockname(fd, (struct sockaddr *)&addr, &addrlen))
            THROW(GETSOCKNAME_ERROR);
        FLOM_TRACE_HEX_DATA("flom_listen_tcp_automatic: addr ",
                            (void *)&addr, addrlen);
        if (AF_INET != addr.ss_family &&
            AF_INET6 != addr.ss_family)
            THROW(INVALID_AI_FAMILY2);
        /* inject address value to configuration */
        FLOM_TRACE(("flom_listen_tcp_automatic: set unicast address to value "
                    "'%s'\n",
                    AF_INET == addr.ss_family ?
                    FLOM_INADDR_ANY_STRING : FLOM_INADDR6_ANY_STRING));
        flom_config_set_unicast_address(
            config, AF_INET == addr.ss_family ?
            FLOM_INADDR_ANY_STRING : FLOM_INADDR6_ANY_STRING);
        /* reuse soin4 and soin6 structs */
        if (AF_INET == addr.ss_family) {
            memcpy(&soin4, &addr, sizeof(soin4));
            sa_port = soin4.sin_port;
        } else {
            memcpy(&soin6, &addr, sizeof(soin6));
            sa_port = soin6.sin6_port;
        } /* if (AF_INET == addr.sa_family) */
        /* inject port value to configuration */
        unicast_port = ntohs(sa_port);
        FLOM_TRACE(("flom_listen_tcp_automatic: set unicast port to value "
                    "%d\n", unicast_port));
        flom_config_set_unicast_port(config, unicast_port);
        /* create a new connection object */
        if (NULL == (conn = flom_conn_new(NULL)))
            THROW(NEW_OBJ);
        FLOM_TRACE(("flom_listen_tcp_automatic: allocated a new connection "
                    "(%p)\n", conn));
        /* initialize the connection */
        if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                               conn,
                               flom_conns_get_domain(conns),
                               fd, SOCK_STREAM, addrlen,
                               (struct sockaddr *)&addr, TRUE)))
            THROW(CONN_INIT_ERROR);
        /* add connection */
        flom_conns_add_conn(conns, conn);
        conn = NULL; /* avoid connection delete from this function */
        fd = FLOM_NULL_FD; /* avoid socket close by clean-up section */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case INVALID_AI_FAMILY1:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case BIND_ERROR:
                ret_cod = FLOM_RC_BIND_ERROR;
                break;
            case LISTEN_ERROR:
                ret_cod = FLOM_RC_LISTEN_ERROR;
                break;
            case GETSOCKNAME_ERROR:
                ret_cod = FLOM_RC_GETSOCKNAME_ERROR;
                break;
            case INVALID_AI_FAMILY2:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (FLOM_NULL_FD != fd)
        close(fd);
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    FLOM_TRACE(("flom_listen_tcp_automatic/excp=%d/"
                "ret_cod=%d/errno=%d ('%s')\n", excp, ret_cod, errno,
                strerror(errno)));
    return ret_cod;
}



int flom_listen_udp(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { NO_MULTICAST
                     , INVALID_AI_FAMILY_ERROR1
                     , GETADDRINFO_ERROR
                     , INVALID_AI_FAMILY_ERROR2
                     , CONNECT_ERROR
                     , NEW_OBJ
                     , CONN_INIT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    struct addrinfo *result = NULL;
    int fd = FLOM_NULL_FD;
    flom_conn_t *conn = NULL;
    
    FLOM_TRACE(("flom_listen_udp\n"));
    TRY {
        struct addrinfo hints;
        struct sockaddr *local_addr;
        struct sockaddr_in local_addr_in;
        struct sockaddr_in6 local_addr_in6;
        socklen_t local_addr_len;
        sa_family_t family = flom_conns_get_domain(conns);
        char port[100];
        int errcode;
        int found = FALSE;
        const struct addrinfo *gai = result;
        
        if (NULL == flom_config_get_multicast_address(config)) {
            FLOM_TRACE(("flom_listen_udp: no multicast address specified, "
                        "this listener will not answer to daemon location "
                        "inquiries\n"));
            THROW(NO_MULTICAST);
        }

        FLOM_TRACE(("flom_listen_udp: creating multicast daemon locator "
                    "using address '%s' and port %d\n",
                    flom_config_get_multicast_address(config),
                    flom_config_get_multicast_port(config)));
        /* prepare hints for getaddressinfo() */
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME;
        if (NULL != flom_config_get_network_interface(config))
            hints.ai_family = AF_INET6;
        else
            hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        /* prepare a local address structure for incoming datagrams */
        switch (family) {
            case AF_INET:
                local_addr_len = sizeof(local_addr_in);
                memset(&local_addr_in, 0, local_addr_len);
                local_addr_in.sin_family = AF_INET;
                local_addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
                local_addr_in.sin_port = htons(
                    flom_config_get_multicast_port(config));
                local_addr = (struct sockaddr *)&local_addr_in;
                break;
            case AF_INET6:
                local_addr_len = sizeof(local_addr_in6);
                memset(&local_addr_in6, 0, local_addr_len);
                local_addr_in6.sin6_family = AF_INET6;
                local_addr_in6.sin6_addr = in6addr_any;
                local_addr_in6.sin6_port = htons(
                    flom_config_get_multicast_port(config));
                local_addr = (struct sockaddr *)&local_addr_in6;
                break;
            default:
                FLOM_TRACE(("flom_listen_udp: family=%d\n", family));
                THROW(INVALID_AI_FAMILY_ERROR1);
        } /* switch (family) */
        snprintf(port, sizeof(port), "%u",
                 flom_config_get_multicast_port(config));

        if (0 != (errcode = getaddrinfo(
                      flom_config_get_multicast_address(config),
                      port, &hints, &result))) {
            FLOM_TRACE(("flom_listen_udp/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            int sock_opt = 1;
            
            FLOM_TRACE_ADDRINFO("flom_listen_udp/getaddrinfo(): ",
                                result);
            /* traverse the list and try to connect... */
            gai = result;
            while (NULL != gai && !found) {
                FLOM_TRACE_HEX_DATA("flom_listen_udp: ai_addr ",
                                    (void *)gai->ai_addr, gai->ai_addrlen);
                if (-1 == (fd = socket(gai->ai_family, gai->ai_socktype,
                                       gai->ai_protocol))) {
                    FLOM_TRACE(("flom_listen_udp/socket(): "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                } else if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                            (void *)&sock_opt,
                                            sizeof(sock_opt))) {
                    FLOM_TRACE(("flom_listen_udp/setsockopt("
                                "SO_REUSEADDR) : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                } else if (-1 == bind(fd, local_addr, local_addr_len)) {
                    FLOM_TRACE(("flom_listen_udp/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = FLOM_NULL_FD;
                } else { /* switching to multicast mode */
                    struct ip_mreq mreq;
                    struct ipv6_mreq mreq6;
                    int setsockopt_return;

                    switch (gai->ai_family) {
                        case AF_INET:
                            memcpy(&mreq.imr_multiaddr,
                                   &((struct sockaddr_in *)gai->ai_addr)
                                   ->sin_addr,
                                   sizeof(struct in_addr));
                            /* all the interfaces... */
                            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                            setsockopt_return = setsockopt(
                                fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                &mreq, sizeof(mreq));
                            break;
                        case AF_INET6:
                            memcpy(&mreq6.ipv6mr_multiaddr,
                                   &((struct sockaddr_in6 *)gai->ai_addr)
                                   ->sin6_addr,
                                   sizeof(struct in6_addr));
                            /* all the interfaces... */
                            mreq6.ipv6mr_interface = 0;
                            setsockopt_return = setsockopt(
                                fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                                &mreq6, sizeof(mreq6));
                            break;
                        default:
                            FLOM_TRACE(("flom_listen_udp: gai->ai_family=%d\n",
                                        gai->ai_family));
                            THROW(INVALID_AI_FAMILY_ERROR2);
                    } /* switch (gai->ai_family) */
                    if (-1 == setsockopt_return) {
                        FLOM_TRACE(("flom_listen_udp/setsockopt("
                                    "IP_ADD_MEMBERSHIP/IPV6_ADD_MEMBERSHIP) : "
                                    "errno=%d '%s', skipping...\n", errno,
                                    strerror(errno)));
                        gai = gai->ai_next;
                        close(fd);
                        fd = FLOM_NULL_FD;
                    } else {
                        found = TRUE;
                    }  /* else */
                } /* if (-1 == (*fd = socket( */
            } /* while (NULL != gai && !connected) */            
        }
        if (!found) {
            FLOM_TRACE(("flom_listen_udp: unable to use multicast\n"));
            THROW(CONNECT_ERROR);
        }
        /* create a new connection object */
        if (NULL == (conn = flom_conn_new(NULL)))
            THROW(NEW_OBJ);
        FLOM_TRACE(("flom_listen_udp: allocated a new connection (%p)\n",
                    conn));
        /* initialize the connection */
        if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                               conn, family, fd, SOCK_DGRAM, gai->ai_addrlen,
                               gai->ai_addr, TRUE)))
            THROW(CONN_INIT_ERROR);
        
        /* add connection */
        flom_conns_add_conn(conns, conn);
        conn = NULL; /* avoid connection delete from this function */
        fd = FLOM_NULL_FD; /* avoid socket close by clean-up section */        
        syslog(LOG_NOTICE, FLOM_SYSLOG_FLM002I,
               flom_config_get_multicast_address(config),
               flom_config_get_multicast_port(config));
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NO_MULTICAST:
                ret_cod = FLOM_RC_OK;
                break;
            case INVALID_AI_FAMILY_ERROR1:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case INVALID_AI_FAMILY_ERROR2:
                ret_cod = FLOM_RC_INVALID_AI_FAMILY_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    if (NULL != result)
        freeaddrinfo(result);
    if (FLOM_NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_listen_udp/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_clean(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_listen_clean\n"));
    TRY {
        int domain = flom_conns_get_domain(conns);
        flom_conns_free(conns);
        if (AF_LOCAL == domain &&
            -1 == unlink(flom_config_get_socket_name(config))) {
            FLOM_TRACE(("flom_listen_clean: unlink errno=%d\n", errno));
        }
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_listen_clean/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}


int flom_accept_loop(flom_config_t *config, flom_conns_t *conns)
{
    enum Exception { VFS_RAM_TREE_INIT_ERROR
                     , G_THREAD_NEW_ERROR
                     , CONNS_CLEAN_ERROR
                     , CONNS_GET_FDS_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , ACCEPT_LOOP_CHKLOCKERS_ERROR1
                     , NEGATIVE_NUMBER_OF_LOCKERS_ERROR1
                     , CONNS_CLOSE_ERROR1
                     , CONNS_CLOSE_ERROR2
                     , ACCEPT_LOOP_POLLIN_ERROR
                     , NETWORK_ERROR
                     , INTERNAL_ERROR
                     , ACCEPT_LOOP_CHKLOCKERS_ERROR2
                     , NEGATIVE_NUMBER_OF_LOCKERS_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    flom_locker_array_t lockers;
    
    FLOM_TRACE(("flom_accept_loop\n"));
    TRY {
        int loop = TRUE;
        int chklockers_again = FALSE;
        GThread *vfs_thread;
        int activate_vfs = flom_config_get_mount_point_vfs(config) != NULL;

        flom_locker_array_init(&lockers);

        /* initialize the RAM representation of the VFS */
        ret_cod = flom_vfs_ram_tree_init(activate_vfs);
        if ((activate_vfs && FLOM_RC_OK != ret_cod) ||
            (!activate_vfs && FLOM_RC_INACTIVE_FEATURE != ret_cod))
            THROW(VFS_RAM_TREE_INIT_ERROR);
        
        /* activate the thread only if VFS is required */
        if (activate_vfs) {
            FLOM_TRACE(("flom_accept_loop: activating VFS thread...\n"));
            if (NULL == (vfs_thread = g_thread_new(
                             "FUSE VFS", flom_daemon_mngmnt_activate_vfs,
                             (gpointer)flom_config_get_mount_point_vfs(
                                 config))))
                THROW(G_THREAD_NEW_ERROR);
        }
        
        while (loop) {
            int ready_fd;
            guint i, n;
            struct pollfd *fds;
            guint number_of_lockers;
            int poll_timeout = flom_config_get_lifespan(config);

            /* the completion needs three polling cycles, so timeout must
               be a third of estimated lifespan */
            if (chklockers_again)
                poll_timeout = 0;
            else if (3 < poll_timeout)
                poll_timeout /= 3;
            
            if (FLOM_RC_OK != (ret_cod = flom_conns_clean(conns)))
                THROW(CONNS_CLEAN_ERROR);
            if (NULL == (fds = flom_conns_get_fds(conns)))
                THROW(CONNS_GET_FDS_ERROR);
            if (FLOM_RC_OK != (ret_cod = flom_conns_set_events(conns, POLLIN)))
                THROW(CONNS_SET_EVENTS_ERROR);
            FLOM_TRACE(("flom_accept_loop: entering poll...\n"));
            ready_fd = poll(fds, flom_conns_get_used(conns), poll_timeout);
            FLOM_TRACE(("flom_accept_loop: ready_fd=%d\n", ready_fd));
            /* error on poll function */
            if (0 > ready_fd) {
                THROW(POLL_ERROR);
            }
            /* poll exited due to time out */
            if (0 == ready_fd) {
                chklockers_again = FALSE;
                number_of_lockers = flom_locker_array_count(&lockers);
                FLOM_TRACE(("flom_accept_loop: idle time exceeded %d "
                            "milliseconds, number of lockers=%u\n",
                            poll_timeout, number_of_lockers));
                if (0 == number_of_lockers && 0 < poll_timeout) {
                    if (1 == flom_conns_get_used(conns) ||
                        (2 == flom_conns_get_used(conns) &&
                         SOCK_DGRAM == flom_conns_get_type(conns, 1))) {
                        FLOM_TRACE(("flom_accept_loop: only listener "
                                    "connection is active, exiting...\n"));
                        loop = FALSE;
                    }
                } else if (0 < number_of_lockers) {
                    if (FLOM_RC_OK != (ret_cod =
                                       flom_accept_loop_chklockers(
                                           &lockers, &chklockers_again)))
                        THROW(ACCEPT_LOOP_CHKLOCKERS_ERROR1);
                } else if (0 > number_of_lockers) {
                    THROW(NEGATIVE_NUMBER_OF_LOCKERS_ERROR1);
                }
                continue;
            }
            /* scanning file descriptors */
            n = flom_conns_get_used(conns);
            for (i=0; i<n; ++i) {
                FLOM_TRACE(("flom_accept_loop: i=%u, fd=%d, POLLIN=%d, "
                            "POLLERR=%d, POLLHUP=%d, POLLNVAL=%d\n", i,
                            fds[i].fd,
                            fds[i].revents & POLLIN,
                            fds[i].revents & POLLERR,
                            fds[i].revents & POLLHUP,
                            fds[i].revents & POLLNVAL));
                if ((fds[i].revents & POLLHUP) && (0 != i)) {
                    FLOM_TRACE(("flom_accept_loop: client %u disconnected "
                                "before categorization!\n", i));
                    if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                           conns, i)))
                        THROW(CONNS_CLOSE_ERROR1);
                    /* this file descriptor is no more valid, continue to
                       next one */
                    continue;
                }
                if (fds[i].revents & POLLIN) {
                    int conn_moved = FALSE;
                    ret_cod = flom_accept_loop_pollin(
                        config, conns, i, &lockers, &conn_moved);
                    if (FLOM_RC_CONNECTION_CLOSED == ret_cod) {
                        FLOM_TRACE(("flom_accept_loop: peer closed the "
                                    "connection, terminating it...\n"));
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               conns, i)))
                            THROW(CONNS_CLOSE_ERROR2);
                        /* this file descriptor is no more valid, continue to
                           next one */
                        continue;
                    } else if (FLOM_RC_OK != ret_cod)
                        THROW(ACCEPT_LOOP_POLLIN_ERROR);
                    if (conn_moved)
                        /* connection is no more available for main thread,
                           a new main loop must be started */
                        break;
                }
                if (fds[i].revents &
                    (POLLERR | POLLHUP | POLLNVAL))
                    THROW(NETWORK_ERROR);
            } /* for (i... */
            /* check if any locker is ready for termination... */
            number_of_lockers = flom_locker_array_count(&lockers);
            FLOM_TRACE(("flom_accept_loop: number of lockers=%u\n",
                        number_of_lockers));
            if (0 < number_of_lockers) {
                if (FLOM_RC_OK != (ret_cod =
                                   flom_accept_loop_chklockers(
                                       &lockers, &chklockers_again)))
                    THROW(ACCEPT_LOOP_CHKLOCKERS_ERROR2);
            } else if (0 > number_of_lockers) {
                THROW(NEGATIVE_NUMBER_OF_LOCKERS_ERROR2);
            }
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case VFS_RAM_TREE_INIT_ERROR:
                break;
            case G_THREAD_NEW_ERROR:
                ret_cod = FLOM_RC_G_THREAD_CREATE_ERROR;
                break;
            case CONNS_CLEAN_ERROR:
                break;
            case CONNS_GET_FDS_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_SET_EVENTS_ERROR:
                break;
            case POLL_ERROR:
                ret_cod = FLOM_RC_POLL_ERROR;
                break;
            case ACCEPT_LOOP_CHKLOCKERS_ERROR1:
                break;
            case NEGATIVE_NUMBER_OF_LOCKERS_ERROR1:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case CONNS_CLOSE_ERROR1:
            case CONNS_CLOSE_ERROR2:
            case ACCEPT_LOOP_POLLIN_ERROR:
                break;
            case NETWORK_ERROR:
                ret_cod = FLOM_RC_NETWORK_EVENT_ERROR;
                break;
            case INTERNAL_ERROR:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
                break;
            case ACCEPT_LOOP_CHKLOCKERS_ERROR2:
                break;
            case NEGATIVE_NUMBER_OF_LOCKERS_ERROR2:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    flom_vfs_ram_tree_cleanup(NULL, FALSE);
    flom_locker_array_free(&lockers);
    FLOM_TRACE(("flom_accept_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_pollin(flom_config_t *config,
                            flom_conns_t *conns, guint id,
                            flom_locker_array_t *lockers,
                            int *moved)
{
    enum Exception { CONNS_GET_CD_ERROR
                     , ACCEPT_ERROR
                     , SETSOCKOPT_ERROR
                     , CONN_SET_KEEPALIVE_ERROR
                     , NEW_OBJ
                     , CONN_INIT_ERROR
                     , CONN_TERMINATE_ERROR
                     , MSG_RETRIEVE_ERROR
                     , EMPTY_MESSAGE
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR1
                     , PROTOCOL_ERROR
                     , NO_TLS_CONNECTION
                     , NULL_OBJECT
                     , CONNS_CLOSE_ERROR2
                     , TLS_CERT_CHECK_ERROR
                     , GETNAMEINFO_ERROR
                     , ACCEPT_DISCOVER_REPLY_ERROR
                     , DAEMON_MANAGEMENT_ERROR
                     , ACCEPT_LOOP_TRANSFER_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    flom_conn_t *conn = NULL;
    gchar *peer_name = NULL;
    
    FLOM_TRACE(("flom_accept_loop_pollin\n"));
    TRY {
        flom_conn_t *c;

        *moved = FALSE;
        if (NULL == (c = flom_conns_get_conn(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        FLOM_TRACE(("flom_accept_loop_pollin: id=%u, fd=%d\n",
                    id, flom_tcp_get_sockfd(flom_conn_get_tcp(c))));
        if (0 == id) {
            /* it's a new connection */
            int conn_fd;
            struct sockaddr_storage cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            if (-1 == (conn_fd = accept(flom_tcp_get_sockfd(
                                            flom_conn_get_tcp(c)),
                                        (struct sockaddr *)&cliaddr,
                                        &clilen)))
                THROW(ACCEPT_ERROR);
            FLOM_TRACE(("flom_accept_loop_pollin: new client connected "
                        "with fd=%d\n", conn_fd));
            
            if (AF_INET == flom_conns_get_domain(conns)) {
                int sock_opt = 1;
                /* set TCP_NODELAY for socket */
                if (0 != setsockopt(conn_fd, IPPROTO_TCP, TCP_NODELAY,
                                    (void *)(&sock_opt), sizeof(sock_opt)))
                    THROW(SETSOCKOPT_ERROR);
                /* set SO_KEEPALIVE for socket */
                if (FLOM_RC_OK != (ret_cod = flom_conn_set_keepalive(
                                       config, conn_fd)))
                    THROW(CONN_SET_KEEPALIVE_ERROR);
            }
            /* create a new connection object */
            if (NULL == (conn = flom_conn_new(NULL)))
                THROW(NEW_OBJ);
            FLOM_TRACE(("flom_accept_loop_pollin: allocated a new connection "
                        "(%p)\n", conn));
            /* initialize the connection */
            if (FLOM_RC_OK != (ret_cod = flom_conn_init(
                                   conn,
                                   flom_conns_get_domain(conns),
                                   conn_fd, SOCK_STREAM, clilen,
                                   (struct sockaddr *)&cliaddr, TRUE)))
                THROW(CONN_INIT_ERROR);
            /* switch the connection to TLS if required by the configuration */
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop_pollin_tls(
                                   config, conn))) {
                FLOM_TRACE(("flom_accept_loop_pollin: TLS negotiation "
                            "returned %d, closing the TCP connection...\n",
                            ret_cod));
                if (FLOM_RC_OK != (ret_cod = flom_conn_terminate(conn)))
                    THROW(CONN_TERMINATE_ERROR);
            }
            
            /* add connection */
            flom_conns_add_conn(conns, conn);
            conn = NULL; /* avoid connection delete from this function */
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            size_t read_bytes;
            struct flom_msg_s *msg;
            GMarkupParseContext *gmpc;
            struct sockaddr_storage src_addr;
            socklen_t addrlen = sizeof(src_addr);
            memset(&src_addr, 0, addrlen);
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_conn_recv(
                                   c, buffer, sizeof(buffer),
                                   &read_bytes, FLOM_NETWORK_WAIT_TIMEOUT,
                                   (struct sockaddr *)&src_addr, &addrlen))) {
                FLOM_TRACE(("flom_accept_loop_pollin/flom_conn_recv: "
                            "ret_cod=%d (%s), leaving...\n",
                            ret_cod, flom_strerror(ret_cod)));
                THROW(MSG_RETRIEVE_ERROR);
            }

            /* has the client disconnected in the meantime? */
            if (0 == read_bytes) {
                FLOM_TRACE(("flom_accept_loop_pollin: returned 0 bytes, "
                            "the client has probably disconnected, "
                            "leaving...\n"));
                THROW(EMPTY_MESSAGE);
            }

            if (NULL == (msg = flom_conns_get_msg(conns, id)))
                THROW(CONNS_GET_MSG_ERROR);

            if (NULL == (gmpc = flom_conns_get_parser(conns, id)))
                THROW(CONNS_GET_GMPC_ERROR);
            
            if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                   buffer, read_bytes, msg, gmpc)))
                THROW(MSG_DESERIALIZE_ERROR);
            flom_conn_set_last_step(c, msg->header.pvs.step);
            flom_msg_trace(msg);
            /* if the message is not valid the client must be terminated */
            if (FLOM_MSG_STATE_INVALID == msg->state) {
                if (FLOM_MSG_LEVEL != msg->header.level) {
                    FLOM_TRACE(("flom_accept_loop_pollin: this flom daemon "
                                "is using communication level %d, client is "
                                "using communication level %d\n",
                                FLOM_MSG_LEVEL, msg->header.level));
                    syslog(LOG_WARNING, FLOM_SYSLOG_FLM006W,
                           FLOM_MSG_LEVEL, msg->header.level);
                }
                FLOM_TRACE(("flom_accept_loop_pollin: message from client %u "
                            "is invalid, disconneting...\n", id));
                if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                       conns, id)))
                    THROW(CONNS_CLOSE_ERROR1);
            }
            /* check if the message is completely parsed and can be transferred
               to a slave thread (a locker) */
            if (FLOM_MSG_STATE_READY == msg->state) {
                gchar *peerid = NULL;
                /* check the message is protocol correct */
                if (!flom_msg_check_protocol(msg, TRUE))
                    THROW(PROTOCOL_ERROR);
                /* retrieve peer id */
                if (NULL != (peerid = flom_msg_get_peerid(msg))) {
                    FLOM_TRACE(("flom_accept_loop_pollin: remote peer is "
                                "presenting itself with id='%s'\n", peerid));
                    syslog(LOG_INFO, FLOM_SYSLOG_FLM015I, peerid,
                           msg->header.pvs.verb, msg->header.pvs.step);
                }
                /* check peer id if requested */
                if (FLOM_MSG_VERB_DISCOVER != msg->header.pvs.verb &&
                    flom_config_get_tls_check_peer_id(config)) {
                    flom_tls_t *tls = NULL;
                    /* check it's a TLS connection; if not, maybe an internal
                       error */
                    if (NULL == (tls = flom_conn_get_tls(c)))
                        THROW(NO_TLS_CONNECTION);
                    if (NULL == (peer_name = flom_tcp_retrieve_peer_name(
                                     flom_conn_get_tcp(c))))
                        THROW(NULL_OBJECT);
                    if (FLOM_RC_OK != (ret_cod = flom_tls_cert_check(
                                           tls, peerid, peer_name))) {
                        if (FLOM_RC_OK != (ret_cod = flom_conns_close_fd(
                                               conns, id)))
                            THROW(CONNS_CLOSE_ERROR2);
                        THROW(TLS_CERT_CHECK_ERROR);
                    }
                } /* if (FLOM_MSG_VERB_DISCOVER != msg->header.pvs.verb) */
                /* is the message a discover message? */
                if (FLOM_MSG_VERB_DISCOVER == msg->header.pvs.verb) {
                    char host[256];
                    char port[25];
                    *host = *port = '\0';
                    if (-1 == getnameinfo((const struct sockaddr *)&src_addr,
                                          addrlen, host, sizeof(host),
                                          port, sizeof(port),
                                          NI_DGRAM | NI_NUMERICHOST |
                                          NI_NUMERICSERV))
                        THROW(GETNAMEINFO_ERROR);
                    syslog(LOG_INFO, FLOM_SYSLOG_FLM005I, host, port);
                    if (FLOM_RC_OK != (ret_cod = flom_accept_discover_reply(
                                           config,
                                           flom_conns_get_fd(conns, id),
                                           (const struct sockaddr *)&src_addr,
                                           addrlen)))
                        THROW(ACCEPT_DISCOVER_REPLY_ERROR);
                } else if (FLOM_MSG_VERB_MNGMNT == msg->header.pvs.verb) {
                    /* this is a management message, not a lock request */
                    if (FLOM_RC_OK != (ret_cod = flom_daemon_mngmnt(
                                           config, conns, id)))
                        THROW(DAEMON_MANAGEMENT_ERROR);
                } else {
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_transfer(
                                           conns, id, lockers)))
                        THROW(ACCEPT_LOOP_TRANSFER_ERROR);
                    *moved = TRUE;
                }
            } /* if (FLOM_MSG_STATE_READY == msg->state) */
        } /* if (0 == id) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case CONNS_GET_CD_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case ACCEPT_ERROR:
                ret_cod = FLOM_RC_ACCEPT_ERROR;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
                break;
            case CONN_SET_KEEPALIVE_ERROR:
                break;
            case NEW_OBJ:
                ret_cod = FLOM_RC_NEW_OBJ;
                break;
            case CONN_INIT_ERROR:
            case CONN_TERMINATE_ERROR:
                break;
            case MSG_RETRIEVE_ERROR:
            case EMPTY_MESSAGE:
                ret_cod = FLOM_RC_CONNECTION_CLOSED;
                break;
            case MSG_DESERIALIZE_ERROR:
                break;
            case CONNS_GET_MSG_ERROR:
            case CONNS_GET_GMPC_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_CLOSE_ERROR1:
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case NO_TLS_CONNECTION:
                ret_cod = FLOM_RC_NO_TLS_CONNECTION;
                break;
            case NULL_OBJECT:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_CLOSE_ERROR2:
                break;
            case TLS_CERT_CHECK_ERROR:
                ret_cod = FLOM_RC_CONNECTION_CLOSED;
                break;
            case GETNAMEINFO_ERROR:
                ret_cod = FLOM_RC_GETNAMEINFO_ERROR;
                break;
            case ACCEPT_DISCOVER_REPLY_ERROR:
            case ACCEPT_LOOP_TRANSFER_ERROR:
            case DAEMON_MANAGEMENT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* release peer address if necessary */
    if (NULL != peer_name)
        g_free(peer_name);
    /* release conn if necessary */
    if (NULL != conn)
        flom_conn_delete(conn);
    FLOM_TRACE(("flom_accept_loop_pollin/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_pollin_tls(flom_config_t *config,
                                flom_conn_t *conn)
{
    enum Exception { TLS_NOT_REQUIRED
                     , TLS_CREATE_CONTEXT_ERROR
                     , TLS_SET_CERT_ERROR
                     , TLS_ACCEPT_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_pollin_tls: conn=%p\n", conn));
    TRY {
        /* switch to TLS? */
        if (NULL == flom_config_get_tls_certificate(config) ||
            NULL == flom_config_get_tls_private_key(config) ||
            NULL == flom_config_get_tls_ca_certificate(config)) {
            FLOM_TRACE(("flom_accept_loop_pollin_tls: TLS parameters are "
                        "not configured, TLS will not be used for this "
                        "connection...\n"));
            THROW(TLS_NOT_REQUIRED);
        }
        
        /* initialize TLS/SSL support */
        flom_conn_init_tls(conn, FALSE);
        
        /* create a TLS/SSL context */
        if (FLOM_RC_OK != (ret_cod = flom_tls_context(
                               flom_conn_get_tls(conn))))
            THROW(TLS_CREATE_CONTEXT_ERROR);

        /* set certificates */
        if (FLOM_RC_OK != (ret_cod = flom_tls_set_cert(
                               flom_conn_get_tls(conn),
                               flom_config_get_tls_certificate(config),
                               flom_config_get_tls_private_key(config),
                               flom_config_get_tls_ca_certificate(config))))
            THROW(TLS_SET_CERT_ERROR);

        /* switch the server connection to TLS */
        if (FLOM_RC_OK != (ret_cod = flom_tls_accept(
                               flom_conn_get_tls(conn),
                               flom_tcp_get_sockfd(flom_conn_get_tcp(conn)))))
            THROW(TLS_ACCEPT_ERROR);

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case TLS_NOT_REQUIRED:
                ret_cod = FLOM_RC_OK;
                break;
            case TLS_CREATE_CONTEXT_ERROR:
            case TLS_SET_CERT_ERROR:
            case TLS_ACCEPT_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_pollin_tls/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_transfer(flom_conns_t *conns, guint id,
                              flom_locker_array_t *lockers)
{
    enum Exception { NULL_OBJECT1
                     , CONNS_GET_CD_ERROR1
                     , CONNS_GET_MSG_ERROR
                     , INVALID_VERB_STEP
                     , ACCEPT_LOOP_REPLY_ERROR1
                     , INVALID_RESOURCE_NAME_ERROR
                     , NULL_OBJECT2
                     , ACCEPT_LOOP_REPLY_ERROR2
                     , CANT_WAIT_CONDITION
                     , ACCEPT_LOOP_REPLY_ERROR3
                     , PUT_INTO_INCUBATOR
                     , RESOURCE_INIT_ERROR
                     , ACCEPT_LOOP_START_LOCKER_ERROR
                     , ACCEPT_LOOP_TRANSFER_CONN_ERROR1
                     , CONNS_GET_CD_ERROR2
                     , ACCEPT_LOOP_TRANSFER_CONN_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_accept_loop_transfer\n"));
    TRY {
        guint i, n;
        int found = FALSE;
        GThread *locker_thread = NULL;
        struct flom_msg_s *msg = NULL;
        flom_rsrc_type_t flrt;
        flom_conn_t *conn = NULL;
        struct flom_locker_s *locker = NULL;
        int locker_is_new = FALSE;
        /* check if there is a locker running for this request */
        if (NULL == lockers)
            THROW(NULL_OBJECT1);
        /* retrieve client connection */
        if (NULL == (conn = flom_conns_get_conn(conns, id)))
            THROW(CONNS_GET_CD_ERROR1);
        /* check message */
        if (NULL == (msg = flom_conns_get_msg(conns, id)))
            THROW(CONNS_GET_MSG_ERROR);
        if (FLOM_MSG_VERB_LOCK != msg->header.pvs.verb ||
            FLOM_MSG_STEP_INCR != msg->header.pvs.step) {
            FLOM_TRACE(("flom_accept_loop_transfer: message verb (%d) and/or "
                        "step (%d) are not valid at this point;\n",
                        msg->header.pvs.verb, msg->header.pvs.step));
            THROW(INVALID_VERB_STEP);
        }
        /* is the asked resource a valid resource name (and type!) ? */
        if (FLOM_RSRC_TYPE_NULL == (
                flrt = flom_rsrc_get_type(
                    msg->body.lock_8.resource.name))) {
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop_reply(
                                   conn, FLOM_RC_INVALID_RESOURCE_NAME)))
                THROW(ACCEPT_LOOP_REPLY_ERROR1);
            /* start socket termination... */
            FLOM_TRACE(("flom_accept_loop_transfer: client sent an invalid "
                        "resource name ('%s'), "
                        "starting connection termination for fd=%d\n",
                        msg->body.lock_8.resource.name,
                        flom_tcp_get_sockfd(flom_conn_get_tcp(conn))));
            if (-1 == shutdown(flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                               SHUT_WR))
                FLOM_TRACE(("flom_accept_loop_transfer/shutdown"
                            "(%d,SHUT_WR)=%d "
                            "('%s')\n",
                            flom_tcp_get_sockfd(flom_conn_get_tcp(conn)),
                            errno, strerror(errno)));
            /* return to caller */
            THROW(INVALID_RESOURCE_NAME_ERROR);
        }
                                              
        /* is there a locker already active? */
        n = flom_locker_array_count(lockers);
        for (i=0; i<n; ++i) {
            if (NULL == (locker = flom_locker_array_get(lockers, i)))
                THROW(NULL_OBJECT2);
            if (FLOM_NULL_FD == locker->write_pipe ||
                FLOM_NULL_FD == locker->read_pipe) {
                FLOM_TRACE(("flom_accept_loop_transfer: locker # %u is "
                            "terminating (write_pipe=%d, read_pipe=%d), "
                            "skipping...\n", i, locker->write_pipe,
                            locker->read_pipe));
                continue;
            }
            FLOM_TRACE(("flom_accept_loop_transfer: locker # %u is managing "
                        "resource '%s'\n", i,
                        flom_resource_get_name(&locker->resource)));
            if (!locker->resource.compare_name(
                    &locker->resource, msg->body.lock_8.resource.name)) {
                FLOM_TRACE(("flom_accept_loop_transfer: found locker %u for "
                            "resource '%s'\n", i,
                            msg->body.lock_8.resource.name));
                found = TRUE;
                break;
            }
        } /* for (i=0; i<lockers->n; ++i) */
        if (!found) {
            /* resources with "create=0" (NO) attribute, can not start a
               new locker, but must be kept */
            if (!msg->body.lock_8.resource.create) {
                /* resources with "create=0" (NO) and "wait=0" (NO) attributes,
                   can not be kept and returns immediately to the requester */
                if (!msg->body.lock_8.resource.wait) {
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_reply(
                                           conn, FLOM_RC_LOCK_CANT_WAIT)))
                        THROW(ACCEPT_LOOP_REPLY_ERROR2);
                    /* start socket termination... */
                    FLOM_TRACE(("flom_accept_loop_transfer: client can't "
                                "create a new resource and can't wait, "
                                "starting connection termination for fd=%d\n",
                                flom_tcp_get_sockfd(flom_conn_get_tcp(conn))));
                    if (-1 == shutdown(flom_tcp_get_sockfd(
                                           flom_conn_get_tcp(conn)), SHUT_WR))
                        FLOM_TRACE(("flom_accept_loop_transfer/shutdown"
                                    "(%d,SHUT_WR)=%d "
                                    "('%s')\n",
                                    flom_tcp_get_sockfd(
                                        flom_conn_get_tcp(conn)), errno,
                                    strerror(errno)));
                    /* return to caller */
                    THROW(CANT_WAIT_CONDITION);
                } else {
                    FLOM_TRACE(("flom_accept_loop_transfer: client can't "
                                "create a new resource but can wait, "
                                "putting it inside 'incubator'\n"));
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_reply(
                                           conn, FLOM_RC_LOCK_WAIT_RESOURCE)))
                        THROW(ACCEPT_LOOP_REPLY_ERROR3);
                    flom_conn_set_wait(conn, TRUE);
                    /* return to caller */
                    THROW(PUT_INTO_INCUBATOR);
                } /* if (!msg->body.lock_8.resource.wait) */
            } else {
                /* generate a new unique id */
                uint64_t uid = flom_conns_get_new_uid(conns);
                /* start a new locker */
                ret_cod = flom_accept_loop_start_locker(
                    lockers, msg, flrt, uid, &locker, &locker_thread);
                if (FLOM_RC_RESOURCE_INIT_ERROR == ret_cod) {
                    FLOM_TRACE(("flom_accept_loop_transfer: client requested "
                                "an invalid resource, starting connection "
                                "termination for fd=%d\n",
                                flom_tcp_get_sockfd(flom_conn_get_tcp(conn))));
                    if (-1 == shutdown(flom_tcp_get_sockfd(
                                           flom_conn_get_tcp(conn)), SHUT_WR))
                        FLOM_TRACE(("flom_accept_loop_transfer/shutdown"
                                    "(%d,SHUT_WR)=%d ('%s')\n",
                                    flom_tcp_get_sockfd(
                                        flom_conn_get_tcp(conn)), errno,
                                    strerror(errno)));
                    /* return to caller */
                    THROW(RESOURCE_INIT_ERROR);
                } else if (FLOM_RC_OK != ret_cod) {
                    THROW(ACCEPT_LOOP_START_LOCKER_ERROR);
                } else
                    locker_is_new = TRUE;
            } /* if (!msg->body.lock_8.resource.create) */
        } else
            locker_thread = locker->thread;

        if (FLOM_RC_OK != (ret_cod = flom_accept_loop_transfer_conn(
                               conns, id, locker, conn)))
            THROW(ACCEPT_LOOP_TRANSFER_CONN_ERROR1);

        /* if the a new locker has been created, check the resources
           inside the incubator: it might be some resource can be assigned
           to the new locker */
        if (locker_is_new) {
            /* scanning incubator to retrieve clients waiting for this
               resource */
            i=1;
            while (i < flom_conns_get_used(conns)) {
                flom_conn_t *loop_conn = NULL;
                if (NULL == (loop_conn = flom_conns_get_conn(conns, i)))
                    THROW(CONNS_GET_CD_ERROR2);
                /* check if the connection is really waiting resource
                   creation, else skip it */
                if (!flom_conn_get_wait(loop_conn)) {
                    i++;
                    continue;
                }
                if (!locker->resource.compare_name(
                        &locker->resource,
                        flom_conn_get_msg(loop_conn)->
                        body.lock_8.resource.name)) {
                    FLOM_TRACE(("flom_accept_loop_transfer: connection %u "
                                "(fd=%d) is waiting for resource '%s' and "
                                "can be transferred to this locker\n",
                                i, flom_tcp_get_sockfd(
                                    flom_conn_get_tcp(loop_conn)),
                                flom_conn_get_msg(loop_conn)->
                                body.lock_8.resource.name));
                    if (FLOM_RC_OK != (ret_cod =
                                       flom_accept_loop_transfer_conn(
                                           conns, i, locker, loop_conn)))
                        THROW(ACCEPT_LOOP_TRANSFER_CONN_ERROR2);
                } else
                    i++;
            } /* while (i < flom_conns_get_used(conns))) */
        } /* if (locker_is_new) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_GET_CD_ERROR1:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case CONNS_GET_MSG_ERROR:
                break;
            case INVALID_VERB_STEP:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case ACCEPT_LOOP_REPLY_ERROR1:
                break;
            case INVALID_RESOURCE_NAME_ERROR:
                ret_cod = FLOM_RC_OK;
                break;
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case ACCEPT_LOOP_REPLY_ERROR2:
                break;
            case CANT_WAIT_CONDITION:
                ret_cod = FLOM_RC_OK;
                break;
            case ACCEPT_LOOP_REPLY_ERROR3:
                break;
            case PUT_INTO_INCUBATOR:
            case RESOURCE_INIT_ERROR:
                ret_cod = FLOM_RC_OK;
                break;
            case ACCEPT_LOOP_START_LOCKER_ERROR:
                break;
            case ACCEPT_LOOP_TRANSFER_CONN_ERROR1:
                break;
            case CONNS_GET_CD_ERROR2:
                ret_cod = FLOM_RC_OBJ_CORRUPTED;
                break;
            case ACCEPT_LOOP_TRANSFER_CONN_ERROR2:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_transfer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_transfer_conn(flom_conns_t *conns, guint id,
                                   struct flom_locker_s *locker,
                                   flom_conn_t *conn)
{
    enum Exception { WRITE_ERROR1
                     , CONNS_TRNS_FD
                     , WRITE_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_transfer_conn\n"));
    TRY {
        struct flom_locker_token_s flt;
        GThread *locker_thread = locker->thread;
        
        /* prepare the token for locker thread */
        flt.domain = flom_conns_get_domain(conns);
        flt.client_fd = flom_conns_get_fd(conns, id);
        flt.sequence = ++locker->write_sequence;
        FLOM_TRACE(("flom_accept_loop_transfer_conn: transferring "
                    "connection %u "
                    "(domain=%d, client_fd=%d, sequence=%d) to thread %p "
                    "using pipe %d\n", id, flt.domain, flt.client_fd,
                    flt.sequence, locker_thread, locker->write_pipe));
        /* send token */
        if (sizeof(flt) != write(
                locker->write_pipe, &flt, sizeof(flt)))
            THROW(WRITE_ERROR1);
        /* set the connection as transferred to another thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_trns_fd(conns, id)))
            THROW(CONNS_TRNS_FD);
        /* send connection data (pointer is used because this object will
           be managed by child thread */
        if (sizeof(conn) != write(locker->write_pipe, &conn, sizeof(conn)))
            THROW(WRITE_ERROR2);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case WRITE_ERROR1:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case CONNS_TRNS_FD:
                break;
            case WRITE_ERROR2:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_transfer_conn/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_start_locker(flom_locker_array_t *lockers,
                                  struct flom_msg_s *msg,
                                  flom_rsrc_type_t flrt,
                                  uint64_t uid,
                                  struct flom_locker_s **new_locker,
                                  GThread **new_thread)
{
    enum Exception { RESOURCE_INIT_ERROR
                     , PIPE_ERROR
                     , G_THREAD_CREATE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_locker_s *locker = NULL;

    FLOM_TRACE(("flom_accept_loop_start_locker\n"));
    TRY {
        locker = g_malloc0(sizeof(struct flom_locker_s));
        int pipefd[2];
        GError *error_thread;
        FLOM_TRACE(("flom_accept_loop_start_locker: creating a new locker "
                    "for resource '%s' with uid=" UINT64_T_FORMAT "\n",
                    msg->body.lock_8.resource.name, uid));
        flom_locker_init(locker);
        if (FLOM_RC_OK != (ret_cod = flom_resource_init(
                               &locker->resource, flrt,
                               msg->body.lock_8.resource.name)))
            THROW(RESOURCE_INIT_ERROR);
        /* creating a communication pipe for the new thread */
        if (0 != pipe(pipefd))
            THROW(PIPE_ERROR);
        locker->uid = uid;
        locker->read_pipe = pipefd[0];
        locker->write_pipe = pipefd[1];
        locker->idle_lifespan = msg->body.lock_8.resource.lifespan;
        *new_thread = g_thread_create(flom_locker_loop, (gpointer)locker,
                                      TRUE, &error_thread);
        if (NULL == *new_thread) {
            FLOM_TRACE(("flom_accept_loop_start_locker: "
                        "error_thread->code=%d, "
                        "error_thread->message='%s'\n",
                        error_thread->code, error_thread->message));
            g_free(error_thread);
            THROW(G_THREAD_CREATE_ERROR);
        } else {
            locker->thread = *new_thread;
            FLOM_TRACE(("flom_accept_loop_start_locker: created thread %p\n",
                        *new_thread));
        }
        /* add this locker to the array of all lockers */
        flom_locker_array_add(lockers, locker);
        /* return the new locker to caller function */
        *new_locker = locker;
        locker = NULL;

        THROW(NONE);
    } CATCH {
        switch (excp) {
            case RESOURCE_INIT_ERROR:
                ret_cod = FLOM_RC_RESOURCE_INIT_ERROR;
                break;
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case G_THREAD_CREATE_ERROR:
                ret_cod = FLOM_RC_G_THREAD_CREATE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != locker) {
        /* clean-up locker */
        FLOM_TRACE(("flom_accept_start_locker: clean-up due to excp=%d\n",
                    excp));
        flom_resource_free(&locker->resource);
        g_free(locker);
    } /* if (NULL != locker) */
    FLOM_TRACE(("flom_accept_loop_start_locker/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_chklockers(flom_locker_array_t *lockers, int *again)
{
    enum Exception { NULL_LOCKER
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_chklockers\n"));
    TRY {
        guint i;
        guint number_of_lockers = flom_locker_array_count(lockers);
        *again = FALSE;
        
        for (i=0; i<number_of_lockers; ++i) {
            struct flom_locker_s *fl = flom_locker_array_get(lockers, i);
            if (NULL == fl)
                THROW(NULL_LOCKER);
            if (fl->write_sequence == fl->read_sequence &&
                fl->idle_periods > FLOM_LOCKER_MAX_IDLE_PERIODS) {
                if (fl->write_pipe != FLOM_NULL_FD) {
                    FLOM_TRACE(("flom_accept_loop_chklockers: starting "
                                "termination for locker %u (thread=%p, "
                                "uid=" UINT64_T_FORMAT
                                ", write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread, fl->uid,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    if (-1 == close(fl->write_pipe))
                        THROW(CLOSE_ERROR);
                    fl->write_pipe = FLOM_NULL_FD;
                } else if (fl->write_pipe == FLOM_NULL_FD &&
                           fl->read_pipe == FLOM_NULL_FD) {
                    gpointer thread_ret_cod;
                    FLOM_TRACE(("flom_accept_loop_chklockers: completing "
                                "termination for locker %u (thread=%p, "
                                "uid=" UINT64_T_FORMAT
                                ", write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread, fl->uid,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    /* join already terminated child thread */
                    thread_ret_cod = g_thread_join(fl->thread);
                    FLOM_TRACE(("flom_accept_loop_chklockers/g_thread_join"
                                "(%p)=%p\n", fl->thread, thread_ret_cod));
                    flom_locker_array_del(lockers, fl);
                    /* lockers object changed, break the loop, but call me
                       as soon as possible */
                    *again = TRUE;
                    break;
                }
            } /* if (fl->write_sequence == fl->read_sequence && ... */
        } /* for (i=0; i<number_of_lockers; ++i) */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_LOCKER:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CLOSE_ERROR:
                ret_cod = FLOM_RC_CLOSE_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_chklockers/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_reply(flom_conn_t *conn, int rc)
{
    enum Exception { MSG_BUILD_ANSWER_ERROR
                     , MSG_SERIALIZE_ERROR
                     , MSG_SEND_ERROR
                     , MSG_FREE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_reply\n"));
    TRY {
        struct flom_msg_s msg;
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        
        flom_msg_init(&msg);
        /* prepare answer message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_build_answer(
                               &msg, FLOM_MSG_VERB_LOCK, 2*FLOM_MSG_STEP_INCR,
                               rc, NULL)))
            THROW(MSG_BUILD_ANSWER_ERROR);
        /* serialize the message to the buffer */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);
        /* send message to client (requester) */
        if (FLOM_RC_OK != (ret_cod = flom_conn_send(conn, buffer, to_send)))
            THROW(MSG_SEND_ERROR);
        flom_conn_set_last_step(conn, msg.header.pvs.step);
        /* free message dynamic allocated memory (if any) */
        if (FLOM_RC_OK != (ret_cod = flom_msg_free(&msg)))
            THROW(MSG_FREE_ERROR);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_BUILD_ANSWER_ERROR:
            case MSG_SERIALIZE_ERROR:
            case MSG_SEND_ERROR:
            case MSG_FREE_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_reply/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_discover_reply(flom_config_t *config, int fd,
                               const struct sockaddr *src_addr,
                               socklen_t addrlen)
{
    enum Exception { MSG_SERIALIZE_ERROR
                     , SENDTO_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    struct flom_msg_s msg;
    
    FLOM_TRACE(("flom_accept_discover_reply\n"));
    TRY {
        char buffer[FLOM_NETWORK_BUFFER_SIZE];
        size_t to_send;
        ssize_t sent;
        
        flom_msg_init(&msg);
        /* prepare a reply to discover message */
        msg.header.level = FLOM_MSG_LEVEL;
        msg.header.pvs.verb = FLOM_MSG_VERB_DISCOVER;
        msg.header.pvs.step = 2*FLOM_MSG_STEP_INCR;
        FLOM_TRACE(("flom_accept_discover_reply: unicast_address=%p (%s)\n",
                    flom_config_get_unicast_address(config),
                    flom_config_get_unicast_address(config)));
        msg.body.discover_16.network.port =
            (in_port_t)flom_config_get_unicast_port(config);
        if (NULL != flom_config_get_unicast_address(config) &&
            0 != g_strcmp0(FLOM_INADDR_ANY_STRING,
                          flom_config_get_unicast_address(config)) &&
            0 != g_strcmp0(FLOM_INADDR6_ANY_STRING,
                           flom_config_get_unicast_address(config)))
            msg.body.discover_16.network.address = g_strdup(
                flom_config_get_unicast_address(config));
        
        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);
        FLOM_TRACE(("flom_accept_discover_reply: fd=%d, src_addr=%p, "
                    "addrlen=%d, (%d/%d)\n", fd, src_addr, addrlen,
                    sizeof(struct sockaddr_in), sizeof(struct sockaddr_in6)));
        FLOM_TRACE_HEX_DATA("flom_accept_discover_reply: src_addr ",
                            (void *)src_addr, addrlen);        
        FLOM_TRACE_TEXT_DATA("flom_accept_discover_reply: buffer ",
                             (void *)buffer, to_send);

        /* send reply message */
        if (to_send != (sent = sendto(
                            fd, buffer, to_send, 0, src_addr, addrlen))) {
            THROW(SENDTO_ERROR);
        }        
        FLOM_TRACE(("flom_accept_discover_reply: sendto() to_send=%u, "
                    "sent=%d\n", to_send, sent));
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case MSG_SERIALIZE_ERROR:
                break;
            case SENDTO_ERROR:
                ret_cod = FLOM_RC_SENDTO_ERROR;
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    flom_msg_free(&msg);
    FLOM_TRACE(("flom_accept_discover_reply/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



