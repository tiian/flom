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



#include "flom_types.h"



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
     * The parser is filling the message fields, but the values are not ready
     * to be read
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
#define FLOM_MSG_LEVEL           3



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
 * Id assigned to verb "management"
 */
#define FLOM_MSG_VERB_MNGMNT    5

/**
 * Default increment for message step
 */
#define FLOM_MSG_STEP_INCR      8



/**
 * Label used to specify initial XML header
 */
extern const gchar *FLOM_MSG_HEADER;
/**
 * Label used to specify "address" property
 */
extern const gchar *FLOM_MSG_PROP_ADDRESS;
/**
 * Label used to specify "create" property
 */
extern const gchar *FLOM_MSG_PROP_CREATE;
/**
 * Label used to specify "element" property
 */
extern const gchar *FLOM_MSG_PROP_ELEMENT;
/**
 * Label used to specify "immediate" property
 */
extern const gchar *FLOM_MSG_PROP_IMMEDIATE;
/**
 * Label used to specify "level" property
 */
extern const gchar *FLOM_MSG_PROP_LEVEL;
/**
 * Label used to specify "lifespan" property
 */
extern const gchar *FLOM_MSG_PROP_LIFESPAN;
/**
 * Label used to specify "mode" property
 */
extern const gchar *FLOM_MSG_PROP_MODE;
/**
 * Label used to specify "name" property
 */
extern const gchar *FLOM_MSG_PROP_NAME;
/**
 * Label used to specify "peerid" property
 */
extern const gchar *FLOM_MSG_PROP_PEERID;
/**
 * Label used to specify "port" property
 */
extern const gchar *FLOM_MSG_PROP_PORT;
/**
 * Label used to specify "quantity" property
 */
extern const gchar *FLOM_MSG_PROP_QUANTITY;
/**
 * Label used to specify "rc" property
 */
extern const gchar *FLOM_MSG_PROP_RC;
/**
 * Label used to specify "rollback" property
 */
extern const gchar *FLOM_MSG_PROP_ROLLBACK;
/**
 * Label used to specify "step" property
 */
extern const gchar *FLOM_MSG_PROP_STEP;
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
 * Label used to specify "session" tag
 */
extern const gchar *FLOM_MSG_TAG_SESSION;
/**
 * Label used to specify "shutdown" tag
 */
extern const gchar *FLOM_MSG_TAG_SHUTDOWN;



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
    int       rc;
    /**
     * Locked element (resource set); optional field, NULL means "no element"
     */
    gchar    *element;
};



/**
 * Convenience struct for @ref flom_msg_body_lock_8_s
 */
struct flom_msg_body_lock_8_session_s {
    /**
     * unique id sent by the connecting peer (client)
     */
    gchar     *peerid;
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
    /**
     * create if the resource does not exist
     */
    int               create;
    /**
     * minimum number of milliseconds a resource must be kept after last
     * usage
     */
    gint              lifespan;
};

    

/**
 * Message body for verb "lock", step "8"
 */
struct flom_msg_body_lock_8_s {
    struct flom_msg_body_lock_8_session_s    session;
    struct flom_msg_body_lock_8_resource_s   resource;
};



/**
 * Convenience struct for @ref flom_msg_body_lock_16_s
 */
struct flom_msg_body_lock_16_session_s {
    /**
     * unique id sent by the listening peer (server)
     */
    gchar     *peerid;
};

    

/**
 * Message body for verb "lock", step "16"
 */
struct flom_msg_body_lock_16_s {
    struct flom_msg_body_lock_16_session_s   session;
    struct flom_msg_body_answer_s            answer;
};



/**
 * Message body for verb "lock", step "24"
 */
struct flom_msg_body_lock_24_s {
    struct flom_msg_body_answer_s            answer;
};



/**
 * Message body for verb "lock", step "32"
 */
struct flom_msg_body_lock_32_s {
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
    /**
     * boolean value: if TRUE, the value of the resource must be rolled back
     */
    int        rollback;
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
 * Convenience struct for @ref flom_msg_body_mngmnt_8_s
 */
struct flom_msg_body_mngmnt_8_session_s {
    /**
     * unique id sent by the connecting peer (client)
     */
    gchar     *peerid;
};

    

/**
 * Convenience struct for @ref flom_msg_body_mngmnt_8_s
 */
struct flom_msg_body_mngmnt_8_shutdown_s {
    int    immediate;
};



/**
 * Action that can be performed by a management message
 */
typedef enum flom_msg_mngmnt_action_e {
    /**
     * Null action
     */
    FLOM_MSG_MNGMNT_ACTION_NULL,
    /**
     * Shutdown
     */
    FLOM_MSG_MNGMNT_ACTION_SHUTDOWN,
    /**
     * Special value used to encode an invalid value
     */
    FLOM_MSG_MNGMNT_ACTION_INVALID
} flom_msg_mngmnt_action_t;



/**
 * Message body for verb "management", step "8"
 */
struct flom_msg_body_mngmnt_8_s {
    struct flom_msg_body_mngmnt_8_session_s    session;
    /**
     * management action
     */
    flom_msg_mngmnt_action_t                       action;
    /**
     * action data, it depends on action field
     */
    union {
        struct flom_msg_body_mngmnt_8_shutdown_s   shutdown;
    } action_data;
};



/**
 * This structure maps the messages flowing between FLoM client and
 * FLoM server (daemon). The struct is not used for the transmission over the
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
        struct flom_msg_body_lock_32_s        lock_32;
        struct flom_msg_body_unlock_8_s       unlock_8;
        struct flom_msg_body_ping_8_s         ping_8;
        struct flom_msg_body_ping_16_s        ping_16;
        struct flom_msg_body_discover_8_s     discover_8;
        struct flom_msg_body_discover_16_s    discover_16;
        struct flom_msg_body_mngmnt_8_s       mngmnt_8;
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
     * Serialize the "lock_32" specific body part of a message
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
    int flom_msg_serialize_lock_32(const struct flom_msg_s *msg,
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
     * Serialize the "mngmnt_8" specific body part of a message
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
    int flom_msg_serialize_mngmnt_8(const struct flom_msg_s *msg,
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
     * Display the content of a management message
     * @param msg IN the message must be massaged
     * @return a reason code
     */
    int flom_msg_trace_mngmnt(const struct flom_msg_s *msg);

    
    
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
     * Deserialize a resource name encoded using base64 and returns the
     * corrisponding null terminated string
     * @param base64 IN resource name encoded using base64 (it's a null
     *                  terminated string)
     * @param resource_name OUT a newly allocated null terminated string
     *                          that contains the plain resource name
     * @return a reason code
     */
    int flom_msg_deserialize_resource_name(const gchar *base64,
                                           gchar **resource_name);

    

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
     * @param element IN answer element (NULL if no element is available)
     * @return a reason code
     */
    int flom_msg_build_answer(struct flom_msg_s *msg,
                              int verb, int step, int rc,
                              const gchar *element);




    /**
     * Retrieve, if available, the ID of the peer that sent the message
     * @param msg IN message struct
     * @return a reference to the peerid value stored inside msg or NULL
     */
    gchar *flom_msg_get_peerid(const struct flom_msg_s *msg);

    
    
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
