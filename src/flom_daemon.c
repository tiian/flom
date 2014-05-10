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
#include <config.h>



#include <stdio.h>
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif
#ifdef HAVE_POLL_H
# include <poll.h>
#endif
#ifdef HAVE_SYSLOG_H
# include <syslog.h>
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
#include "flom_errors.h"
#include "flom_locker.h"
#include "flom_msg.h"
#include "flom_syslog.h"



/* set module trace flag */
#ifdef FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE   FLOM_TRACE_MOD_DAEMON



int flom_daemon(int family)
{
    enum Exception { PIPE_ERROR
                     , FORK_ERROR
                     , WAIT_ERROR
                     , CHILD_PID_ERROR
                     , READ_ERROR
                     , DAEMON_NOT_OK
                     , FORK_ERROR2
                     , SETSID_ERROR
                     , SIGNAL_ERROR
                     , FORK_ERROR3
                     , CHDIR_ERROR
                     , WRITE_ERROR
                     , FLOM_LISTEN_ERROR
                     , FLOM_ACCEPT_LOOP_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int pipefd[2];
    int daemon = FALSE;
    int daemon_rc = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_daemon\n"));
    TRY {
        pid_t pid;

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
            FLOM_TRACE(("flom_daemon: daemonizing... signal()\n"));
            if (SIG_ERR == signal(SIGHUP, SIG_IGN))
                THROW(SIGNAL_ERROR);
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

            FLOM_TRACE_REOPEN(flom_config_get_daemon_trace_file());
            FLOM_TRACE(("flom_daemon: now daemonized!\n"));

            /* activate service */
            openlog("flom", LOG_PID, LOG_DAEMON);
            flom_conns_init(&conns, family);
            daemon_rc = flom_listen(&conns);
            
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
            if (FLOM_RC_OK != (ret_cod = flom_accept_loop(&conns)))
                THROW(FLOM_ACCEPT_LOOP_ERROR);
            syslog(LOG_NOTICE, FLOM_SYSLOG_FLM004N);
            flom_listen_clean(&conns);
        }
        THROW(NONE);
    } CATCH {
        switch (excp) {
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
            case SIGNAL_ERROR:
                ret_cod = FLOM_RC_SIGNAL_ERROR;
                break;
            case FORK_ERROR3:
                ret_cod = FLOM_RC_FORK_ERROR;
                break;
            case CHDIR_ERROR:
                ret_cod = FLOM_RC_CHDIR_ERROR;
                break;
            case WRITE_ERROR:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case FLOM_LISTEN_ERROR:
                ret_cod = daemon_rc;
                break;
            case FLOM_ACCEPT_LOOP_ERROR:
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



int flom_listen(flom_conns_t *conns)
{
    enum Exception { LISTEN_LOCAL_ERROR
                     , LISTEN_TCP_ERROR
                     , LISTEN_UDP_ERROR
                     , INVALID_DOMAIN
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    FLOM_TRACE(("flom_listen\n"));
    TRY {
        switch (flom_conns_get_domain(conns)) {
            case AF_LOCAL:
                if (FLOM_RC_OK != (ret_cod = flom_listen_local(conns)))
                    THROW(LISTEN_LOCAL_ERROR);
                break;
            case AF_INET:
                /* create TCP/IP unicast listener */
                if (FLOM_RC_OK != (ret_cod = flom_listen_tcp(conns)))
                    THROW(LISTEN_TCP_ERROR);
                /* create UDP/IP multicast listener (resolver) */
                if (FLOM_RC_OK != (ret_cod = flom_listen_udp(conns)))
                    THROW(LISTEN_UDP_ERROR);
                break;
            default:
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



int flom_listen_local(flom_conns_t *conns)
{
    enum Exception { SOCKET_ERROR
                     , UNLINK_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , CONNS_ADD_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    int fd = 0;
    
    FLOM_TRACE(("flom_listen_local\n"));
    TRY {
        struct sockaddr_un servaddr;
            
        if (-1 == (fd = socket(flom_conns_get_domain(conns), SOCK_STREAM, 0)))
            THROW(SOCKET_ERROR);
        if (-1 == unlink(flom_config_get_socket_name()) && ENOENT != errno)
            THROW(UNLINK_ERROR);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sun_family = flom_conns_get_domain(conns);
        strcpy(servaddr.sun_path, flom_config_get_socket_name());
        if (-1 == bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
            THROW(BIND_ERROR);
        if (-1 == listen(fd, LISTEN_BACKLOG))
            THROW(LISTEN_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               conns, fd, SOCK_STREAM, sizeof(servaddr),
                               (struct sockaddr *)&servaddr, TRUE)))
            THROW(CONNS_ADD_ERROR);
        syslog(LOG_NOTICE, FLOM_SYSLOG_FLM000I, flom_config_get_socket_name());
        THROW(NONE);
    } CATCH {
        switch (excp) {
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
            case CONNS_ADD_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_listen_local/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp(flom_conns_t *conns)
{
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    FLOM_TRACE(("flom_listen_tcp\n"));
    if (NULL != flom_config_get_unicast_address())
        ret_cod = flom_listen_tcp_configured(conns);
    else
        ret_cod = flom_listen_tcp_automatic(conns);
    syslog(LOG_NOTICE, FLOM_SYSLOG_FLM001I,
           flom_config_get_unicast_address(),
           flom_config_get_unicast_port());
    FLOM_TRACE(("flom_listen_tcp/ret_cod=%d/errno=%d\n", ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp_configured(flom_conns_t *conns)
{
    enum Exception { GETADDRINFO_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , CONNS_ADD_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct addrinfo *result = NULL;
    int fd = NULL_FD;
    
    FLOM_TRACE(("flom_listen_tcp_configured\n"));
    TRY {
        struct addrinfo hints, *gai = NULL;
        int errcode;
        char port[100];

        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = flom_conns_get_domain(conns);
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        snprintf(port, sizeof(port), "%u", flom_config_get_unicast_port());
        FLOM_TRACE(("flom_listen_tcp_configured: binding address '%s' "
                    "and port %s\n", flom_config_get_unicast_address(),
                    port));

        if (0 != (errcode = getaddrinfo(flom_config_get_unicast_address(),
                                        port, &hints, &result))) {
            FLOM_TRACE(("flom_listen_tcp_configured/getaddrinfo(): "
                        "errcode=%d '%s'\n", errcode, gai_strerror(errcode)));
            THROW(GETADDRINFO_ERROR);
        } else {
            int bound = FALSE;
            int sock_opt = 1;
            FLOM_TRACE_ADDRINFO("flom_listen_tcp_configured/getaddrinfo(): ",
                                result);
            /* traverse the list and try to bind... */
            gai = result;
            while (NULL != gai && !bound) {
                FLOM_TRACE_HEX_DATA("flom_listen_tcp_configured: ai_addr ",
                                    (void *)gai->ai_addr, gai->ai_addrlen);
                if (-1 == (fd = socket(gai->ai_family, gai->ai_socktype,
                                       gai->ai_protocol))) {
                    FLOM_TRACE(("flom_listen_tcp_configured/socket(): "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                } else if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                                            (void *)&sock_opt,
                                            sizeof(sock_opt))) {
                    FLOM_TRACE(("flom_listen_tcp_configured/setsockopt() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = NULL_FD;
                } else  if (-1 == bind(fd, gai->ai_addr, gai->ai_addrlen)) {
                    FLOM_TRACE(("flom_listen_tcp_configured/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = NULL_FD;
                } else {
                    bound = TRUE;
                    FLOM_TRACE(("flom_listen_tcp_configured: bound!\n"));
                }
            } /* while (NULL != gai && !bound) */
            if (!bound)
                THROW(BIND_ERROR);
        }        
        if (-1 == listen(fd, LISTEN_BACKLOG))
            THROW(LISTEN_ERROR);
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               conns, fd, SOCK_STREAM, gai->ai_addrlen,
                               gai->ai_addr, TRUE)))
            THROW(CONNS_ADD_ERROR);
        fd = NULL_FD; /* avoid socket close by clean-up section */
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case BIND_ERROR:
                ret_cod = FLOM_RC_BIND_ERROR;
                break;
            case LISTEN_ERROR:
                ret_cod = FLOM_RC_LISTEN_ERROR;
                break;
            case CONNS_ADD_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != result)
        freeaddrinfo(result);
    if (NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_listen_tcp_configured/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_tcp_automatic(flom_conns_t *conns)
{
    enum Exception { SOCKET_ERROR
                     , SETSOCKOPT_ERROR
                     , BIND_ERROR
                     , LISTEN_ERROR
                     , GETSOCKNAME_ERROR
                     , CONNS_ADD_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    int fd = NULL_FD;
    
    FLOM_TRACE(("flom_listen_tcp_automatic\n"));
    TRY {
        struct sockaddr_in soin, addr;
        socklen_t addrlen;
        int sock_opt = 1;
        gint unicast_port;

        if (-1 == (fd = socket(flom_conns_get_domain(conns), SOCK_STREAM,
                               IPPROTO_TCP)))
            THROW(SOCKET_ERROR);
        if (-1 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                             (void *)&sock_opt, sizeof(sock_opt)))
            THROW(SETSOCKOPT_ERROR);
        /* binding local address, ephemeral port */
        memset(&soin, 0, sizeof(soin));
        soin.sin_addr.s_addr = htonl(INADDR_ANY);
        soin.sin_port = 0;
        if (-1 == bind(fd, (struct sockaddr *)&soin, sizeof(soin)))
            THROW(BIND_ERROR);
        if (-1 ==listen(fd, 100))
            THROW(LISTEN_ERROR);
        /* retrieve address and port */
        addrlen = sizeof(addr);
        memset(&addr, 0, addrlen);
        if (-1 == getsockname(fd, (struct sockaddr *)&addr, &addrlen))
            THROW(GETSOCKNAME_ERROR);
        FLOM_TRACE_HEX_DATA("flom_listen_tcp_automatic: addr ",
                            (void *)&addr, addrlen);
        /* inject address value to configuration */
        FLOM_TRACE(("flom_listen_tcp_automatic: set unicast address to value "
                    "'0.0.0.0'\n"));
        flom_config_set_unicast_address("0.0.0.0");
        /* inject port value to configuration */
        unicast_port = ntohs(addr.sin_port);
        FLOM_TRACE(("flom_listen_tcp_automatic: set unicast port to value "
                    "%d\n", unicast_port));
        flom_config_set_unicast_port(unicast_port);
        /* add connection to pool */
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               conns, fd, SOCK_STREAM, addrlen,
                               (struct sockaddr *)&addr, TRUE)))
            THROW(CONNS_ADD_ERROR);
        fd = NULL_FD; /* avoid socket close by clean-up section */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case SOCKET_ERROR:
                ret_cod = FLOM_RC_SOCKET_ERROR;
                break;
            case SETSOCKOPT_ERROR:
                ret_cod = FLOM_RC_SETSOCKOPT_ERROR;
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
            case CONNS_ADD_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_listen_tcp_automatic/excp=%d/"
                "ret_cod=%d/errno=%d ('%s')\n", excp, ret_cod, errno,
                strerror(errno)));
    return ret_cod;
}



int flom_listen_udp(flom_conns_t *conns)
{
    enum Exception { NO_MULTICAST
                     , GETADDRINFO_ERROR
                     , CONNECT_ERROR
                     , CONNS_ADD_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    struct addrinfo *result = NULL;
    int fd = NULL_FD;
    
    FLOM_TRACE(("flom_listen_udp\n"));
    TRY {
        struct addrinfo hints;
        struct sockaddr_in local_address;
        char port[100];
        int errcode;
        int found = FALSE;
        const struct addrinfo *gai = result;
        
        if (NULL == flom_config_get_multicast_address()) {
            FLOM_TRACE(("flom_listen_udp: no multicast address specified, "
                        "this listener will not answer to daemon location "
                        "inquiries\n"));
            THROW(NO_MULTICAST);
        }

        FLOM_TRACE(("flom_listen_udp: creating multicast daemon locator "
                    "using address '%s' and port %d\n",
                    flom_config_get_multicast_address(),
                    flom_config_get_multicast_port()));
        memset(&hints, 0, sizeof(hints));
        hints.ai_flags = AI_CANONNAME;
        /* prepare a local address structure for incoming datagrams */
        memset(&local_address, 0, sizeof(local_address));
        local_address.sin_family = AF_INET;
        local_address.sin_addr.s_addr = htonl(INADDR_ANY);
        local_address.sin_port = htons(flom_config_get_multicast_port());
        /* remove this filter to support IPV6, but most of the following
           calls must be fixed! */
        hints.ai_family = AF_INET; 
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
        snprintf(port, sizeof(port), "%u", flom_config_get_multicast_port());

        if (0 != (errcode = getaddrinfo(flom_config_get_multicast_address(),
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
                    fd = NULL_FD;
                } else if (-1 == bind(fd, (struct sockaddr *)&local_address,
                                      sizeof(local_address))) {
                    FLOM_TRACE(("flom_listen_udp/bind() : "
                                "errno=%d '%s', skipping...\n", errno,
                                strerror(errno)));
                    gai = gai->ai_next;
                    close(fd);
                    fd = NULL_FD;
                } else { /* switching to multicast mode */
                    struct ip_mreq mreq;
                    
                    memcpy(&mreq.imr_multiaddr,
                           &((struct sockaddr_in *)gai->ai_addr)->sin_addr,
                           sizeof(struct in_addr));
                    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
                    if (-1 == setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                                            &mreq, sizeof(mreq))) {
                        FLOM_TRACE(("flom_listen_udp/setsockopt("
                                    "IP_ADD_MEMBERSHIP) : "
                                    "errno=%d '%s', skipping...\n", errno,
                                    strerror(errno)));
                        gai = gai->ai_next;
                        close(fd);
                        fd = NULL_FD;
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
        /* add connection */
        if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                               conns, fd, SOCK_DGRAM, gai->ai_addrlen,
                               gai->ai_addr, TRUE)))
            THROW(CONNS_ADD_ERROR);
        fd = NULL_FD; /* avoid socket close by clean-up section */        
        syslog(LOG_NOTICE, FLOM_SYSLOG_FLM002I,
               flom_config_get_multicast_address(),
               flom_config_get_multicast_port());
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NO_MULTICAST:
                ret_cod = FLOM_RC_OK;
                break;
            case GETADDRINFO_ERROR:
                ret_cod = FLOM_RC_GETADDRINFO_ERROR;
                break;
            case CONNECT_ERROR:
                ret_cod = FLOM_RC_CONNECT_ERROR;
                break;
            case CONNS_ADD_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    if (NULL != result)
        freeaddrinfo(result);
    if (NULL_FD != fd)
        close(fd);
    FLOM_TRACE(("flom_listen_udp/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_listen_clean(flom_conns_t *conns)
{
    enum Exception { NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_listen_clean\n"));
    TRY {
        flom_conns_free(conns);
        if (-1 == unlink(global_config.socket_name)) {
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


int flom_accept_loop(flom_conns_t *conns)
{
    enum Exception { CONNS_CLEAN_ERROR
                     , CONNS_GET_FDS_ERROR
                     , CONNS_SET_EVENTS_ERROR
                     , POLL_ERROR
                     , ACCEPT_LOOP_CHKLOCKERS_ERROR1
                     , NEGATIVE_NUMBER_OF_LOCKERS_ERROR1
                     , CONNS_CLOSE_ERROR
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

        flom_locker_array_init(&lockers);
        
        while (loop) {
            int ready_fd;
            guint i, n;
            struct pollfd *fds;
            guint number_of_lockers;
            int poll_timeout = flom_config_get_lifespan();

            /* the completion needs three polling cycles, so timeout must
               be a third of estimated lifespan */
            if (3 < poll_timeout)
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
            if (0 > ready_fd)
                THROW(POLL_ERROR);
            /* poll exited due to time out */
            if (0 == ready_fd) {
                number_of_lockers = flom_locker_array_count(&lockers);
                FLOM_TRACE(("flom_accept_loop: idle time exceeded %d "
                            "milliseconds, number of lockers=%u\n",
                            poll_timeout, number_of_lockers));
                if (0 == number_of_lockers) {
                    if (1 == flom_conns_get_used(conns) ||
                        (2 == flom_conns_get_used(conns) &&
                         SOCK_DGRAM == flom_conns_get_type(conns, 1))) {
                        FLOM_TRACE(("flom_accept_loop: only listener "
                                    "connection is active, exiting...\n"));
                        loop = FALSE;
                    }
                } else if (0 < number_of_lockers) {
                    if (FLOM_RC_OK != (ret_cod =
                                       flom_accept_loop_chklockers(&lockers)))
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
                        THROW(CONNS_CLOSE_ERROR);
                    /* this file descriptor is no more valid, continue to
                       next one */
                    continue;
                }
                if (fds[i].revents & POLLIN) {
                    int conn_moved = FALSE;
                    if (FLOM_RC_OK != (ret_cod = flom_accept_loop_pollin(
                                           conns, i, &lockers, &conn_moved)))
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
                                   flom_accept_loop_chklockers(&lockers)))
                    THROW(ACCEPT_LOOP_CHKLOCKERS_ERROR2);
            } else if (0 > number_of_lockers) {
                THROW(NEGATIVE_NUMBER_OF_LOCKERS_ERROR2);
            }
        } /* while (loop) */
        THROW(NONE);
    } CATCH {
        switch (excp) {
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
            case CONNS_CLOSE_ERROR:
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
    flom_locker_array_free(&lockers);
    FLOM_TRACE(("flom_accept_loop/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_pollin(flom_conns_t *conns, guint id,
                            flom_locker_array_t *lockers,
                            int *moved)
{
    enum Exception { CONNS_GET_CD_ERROR
                     , ACCEPT_ERROR
                     , SETSOCKOPT_ERROR
                     , CONN_SET_KEEPALIVE_ERROR
                     , CONNS_ADD_ERROR
                     , MSG_RETRIEVE_ERROR
                     , CONNS_GET_MSG_ERROR
                     , CONNS_GET_GMPC_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , CONNS_CLOSE_ERROR
                     , PROTOCOL_ERROR
                     , GETNAMEINFO_ERROR
                     , ACCEPT_DISCOVER_REPLY_ERROR
                     , ACCEPT_LOOP_TRANSFER_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_pollin\n"));
    TRY {
        struct flom_conn_data_s *c;

        *moved = FALSE;
        if (NULL == (c = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        FLOM_TRACE(("flom_accept_loop_pollin: id=%u, fd=%d\n",
                    id, c->fd));
        if (0 == id) {
            /* it's a new connection */
            int conn_fd;
            struct sockaddr cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            if (-1 == (conn_fd = accept(c->fd, &cliaddr, &clilen)))
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
                if (FLOM_RC_OK != (ret_cod = flom_conn_set_keepalive(conn_fd)))
                    THROW(CONN_SET_KEEPALIVE_ERROR);
            }
            if (FLOM_RC_OK != (ret_cod = flom_conns_add(
                                   conns, conn_fd, SOCK_STREAM, clilen,
                                   &cliaddr, TRUE)))
                THROW(CONNS_ADD_ERROR);
        } else {
            char buffer[FLOM_MSG_BUFFER_SIZE];
            ssize_t read_bytes;
            struct flom_msg_s *msg;
            GMarkupParseContext *gmpc;
            struct sockaddr_in src_addr;
            socklen_t addrlen = sizeof(src_addr);
            /* it's data from an existing connection */
            if (FLOM_RC_OK != (ret_cod = flom_msg_retrieve(
                                   c->fd, c->type, buffer, sizeof(buffer),
                                   &read_bytes, FLOM_NETWORK_WAIT_TIMEOUT,
                                   (struct sockaddr *)&src_addr, &addrlen)))
                THROW(MSG_RETRIEVE_ERROR);

            if (NULL == (msg = flom_conns_get_msg(conns, id)))
                THROW(CONNS_GET_MSG_ERROR);

            if (NULL == (gmpc = flom_conns_get_gmpc(conns, id)))
                THROW(CONNS_GET_GMPC_ERROR);
            
            if (FLOM_RC_OK != (ret_cod = flom_msg_deserialize(
                                   buffer, read_bytes, msg, gmpc)))
                THROW(MSG_DESERIALIZE_ERROR);
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
                    THROW(CONNS_CLOSE_ERROR);            
            }
            /* check if the message is completely parsed and can be transferred
               to a slave thread (a locker) */
            if (FLOM_MSG_STATE_READY == msg->state) {
                /* check the message is protocol correct */
                if (!flom_msg_check_protocol(msg, TRUE))
                    THROW(PROTOCOL_ERROR);
                /* check the message is discover */
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
                                           flom_conns_get_fd(conns, id),
                                           (const struct sockaddr *)&src_addr,
                                           addrlen)))
                        THROW(ACCEPT_DISCOVER_REPLY_ERROR);
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
            case CONNS_ADD_ERROR:
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:
                break;
            case CONNS_GET_MSG_ERROR:
            case CONNS_GET_GMPC_ERROR:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_CLOSE_ERROR:
                break;
            case PROTOCOL_ERROR:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case GETNAMEINFO_ERROR:
                ret_cod = FLOM_RC_GETNAMEINFO_ERROR;
                break;
            case ACCEPT_DISCOVER_REPLY_ERROR:
            case ACCEPT_LOOP_TRANSFER_ERROR:
                break;
            case NONE:
                ret_cod = FLOM_RC_OK;
                break;
            default:
                ret_cod = FLOM_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_pollin/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_transfer(flom_conns_t *conns, guint id,
                              flom_locker_array_t *lockers)
{
    enum Exception { NULL_OBJECT1
                     , INVALID_VERB_STEP
                     , LOCKER_CHECK_RESOURCE_NAME_ERROR
                     , NULL_OBJECT2
                     , CONNS_GET_MSG_ERROR
                     , RESOURCE_INIT_ERROR
                     , PIPE_ERROR
                     , G_THREAD_CREATE_ERROR
                     , WRITE_ERROR1
                     , CONNS_GET_CD_ERROR
                     , CONNS_TRNS_FD
                     , WRITE_ERROR2
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;

    struct flom_locker_s *locker = NULL;
    int locker_allocated = FALSE;
    
    FLOM_TRACE(("flom_accept_loop_transfer\n"));
    TRY {
        guint i, n;
        int found = FALSE;
        GThread *locker_thread = NULL;
        struct flom_msg_s *msg = NULL;
        struct flom_locker_token_s flt;
        flom_rsrc_type_t flrt;
        const struct flom_conn_data_s *cd = NULL;
        /* check if there is a locker running for this request */
        if (NULL == lockers)
            THROW(NULL_OBJECT1);
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
                    msg->body.lock_8.resource.name)))
            /* @@@ send and error message to client and disconnect instead of
             exiting! */
            THROW(LOCKER_CHECK_RESOURCE_NAME_ERROR);
                                              
        /* is there a locker already active? */
        n = flom_locker_array_count(lockers);
        for (i=0; i<n; ++i) {
            if (NULL == (locker = flom_locker_array_get(lockers, i)))
                THROW(NULL_OBJECT2);
            if (NULL_FD == locker->write_pipe ||
                NULL_FD == locker->read_pipe) {
                FLOM_TRACE(("flom_accept_loop_transfer: locker # %u is "
                            "terminating (write_pipe=%d, read_pipe=%d), "
                            "skipping...\n", i, locker->write_pipe,
                            locker->read_pipe));
                continue;
            }
            FLOM_TRACE(("flom_accept_loop_transfer: locker # %u is managing "
                        "resource '%s'\n", i,
                        flom_resource_get_name(&locker->resource)));
            if (!g_strcmp0(flom_resource_get_name(&locker->resource),
                           msg->body.lock_8.resource.name)) {
                FLOM_TRACE(("flom_accept_loop_transfer: found locker %u for "
                            "resource '%s'\n", i,
                            msg->body.lock_8.resource.name));
                found = TRUE;
                break;
            }
        } /* for (i=0; i<lockers->n; ++i) */
        if (!found) {
            /* start a new locker */
            locker = g_malloc0(sizeof(struct flom_locker_s));
            int pipefd[2];
            GError *error_thread;
            locker_allocated = TRUE;
            FLOM_TRACE(("flom_accept_loop_transfer: creating a new locker "
                        "for resource '%s'...\n",
                        msg->body.lock_8.resource.name));
            flom_locker_init(locker);
            if (FLOM_RC_OK != (ret_cod = flom_resource_init(
                                   &locker->resource, flrt,
                                   msg->body.lock_8.resource.name)))
                THROW(RESOURCE_INIT_ERROR);
            /* creating a communication pipe for the new thread */
            if (0 != pipe(pipefd))
                THROW(PIPE_ERROR);
            locker->read_pipe = pipefd[0];
            locker->write_pipe = pipefd[1];
            locker_thread = g_thread_create(flom_locker_loop, (gpointer)locker,
                                            TRUE, &error_thread);
            if (NULL == locker_thread) {
                FLOM_TRACE(("flom_accept_loop_transfer: "
                            "error_thread->code=%d, "
                            "error_thread->message='%s'\n",
                            error_thread->code, error_thread->message));
                g_free(error_thread);
                THROW(G_THREAD_CREATE_ERROR);
            } else {
                FLOM_TRACE(("flom_accept_loop_transfer: created thread %p\n",
                            locker_thread));
            }
            /* add this locker to the array of all lockers */
            flom_locker_array_add(lockers, locker);
        } else
            locker_thread = locker->thread;

        /* prepare the token for locker thread */
        flt.domain = flom_conns_get_domain(conns);
        flt.client_fd = flom_conns_get_fd(conns, id);
        flt.sequence = ++locker->write_sequence;
        FLOM_TRACE(("flom_accept_loop_transfer: transferring connection %u "
                    "(domain=%d, client_fd=%d, sequence=%d) to thread %p "
                    "using pipe %d\n", id, flt.domain, flt.client_fd,
                    flt.sequence, locker_thread, locker->write_pipe));
        /* send token */
        if (sizeof(flt) != write(
                locker->write_pipe, &flt, sizeof(flt)))
            THROW(WRITE_ERROR1);
        /* send connection data (pointer is used because this object will
           be managed by child thread */
        if (NULL == (cd = flom_conns_get_cd(conns, id)))
            THROW(CONNS_GET_CD_ERROR);
        /* set the connection as transferred to another thread */
        if (FLOM_RC_OK != (ret_cod = flom_conns_trns_fd(conns, id)))
            THROW(CONNS_TRNS_FD);
        if (sizeof(cd) != write(locker->write_pipe, &cd, sizeof(cd)))
            THROW(WRITE_ERROR2);
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NULL_OBJECT1:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case INVALID_VERB_STEP:
                ret_cod = FLOM_RC_PROTOCOL_ERROR;
                break;
            case LOCKER_CHECK_RESOURCE_NAME_ERROR:
                break;
            case NULL_OBJECT2:
                ret_cod = FLOM_RC_NULL_OBJECT;
                break;
            case CONNS_GET_MSG_ERROR:
            case RESOURCE_INIT_ERROR:
                break;
            case PIPE_ERROR:
                ret_cod = FLOM_RC_PIPE_ERROR;
                break;
            case G_THREAD_CREATE_ERROR:
                ret_cod = FLOM_RC_G_THREAD_CREATE_ERROR;
                break;
            case WRITE_ERROR1:
                ret_cod = FLOM_RC_WRITE_ERROR;
                break;
            case CONNS_GET_CD_ERROR:
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
        if (NONE != excp && locker_allocated) {
            /* clean-up locker */
            FLOM_TRACE(("flom_accept_loop_transfer: clean-up due to excp=%d\n",
                        excp));
            flom_resource_free(&locker->resource);
            g_free(locker);
        }
    } /* TRY-CATCH */
    FLOM_TRACE(("flom_accept_loop_transfer/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int flom_accept_loop_chklockers(flom_locker_array_t *lockers)
{
    enum Exception { NULL_LOCKER
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = FLOM_RC_INTERNAL_ERROR;
    
    FLOM_TRACE(("flom_accept_loop_chklockers\n"));
    TRY {
        guint i;
        guint number_of_lockers = flom_locker_array_count(lockers);
        
        for (i=0; i<number_of_lockers; ++i) {
            struct flom_locker_s *fl = flom_locker_array_get(lockers, i);
            if (NULL == fl)
                THROW(NULL_LOCKER);
            if (fl->write_sequence == fl->read_sequence &&
                fl->idle_periods > 1) {
                if (fl->write_pipe != NULL_FD) {
                    FLOM_TRACE(("flom_accept_loop_chklockers: starting "
                                "termination for locker %u (thread=%p, "
                                "write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    if (-1 == close(fl->write_pipe))
                        THROW(CLOSE_ERROR);
                    fl->write_pipe = NULL_FD;
                } else if (fl->write_pipe == NULL_FD &&
                           fl->read_pipe == NULL_FD) {
                    gpointer thread_ret_cod;
                    FLOM_TRACE(("flom_accept_loop_chklockers: completing "
                                "termination for locker %u (thread=%p, "
                                "write_pipe=%d, read_pipe=%d, "
                                "resource_name='%s', "
                                "write_sequence=%d, read_sequence=%d, "
                                "idle_periods=%d\n", i, fl->thread,
                                fl->write_pipe, fl->read_pipe,
                                flom_resource_get_name(&fl->resource),
                                fl->write_sequence,
                                fl->read_sequence, fl->idle_periods));
                    /* join already terminated child thread */
                    thread_ret_cod = g_thread_join(fl->thread);
                    FLOM_TRACE(("flom_accept_loop_chklockers/g_thread_join"
                                "(%p)=%p\n", fl->thread, thread_ret_cod));
                    flom_locker_array_del(lockers, fl);
                    /* lockers object changed, break the loop */
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



int flom_accept_discover_reply(int fd, const struct sockaddr *src_addr,
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
        msg.body.discover_16.network.port =
            (in_port_t)flom_config_get_unicast_port();
        if (NULL != flom_config_get_unicast_address())
            msg.body.discover_16.network.address = g_strdup(
                flom_config_get_unicast_address());
        /* serialize the request message */
        if (FLOM_RC_OK != (ret_cod = flom_msg_serialize(
                               &msg, buffer, sizeof(buffer), &to_send)))
            THROW(MSG_SERIALIZE_ERROR);
        FLOM_TRACE(("flom_accept_discover_reply: fd=%d, src_addr=%p, "
                    "addrlen=%d\n", fd, src_addr, addrlen));
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

