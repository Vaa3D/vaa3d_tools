#ifndef __NS_STD_ERROR_H__
#define __NS_STD_ERROR_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>

NS_DECLS_BEGIN

typedef nsuint NsError;


/* Standard UNIX error names defined in <errno.h>
	See also http://www.unix.org/  */
#define NS_ERROR_2BIG              1/* Argument list too long. */
#define NS_ERROR_ACCES             2/* Permission denied. */
#define NS_ERROR_ADDRINUSE         3/* Address in use. */
#define NS_ERROR_ADDRNOTAVAIL      4/* Address not available. */
#define NS_ERROR_AFNOSUPPORT       5/* Address family not supported. */
#define NS_ERROR_AGAIN             6/* Resource unavailable, try again. */
#define NS_ERROR_ALREADY           7/* Connection already in progress. */
#define NS_ERROR_BADF              8/* Bad file descriptor. */
#define NS_ERROR_BADMSG            9/* Bad message. */
#define NS_ERROR_BUSY             10/* Device or resource busy. */
#define NS_ERROR_CANCELED         11/* Operation canceled. */
#define NS_ERROR_CHILD            12/* No child processes. */
#define NS_ERROR_CONNABORTED      13/* Connection aborted. */
#define NS_ERROR_CONNREFUSED      14/* Connection refused. */
#define NS_ERROR_CONNRESET        15/* Connection reset. */
#define NS_ERROR_DEADLK           16/* Resource deadlock would occur. */
#define NS_ERROR_DESTADDRREQ      17/* Destination address required. */
#define NS_ERROR_DOM              18/* Mathematics argument out of domain of function. */
#define NS_ERROR_DQUOT            19/* Reserved. */
#define NS_ERROR_EXIST            20/* File exists. */
#define NS_ERROR_FAULT            21/* Bad address. */
#define NS_ERROR_FBIG             22/* File too large. */
#define NS_ERROR_HOSTUNREACH      23/* Host is unreachable. */
#define NS_ERROR_IDRM             24/* Identifier removed. */
#define NS_ERROR_ILSEQ            25/* Illegal byte sequence. */
#define NS_ERROR_INPROGRESS       26/* Operation in progress. */
#define NS_ERROR_INTR             27/* Interrupted function. */
#define NS_ERROR_INVAL            28/* Invalid argument. */
#define NS_ERROR_IO               29/* I/O error. */
#define NS_ERROR_ISCONN           30/* Socket is connected. */
#define NS_ERROR_ISDIR            31/* Is a directory. */
#define NS_ERROR_LOOP             32/* Too many levels of symbolic links. */
#define NS_ERROR_MFILE            33/* Too many open files. */
#define NS_ERROR_MLINK            34/* Too many links. */
#define NS_ERROR_MSGSIZE          35/* Message too large. */
#define NS_ERROR_MULTIHOP         36/* Reserved. */
#define NS_ERROR_NAMETOOLONG      37/* Filename too long. */
#define NS_ERROR_NETDOWN          38/* Network is down. */
#define NS_ERROR_NETRESET         39/* Connection aborted by network. */
#define NS_ERROR_NETUNREACH       40/* Network unreachable. */
#define NS_ERROR_NFILE            41/* Too many files open in system. */
#define NS_ERROR_NOBUFS           42/* No buffer space available. */
#define NS_ERROR_NODATA           43/* No message is available on the STREAM head read queue.  */
#define NS_ERROR_NODEV            44/* No such device. */
#define NS_ERROR_NOENT            45/* No such file or directory. */
#define NS_ERROR_NOEXEC           46/* Executable file format error. */
#define NS_ERROR_NOLCK            47/* No locks available. */
#define NS_ERROR_NOLINK           48/* Reserved. */
#define NS_ERROR_NOMEM            49/* Not enough space. */
#define NS_ERROR_NOMSG            50/* No message of the desired type. */
#define NS_ERROR_NOPROTOOPT       51/* Protocol not available. */
#define NS_ERROR_NOSPC            52/* No space left on device. */
#define NS_ERROR_NOSR             53/* No STREAM resources.  */
#define NS_ERROR_NOSTR            54/* Not a STREAM.  */
#define NS_ERROR_NOSYS            55/* Function not supported. */
#define NS_ERROR_NOTCONN          56/* The socket is not connected. */
#define NS_ERROR_NOTDIR           57/* Not a directory. */
#define NS_ERROR_NOTEMPTY         58/* Directory not empty. */
#define NS_ERROR_NOTSOCK          59/* Not a socket. */
#define NS_ERROR_NOTSUP           60/* Not supported. */
#define NS_ERROR_NOTTY            61/* Inappropriate I/O control operation. */
#define NS_ERROR_NXIO             62/* No such device or address. */
#define NS_ERROR_OPNOTSUPP        63/* Operation not supported on socket. */
#define NS_ERROR_OVERFLOW         64/* Value too large to be stored in data type. */
#define NS_ERROR_PERM             65/* Operation not permitted. */
#define NS_ERROR_PIPE             66/* Broken pipe. */
#define NS_ERROR_PROTO            67/* Protocol error. */
#define NS_ERROR_PROTONOSUPPORT   68/* Protocol not supported. */
#define NS_ERROR_PROTOTYPE        69/* Protocol wrong type for socket. */
#define NS_ERROR_RANGE            70/* Result too large. */
#define NS_ERROR_ROFS             71/* Read-only file system. */
#define NS_ERROR_SPIPE            72/* Invalid seek. */
#define NS_ERROR_SRCH             73/* No such process. */
#define NS_ERROR_STALE            74/* Reserved. */
#define NS_ERROR_TIME             75/* Stream ioctl() timeout.  */
#define NS_ERROR_TIMEDOUT         76/* Connection timed out. */
#define NS_ERROR_TXTBSY           77/* Text file busy. */
#define NS_ERROR_WOULDBLOCK       78/* Operation would block. */
#define NS_ERROR_XDEV             79/* Cross-device link. */

