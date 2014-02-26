#include "nsmisc.h"


#define __KILOBYTE        1024
#define __MEGABYTE     1048576
#define __GIGABYTE  1073741824


NS_PRIVATE void _ns_num_bytes_to_string_insert_commas( nschar *string )
	{
	nschar   *dot;
	nssize    at, i, length, num_commas, num_leading_digits, num_remaining_digits;


	if( NULL != ( dot = ns_ascii_strchr( string, NS_ASCII_FULL_STOP ) ) )
		length = ( nssize )( dot - string );
	else
		length = ns_ascii_strlen( string );

   num_commas           = ( length - 1 ) / 3;
   num_leading_digits   = length - num_commas * 3;
   num_remaining_digits = length - num_leading_digits;

   at = num_leading_digits;

   for( i = 0; i < num_remaining_digits; ++i, ++at )
      if( 0 == i % 3 )
         ns_ascii_chrins( string, at++, NS_ASCII_COMMA );
	}


NS_PRIVATE void _ns_do_num_bytes_to_string( nssize num_bytes, nschar *string, nssize max_chars, nsulong flags )
   {
   nsdouble   value;
   nschar    *units;
	nschar     temp[ NS_INTEGER_TYPE_MAX_DIGITS ];


   ns_assert( NULL != string );

/*TEMP*/string[ max_chars - 1 ] = ( nschar )( nsuint8 )255;

	units = NULL;

	value = 0.0;

   if( 0 < num_bytes / __GIGABYTE )
      {
      value = ( nsdouble )num_bytes / ( nsdouble )__GIGABYTE;
      units = "GB";
      }
   else if( 0 < num_bytes / __MEGABYTE )
      {
      value = ( nsdouble )num_bytes / ( nsdouble )__MEGABYTE;
      units = "MB";
      }
   else if( 0 < num_bytes / __KILOBYTE )
      {
      value = ( nsdouble )num_bytes / ( nsdouble )__KILOBYTE;
      units = "KB";
      }

   if( NULL != units )
      {
		ns_sprint(
			string,
			NS_MAKE_FMT( ".1", NS_FMT_TYPE_DOUBLE ),
			value
			);

		_ns_num_bytes_to_string_insert_commas( string );

		ns_ascii_strcat( string, " " );
		ns_ascii_strcat( string, units );

		if( ( nsboolean )( flags & NS_NUM_BYTES_TO_STRING_LONG_FORM ) )
			{
			ns_ascii_strcat( string, " (" );

			ns_sprint( temp, NS_FMT_ULONG, num_bytes );
			_ns_num_bytes_to_string_insert_commas( temp );
			ns_ascii_strcat( string, temp );

			ns_ascii_strcat( string, ")" /*" bytes)"*/ );
			}
      }
	else
		{
		/* Less than 1 KB. */
		ns_sprint( string, NS_FMT_ULONG, num_bytes );

		_ns_num_bytes_to_string_insert_commas( string );

		/*ns_ascii_strcat( string, 1 != num_bytes ? " bytes" : " byte" );*/
		}

	/*TEMP*/ns_assert( string[ max_chars - 1 ] == ( nschar )( nsuint8 )255 );
	/*TEMP*/string[ max_chars - 1 ] = ( nschar )'\0';
   }


#define NS_NUM_BYTES_TO_STRING_LONG_FORM  0x01

void ns_num_bytes_to_string
	(
	nssize   num_bytes,
	nschar  *string,
	nssize   max_chars
	)
	{  _ns_do_num_bytes_to_string( num_bytes, string, max_chars, 0xffff );  }

void ns_num_bytes_to_string_ex
	(
	nssize   num_bytes,
	nschar  *string,
	nssize   max_chars,
	nsulong  flags
	)
	{  _ns_do_num_bytes_to_string( num_bytes, string, max_chars, flags );  }
