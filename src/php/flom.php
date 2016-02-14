<?php

/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.40
 * 
 * This file is not intended to be easily readable and contains a number of 
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG 
 * interface file instead. 
 * ----------------------------------------------------------------------------- */

// Try to load our extension if it's not already loaded.
if (!extension_loaded('flom')) {
  if (strtolower(substr(PHP_OS, 0, 3)) === 'win') {
    if (!dl('php_flom.dll')) return;
  } else {
    // PHP_SHLIB_SUFFIX gives 'dylib' on MacOS X but modules are 'so'.
    if (PHP_SHLIB_SUFFIX === 'dylib') {
      if (!dl('flom.so')) return;
    } else {
      if (!dl('flom.'.PHP_SHLIB_SUFFIX)) return;
    }
  }
}



abstract class flom {
	const FLOM_ES_REQUESTER_CANT_WAIT = FLOM_ES_REQUESTER_CANT_WAIT;

	const FLOM_ES_UNABLE_TO_EXECUTE_COMMAND = FLOM_ES_UNABLE_TO_EXECUTE_COMMAND;

	const FLOM_ES_RESOURCE_BUSY = FLOM_ES_RESOURCE_BUSY;

	const FLOM_ES_GENERIC_ERROR = FLOM_ES_GENERIC_ERROR;

	const FLOM_ES_OK = FLOM_ES_OK;

	const FLOM_RC_API_IMMUTABLE_HANDLE = FLOM_RC_API_IMMUTABLE_HANDLE;

	const FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE = FLOM_RC_ELEMENT_NAME_NOT_AVAILABLE;

	const FLOM_RC_NETWORK_TIMEOUT = FLOM_RC_NETWORK_TIMEOUT;

	const FLOM_RC_CONNECTION_CLOSED = FLOM_RC_CONNECTION_CLOSED;

	const FLOM_RC_CONNECTION_REFUSED = FLOM_RC_CONNECTION_REFUSED;

	const FLOM_RC_LOCK_IMPOSSIBLE = FLOM_RC_LOCK_IMPOSSIBLE;

	const FLOM_RC_LOCK_CANT_WAIT = FLOM_RC_LOCK_CANT_WAIT;

	const FLOM_RC_LOCK_BUSY = FLOM_RC_LOCK_BUSY;

	const FLOM_RC_LOCK_CANT_LOCK = FLOM_RC_LOCK_CANT_LOCK;

	const FLOM_RC_LOCK_WAIT_RESOURCE = FLOM_RC_LOCK_WAIT_RESOURCE;

	const FLOM_RC_LOCK_ENQUEUED = FLOM_RC_LOCK_ENQUEUED;

	const FLOM_RC_OK = FLOM_RC_OK;

	const FLOM_RC_INTERNAL_ERROR = FLOM_RC_INTERNAL_ERROR;

	const FLOM_RC_DAEMON_NOT_STARTED = FLOM_RC_DAEMON_NOT_STARTED;

	const FLOM_RC_NETWORK_EVENT_ERROR = FLOM_RC_NETWORK_EVENT_ERROR;

	const FLOM_RC_NULL_OBJECT = FLOM_RC_NULL_OBJECT;

	const FLOM_RC_INVALID_OPTION = FLOM_RC_INVALID_OPTION;

	const FLOM_RC_OBJ_CORRUPTED = FLOM_RC_OBJ_CORRUPTED;

	const FLOM_RC_OUT_OF_RANGE = FLOM_RC_OUT_OF_RANGE;

	const FLOM_RC_INVALID_PREFIX_SIZE = FLOM_RC_INVALID_PREFIX_SIZE;

	const FLOM_RC_BUFFER_OVERFLOW = FLOM_RC_BUFFER_OVERFLOW;

	const FLOM_RC_INVALID_MSG_LENGTH = FLOM_RC_INVALID_MSG_LENGTH;

	const FLOM_RC_INVALID_PROPERTY_VALUE = FLOM_RC_INVALID_PROPERTY_VALUE;

	const FLOM_RC_CONTAINER_FULL = FLOM_RC_CONTAINER_FULL;

	const FLOM_RC_PROTOCOL_ERROR = FLOM_RC_PROTOCOL_ERROR;

