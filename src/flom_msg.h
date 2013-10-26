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
#ifndef MSG_H
# define MSG_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_



/**
 * Default buffer size for XML messages (used for serialization/
 * deserialization)
 **/
#define FLOM_MSG_BUFFER_SIZE 512
/**
 * Number of digits prefix of a message
 */
#define FLOM_MSG_PREFIX_DIGITS  3
/**
 * Current protocol level; it's used to recognize incompatible client/server
 * configuration at run-time
 */
#define FLOM_MSG_LEVEL          0
/**
 * Id reserved for a null message: do NOT change this value because it
 * would break the @ref flom_msg_init behavior
 */
#define FLOM_MSG_VERB_NULL      0
/**
 * Id assigned to verb "lock"
 */
#define FLOM_MSG_VERB_LOCK      1
/**
 * Id assigned to verb "unlock"
 */
#define FLOM_MSG_VERB_UNLOCK    2
/**
 * Id assigned to verb "ping"
 */
#define FLOM_MSG_VERB_PING      3

/**
 * Default increment for message step
 */
#define FLOM_MSG_STEP_INCR      8



/**
 * Label used to specify initial XML header
 */
extern const gchar *FLOM_MSG_HEADER;
/**
 * Label used to specify "name" property
 */
extern const gchar *FLOM_MSG_PROP_NAME;
/**
 * Label used to specify "type" property
 */
extern const gchar *FLOM_MSG_PROP_TYPE;
/**
 * Label used to specify "wait" property
 */
extern const gchar *FLOM_MSG_PROP_WAIT;
/**
 * Label used to specify "answer" tag
 */
extern const gchar *FLOM_MSG_TAG_ANSWER;
/**
 * Label used to specify "msg" tag
 */
extern const gchar *FLOM_MSG_TAG_MSG;
/**
 * Label used to specify "resource" tag
 */
extern const gchar *FLOM_MSG_TAG_RESOURCE;



/**
 * The communication protocol is discrete and the values are in the set
 * (verb x step)
 */
struct flom_msg_verb_step_s {
    /**
     * Specifies the verb (open, close, begin, commit, ecc...)
     */
    int verb;
    /**
     * Specifies the step inside the verb (1, 2, 3, ...)
     */
    int step; 
};



/**
 * Mandatory header for every message encoded as @ref flom_msg_s
 */
struct flom_msg_header_s {
    /**
     * Protocol level must be applied to this message
     */
    int                         level;
    /**
     * Protocol verb and step of the message
     */
    struct flom_msg_verb_step_s pvs;
};

 

/**
 * Generic answer message struct
 */
struct flom_msg_body_answer_s {
    /**
     * Return code of the invoked operation
     */
    int rc;
};



/**
 * Convenience struct for @ref flom_msg_body_open_8_s
 */
struct flom_msg_body_lock_8_resource_s {
    /**
     * name of the resource to lock
     */
    gchar     *name;
    /**
     * type of lock to acquire
     */
    int        type;
    /**
     * wait if lock is currently not available
     */
    int        wait;
};

    

/**
 * Message body for verb "lock", step "8"
 */
struct flom_msg_body_lock_8_s {
    struct flom_msg_body_lock_8_resource_s   resource;
};



/**
 * Message body for verb "lock", step "16"
 */
struct flom_msg_body_lock_16_s {
    struct flom_msg_body_answer_s            answer;
};



/**
 * Message body for verb "lock", step "24"
 */
struct flom_msg_body_lock_24_s {
    struct flom_msg_body_answer_s            answer;
};





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



#endif /* MSG_H */



/*
 * Copyright (c) 2009-2012, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of LIXA.
 *
 * LIXA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * LIXA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LIXA.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIXA_XML_MSG_H
# define LIXA_XML_MSG_H



#include <config.h>



#ifdef HAVE_LIBXML_TREE_H
# include <libxml/tree.h>
#endif
#ifdef HAVE_LIBXML_PARSER_H
# include <libxml/parser.h>
#endif
#ifdef HAVE_GLIB_H
# include <glib.h>
#endif



#include <lixa_config.h>
#include <xa.h>



/* save old LIXA_TRACE_MODULE and set a new value */
#ifdef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE_SAVE LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#else
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE      LIXA_TRACE_MOD_COMMON_XML_MSG







