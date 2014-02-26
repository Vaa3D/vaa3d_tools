
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nsulong, HANDLE );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nsulong, DWORD );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nssize, DWORD );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nspointer, HANDLE );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nsintmax, LONGLONG );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nsuintmax, nsuint64 );
NS_COMPILE_TIME_SIZE_EQUAL( nsint, LONG );
NS_COMPILE_TIME_SIZE_GREATER_EQUAL( nslong, long );
NS_COMPILE_TIME_SIZE_LESS_EQUAL( nsenum, int );


NS_PRIVATE NsError _ns_os_windows_do_ungetc( FILE *stream, nsunichar c )
	{
	NS_USE_VARIABLE( stream );
	return EOF != _ungetch( c ) ? ns_no_error() : _ns_errno_to_error();
	}


NS_PRIVATE NsError _ns_os_windows_do_getc( FILE *stream, nsunichar *c )
	{
	NS_USE_VARIABLE( stream );

	*c = _getche();
	return ns_no_error();
	}


NsError _ns_os_windows_console_gets( nschar *s, nssize length, nsboolean *eof )
	{  return _ns_do_gets( NULL, s, length, eof, _ns_os_windows_do_getc, _ns_os_windows_do_ungetc );  }


NsError _ns_os_windows_console_puts( const nschar *string )
	{
	return 0 == _cputs( string ) ?
			 ns_no_error() : ns_error_io( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NsError _ns_os_windows_find_first( nslong *handle, nspointer pinfo )
	{
	struct _finddata_t *info;

	if( NULL == ( info = ns_new( struct _finddata_t ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	*( ( struct _finddata_t** )pinfo ) = info;

	/* NOTE: Assuming that the two possible errors, invalid spec and
		no files will never occur with the "*" file specification because
		"*" is obviously valid and "*" should always find at least the
		current directory, i.e. the first string returned by ..._findname()
		should be ".". */
	*handle = _findfirst( "*", info );
	ns_verify( -1 != *handle );

	return ns_no_error();
	}


nsboolean _ns_os_windows_find_next( nslong handle, nspointer info )
	{  return 0 == _findnext( handle, info );  }


void _ns_os_windows_find_close( nslong handle, nspointer info )
	{
	_findclose( handle );
	ns_delete( info );
	}


const nschar* _ns_os_windows_find_name( nsconstpointer info )
	{  return ( ( const struct _finddata_t* )info )->name;  }


NsError _ns_os_windows_mutex_create( nspointer *handle )
	{
	ns_assert( NULL != handle );
	*handle = ( nspointer )CreateMutex( NULL, FALSE, NULL );

	return NULL != *handle ?
			 ns_no_error() : ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NsError _ns_os_windows_mutex_destroy( nspointer handle )
	{
	ns_assert( NULL != handle );

	return CloseHandle( ( HANDLE )handle ) ?
			 ns_no_error() : ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NsError _ns_os_windows_mutex_lock( nspointer handle )
	{
	ns_assert( NULL != handle );

	return WAIT_FAILED != WaitForSingleObject( ( HANDLE )handle, INFINITE ) ?
			 ns_no_error() : ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


NsError _ns_os_windows_mutex_try_lock( nspointer handle, nsboolean *did_lock )
	{
	DWORD ret_value;

	ns_assert( NULL != handle );
	ns_assert( NULL != did_lock );

	ret_value = WaitForSingleObject( ( HANDLE )handle, 0 );

   if( WAIT_FAILED == ret_value )
		return ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

	*did_lock = ( WAIT_TIMEOUT != ret_value );
	return ns_no_error();
	}


NsError _ns_os_windows_mutex_unlock( nspointer handle )
	{
	ns_assert( NULL != handle );

	return ReleaseMutex( ( HANDLE )handle ) ?
			 ns_no_error() : ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


nsulong _ns_os_windows_thread_id( void )
	{  return GetCurrentThreadId();  }


nsulong _ns_os_windows_process_id( void )
	{  return GetCurrentProcessId();  }


NsError _ns_os_windows_thread_begin( nsulong *handle, void ( *func )( nspointer ), nspointer args )
	{
   *handle = _beginthread( func, 0, args );

   return -1 == ( nslong )*handle ?
			 _ns_errno_to_error() : ns_no_error();
	}


void _ns_os_windows_thread_end( void )
	{  _endthread();  }


NsError _ns_os_windows_thread_get_priority( nsulong handle, nspointer *ret_priority )
	{
	NsThreadPriority  priority;
	NsError           error;


	ns_assert( NULL != ret_priority );

	priority = NS_THREAD_PRIORITY_UNKNOWN;
	error    = ns_no_error();

	switch( GetThreadPriority( ( HANDLE )handle ) )
		{
      case THREAD_PRIORITY_NORMAL:
         priority = NS_THREAD_PRIORITY_NORMAL;
         break;

      case THREAD_PRIORITY_LOWEST:
      case THREAD_PRIORITY_BELOW_NORMAL:
         priority = NS_THREAD_PRIORITY_LOW;
         break;

		case THREAD_PRIORITY_HIGHEST:
      case THREAD_PRIORITY_ABOVE_NORMAL:
         priority = NS_THREAD_PRIORITY_HIGH;
         break;

		case THREAD_PRIORITY_ERROR_RETURN:
			error = ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
			break;
		}

	*( ( NsThreadPriority* )ret_priority ) = priority;
	return error;
	}


NsError _ns_os_windows_thread_set_priority( nsulong handle, nsenum priority )
	{
	switch( priority )
		{
		case NS_THREAD_PRIORITY_NORMAL:
			priority = ( nsenum )THREAD_PRIORITY_NORMAL;
			break;

		case NS_THREAD_PRIORITY_LOW:
			priority = ( nsenum )THREAD_PRIORITY_BELOW_NORMAL;
			break;
	
		case NS_THREAD_PRIORITY_HIGH:
			priority = ( nsenum )THREAD_PRIORITY_ABOVE_NORMAL;
			break;

		default:
			ns_assert_not_reached();
		}

   return SetThreadPriority( ( HANDLE )handle, ( nsint )priority ) ?
			 ns_no_error() : ns_error_os( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );
	}


nsulong _ns_os_windows_thread_self( void )
	{  return ( nsulong )GetCurrentThread();  }


void _ns_os_windows_thread_sleep( nssize milliseconds )
	{  Sleep( ( DWORD )milliseconds );  }


union _NsOsWindowsTime
	{
	nsuint64   value;
	FILETIME   file_time;
	};


nsintmax _ns_os_windows_time( nsintmax *ret_t )
	{
	nsintmax                t;
	union _NsOsWindowsTime  u;


	/* Number of 100 nanosecond units from 1/1/1601 to 1/1/1970 */
	#define _NS_NANO100_EPOCH_BIAS\
		( ( nsuint64 )116444736000000000 )

	/* Convert 100 nanosecond units to seconds, i.e. 10^7 to 10^0. */
	#define _NS_NANO100_TO_SECONDS( v )\
		( (v) / ( ( nsuint64 )10000000 ) )

	GetSystemTimeAsFileTime( &u.file_time );

	t = ( nsintmax )_NS_NANO100_TO_SECONDS( u.value - _NS_NANO100_EPOCH_BIAS );

	if( ret_t )
		*ret_t = t;

	return t;
	}


nsintmax _ns_os_windows_timer( void )
	{
	LARGE_INTEGER t;
   QueryPerformanceCounter( &t );

	return t.QuadPart;
	}


nsdouble _ns_os_windows_difftimer( nsintmax stop, nsintmax start )
	{
	LARGE_INTEGER t;
   QueryPerformanceFrequency( &t );

	return ( nsdouble )( stop - start ) / ( nsdouble )t.QuadPart;
	}