/* Additional error codes, not standard! */
#define NS_NO_ERROR                0
#define NS_ERROR_OS               80/* Operating system. */
#define NS_ERROR_NOIMP            81/* No implementation. */
#define NS_ERROR_UNKNOWN          82/* Unknown. */

#define NS_ERROR_NUM_PREDEFINED_CODES  83


/* NOTE: Level 0 not used. */
#define NS_ERROR_LEVEL_CAUTION      1
#define NS_ERROR_LEVEL_RECOVERABLE  2
#define NS_ERROR_LEVEL_CRITICAL     3
#define NS_ERROR_LEVEL_FATAL        4

#define NS_ERROR_NUM_PREDEFINED_LEVELS  5


/* NOTE: User can define error codes NS_ERROR_NUM_PREDEFINED_CODES + N
	and error levels NS_ERROR_NUM_PREDEFINED_LEVELS + N */


#define _NS_ERROR_LEVEL_SHIFT     16
#define NS_ERROR_CODE_MAX      65535


/* Makes the error object. */
#ifdef NS_DEBUG

NS_IMPEXP NsError ____ns_error( nsuint code, nsuint level, const nschar *details );

#define ns_error( code, level )\
	____ns_error( (code), (level), NS_MODULE )
	
#else

NS_IMPEXP NsError ____ns_error( nsuint code, nsuint level );

#define ns_error( code, level )\
	____ns_error( (code), (level) )

#endif/* NS_DEBUG */


#define ns_error_code( error )\
	( (error) & NS_ERROR_CODE_MAX )

#define ns_error_level( error )\
	( (error) >> _NS_ERROR_LEVEL_SHIFT )


NS_IMPEXP nsboolean ns_is_error( NsError error );


NS_IMPEXP const nschar* ns_error_code_to_string( NsError error );
NS_IMPEXP const nschar* ns_error_level_to_string( NsError error );


#define ns_no_error()  0

/* NOTE: Including 'user_data' for future enhancement.
	Specifically allow additional data to be passed with
	the error. */

#define ns_error_2big( level, user_data )\
	ns_error( NS_ERROR_2BIG, (level) )

#define ns_error_acces( level, user_data )\
	ns_error( NS_ERROR_ACCES, (level) )

#define ns_error_addrinuse( level, user_data )\
	ns_error( NS_ERROR_ADDRINUSE, (level) )

#define ns_error_addrnotavail( level, user_data )\
	ns_error( NS_ERROR_ADDRNOTAVAIL, (level) )

#define ns_error_afnosupport( level, user_data )\
	ns_error( NS_ERROR_AFNOSUPPORT, (level) )

#define ns_error_again( level, user_data )\
	ns_error( NS_ERROR_AGAIN, (level) )

#define ns_error_already( level, user_data )\
	ns_error( NS_ERROR_ALREADY, (level) )