	const FLOM_RC_INVALID_RESOURCE_NAME = FLOM_RC_INVALID_RESOURCE_NAME;

	const FLOM_RC_PROTOCOL_LEVEL_MISMATCH = FLOM_RC_PROTOCOL_LEVEL_MISMATCH;

	const FLOM_RC_MSG_DESERIALIZE_ERROR = FLOM_RC_MSG_DESERIALIZE_ERROR;

	const FLOM_RC_API_INVALID_SEQUENCE = FLOM_RC_API_INVALID_SEQUENCE;

	const FLOM_RC_INVALID_AI_FAMILY_ERROR = FLOM_RC_INVALID_AI_FAMILY_ERROR;

	const FLOM_RC_INVALID_IP_ADDRESS = FLOM_RC_INVALID_IP_ADDRESS;

	const FLOM_RC_INVALID_IPV6_NETWORK_INTERFACE = FLOM_RC_INVALID_IPV6_NETWORK_INTERFACE;

	const FLOM_RC_NEW_OBJ = FLOM_RC_NEW_OBJ;

	const FLOM_RC_NO_CERTIFICATE = FLOM_RC_NO_CERTIFICATE;

	const FLOM_RC_ACCEPT_ERROR = FLOM_RC_ACCEPT_ERROR;

	const FLOM_RC_BIND_ERROR = FLOM_RC_BIND_ERROR;

	const FLOM_RC_CHDIR_ERROR = FLOM_RC_CHDIR_ERROR;

	const FLOM_RC_CLOSE_ERROR = FLOM_RC_CLOSE_ERROR;

	const FLOM_RC_CONNECT_ERROR = FLOM_RC_CONNECT_ERROR;

	const FLOM_RC_EXECVP_ERROR = FLOM_RC_EXECVP_ERROR;

	const FLOM_RC_FCNTL_ERROR = FLOM_RC_FCNTL_ERROR;

	const FLOM_RC_FORK_ERROR = FLOM_RC_FORK_ERROR;

	const FLOM_RC_GETADDRINFO_ERROR = FLOM_RC_GETADDRINFO_ERROR;

	const FLOM_RC_GETIFADDRS_ERROR = FLOM_RC_GETIFADDRS_ERROR;

	const FLOM_RC_GETNAMEINFO_ERROR = FLOM_RC_GETNAMEINFO_ERROR;

	const FLOM_RC_GETSOCKNAME_ERROR = FLOM_RC_GETSOCKNAME_ERROR;

	const FLOM_RC_GETSOCKOPT_ERROR = FLOM_RC_GETSOCKOPT_ERROR;

	const FLOM_RC_INET_NTOP_ERROR = FLOM_RC_INET_NTOP_ERROR;

	const FLOM_RC_LISTEN_ERROR = FLOM_RC_LISTEN_ERROR;

	const FLOM_RC_MALLOC_ERROR = FLOM_RC_MALLOC_ERROR;

	const FLOM_RC_PIPE_ERROR = FLOM_RC_PIPE_ERROR;

	const FLOM_RC_POLL_ERROR = FLOM_RC_POLL_ERROR;

	const FLOM_RC_READ_ERROR = FLOM_RC_READ_ERROR;

	const FLOM_RC_RECV_ERROR = FLOM_RC_RECV_ERROR;

	const FLOM_RC_RECVFROM_ERROR = FLOM_RC_RECVFROM_ERROR;

	const FLOM_RC_REGCOMP_ERROR = FLOM_RC_REGCOMP_ERROR;

	const FLOM_RC_REGEXEC_ERROR = FLOM_RC_REGEXEC_ERROR;

	const FLOM_RC_SEND_ERROR = FLOM_RC_SEND_ERROR;

	const FLOM_RC_SENDTO_ERROR = FLOM_RC_SENDTO_ERROR;

	const FLOM_RC_SETSID_ERROR = FLOM_RC_SETSID_ERROR;

	const FLOM_RC_SETSOCKOPT_ERROR = FLOM_RC_SETSOCKOPT_ERROR;

	const FLOM_RC_SIGACTION_ERROR = FLOM_RC_SIGACTION_ERROR;

