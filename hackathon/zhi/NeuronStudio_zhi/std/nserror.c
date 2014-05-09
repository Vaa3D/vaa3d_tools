#include "nserror.h"

/* NOTE: Avoid circular inclusion! */
#include <std/nslog.h>


#ifdef NS_DEBUG

NsError ____ns_error( nsuint code, nsuint level, const nschar *details )
	{
	NsError error = ( code | ( level << _NS_ERROR_LEVEL_SHIFT ) );

	if( code )
		ns_log_entry(
			NS_LOG_ENTRY_ERROR,
			"code=" NS_FMT_STRING_DOUBLE_QUOTED
			" level=" NS_FMT_STRING_DOUBLE_QUOTED
			" details=" NS_FMT_STRING_DOUBLE_QUOTED,
			ns_error_code_to_string( error ),
			ns_error_level_to_string( error ),
			details
			);

	return error;
	}

#else

NsError ____ns_error( nsuint code, nsuint level, const nschar *details )
	{
	NsError error = ( code | ( level << _NS_ERROR_LEVEL_SHIFT ) );
	return error;
	}

#endif/* NS_DEBUG */


nsboolean ns_is_error( NsError error )
	{  return NS_NO_ERROR != ns_error_code( error );  }


const nschar* ns_error_code_to_string( NsError error )
	{
	nsuint code;

	NS_PRIVATE const nschar* _ns_error_code_strings[ NS_ERROR_NUM_PREDEFINED_CODES ] =
		{
		/*NS_NO_ERROR            */"No error.",
		/*NS_ERROR_2BIG          */"Argument list too long.",
		/*NS_ERROR_ACCES         */"Permission denied.",
		/*NS_ERROR_ADDRINUSE     */"Address in use.",
		/*NS_ERROR_ADDRNOTAVAIL  */"Address not available.",
		/*NS_ERROR_AFNOSUPPORT   */"Address family not supported.",
		/*NS_ERROR_AGAIN         */"Resource unavailable, try again.",
		/*NS_ERROR_ALREADY       */"Connection already in progress.",
		/*NS_ERROR_BADF          */"Bad file descriptor.",
		/*NS_ERROR_BADMSG        */"Bad message.",
		/*NS_ERROR_BUSY          */"Device or resource busy.",
		/*NS_ERROR_CANCELED      */"Operation canceled.",
		/*NS_ERROR_CHILD         */"No child processes.",
		/*NS_ERROR_CONNABORTED   */"Connection aborted.",
		/*NS_ERROR_CONNREFUSED   */"Connection refused.",
		/*NS_ERROR_CONNRESET     */"Connection reset.",
		/*NS_ERROR_DEADLK        */"Resource deadlock would occur.",
		/*NS_ERROR_DESTADDRREQ   */"Destination address required.",
		/*NS_ERROR_DOM           */"Mathematics argument out of domain of function.",
		/*NS_ERROR_DQUOT         */"Reserved.",
		/*NS_ERROR_EXIST         */"File exists.",
		/*NS_ERROR_FAULT         */"Bad address.",
		/*NS_ERROR_FBIG          */"File too large.",
		/*NS_ERROR_HOSTUNREACH   */"Host is unreachable.",
		/*NS_ERROR_IDRM          */"Identifier removed.",
		/*NS_ERROR_ILSEQ         */"Illegal byte sequence.",
		/*NS_ERROR_INPROGRESS    */"Operation in progress.",
		/*NS_ERROR_INTR          */"Interrupted function.",
		/*NS_ERROR_INVAL         */"Invalid argument.",
		/*NS_ERROR_IO            */"I/O error.",
		/*NS_ERROR_ISCONN        */"Socket is connected.",
		/*NS_ERROR_ISDIR         */"Is a directory.",
		/*NS_ERROR_LOOP          */"Too many levels of symbolic links.",
		/*NS_ERROR_MFILE         */"Too many open files.",
		/*NS_ERROR_MLINK         */"Too many links.",
		/*NS_ERROR_MSGSIZE       */"Message too large.",
		/*NS_ERROR_MULTIHOP      */"Reserved.",
		/*NS_ERROR_NAMETOOLONG   */"Filename too long.",
		/*NS_ERROR_NETDOWN       */"Network is down.",
		/*NS_ERROR_NETRESET      */"Connection aborted by network.",
		/*NS_ERROR_NETUNREACH    */"Network unreachable.",
		/*NS_ERROR_NFILE         */"Too many files open in system.",
		/*NS_ERROR_NOBUFS        */"No buffer space available.",
		/*NS_ERROR_NODATA        */"No message is available on the STREAM head read queue. ",
		/*NS_ERROR_NODEV         */"No such device.",
		/*NS_ERROR_NOENT         */"No such file or directory.",
		/*NS_ERROR_NOEXEC        */"Executable file format error.",
		/*NS_ERROR_NOLCK         */"No locks available.",
		/*NS_ERROR_NOLINK        */"Reserved.",
		/*NS_ERROR_NOMEM         */"Not enough space.",
		/*NS_ERROR_NOMSG         */"No message of the desired type.",
		/*NS_ERROR_NOPROTOOPT    */"Protocol not available.",
		/*NS_ERROR_NOSPC         */"No space left on device.",
		/*NS_ERROR_NOSR          */"No STREAM resources. ",
		/*NS_ERROR_NOSTR         */"Not a STREAM. ",
		/*NS_ERROR_NOSYS         */"Function not supported.",
		/*NS_ERROR_NOTCONN       */"The socket is not connected.",
		/*NS_ERROR_NOTDIR        */"Not a directory.",
		/*NS_ERROR_NOTEMPTY      */"Directory not empty.",
		/*NS_ERROR_NOTSOCK       */"Not a socket.",
		/*NS_ERROR_NOTSUP        */"Not supported.",
		/*NS_ERROR_NOTTY         */"Inappropriate I/O control operation.",
		/*NS_ERROR_NXIO          */"No such device or address.",
		/*NS_ERROR_OPNOTSUPP     */"Operation not supported on socket.",
		/*NS_ERROR_OVERFLOW      */"Value too large to be stored in data type.",
		/*NS_ERROR_PERM          */"Operation not permitted.",
		/*NS_ERROR_PIPE          */"Broken pipe.",
		/*NS_ERROR_PROTO         */"Protocol error.",
		/*NS_ERROR_PROTONOSUPPORT*/"Protocol not supported.",
		/*NS_ERROR_PROTOTYPE     */"Protocol wrong type for socket.",
		/*NS_ERROR_RANGE         */"Result too large.",
		/*NS_ERROR_ROFS          */"Read-only file system.",
		/*NS_ERROR_SPIPE         */"Invalid seek.",
		/*NS_ERROR_SRCH          */"No such process.",
		/*NS_ERROR_STALE         */"Reserved.",
		/*NS_ERROR_TIME          */"Stream ioctl() timeout. ",
		/*NS_ERROR_TIMEDOUT      */"Connection timed out.",
		/*NS_ERROR_TXTBSY        */"Text file busy.",
		/*NS_ERROR_WOULDBLOCK    */"Operation would block.",
		/*NS_ERROR_XDEV          */"Cross-device link.",
		/*NS_ERROR_OS            */"Operating system.",
		/*NS_ERROR_NOIMP         */"No implementation.",
		/*NS_ERROR_UNKNOWN       */"Unknown."
		};

	code = ns_error_code( error );

	return code < NS_ERROR_NUM_PREDEFINED_CODES ? _ns_error_code_strings[ code ] : "<unknown>";
	}


const nschar* ns_error_level_to_string( NsError error )
	{
	nsuint level;

	NS_PRIVATE const nschar* _ns_error_level_strings[ NS_ERROR_NUM_PREDEFINED_LEVELS ] =
		{
		NULL,
		"caution",
		"recoverable",
		"critical",
		"fatal"
		};

	level = ns_error_level( error );

	return level < NS_ERROR_NUM_PREDEFINED_LEVELS ? _ns_error_level_strings[ level ] : "<unknown>";
	}