/**
 * Control thread status
 */
struct lixa_msg_body_open_24_conthr_s {
    /**
     * State of the control thread
     */
    int                       txstate;
};



/**
 * Convenience struct for @ref lixa_msg_body_open_24_s
 */
struct lixa_msg_body_open_24_xa_open_execs_s {
    /**
     * rmid parameter as passed to xa_open routine
     */
    int             rmid;
    /**
     * xa_info parameter as passed to xa_open routine
     */
    xmlChar        *xa_info;
    /**
     * flags parameter as passed to xa_open routine
     */
    long            flags;
    /**
     * return code of xa_open routine
     */
    int             rc;
    /**
     * the new resource manager state after xa_open execution
     */
    int             r_state;
};



/**
 * Message body for verb "open", step "24"
 */
struct lixa_msg_body_open_24_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_open_24_conthr_s   conthr;
    /**
     * Parameters and return value of xa_open executions
     */
    GArray                                 *xa_open_execs;
};



/**
 * Convenience struct for @ref lixa_msg_body_open_8_s
 */
struct lixa_msg_body_close_8_rsrmgr_s {
    /**
     * rmid parameter as passed to xa_close routine
     */
    int        rmid;
};

    

/**
 * Message body for verb "close", step "8"
 */
struct lixa_msg_body_close_8_s {
    GArray                   *rsrmgrs;
};



/**
 * Convenience struct for @ref lixa_msg_body_start_8_s
 */
struct lixa_msg_body_start_8_conthr_s {
    /**
     * Transaction id
     */
    XID   xid;
};

    

/**
 * Convenience struct for @ref lixa_msg_body_start_8_s
 */
struct lixa_msg_body_start_8_rsrmgr_s {
    /**
     * rmid parameter as passed to xa_start routine
     */
    int        rmid;
};

    

/**
 * Message body for verb "start", step "8"
 */
struct lixa_msg_body_start_8_s {
    struct lixa_msg_body_start_8_conthr_s   conthr;
    GArray                                 *rsrmgrs;
};



/**
 * Message body for verb "start", step "16"
 */
struct lixa_msg_body_start_16_s {
    struct lixa_msg_body_answer_s   answer;
};



/**
 * Control thread status
 */
struct lixa_msg_body_start_24_conthr_s {
    /**
     * State of the control thread
     */
    int                       txstate;
};



/**
 * Convenience struct for @ref lixa_msg_body_start_24_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_start_8_s
 */
struct lixa_msg_body_start_24_xa_start_execs_s {
    /**
     * rmid parameter as passed to xa_start routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_start routine
     */
    long            flags;
    /**
     * return code of xa_start routine
     */
    int             rc;
    /**
     * the new transaction branch association state associated to the resource
     * manager after xa_start execution
     */
    int             td_state;
    /**
     * the new transaction branch state associated to the resource
     * manager after xa_start execution
     */
    int             s_state;
};



/**
 * Message body for verb "start", step "24"
 */
struct lixa_msg_body_start_24_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_start_24_conthr_s   conthr;
    /**
     * Parameters and return value of xa_start executions
     */
    GArray                                  *xa_start_execs;
};



/**
 * Convenience struct for @ref lixa_msg_body_end_8_s
 */
struct lixa_msg_body_end_8_conthr_s {
    /**
     * TRUE = commit
     * FALSE = rollback
     */
    int   commit;
};

    

/**
 * Convenience struct for @ref lixa_msg_body_end_8_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_end_8_s
 */
struct lixa_msg_body_end_8_xa_end_execs_s {
    /**
     * rmid parameter as passed to xa_end routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_end routine
     */
    long            flags;
    /**
     * return code of xa_end routine
     */
    int             rc;
    /**
     * the new transaction branch association state associated to the resource
     * manager after xa_end execution
     */
    int             td_state;
    /**
     * the new transaction branch state associated to the resource
     * manager after xa_end execution
     */
    int             s_state;
};