	const FLOM_RC_SIGNAL_ERROR = FLOM_RC_SIGNAL_ERROR;

	const FLOM_RC_SOCKET_ERROR = FLOM_RC_SOCKET_ERROR;

	const FLOM_RC_SNPRINTF_ERROR = FLOM_RC_SNPRINTF_ERROR;

	const FLOM_RC_UNLINK_ERROR = FLOM_RC_UNLINK_ERROR;

	const FLOM_RC_WAIT_ERROR = FLOM_RC_WAIT_ERROR;

	const FLOM_RC_WRITE_ERROR = FLOM_RC_WRITE_ERROR;

	const FLOM_RC_G_ARRAY_NEW_ERROR = FLOM_RC_G_ARRAY_NEW_ERROR;

	const FLOM_RC_G_BASE64_DECODE_ERROR = FLOM_RC_G_BASE64_DECODE_ERROR;

	const FLOM_RC_G_BASE64_ENCODE_ERROR = FLOM_RC_G_BASE64_ENCODE_ERROR;

	const FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR = FLOM_RC_G_KEY_FILE_LOAD_FROM_FILE_ERROR;

	const FLOM_RC_G_KEY_FILE_NEW_ERROR = FLOM_RC_G_KEY_FILE_NEW_ERROR;

	const FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR = FLOM_RC_G_MARKUP_PARSE_CONTEXT_NEW_ERROR;

	const FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR = FLOM_RC_G_MARKUP_PARSE_CONTEXT_PARSE_ERROR;

	const FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR = FLOM_RC_G_PTR_ARRAY_REMOVE_INDEX_FAST_ERROR;

	const FLOM_RC_G_QUEUE_NEW_ERROR = FLOM_RC_G_QUEUE_NEW_ERROR;

	const FLOM_RC_G_STRDUP_ERROR = FLOM_RC_G_STRDUP_ERROR;

	const FLOM_RC_G_STRNDUP_ERROR = FLOM_RC_G_STRNDUP_ERROR;

	const FLOM_RC_G_STRSPLIT_ERROR = FLOM_RC_G_STRSPLIT_ERROR;

	const FLOM_RC_G_THREAD_CREATE_ERROR = FLOM_RC_G_THREAD_CREATE_ERROR;

	const FLOM_RC_G_TRY_MALLOC_ERROR = FLOM_RC_G_TRY_MALLOC_ERROR;

	const FLOM_RC_G_TRY_REALLOC_ERROR = FLOM_RC_G_TRY_REALLOC_ERROR;

	const FLOM_RC_GET_FIELD_ID_ERROR = FLOM_RC_GET_FIELD_ID_ERROR;

	const FLOM_RC_GET_OBJECT_CLASS_ERROR = FLOM_RC_GET_OBJECT_CLASS_ERROR;

	const FLOM_RC_NEW_DIRECT_BYTE_BUFFER_ERROR = FLOM_RC_NEW_DIRECT_BYTE_BUFFER_ERROR;

	const FLOM_RC_SSL_CTX_CHECK_PRIVATE_KEY_ERROR = FLOM_RC_SSL_CTX_CHECK_PRIVATE_KEY_ERROR;

	const FLOM_RC_SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR = FLOM_RC_SSL_CTX_LOAD_VERIFY_LOCATIONS_ERROR;

	const FLOM_RC_SSL_CTX_NEW_ERROR = FLOM_RC_SSL_CTX_NEW_ERROR;

	const FLOM_RC_SSL_CTX_USE_CERTIFICATE_FILE_ERROR = FLOM_RC_SSL_CTX_USE_CERTIFICATE_FILE_ERROR;

	const FLOM_RC_SSL_CTX_USE_PRIVATEKEY_FILE_ERROR = FLOM_RC_SSL_CTX_USE_PRIVATEKEY_FILE_ERROR;

	const FLOM_RC_SSL_ACCEPT_ERROR = FLOM_RC_SSL_ACCEPT_ERROR;

	const FLOM_RC_SSL_CONNECT_ERROR = FLOM_RC_SSL_CONNECT_ERROR;

	const FLOM_RC_SSL_GET_VERIFY_RESULT_ERROR = FLOM_RC_SSL_GET_VERIFY_RESULT_ERROR;

