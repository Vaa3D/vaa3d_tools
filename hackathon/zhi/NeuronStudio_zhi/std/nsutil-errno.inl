
NS_PRIVATE NsError _ns_errno_to_error( void )
	{
	NsError error;

	switch( errno )
		{
		#ifdef E2BIG
		case E2BIG:
			error = ns_error_2big( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EACCES
		case EACCES:
			error = ns_error_acces( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EADDRINUSE
		case EADDRINUSE:
			error = ns_error_addrinuse( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EADDRNOTAVAIL
		case EADDRNOTAVAIL:
			error = ns_error_addrnotavail( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EAFNOSUPPORT
		case EAFNOSUPPORT:
			error = ns_error_afnosupport( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EAGAIN
		case EAGAIN:
			error = ns_error_again( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EALREADY
		case EALREADY:
			error = ns_error_already( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EBADF
		case EBADF:
			error = ns_error_badf( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EBADMSG
		case EBADMSG:
			error = ns_error_badmsg( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EBUSY
		case EBUSY:
			error = ns_error_busy( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ECANCELED
		case ECANCELED:
			error = ns_error_canceled( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ECHILD
		case ECHILD:
			error = ns_error_child( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ECONNABORTED
		case ECONNABORTED:
			error = ns_error_connaborted( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ECONNREFUSED
		case ECONNREFUSED:
			error = ns_error_connrefused( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ECONNRESET
		case ECONNRESET:
			error = ns_error_connreset( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EDEADLK
		case EDEADLK:
			error = ns_error_deadlk( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EDESTADDRREQ
		case EDESTADDRREQ:
			error = ns_error_destaddrreq( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EDOM
		case EDOM:
			error = ns_error_dom( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EDQUOT
		case EDQUOT:
			error = ns_error_dquot( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EEXIST
		case EEXIST:
			error = ns_error_exist( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EFAULT
		case EFAULT:
			error = ns_error_fault( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EFBIG
		case EFBIG:
			error = ns_error_fbig( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EHOSTUNREACH
		case EHOSTUNREACH:
			error = ns_error_hostunreach( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EIDRM
		case EIDRM:
			error = ns_error_idrm( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EILSEQ
		case EILSEQ:
			error = ns_error_ilseq( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EINPROGRESS
		case EINPROGRESS:
			error = ns_error_inprogress( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EINTR
		case EINTR:
			error = ns_error_intr( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EINVAL
		case EINVAL:
			error = ns_error_inval( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EIO
		case EIO:
			error = ns_error_io( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EISCONN
		case EISCONN:
			error = ns_error_isconn( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EISDIR
		case EISDIR:
			error = ns_error_isdir( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ELOOP
		case ELOOP:
			error = ns_error_loop( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EMFILE
		case EMFILE:
			error = ns_error_mfile( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EMLINK
		case EMLINK:
			error = ns_error_mlink( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EMSGSIZE
		case EMSGSIZE:
			error = ns_error_msgsize( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EMULTIHOP
		case EMULTIHOP:
			error = ns_error_multihop( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENAMETOOLONG
		case ENAMETOOLONG:
			error = ns_error_nametoolong( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENETDOWN
		case ENETDOWN:
			error = ns_error_netdown( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENETRESET
		case ENETRESET:
			error = ns_error_netreset( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENETUNREACH
		case ENETUNREACH:
			error = ns_error_netunreach( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENFILE
		case ENFILE:
			error = ns_error_nfile( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOBUFS
		case ENOBUFS:
			error = ns_error_nobufs( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENODATA
		case ENODATA:
			error = ns_error_nodata( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENODEV
		case ENODEV:
			error = ns_error_nodev( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOENT
		case ENOENT:
			error = ns_error_noent( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOEXEC
		case ENOEXEC:
			error = ns_error_noexec( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOLCK
		case ENOLCK:
			error = ns_error_nolck( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOLINK
		case ENOLINK:
			error = ns_error_nolink( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOMEM
		case ENOMEM:
			error = ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOMSG
		case ENOMSG:
			error = ns_error_nomsg( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOPROTOOPT
		case ENOPROTOOPT:
			error = ns_error_noprotoopt( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOSPC
		case ENOSPC:
			error = ns_error_nospc( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOSR
		case ENOSR:
			error = ns_error_nosr( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOSTR
		case ENOSTR:
			error = ns_error_nostr( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOSYS
		case ENOSYS:
			error = ns_error_nosys( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTCONN
		case ENOTCONN:
			error = ns_error_notconn( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTDIR
		case ENOTDIR:
			error = ns_error_notdir( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTEMPTY
		case ENOTEMPTY:
			error = ns_error_notempty( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTSOCK
		case ENOTSOCK:
			error = ns_error_notsock( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTSUP
		case ENOTSUP:
			error = ns_error_notsup( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENOTTY
		case ENOTTY:
			error = ns_error_notty( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ENXIO
		case ENXIO:
			error = ns_error_nxio( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EOPNOTSUPP
		case EOPNOTSUPP:
			error = ns_error_opnotsupp( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EOVERFLOW
		case EOVERFLOW:
			error = ns_error_overflow( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EPERM
		case EPERM:
			error = ns_error_perm( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EPIPE
		case EPIPE:
			error = ns_error_pipe( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EPROTO
		case EPROTO:
			error = ns_error_proto( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EPROTONOSUPPORT
		case EPROTONOSUPPORT:
			error = ns_error_protonosupport( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EPROTOTYPE
		case EPROTOTYPE:
			error = ns_error_prototype( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ERANGE
		case ERANGE:
			error = ns_error_range( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EROFS
		case EROFS:
			error = ns_error_rofs( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ESPIPE
		case ESPIPE:
			error = ns_error_spipe( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ESRCH
		case ESRCH:
			error = ns_error_srch( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ESTALE
		case ESTALE:
			error = ns_error_stale( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ETIME
		case ETIME:
			error = ns_error_time( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ETIMEDOUT
		case ETIMEDOUT:
			error = ns_error_timedout( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef ETXTBSY
		case ETXTBSY:
			error = ns_error_txtbsy( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EWOULDBLOCK
		case EWOULDBLOCK:
			error = ns_error_wouldblock( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		#ifdef EXDEV
		case EXDEV:
			error = ns_error_xdev( NS_ERROR_LEVEL_CRITICAL, NULL );
			break;
		#endif

		default:
			error = ns_error_unknown( NS_ERROR_LEVEL_CRITICAL, NULL );
		}

	return error;
	}