/**
 * Message body for verb "end", step "8"
 */
struct lixa_msg_body_end_8_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_end_8_conthr_s      conthr;
    /**
     * Parameters and return value of xa_end executions
     */
    GArray                                  *xa_end_execs;
};



/**
 * Message body for verb "end", step "16"
 */
struct lixa_msg_body_end_16_s {
    struct lixa_msg_body_answer_s            answer;
};



/**
 * Control thread status
 */
struct lixa_msg_body_prepare_8_conthr_s {
    /**
     * TRUE = commit
     * FALSE = rollback
     */
    int   commit;
};



/**
 * Convenience struct for @ref lixa_msg_body_prepare_8_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_prepare_8_s
 */
struct lixa_msg_body_prepare_8_xa_prepare_execs_s {
    /**
     * rmid parameter as passed to xa_prepare routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_prepare routine
     */
    long            flags;
    /**
     * return code of xa_prepare routine
     */
    int             rc;
    /**
     * the new transaction branch state associated to the resource
     * manager after xa_prepare execution
     */
    int             s_state;
    /**
     * the new transaction branch association state associated to the resource
     * manager after xa_prepare execution
     */
    int             td_state;
};



/**
 * Message body for verb "prepare", step "8"
 */
struct lixa_msg_body_prepare_8_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_prepare_8_conthr_s   conthr;
    /**
     * Parameters and return value of xa_prepare executions
     */
    GArray                                   *xa_prepare_execs;
};



/**
 * Message body for verb "prepare", step "16"
 */
struct lixa_msg_body_prepare_16_s {
    struct lixa_msg_body_answer_s   answer;
};



/**
 * Control thread status
 */
struct lixa_msg_body_commit_8_conthr_s {
    /**
     * TRUE = yes
     * FALSE = no
     */
    int   finished;
};



/**
 * Convenience struct for @ref lixa_msg_body_commit_8_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_commit_8_s
 */
struct lixa_msg_body_commit_8_xa_commit_execs_s {
    /**
     * rmid parameter as passed to xa_commit routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_commit routine
     */
    long            flags;
    /**
     * return code of xa_commit routine
     */
    int             rc;
    /**
     * the new resource manager state after xa_commit execution
     */
    int             r_state;
    /**
     * the new transaction branch state after xa_commit execution
     */
    int             s_state;
};



/**
 * Message body for verb "commit", step "8"
 */
struct lixa_msg_body_commit_8_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_commit_8_conthr_s    conthr;
    /**
     * Parameters and return value of xa_commit executions
     */
    GArray                                   *xa_commit_execs;
};



/**
 * Control thread status
 */
struct lixa_msg_body_rollback_8_conthr_s {
    /**
     * TRUE = yes
     * FALSE = no
     */
    int   finished;
};



/**
 * Convenience struct for @ref lixa_msg_body_rollback_8_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_rollback_8_s
 */
struct lixa_msg_body_rollback_8_xa_rollback_execs_s {
    /**
     * rmid parameter as passed to xa_rollback routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_rollback routine
     */
    long            flags;
    /**
     * return code of xa_rollback routine
     */
    int             rc;
    /**
     * the new resource manager state after xa_rollback execution
     */
    int             r_state;
    /**
     * the new transaction branch state after xa_rollback execution
     */
    int             s_state;
};



/**
 * Message body for verb "rollback", step "8"
 */
struct lixa_msg_body_rollback_8_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_rollback_8_conthr_s    conthr;
    /**
     * Parameters and return value of xa_rollback executions
     */
    GArray                                     *xa_rollback_execs;
};



/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_8_s
 */
struct lixa_msg_body_qrcvr_8_client_s {
    xmlChar           *job;
    md5_digest_hex_t   config_digest;
};

    

/**
 * Message body for verb "qrcvr", step "8"
 */