	const FLOM_RC_SSL_NEW_ERROR = FLOM_RC_SSL_NEW_ERROR;

	const FLOM_RC_SSL_READ_ERROR = FLOM_RC_SSL_READ_ERROR;

	const FLOM_RC_SSL_SET_EX_DATA_ERROR = FLOM_RC_SSL_SET_EX_DATA_ERROR;

	const FLOM_RC_SSL_SET_FD_ERROR = FLOM_RC_SSL_SET_FD_ERROR;

	const FLOM_RC_SSL_WRITE_ERROR = FLOM_RC_SSL_WRITE_ERROR;

	const FLOM_RC_TLS_NO_VALID_METHOD = FLOM_RC_TLS_NO_VALID_METHOD;

	static function flom_strerror($ret_cod) {
		return flom_strerror($ret_cod);
	}

	const TRUE = TRUE;

	const FALSE = FALSE;

	const FLOM_LOCK_MODE_NL = 0;

	const FLOM_LOCK_MODE_CR = FLOM_LOCK_MODE_CR;

	const FLOM_LOCK_MODE_CW = FLOM_LOCK_MODE_CW;

	const FLOM_LOCK_MODE_PR = FLOM_LOCK_MODE_PR;

	const FLOM_LOCK_MODE_PW = FLOM_LOCK_MODE_PW;

	const FLOM_LOCK_MODE_EX = FLOM_LOCK_MODE_EX;

	const FLOM_LOCK_MODE_N = FLOM_LOCK_MODE_N;

	const FLOM_LOCK_MODE_INVALID = FLOM_LOCK_MODE_INVALID;

	const FLOM_HANDLE_STATE_INIT = 22;

	const FLOM_HANDLE_STATE_CONNECTED = FLOM_HANDLE_STATE_CONNECTED;

	const FLOM_HANDLE_STATE_LOCKED = FLOM_HANDLE_STATE_LOCKED;

	const FLOM_HANDLE_STATE_DISCONNECTED = FLOM_HANDLE_STATE_DISCONNECTED;

	const FLOM_HANDLE_STATE_CLEANED = FLOM_HANDLE_STATE_CLEANED;

	static function flom_handle_init($handle) {
		return flom_handle_init($handle);
	}

	static function flom_handle_clean($handle) {
		return flom_handle_clean($handle);
	}

	static function flom_handle_new() {
		$r=flom_handle_new();
		if (is_resource($r)) {
			$c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
			if (!class_exists($c)) {
				return new flom_handle_t($r);
			}
			return new $c($r);
		}
		return $r;
	}

	static function flom_handle_delete($handle) {
		flom_handle_delete($handle);
	}

	static function flom_handle_lock($handle) {
		return flom_handle_lock($handle);
	}

	static function flom_handle_unlock($handle) {
		return flom_handle_unlock($handle);
	}

	static function flom_handle_get_locked_element($handle) {
		return flom_handle_get_locked_element($handle);
	}

	static function flom_handle_get_discovery_attempts($handle) {
		return flom_handle_get_discovery_attempts($handle);
	}

	static function flom_handle_set_discovery_attempts($handle,$value) {
		return flom_handle_set_discovery_attempts($handle,$value);
	}

	static function flom_handle_get_discovery_timeout($handle) {
		return flom_handle_get_discovery_timeout($handle);
	}

	static function flom_handle_set_discovery_timeout($handle,$value) {
		return flom_handle_set_discovery_timeout($handle,$value);
	}

	static function flom_handle_get_discovery_ttl($handle) {
		return flom_handle_get_discovery_ttl($handle);
	}

	static function flom_handle_set_discovery_ttl($handle,$value) {
		return flom_handle_set_discovery_ttl($handle,$value);
	}

	static function flom_handle_get_lock_mode($handle) {
		return flom_handle_get_lock_mode($handle);
	}

	static function flom_handle_set_lock_mode($handle,$value) {
		return flom_handle_set_lock_mode($handle,$value);
	}

	static function flom_handle_get_multicast_address($handle) {
		return flom_handle_get_multicast_address($handle);
	}

