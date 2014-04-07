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
#ifndef MSG_H
# define MSG_H



#include <config.h>



#ifdef HAVE_GLIB_H
# include <glib.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif



/* save old FLOM_TRACE_MODULE and set a new value */
#ifdef FLOM_TRACE_MODULE
# define FLOM_TRACE_MODULE_SAVE FLOM_TRACE_MODULE
# undef FLOM_TRACE_MODULE
#else
# undef FLOM_TRACE_MODULE_SAVE
#endif /* FLOM_TRACE_MODULE */
#define FLOM_TRACE_MODULE      FLOM_TRACE_MOD_MSG



/**
 * Default buffer size for XML messages (used for serialization/
 * deserialization)
 **/
#define FLOM_MSG_BUFFER_SIZE        512



/**
 * Message state enum type
 */
typedef enum flom_msg_state_e {
    /**
     * The message has been initialized, but the values are not ready to be
     * read
     */
    FLOM_MSG_STATE_INITIALIZED,
    /**
     * The parser is filling the message fields, but the values are not ready to
     * be read
     */
    FLOM_MSG_STATE_PARSING,
    /**
     * The parser completed its job and the values are ready to be read
     */
    FLOM_MSG_STATE_READY,
    /**
     * An error occurred and the content of the messafe is INVALID
     */
    FLOM_MSG_STATE_INVALID
} flom_msg_state_t;



/**
 * Current protocol level; it's used to recognize incompatible client/server
 * configuration at run-time
 */
#define FLOM_MSG_LEVEL           0



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
 * Id assigned to verb "discover"
 */
#define FLOM_MSG_VERB_DISCOVER  4

/**
 * Default increment for message step
 */
#define FLOM_MSG_STEP_INCR      8



/**
 * Lock mode that can be asked for a resource
 */
typedef enum flom_lock_mode_e {
    /**
     * Null lock mode
     */
    FLOM_LOCK_MODE_NL,
    /**
     * Concurrent read lock mode
     */
    FLOM_LOCK_MODE_CR,
    /**
     * Concurrent write lock mode
     */
    FLOM_LOCK_MODE_CW,
    /**
     * Protected read / shared lock mode
     */
    FLOM_LOCK_MODE_PR,
    /**
     * Protected write / update lock mode
     */
    FLOM_LOCK_MODE_PW,
    /**
     * Exclusive lock mode
     */
    FLOM_LOCK_MODE_EX,
    /**
     * Number of lock modes
     */
    FLOM_LOCK_MODE_N,
    /**
     * Special value used to encode an invalid value
     */
    FLOM_LOCK_MODE_INVALID
} flom_lock_mode_t;



/**
 * Label used to specify initial XML header
 */
extern const gchar *FLOM_MSG_HEADER;
/**
 * Label used to specify "address" property
 */
extern const gchar *FLOM_MSG_PROP_ADDRESS;
/**
 * Label used to specify "level" property
 */
extern const gchar *FLOM_MSG_PROP_LEVEL;
/**
 * Label used to specify "name" property
 */
extern const gchar *FLOM_MSG_PROP_NAME;
/**
 * Label used to specify "quantity" property
 */
extern const gchar *FLOM_MSG_PROP_QUANTITY;
/**
 * Label used to specify "rc" property
 */
extern const gchar *FLOM_MSG_PROP_RC;
/**
 * Label used to specify "step" property
 */
extern const gchar *FLOM_MSG_PROP_STEP;
/**
 * Label used to specify "mode" property
 */
extern const gchar *FLOM_MSG_PROP_MODE;
/**
 * Label used to specify "port" property
 */
extern const gchar *FLOM_MSG_PROP_PORT;
/**
 * Label used to specify "verb" property
 */
extern const gchar *FLOM_MSG_PROP_VERB;
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
 * Label used to specify "network" tag
 */
extern const gchar *FLOM_MSG_TAG_NETWORK;
/**
 * Label used to specify "resource" tag
 */
extern const gchar *FLOM_MSG_TAG_RESOURCE;



/**
 * A static object used by g_markup functions
 */
extern GMarkupParser flom_msg_parser;



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
 * Convenience struct for @ref flom_msg_body_lock_8_s
 */