struct lixa_msg_body_qrcvr_8_s {
    struct lixa_msg_body_qrcvr_8_client_s client;
};



/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_16_client_s
 */
struct lixa_msg_body_qrcvr_16_state_s {
    /**
     * Boolean: did the transaction finish?
     */
    int      finished;
    /**
     * Client TX state
     */
    int      txstate;
    /**
     * Boolean: did the transaction asked commit?
     */
    int      will_commit;
    /**
     * Boolean: did the transaction asked rollback?
     */
    int      will_rollback;
    /**
     * Transaction id
     */
    XID      xid;
};

    

/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_16_s
 */
struct lixa_msg_body_qrcvr_16_rsrmgr_s {
    /**
     * rmid parameter as passed to xa_close routine
     */
    int        rmid;
    /**
     * next expected verb at crash time
     */
    int        next_verb;
    /**
     * the resource manager state at crash time
     */
    int        r_state;
    /**
     * the transaction branch state at crash time
     */
    int        s_state;
    /**
     * the transaction branch association state associated to the resource
     * manager at crash time
     */
    int        td_state;
};

    

/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_16_s
 */
struct lixa_msg_body_qrcvr_16_client_s {
    xmlChar                                 *job;
    md5_digest_hex_t                         config_digest;
    struct lixa_msg_verb_step_s              last_verb_step;
    struct lixa_msg_body_qrcvr_16_state_s    state;
};

    

/**
 * Message body for verb "qrcvr", step "16"
 */
struct lixa_msg_body_qrcvr_16_s {
    struct lixa_msg_body_answer_s            answer;
    struct lixa_msg_body_qrcvr_16_client_s   client;
    GArray                                  *rsrmgrs;
};



/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_24_s
 */
struct lixa_msg_body_qrcvr_24_recovery_s {
    /**
     * Boolean: TRUE -> attempted recovery failed;
     */
    int                                      failed;
    /**
     * Boolean: TRUE -> attempted xa_commit; FALSE -> attempted xa_rollback
     */
    int                                      commit;
};

    

/**
 * Convenience struct for @ref lixa_msg_body_qrcvr_24_s
 */
struct lixa_msg_body_qrcvr_24_rsrmgr_s {
    /**
     * rmid parameter as passed to xa_close routine
     */
    int                                      rmid;
    /**
     * xa_rollback / xa_commit return code
     */
    int                                      rc;
};

    

/**
 * Message body for verb "qrcvr", step "24"
 */
struct lixa_msg_body_qrcvr_24_s {
    struct lixa_msg_body_qrcvr_24_recovery_s recovery;
    GArray                                  *rsrmgrs;
};



/**
 * Convenience struct for @ref lixa_msg_body_reg_8_s
 */
struct lixa_msg_body_reg_8_ax_reg_exec_s {
    /**
     * rmid parameter as passed to xa_rollback routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_rollback routine
     */
    long            flags;
    /**
     * return code of ax_reg routine
     */
    int             rc;
    /**
     * the new transaction branch association state after ax_reg execution
     */
    int             td_state;
    /**
     * the new transaction branch state after ax_reg execution
     */
    int             s_state;
};



/**
 * Message body for verb "reg", step "8"
 */
struct lixa_msg_body_reg_8_s {
    /**
     * Parameters and return value of ax_reg executions
     */
    struct lixa_msg_body_reg_8_ax_reg_exec_s   ax_reg_exec;
};



/**
 * Convenience struct for @ref lixa_msg_body_unreg_8_s
 */
struct lixa_msg_body_unreg_8_ax_unreg_exec_s {
    /**
     * rmid parameter as passed to xa_rollback routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_rollback routine
     */
    long            flags;
    /**
     * return code of ax_reg routine
     */
    int             rc;
    /**
     * the new transaction branch association state after ax_reg execution
     */
    int             td_state;
};



/**
 * Message body for verb "unreg", step "8"
 */
struct lixa_msg_body_unreg_8_s {
    /**
     * Parameters and return value of ax_unreg executions
     */
    struct lixa_msg_body_unreg_8_ax_unreg_exec_s   ax_unreg_exec;
};



