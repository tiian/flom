package  org.tiian.flom;
/**
 * This class contains the constants necessary to map the
 * codes returned by the C native functions wrapped by the JNI
 * methods.
 */
public class FlomErrorCodes {
	public final static int FLOM_ES_REQUESTER_CANT_WAIT = 96;
	public final static int FLOM_ES_UNABLE_TO_EXECUTE_COMMAND = 97;
	public final static int FLOM_ES_RESOURCE_BUSY = 98;
	public final static int FLOM_ES_GENERIC_ERROR = 99;
	public final static int FLOM_ES_OK = 0;
	public final static int FLOM_RC_API_IMMUTABLE_HANDLE = +11;
	public final static int FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE = +10;
	public final static int FLOM_RC_NETWORK_TIMEOUT = +9;
	public final static int FLOM_RC_CONNECTION_CLOSED = +8;
	public final static int FLOM_RC_CONNECTION_REFUSED = +7;
	public final static int FLOM_RC_LOCK_IMPOSSIBLE = +6;
	public final static int FLOM_RC_LOCK_CANT_WAIT = +5;
	public final static int FLOM_RC_LOCK_BUSY = +4;
	public final static int FLOM_RC_LOCK_CANT_LOCK = +3;
	public final static int FLOM_RC_LOCK_WAIT_RESOURCE = +2;
	public final static int FLOM_RC_LOCK_ENQUEUED = +1;
	public final static int FLOM_RC_OK = 0;
	public final static int FLOM_RC_INTERNAL_ERROR = -1;
	public final static int FLOM_RC_DAEMON_NOT_STARTED = -2;
	public final static int FLOM_RC_NETWORK_EVENT_ERROR = -3;
	public final static int FLOM_RC_NULL_OBJECT = -4;
	public final static int FLOM_RC_INVALID_OPTION = -5;
	public final static int FLOM_RC_OBJ_CORRUPTED = -6;
	public final static int FLOM_RC_OUT_OF_RANGE = -7;
	public final static int FLOM_RC_INVALID_PREFIX_SIZE = -8;
	public final static int FLOM_RC_BUFFER_OVERFLOW = -9;
	public final static int FLOM_RC_INVALID_MSG_LENGTH = -10;
	public final static int FLOM_RC_INVALID_PROPERTY_VALUE = -11;
	public final static int FLOM_RC_CONTAINER_FULL = -12;
	public final static int FLOM_RC_PROTOCOL_ERROR = -13;
	public final static int FLOM_RC_INVALID_RESOURCE_NAME = -14;
	public final static int FLOM_RC_PROTOCOL_LEVEL_MISMATCH = -15;
	public final static int FLOM_RC_MSG_DESERIALIZE_ERROR = -16;
	public final static int FLOM_RC_API_INVALID_SEQUENCE = -17;
	public final static int FLOM_RC_INVALID_AI_FAMILY_ERROR = -18;
	public final static int FLOM_RC_INVALID_IP_ADDRESS = -19;
	public final static int FLOM_RC_INVALID_IPV6_NETWORK_INTERFACE = -20;
	public final static int FLOM_RC_NEW_OBJ = -21;
	public final static int FLOM_RC_NO_CERTIFICATE = -22;
	public final static int FLOM_RC_UNIQUE_ID_DOES_NOT_MATCH = -23;
	public final static int FLOM_RC_ACCEPT_ERROR = -100;
	public final static int FLOM_RC_BIND_ERROR = -101;
	public final static int FLOM_RC_CHDIR_ERROR = -102;
	public final static int FLOM_RC_CLOSE_ERROR = -103;
	public final static int FLOM_RC_CONNECT_ERROR = -104;
	public final static int FLOM_RC_EXECVP_ERROR = -105;
	public final static int FLOM_RC_FCNTL_ERROR = -106;
	public final static int FLOM_RC_FORK_ERROR = -107;
	public final static int FLOM_RC_GETADDRINFO_ERROR = -108;
	public final static int FLOM_RC_GETIFADDRS_ERROR = -109;
	public final static int FLOM_RC_GETNAMEINFO_ERROR = -110;
	public final static int FLOM_RC_GETSOCKNAME_ERROR = -111;
	public final static int FLOM_RC_GETSOCKOPT_ERROR = -112;
	public final static int FLOM_RC_INET_NTOP_ERROR = -113;
	public final static int FLOM_RC_LISTEN_ERROR = -114;
	public final static int FLOM_RC_MALLOC_ERROR = -115;
	public final static int FLOM_RC_PIPE_ERROR = -116;
	public final static int FLOM_RC_POLL_ERROR = -117;
	public final static int FLOM_RC_READ_ERROR = -118;
	public final static int FLOM_RC_RECV_ERROR = -119;
	public final static int FLOM_RC_RECVFROM_ERROR = -120;
	public final static int FLOM_RC_REGCOMP_ERROR = -121;
	public final static int FLOM_RC_REGEXEC_ERROR = -122;
	public final static int FLOM_RC_SEND_ERROR = -123;
	public final static int FLOM_RC_SENDTO_ERROR = -124;
	public final static int FLOM_RC_SETSID_ERROR = -125;
	public final static int FLOM_RC_SETSOCKOPT_ERROR = -126;
	public final static int FLOM_RC_SIGACTION_ERROR = -127;
	public final static int FLOM_RC_SIGNAL_ERROR = -128;
	public final static int FLOM_RC_SOCKET_ERROR = -129;
	public final static int FLOM_RC_SNPRINTF_ERROR = -130;
	public final static int FLOM_RC_UNLINK_ERROR = -131;
	public final static int FLOM_RC_WAIT_ERROR = -132;
	public final static int FLOM_RC_WRITE_ERROR = -133;
	public final static int FLOM_RC_G_ARRAY_NEW_ERROR = -200;
	public final static int FLOM_RC_G_BASE64_DECODE_ERROR = -201;
	public final static int FLOM_RC_G_BASE64_ENCODE_ERROR = -202;
	public final static int FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR = -203;
	public final static int FLOM_RC_G_KEY_FILE_NEW_ERROR = -204;
	public final static int FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR = -205;
	public final static int FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR = -206;
	public final static int FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR = -207;
	public final static int FLOM_RC_G_QUEUE_NEW_ERROR = -208;
	public final static int FLOM_RC_G_STRDUP_ERROR = -209;
	public final static int FLOM_RC_G_STRNDUP_ERROR = -210;
	public final static int FLOM_RC_G_STRSPLIT_ERROR = -211;
	public final static int FLOM_RC_G_THREAD_CREATE_ERROR = -212;
	public final static int FLOM_RC_G_TRY_MALLOC_ERROR = -213;
	public final static int FLOM_RC_G_TRY_REALLOC_ERROR = -214;
	public final static int FLOM_RC_GET_FIELD_ID_ERROR = -300;
	public final static int FLOM_RC_GET_OBJECT_CLASS_ERROR = -301;
	public final static int FLOM_RC_NEW_DIRECT_BYTE_BUFFER_ERROR = -302;
	public final static int FLOM_RC_SSL_CTX_CHECK_PRIVATE_KEY_ERROR = -400;
	public final static int FLOM_RC_SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR = -401;
	public final static int FLOM_RC_SSL_CTX_NEW_ERROR = -402;
	public final static int FLOM_RC_SSL_CTX_USE_CERTIFICATE_FILE_ERROR = -403;
	public final static int FLOM_RC_SSL_CTX_USE_PRIVATEKEY_FILE_ERROR = -404;
	public final static int FLOM_RC_SSL_ACCEPT_ERROR = -405;
	public final static int FLOM_RC_SSL_CONNECT_ERROR = -406;
	public final static int FLOM_RC_SSL_GET_VERIFY_RESULT_ERROR = -407;
	public final static int FLOM_RC_SSL_NEW_ERROR = -408;
	public final static int FLOM_RC_SSL_READ_ERROR = -409;
	public final static int FLOM_RC_SSL_SET_EX_DATA_ERROR = -410;
	public final static int FLOM_RC_SSL_SET_FD_ERROR = -411;
	public final static int FLOM_RC_SSL_WRITE_ERROR = -412;
	public final static int FLOM_RC_TLS_NO_VALID_METHOD = -413;
	/**
	 * Retrieve the text associated to a FLoM code
	 * @param code is the code returned by the C native functions
	 * @return a string with a human readable description
	 */
	public native static String getText(int code);
}