struct flom_msg_body_lock_8_resource_s {
    /**
     * name of the resource to lock
     */
    gchar            *name;
    /**
     * asked lock mode
     */
    flom_lock_mode_t  mode;
    /**
     * wait if sufficient resource(s) is(are) not available
     */
    int               wait;
    /**
     * number of resources to lock; for numeric resources only
     */
    gint              quantity;
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



/**
 * Convenience struct for @ref flom_msg_body_unlock_8_s
 */
struct flom_msg_body_unlock_8_resource_s {
    /**
     * name of the resource to lock
     */
    gchar     *name;
};

    

/**
 * Message body for verb "unlock", step "8"
 */
struct flom_msg_body_unlock_8_s {
    struct flom_msg_body_unlock_8_resource_s   resource;
};



/**
 * Message body for verb "ping", step "8"
 */
struct flom_msg_body_ping_8_s {
    /**
     * ping verb does not need to carry anything
     */
    int   dummy_field;
};



/**
 * Message body for verb "ping", step "16"
 */
struct flom_msg_body_ping_16_s {
    /**
     * ping verb does not need to carry anything
     */
    int   dummy_field;
};



/**
 * Message body for verb "discover", step "8"
 */
struct flom_msg_body_discover_8_s {
    /**
     * discover verb does not need to carry anything
     */
    int   dummy_field;
};



/**
 * Convenience struct for @ref flom_msg_body_discover_16_s
 */
struct flom_msg_body_discover_16_network_s {
    /**
     * unicast TCP/IP address used by daemon. It can * NOT * be desumed by
     * packet sender because the daemon might be bound to an interface
     * distinct from the interface used to send UDP datagrams
     */
    gchar          *address;
    /**
     * unicast TCP/IP port used by daemon
     */
    in_port_t       port;
};

    

/**
 * Message body for verb "discover", step "16"
 */
struct flom_msg_body_discover_16_s {
    struct flom_msg_body_discover_16_network_s   network;
};



/**
 * This structure maps the messages flowing between FLOM client and
 * FLOM server (daemon). The struct is not used for the transmission over the
 * network, but only inside the client and the server.
 * This is a "fake" object; it's defined and used in the hope of simplicity
 */
struct flom_msg_s {
    /**
     * Message state
     */
    flom_msg_state_t                          state;
    /**
     * Message header, common to all messages
     */
    struct flom_msg_header_s                  header;
    /**
     * Message body, it depends from header
     */
    union {
        struct flom_msg_body_lock_8_s         lock_8;
        struct flom_msg_body_lock_16_s        lock_16;
        struct flom_msg_body_lock_24_s        lock_24;
        struct flom_msg_body_unlock_8_s       unlock_8;
        struct flom_msg_body_ping_8_s         ping_8;
        struct flom_msg_body_ping_16_s        ping_16;
        struct flom_msg_body_discover_8_s     discover_8;
        struct flom_msg_body_discover_16_s    discover_16;
    } body;
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Interpret a string and extract the lock mode
     * @param text IN string to interpret
     * @return a lock mode
     */
    flom_lock_mode_t flom_lock_mode_retrieve(const gchar *text);
    


    /**
     * Initialize an empty message
     * @param msg IN/OUT message must be initialized
     */
    static inline void flom_msg_init(struct flom_msg_s *msg) {
        memset(msg, 0, sizeof(struct flom_msg_s));
    }
    


    /**
     * Retrieve the first XML message from a TCP/IP socket (file descriptor)
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param type IN file descriptor associated type (SOCK_STREAM or
     *             SOCK_DGRAM)
     * @param buf OUT buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @param read_bytes OUT number of bytes read, XML message length
     * @param timeout IN maximum wait time to receive the answer (milliseconds)
     * @param src_addr OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @param addrlen OUT transparently passed to recvfrom if type is
     *                 SOCK_DGRAM (see recvfrom man page)
     * @return a reason code
     */
    int flom_msg_retrieve(int fd, int type,
                          char *buf, size_t buf_size,
                          ssize_t *read_bytes,
                          int timeout,
                          struct sockaddr *src_addr, socklen_t *addrlen);



    /**
     * Send a message to a TCP/IP socket (file descriptor)
     * @param fd IN file descriptor associated to the TCP/IP socket
     * @param buf IN buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @return a reason code
     */
    int flom_msg_send(int fd, const char *buf, size_t buf_size);

    
    