	static function flom_handle_set_multicast_address($handle,$value) {
		return flom_handle_set_multicast_address($handle,$value);
	}

	static function flom_handle_get_multicast_port($handle) {
		return flom_handle_get_multicast_port($handle);
	}

	static function flom_handle_set_multicast_port($handle,$value) {
		return flom_handle_set_multicast_port($handle,$value);
	}

	static function flom_handle_get_network_interface($handle) {
		return flom_handle_get_network_interface($handle);
	}

	static function flom_handle_set_network_interface($handle,$value) {
		return flom_handle_set_network_interface($handle,$value);
	}

	static function flom_handle_get_resource_create($handle) {
		return flom_handle_get_resource_create($handle);
	}

	static function flom_handle_set_resource_create($handle,$value) {
		return flom_handle_set_resource_create($handle,$value);
	}

	static function flom_handle_get_resource_idle_lifespan($handle) {
		return flom_handle_get_resource_idle_lifespan($handle);
	}

	static function flom_handle_set_resource_idle_lifespan($handle,$value) {
		return flom_handle_set_resource_idle_lifespan($handle,$value);
	}

	static function flom_handle_get_resource_name($handle) {
		return flom_handle_get_resource_name($handle);
	}

	static function flom_handle_set_resource_name($handle,$value) {
		return flom_handle_set_resource_name($handle,$value);
	}

	static function flom_handle_get_resource_quantity($handle) {
		return flom_handle_get_resource_quantity($handle);
	}

	static function flom_handle_set_resource_quantity($handle,$value) {
		return flom_handle_set_resource_quantity($handle,$value);
	}

	static function flom_handle_get_resource_timeout($handle) {
		return flom_handle_get_resource_timeout($handle);
	}

	static function flom_handle_set_resource_timeout($handle,$value) {
		return flom_handle_set_resource_timeout($handle,$value);
	}

	static function flom_handle_get_socket_name($handle) {
		return flom_handle_get_socket_name($handle);
	}

	static function flom_handle_set_socket_name($handle,$value) {
		return flom_handle_set_socket_name($handle,$value);
	}

	static function flom_handle_get_trace_filename($handle) {
		return flom_handle_get_trace_filename($handle);
	}

	static function flom_handle_set_trace_filename($handle,$value) {
		return flom_handle_set_trace_filename($handle,$value);
	}

	static function flom_handle_get_unicast_address($handle) {
		return flom_handle_get_unicast_address($handle);
	}

	static function flom_handle_set_unicast_address($handle,$value) {
		return flom_handle_set_unicast_address($handle,$value);
	}

	static function flom_handle_get_unicast_port($handle) {
		return flom_handle_get_unicast_port($handle);
	}

	static function flom_handle_set_unicast_port($handle,$value) {
		return flom_handle_set_unicast_port($handle,$value);
	}
}

/* PHP Proxy Classes */
class flom_handle_t {
	public $_cPtr=null;
	protected $_pData=array();

	function __set($var,$value) {
		$func = 'flom_handle_t_'.$var.'_set';
		if (function_exists($func)) return call_user_func($func,$this->_cPtr,$value);
		if ($var === 'thisown') return swig_flom_alter_newobject($this->_cPtr,$value);
		$this->_pData[$var] = $value;
	}

	function __isset($var) {
		if (function_exists('flom_handle_t_'.$var.'_set')) return true;
		if ($var === 'thisown') return true;
		return array_key_exists($var, $this->_pData);
	}

	function __get($var) {
		$func = 'flom_handle_t_'.$var.'_get';
		if (function_exists($func)) {
			$r = call_user_func($func,$this->_cPtr);
			if (!is_resource($r)) return $r;
			$c=substr(get_resource_type($r), (strpos(get_resource_type($r), '__') ? strpos(get_resource_type($r), '__') + 2 : 3));
			return new $c($r);
		}
		if ($var === 'thisown') return swig_flom_get_newobject($this->_cPtr);
		return $this->_pData[$var];
	}

	public function __construct($res=null) {
		if (is_resource($res) && get_resource_type($res) === '_p_flom_handle_s') {
			$this->_cPtr=$res;
			return;
		}
		$this->_cPtr=new_flom_handle_t();
	}
}


?>