/**
 * Control thread status
 */
struct lixa_msg_body_forget_8_conthr_s {
    /**
     * TRUE = yes
     * FALSE = no
     */
    int   finished;
};



/**
 * Convenience struct for @ref lixa_msg_body_forget_8_s
 * xid is not stored in this structure because it was already stored by the
 * server after receiving step 8 message, see @ref lixa_msg_body_forget_8_s
 */
struct lixa_msg_body_forget_8_xa_forget_execs_s {
    /**
     * rmid parameter as passed to xa_forget routine
     */
    int             rmid;
    /**
     * flags parameter as passed to xa_forget routine
     */
    long            flags;
    /**
     * return code of xa_forget routine
     */
    int             rc;
    /**
     * the new transaction branch state after xa_forget execution
     */
    int             s_state;
};



/**
 * Message body for verb "forget", step "8"
 */
struct lixa_msg_body_forget_8_s {
    /**
     * Control thread information
     */
    struct lixa_msg_body_forget_8_conthr_s    conthr;
    /**
     * Parameters and return value of xa_forget executions
     */
    GArray                                   *xa_forget_execs;
};



/**
 * This structure maps the messages flowing between LIXA client (lixac) and
 * LIXA server (lixad). The struct is not used for the transmission over the
 * network, but only inside the client and the server.
 * This is a "fake" object; it's defined and used in the hope of simplicity
 */
struct lixa_msg_s {
    /**
     * Message header, common to all messages
     */
    struct lixa_msg_header_s                   header;
    /**
     * Message body, it depends from header
     */
    union {
        struct lixa_msg_body_open_8_s          open_8;
        struct lixa_msg_body_open_16_s         open_16;
        struct lixa_msg_body_open_24_s         open_24;
        struct lixa_msg_body_close_8_s         close_8;
        struct lixa_msg_body_start_8_s         start_8;
        struct lixa_msg_body_start_16_s        start_16;
        struct lixa_msg_body_start_24_s        start_24;
        struct lixa_msg_body_end_8_s           end_8;
        struct lixa_msg_body_end_16_s          end_16;
        struct lixa_msg_body_prepare_8_s       prepare_8;
        struct lixa_msg_body_prepare_16_s      prepare_16;
        struct lixa_msg_body_commit_8_s        commit_8;
        struct lixa_msg_body_rollback_8_s      rollback_8;
        struct lixa_msg_body_qrcvr_8_s         qrcvr_8;
        struct lixa_msg_body_qrcvr_16_s        qrcvr_16;
        struct lixa_msg_body_qrcvr_24_s        qrcvr_24;
        struct lixa_msg_body_reg_8_s           reg_8;
        struct lixa_msg_body_unreg_8_s         unreg_8;
        struct lixa_msg_body_forget_8_s        forget_8;
    } body;
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Initialize an empty message
     * @param msg IN/OUT message must be initialized
     */
    static inline void lixa_msg_init(struct lixa_msg_s *msg) {
        memset(msg, 0, sizeof(struct lixa_msg_s));
    }
    


    /**
     * Retrieve the first XML message from a TCP/IP socket (file descriptor)
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param buf OUT buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @param read_bytes OUT number of bytes read, XML message length
     * @return a reason code
     */
    int lixa_msg_retrieve(int fd,
                          char *buf, size_t buf_size,
                          ssize_t *read_bytes);



    /**
     * Send a message to a TCP/IP socket (file descriptor)
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param buf IN buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @return a reason code
     */
    int lixa_msg_send(int fd, const char *buf, size_t buf_size);

    
    
    /**
     * Free all the dynamically allocated strings previously allocated by
     * @ref lixa_msg_deserialize using xmlGetProp method
     * @param msg IN/OUT the message must be massaged
     * @return a reason code
     */
    int lixa_msg_free(struct lixa_msg_s *msg);


    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of LIXA_TRACE_MODULE */
#ifdef LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE_SAVE */



#endif /* LIXA_XML_MSG_H */