    /**
     * Free all the dynamically allocated strings previously allocated by
     * @ref flom_msg_deserialize
     * @param msg IN/OUT the message must be massaged
     * @return a reason code
     */
    int flom_msg_free(struct flom_msg_s *msg);



    /**
     * Check if the message is correct from a protocol point of view:
     * client can send only some verb/step combinations, daemon can send
     * only other verb/step combinations
     * @param msg IN message to be inspected
     * @param client IN true if client sent the message, false if daemon sent
     *        the message
     * @return a boolean value
     */
    int flom_msg_check_protocol(const struct flom_msg_s *msg, int client);

    
    
    /**
     * Serialize a message struct to an XML buffer for external transmission
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param buffer_len IN the space allocated for buffer
     * @param msg_len OUT number of chars used in buffer for serializing msg
     * @return a reason code
     */
    int flom_msg_serialize(const struct flom_msg_s *msg,
                           char *buffer, size_t buffer_len,
                           size_t *msg_len);


    
    /**
     * Serialize the "lock_8" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_lock_8(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "lock_16" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_lock_16(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "lock_24" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_lock_24(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "unlock_8" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_unlock_8(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "ping_8" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_ping_8(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "ping_16" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_ping_16(const struct flom_msg_s *msg,
                                   char *buffer,
                                   size_t *offset, size_t *free_chars);



    /**
     * Serialize the "discover_8" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_discover_8(const struct flom_msg_s *msg,
                                      char *buffer,
                                      size_t *offset, size_t *free_chars);


    
    /**
     * Serialize the "discover_16" specific body part of a message
     * @param msg IN the object must be serialized
     * @param buffer OUT the buffer will contain the XML serialized object
     *                   (the size has fixed size of
     *                   @ref FLOM_MSG_BUFFER_SIZE bytes) and will be
     *                   null terminated
     * @param offset IN/OUT offset must be used to start serialization inside
     *                      the buffer
     * @param free_chars IN/OUT remaing free chars inside the buffer
     * @return a reason code
     */
    int flom_msg_serialize_discover_16(const struct flom_msg_s *msg,
                                       char *buffer,
                                       size_t *offset, size_t *free_chars);



    /**
     * Display the content of a message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace(const struct flom_msg_s *msg);

    
    
    /**
     * Display the content of a lock message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace_lock(const struct flom_msg_s *msg);

    
    
    /**
     * Display the content of an unlock message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace_unlock(const struct flom_msg_s *msg);

    
    
    /**
     * Display the content of a ping message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace_ping(const struct flom_msg_s *msg);

    
    
    /**
     * Display the content of a discover message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace_discover(const struct flom_msg_s *msg);

    
    
    /**
     * Deserialize a serialized buffer to a message struct
     * @param buffer IN/OUT the buffer that's containing the serialized object
     *                  (it does not have to be null terminated)
     * @param buffer_len IN number of significative bytes of buffer
     * @param msg OUT the object after deserialization
     * @param gmpc IN/OUT GMarkup parser context
     * @return a reason code
     */
    int flom_msg_deserialize(char *buffer, size_t buffer_len,
                             struct flom_msg_s *msg,
                             GMarkupParseContext *gmpc);



    /**
     * GMarkupParser start_element callback function
     */
    void flom_msg_deserialize_start_element(
        GMarkupParseContext *context,
        const gchar         *element_name,
        const gchar        **attribute_names,
        const gchar        **attribute_values,
        gpointer             user_data,
        GError             **error);

    

    /**
     * GMarkupParser end_element callback function
     */
    void flom_msg_deserialize_end_element(GMarkupParseContext *context,
                                          const gchar         *element_name,
                                          gpointer             user_data,
                                          GError             **error);

    

    /**
     * GMarkupParser text callback function
     */
    void flom_msg_deserialize_text(GMarkupParseContext *context,
                                   const gchar         *text,
                                   gsize                text_len,
                                   gpointer             user_data,
                                   GError             **error);

    

    /**
     * Build a standard answer message
     * @param msg IN/OUT message allocated and initialized
     * @param verb IN answer verb
     * @param step IN answer step
     * @param rc IN answer rc
     * @return a reason code
     */
    int flom_msg_build_answer(struct flom_msg_s *msg,
                              int verb, int step, int rc);

    
                              
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
