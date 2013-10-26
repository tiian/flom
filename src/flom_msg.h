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
#ifdef HAVE_STRING_H
# include <string.h>
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
 * Lock type NL = null lock
 */
#define FLOM_MSG_LOCK_TYPE_NL   0
/**
 * Lock type CR = concurrent read
 */
#define FLOM_MSG_LOCK_TYPE_CR   1
/**
 * Lock type CW = concurrent write
 */
#define FLOM_MSG_LOCK_TYPE_CW   2
/**
 * Lock type CR = protected read (shared lock)
 */
#define FLOM_MSG_LOCK_TYPE_PR   3
/**
 * Lock type CR = protected write (update lock)
 */
#define FLOM_MSG_LOCK_TYPE_PW   4
/**
 * Lock type EX = exclusive lock
 */
#define FLOM_MSG_LOCK_TYPE_EX   5



/**
 * Label used to specify initial XML header
 */
extern const gchar *FLOM_MSG_HEADER;
/**
 * Label used to specify "level" property
 */
extern const gchar *FLOM_MSG_PROP_LEVEL;
/**
 * Label used to specify "name" property
 */
extern const gchar *FLOM_MSG_PROP_NAME;
/**
 * Label used to specify "rcp" property
 */
extern const gchar *FLOM_MSG_PROP_RC;
/**
 * Label used to specify "step" property
 */
extern const gchar *FLOM_MSG_PROP_STEP;
/**
 * Label used to specify "type" property
 */
extern const gchar *FLOM_MSG_PROP_TYPE;
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
 * This structure maps the messages flowing between FLOM client and
 * FLOM server (daemon). The struct is not used for the transmission over the
 * network, but only inside the client and the server.
 * This is a "fake" object; it's defined and used in the hope of simplicity
 */
struct flom_msg_s {
    /**
     * Message header, common to all messages
     */
    struct flom_msg_header_s                   header;
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
    } body;
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



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
     * @param buf OUT buffer will be used to store the XML message
     * @param buf_size IN size of buf
     * @param read_bytes OUT number of bytes read, XML message length
     * @return a reason code
     */
    int flom_msg_retrieve(int fd,
                          char *buf, size_t buf_size,
                          ssize_t *read_bytes);



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
     * @ref flom_msg_deserialize using xmlGetProp method
     * @param msg IN/OUT the message must be massaged
     * @return a reason code
     */
    int flom_msg_free(struct flom_msg_s *msg);



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