#define ns_error_badf( level, user_data )\
	ns_error( NS_ERROR_BADF, (level) )

#define ns_error_badmsg( level, user_data )\
	ns_error( NS_ERROR_BADMSG, (level) )

#define ns_error_busy( level, user_data )\
	ns_error( NS_ERROR_BUSY, (level) )

#define ns_error_canceled( level, user_data )\
	ns_error( NS_ERROR_CANCELED, (level) )

#define ns_error_child( level, user_data )\
	ns_error( NS_ERROR_CHILD, (level) )

#define ns_error_connaborted( level, user_data )\
	ns_error( NS_ERROR_CONNABORTED, (level) )

#define ns_error_connrefused( level, user_data )\
	ns_error( NS_ERROR_CONNREFUSED, (level) )

#define ns_error_connreset( level, user_data )\
	ns_error( NS_ERROR_CONNRESET, (level) )

#define ns_error_deadlk( level, user_data )\
	ns_error( NS_ERROR_DEADLK, (level) )

#define ns_error_destaddrreq( level, user_data )\
	ns_error( NS_ERROR_DESTADDRREQ, (level) )

#define ns_error_dom( level, user_data )\
	ns_error( NS_ERROR_DOM, (level) )

#define ns_error_dquot( level, user_data )\
	ns_error( NS_ERROR_DQUOT, (level) )

#define ns_error_exist( level, user_data )\
	ns_error( NS_ERROR_EXIST, (level) )

#define ns_error_fault( level, user_data )\
	ns_error( NS_ERROR_FAULT, (level) )

#define ns_error_fbig( level, user_data )\
	ns_error( NS_ERROR_FBIG, (level) )

#define ns_error_hostunreach( level, user_data )\
	ns_error( NS_ERROR_HOSTUNREACH, (level) )

#define ns_error_idrm( level, user_data )\
	ns_error( NS_ERROR_IDRM, (level) )

#define ns_error_ilseq( level, user_data )\
	ns_error( NS_ERROR_ILSEQ, (level) )

#define ns_error_inprogress( level, user_data )\
	ns_error( NS_ERROR_INPROGRESS, (level) )

#define ns_error_intr( level, user_data )\
	ns_error( NS_ERROR_INTR, (level) )

#define ns_error_inval( level, user_data )\
	ns_error( NS_ERROR_INVAL, (level) )

#define ns_error_io( level, user_data )\
	ns_error( NS_ERROR_IO, (level) )

#define ns_error_isconn( level, user_data )\
	ns_error( NS_ERROR_ISCONN, (level) )

#define ns_error_isdir( level, user_data )\
	ns_error( NS_ERROR_ISDIR, (level) )

#define ns_error_loop( level, user_data )\
	ns_error( NS_ERROR_LOOP, (level) )

#define ns_error_mfile( level, user_data )\
	ns_error( NS_ERROR_MFILE, (level) )

#define ns_error_mlink( level, user_data )\
	ns_error( NS_ERROR_MLINK, (level) )

#define ns_error_msgsize( level, user_data )\
	ns_error( NS_ERROR_MSGSIZE, (level) )

#define ns_error_multihop( level, user_data )\
	ns_error( NS_ERROR_MULTIHOP, (level) )

#define ns_error_nametoolong( level, user_data )\
	ns_error( NS_ERROR_NAMETOOLONG, (level) )

#define ns_error_netdown( level, user_data )\
	ns_error( NS_ERROR_NETDOWN, (level) )

#define ns_error_netreset( level, user_data )\
	ns_error( NS_ERROR_NETRESET, (level) )

#define ns_error_netunreach( level, user_data )\
	ns_error( NS_ERROR_NETUNREACH, (level) )

#define ns_error_nfile( level, user_data )\
	ns_error( NS_ERROR_NFILE, (level) )

#define ns_error_nobufs( level, user_data )\
	ns_error( NS_ERROR_NOBUFS, (level) )

#define ns_error_nodata( level, user_data )\
	ns_error( NS_ERROR_NODATA, (level) )

#define ns_error_nodev( level, user_data )\
	ns_error( NS_ERROR_NODEV, (level) )

#define ns_error_noent( level, user_data )\
	ns_error( NS_ERROR_NOENT, (level) )

#define ns_error_noexec( level, user_data )\
	ns_error( NS_ERROR_NOEXEC, (level) )

