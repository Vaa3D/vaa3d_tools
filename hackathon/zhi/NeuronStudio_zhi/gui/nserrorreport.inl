
#include <excpt.h>

nsint ns_error_report( nsint code_value, nsboolean continuable, nspointer address )
	{
	const nschar    *code_string;
	OSVERSIONINFO    version_info;
	MEMORYSTATUSEX   memory_status;
	SYSTEM_INFO      system_info;
	nschar           version[ 32 ];
	nschar           s[ 1024 ] = { '\0' };
	nschar           t[ 1024 ] = { '\0' };
	nschar           buffer[ 2048 ];


	switch( code_value )
		{
		case EXCEPTION_ACCESS_VIOLATION:
			code_string =
				"The thread attempts to read from or write to a virtual"
				" address for which it does not have access.";
			break;

		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			code_string =
				"The thread attempts to access an array element that is"
				" out of bounds, and the underlying hardware supports bounds"
				" checking.";
			break;

		case EXCEPTION_BREAKPOINT:
			code_string = "A breakpoint is encountered.";
			break;

		case EXCEPTION_DATATYPE_MISALIGNMENT:
			code_string =
				"The thread attempts to read or write data that is misaligned"
				" on hardware that does not provide alignment. For example, "
				"16-bit values must be aligned on 2-byte boundaries, 32-bit"
				" values on 4-byte boundaries, and so on.";
			break;

		case EXCEPTION_FLT_DENORMAL_OPERAND:
			code_string =
				"One of the operands in a floating point operation is denormal."
				" A denormal value is one that is too small to represent as a "
				"standard floating point value.";
			break;

		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			code_string =
				"The thread attempts to divide a floating point value by a floating"
				" point divisor of 0 (zero).";
			break;

		case EXCEPTION_FLT_INEXACT_RESULT:
			code_string =
				"The result of a floating point operation cannot be represented"	
				" exactly as a decimal fraction.";
			break;

		case EXCEPTION_FLT_INVALID_OPERATION:
			code_string =
				"A floating point exception that is not included in this list.";
			break;

		case EXCEPTION_FLT_OVERFLOW:
			code_string =
				"The exponent of a floating point operation is greater than the"
				" magnitude allowed by the corresponding type.";
			break;

		case EXCEPTION_FLT_STACK_CHECK:
			code_string =
				"The stack has overflowed or underflowed, because of a floating"
				" point operation.";
			break;

		case EXCEPTION_FLT_UNDERFLOW:
			code_string =
				"The exponent of a floating point operation is less than the"
				" magnitude allowed by the corresponding type.";
			break;

		case EXCEPTION_GUARD_PAGE:
			code_string =
				"The thread accessed memory allocated with the PAGE_GUARD modifier.";
			break;

		case EXCEPTION_ILLEGAL_INSTRUCTION:
			code_string = "The thread tries to execute an invalid instruction.";
			break;

		case EXCEPTION_IN_PAGE_ERROR:
			code_string =
				"The thread tries to access a page that is not present, and the "
				"system is unable to load the page. For example, this exception"
				" might occur if a network connection is lost while running a "
				"program over a network.";
			break;

		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			code_string =
				"The thread attempts to divide an integer value by an integer"
				" divisor of 0 (zero).";
			break;

		case EXCEPTION_INT_OVERFLOW:
			code_string =
				"The result of an integer operation causes a carry out of the"
				" most significant bit of the result.";
			break;

		case EXCEPTION_INVALID_DISPOSITION:
			code_string =
				"An exception handler returns an invalid disposition to the"
				" exception dispatcher. Programmers using a high-level language"
				" such as C should never encounter this exception.";
			break;

		case EXCEPTION_INVALID_HANDLE:
			code_string =
				"The thread used a handle to a kernel object that was invalid"
				" (probably because it had been closed.)";
			break;

		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			code_string =
				"The thread attempts to continue execution after a non-continuable"
				" exception occurs.";
			break;

		case EXCEPTION_PRIV_INSTRUCTION:
			code_string =
				"The thread attempts to execute an instruction with an operation"
				" that is not allowed in the current computer mode.";
			break;

		case EXCEPTION_SINGLE_STEP:
			code_string =
				"A trace trap or other single instruction mechanism signals that"
				" one instruction is executed.";
			break;

		case EXCEPTION_STACK_OVERFLOW:
			code_string = "The thread uses up its stack.";
			break;

		//case STATUS_UNWIND_CONSOLIDATE:
		//	code_string = "A frame consolidation has been executed.";
		//	break;

		default:
			code_string = "<Unknown>";
		}

	version_info.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	GetVersionEx( &version_info );

	switch( version_info.dwMajorVersion )
		{
		case 3:
			switch( version_info.dwMinorVersion )
				{
				case 51: ns_ascii_strcat( s, "Windows NT 3.51" ); break;
				}
			break;

		case 4:
			switch( version_info.dwMinorVersion )
				{
				case 0:
					if( VER_PLATFORM_WIN32_NT == version_info.dwPlatformId )
						ns_ascii_strcat( s, "Windows NT 4.0" );
					else if( VER_PLATFORM_WIN32_WINDOWS == version_info.dwPlatformId )
						ns_ascii_strcat( s, "Windows 95" );
					break;

				case 10: ns_ascii_strcat( s, "Windows 98" ); break;
				case 90: ns_ascii_strcat( s, "Windows Me" ); break;
				}		
			break;

		case 5:
			switch( version_info.dwMinorVersion )
				{
				case 0: ns_ascii_strcat( s, "Windows 2000" ); break;
				case 1: ns_ascii_strcat( s, "Windows XP" ); break;
				case 2: ns_ascii_strcat( s, "Windows Server 2003 family" ); break;
				}
			break;
		}

	ns_ascii_strcat( s, ";" );

	memory_status.dwLength = sizeof( MEMORYSTATUSEX );
	GlobalMemoryStatusEx( &memory_status );

	ns_sprint( t, NS_FMT_ULONG, ( nsulong )memory_status.ullTotalPhys );
	ns_ascii_strcat( s, t );
	ns_ascii_strcat( s, ";" );

	ns_sprint( t, NS_FMT_ULONG, ( nsulong )memory_status.ullTotalVirtual );
	ns_ascii_strcat( s, t );
	ns_ascii_strcat( s, ";" );

	GetSystemInfo( &system_info );

	switch( system_info.wProcessorArchitecture )
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			ns_ascii_strcat( s, "Intel" );

			switch( system_info.wProcessorLevel )
				{
				case 3: ns_ascii_strcat( s, "Intel 80386" ); break;
				case 4: ns_ascii_strcat( s, "Intel 80486" ); break;
				case 5: ns_ascii_strcat( s, "Pentium" ); break;
				}
			break;

		case PROCESSOR_ARCHITECTURE_MIPS:
			ns_ascii_strcat( s, "MIPS" );

			switch( system_info.wProcessorLevel )
				{
				case 0004: ns_ascii_strcat( s, "MIPS R4000" ); break;
				}
			break;

		case PROCESSOR_ARCHITECTURE_ALPHA:
			ns_ascii_strcat( s, "Alpha" );

			switch( system_info.wProcessorLevel )
				{
				case 21064: ns_ascii_strcat( s, "Alpha 21064" ); break;
				case 21066: ns_ascii_strcat( s, "Alpha 21066" ); break;
				case 21164: ns_ascii_strcat( s, "Alpha 21164" ); break; 
				}
			break;

		case PROCESSOR_ARCHITECTURE_PPC:
			ns_ascii_strcat( s, "PPC" );

			switch( system_info.wProcessorLevel )
				{
				case 1: ns_ascii_strcat( s, "PPC 601" ); break;
				case 3: ns_ascii_strcat( s, "PPC 603" ); break;
				case 4: ns_ascii_strcat( s, "PPC 604" ); break;
				case 6: ns_ascii_strcat( s, "PPC 603+" ); break; 
				case 9: ns_ascii_strcat( s, "PPC 604+" ); break;
				case 20: ns_ascii_strcat( s, "PPC 620" ); break;
				}
			break;

		case PROCESSOR_ARCHITECTURE_IA64:
			ns_ascii_strcat( s, "Itanium 64-bit" );
			break;

		case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
			ns_ascii_strcat( s, "Itanium 32-bit on Win64" );
			break;

		case PROCESSOR_ARCHITECTURE_AMD64:
			ns_ascii_strcat( s, "AMD 64-bit" );
			break;

		case PROCESSOR_ARCHITECTURE_UNKNOWN:
			ns_ascii_strcat( s, "Unknown" );
			break;
		}

	ns_ascii_strcat( s, ";" );

	ns_sprint( t, NS_FMT_ULONG, ( nssize )system_info.dwNumberOfProcessors );
	ns_ascii_strcat( s, t );
	ns_ascii_strcat( s, ";" );

	switch( system_info.dwProcessorType )
		{
		case PROCESSOR_INTEL_386: ns_ascii_strcat( s, "Intel 386" ); break;
		case PROCESSOR_INTEL_486: ns_ascii_strcat( s, "Intel 486" ); break;
		case PROCESSOR_INTEL_PENTIUM: ns_ascii_strcat( s, "Intel Pentium" ); break;
		case PROCESSOR_MIPS_R4000: ns_ascii_strcat( s, "MIPS R4000" ); break;
		case PROCESSOR_ALPHA_21064: ns_ascii_strcat( s, "Alpha 21064" ); break;
		}

	ns_ascii_strcat( s, ";" );

	ns_snprint(
		version,
		NS_ARRAY_LENGTH( version ),
		NS_FMT_UINT "." NS_FMT_UINT "." NS_FMT_UINT,
		NS_VERSION_MAJOR,
		NS_VERSION_MINOR,
		NS_VERSION_MICRO
		);

	ns_snprint(
		buffer,
		NS_ARRAY_LENGTH( buffer ),
		"Description: An unhandled exception occurred." NS_STRING_NEWLINE
		"Details: " NS_FMT_STRING NS_STRING_NEWLINE 
		"Continuable: " NS_FMT_STRING NS_STRING_NEWLINE
		"Address: " NS_FMT_POINTER NS_STRING_NEWLINE
		"System: " NS_FMT_STRING NS_STRING_NEWLINE
		"Version: " NS_FMT_STRING " " NS_FMT_STRING NS_STRING_NEWLINE
		NS_STRING_NEWLINE
		"Record(or screen-shot) the above information and visit http://www.mssm.edu/cnic to report this error."
		NS_STRING_NEWLINE
		NS_STRING_NEWLINE
		"** Please include the .log file located in the directory where this application was run. **",
		code_string,
		continuable ? "Yes" : "No",
		address,
		s,
		version,
		32 == NS_POINTER_BITS ? "32-bit" : "64-bit"
		);

	ns_log_close();
	ns_print_record_dump( _startup_directory );

	MessageBox( NULL, buffer, "NeuronStudio Error Report", MB_OK | MB_ICONERROR );

	return EXCEPTION_EXECUTE_HANDLER;
	}