#define ns_error_nolck( level, user_data )\
	ns_error( NS_ERROR_NOLCK, (level) )

#define ns_error_nolink( level, user_data )\
	ns_error( NS_ERROR_NOLINK, (level) )

#define ns_error_nomem( level, user_data )\
	ns_error( NS_ERROR_NOMEM, (level) )

#define ns_error_nomsg( level, user_data )\
	ns_error( NS_ERROR_NOMSG, (level) )

#define ns_error_noprotoopt( level, user_data )\
	ns_error( NS_ERROR_NOPROTOOPT, (level) )

#define ns_error_nospc( level, user_data )\
	ns_error( NS_ERROR_NOSPC, (level) )

#define ns_error_nosr( level, user_data )\
	ns_error( NS_ERROR_NOSR, (level) )

#define ns_error_nostr( level, user_data )\
	ns_error( NS_ERROR_NOSTR, (level) )

#define ns_error_nosys( level, user_data )\
	ns_error( NS_ERROR_NOSYS, (level) )

#define ns_error_notconn( level, user_data )\
	ns_error( NS_ERROR_NOTCONN, (level) )

#define ns_error_notdir( level, user_data )\
	ns_error( NS_ERROR_NOTDIR, (level) )

#define ns_error_notempty( level, user_data )\
	ns_error( NS_ERROR_NOTEMPTY, (level) )

#define ns_error_notsock( level, user_data )\
	ns_error( NS_ERROR_NOTSOCK, (level) )

#define ns_error_notsup( level, user_data )\
	ns_error( NS_ERROR_NOTSUP, (level) )

#define ns_error_notty( level, user_data )\
	ns_error( NS_ERROR_NOTTY, (level) )

#define ns_error_nxio( level, user_data )\
	ns_error( NS_ERROR_NXIO, (level) )

#define ns_error_opnotsupp( level, user_data )\
	ns_error( NS_ERROR_OPNOTSUPP, (level) )

#define ns_error_overflow( level, user_data )\
	ns_error( NS_ERROR_OVERFLOW, (level) )

#define ns_error_perm( level, user_data )\
	ns_error( NS_ERROR_PERM, (level) )

#define ns_error_pipe( level, user_data )\
	ns_error( NS_ERROR_PIPE, (level) )

#define ns_error_proto( level, user_data )\
	ns_error( NS_ERROR_PROTO, (level) )

#define ns_error_protonosupport( level, user_data )\
	ns_error( NS_ERROR_PROTONOSUPPORT, (level) )

#define ns_error_prototype( level, user_data )\
	ns_error( NS_ERROR_PROTOTYPE, (level) )

#define ns_error_range( level, user_data )\
	ns_error( NS_ERROR_RANGE, (level) )

#define ns_error_rofs( level, user_data )\
	ns_error( NS_ERROR_ROFS, (level) )

#define ns_error_spipe( level, user_data )\
	ns_error( NS_ERROR_SPIPE, (level) )

#define ns_error_srch( level, user_data )\
	ns_error( NS_ERROR_SRCH, (level) )

#define ns_error_stale( level, user_data )\
	ns_error( NS_ERROR_STALE, (level) )

#define ns_error_time( level, user_data )\
	ns_error( NS_ERROR_TIME, (level) )

#define ns_error_timedout( level, user_data )\
	ns_error( NS_ERROR_TIMEDOUT, (level) )

#define ns_error_txtbsy( level, user_data )\
	ns_error( NS_ERROR_TXTBSY, (level) )

#define ns_error_wouldblock( level, user_data )\
	ns_error( NS_ERROR_WOULDBLOCK, (level) )

#define ns_error_xdev( level, user_data )\
	ns_error( NS_ERROR_XDEV, (level) )

#define ns_error_os( level, user_data )\
	ns_error( NS_ERROR_OS, (level) )

#define ns_error_noimp( level, user_data )\
	ns_error( NS_ERROR_NOIMP, (level) )

#define ns_error_unknown( level, user_data )\
	ns_error( NS_ERROR_UNKNOWN, (level) )


/* NOTE: It is imperative that 'oper' appear only once in
	the macro definition! */
#define NS_SUCCESS( oper, err ) ( NS_NO_ERROR == ns_error_code( (err) = oper ) )
#define NS_FAILURE( oper, err ) ( NS_NO_ERROR != ns_error_code( (err) = oper ) )

NS_DECLS_END

#endif/* __NS_STD_ERROR_H__ */
