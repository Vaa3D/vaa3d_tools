#include "nsascii.h"


nssize ns_ascii_strlen( const nschar *string )
	{
	const nschar *base;

	ns_assert( NULL != string );
	base = string;

	while( *string )
		++string;

	return ( nssize )( string - base );
	}


nssize ns_ascii_strsize( const nschar *string )
	{
	ns_assert( NULL != string );
	return ns_ascii_strlen( string ) + 1;
	}


nschar* ns_ascii_strchr( const nschar *string, nsint ch )
	{
	ns_assert( NULL != string );

	for( ; *string; ++string )
		if( ( nsint )( *string ) == ch )
			return ( nschar* )string;

	return NULL;
	}


nschar* ns_ascii_strrchr( const nschar *string, nsint ch )
	{
	const nschar *ptr;

	ns_assert( NULL != string );
	ptr = ( string + ns_ascii_strlen( string ) ) - 1;

	for( ; string <= ptr; --ptr )
		if( ( nsint )( *ptr ) == ch )
			return ( nschar* )ptr;

	return NULL;
	}


nschar* ns_ascii_strcat( nschar *dest, const nschar *src )
	{
	nschar *base = dest;

	ns_assert( NULL != dest && NULL != src );

	dest += ns_ascii_strlen( dest );

	while( *src )
		*dest++ = *src++;

	*dest = NS_ASCII_NULL;
	return base;
	}


nschar* ns_ascii_strstr( const nschar *string, const nschar *sub )
	{
	const nschar *scan1, *scan2;

	ns_assert( NULL != string && NULL != sub );

	if( ! *sub )
		return ( nschar* )string;

	while( *string )
		{
		scan1 = string;
		scan2 = sub;

		while( *scan1 && *scan2 && *scan1 == *scan2 )
			{
			++scan1;
			++scan2;
			}

		if( ! *scan2 )
			return ( nschar* )string;

		++string;
		}

	return NULL;
	}


nschar* ns_ascii_strtok( nschar *string, const nschar *delims )
	{  return _ns_strtok( string, delims );  }


nschar* ns_ascii_strcpy( nschar *dest, const nschar *src )
	{
	nschar *base;

	ns_assert( NULL != dest && NULL != src );
	base = dest;

	while( *src )
		*dest++ = *src++;

	*dest = NS_ASCII_NULL;
  	return base;
	}


nschar* ns_ascii_strncpy( nschar *dest, const nschar *src, nssize count )
	{
	nschar *base;

	ns_assert( NULL != dest && NULL != src );
	base = dest;

	while( *src && 0 < count )
		{
		*dest++ = *src++;
		--count;
		}

	while( 0 < count )
		{
		*dest++ = NS_ASCII_NULL;
		--count;
		}

	return base;
	}


nsint ns_ascii_strcmp( const nschar *s1, const nschar *s2 )
	{
	ns_assert( NULL != s1 && NULL != s2 );

	while( *s1 && *s1 == *s2 )
		{
		++s1;
		++s2;
		}
  
	return ( nsint )( *( ( nsuchar* )s1 ) ) - ( nsint )( *( ( nsuchar* )s2 ) );
	}


nsint ns_ascii_stricmp( const nschar *s1, const nschar *s2 )
	{
	nsint ch1, ch2;

	ns_assert( NULL != s1 && NULL != s2 );

	do
		{
		ch1 = ( nsint )( *s1++ );
		ch2 = ( nsint )( *s2++ );

      ch1 = ns_ascii_tolower( ch1 );
		ch2 = ns_ascii_tolower( ch2 );
		}
	while( ch1 && ( ch1 == ch2 ) );

	return ch1 - ch2;
	}


nsint ns_ascii_strncmp( const nschar *s1, const nschar *s2, nssize count )
	{
	ns_assert( NULL != s1 && NULL != s2 );

	if( ! count )
		return 0;

	/* NOTE: Pre-decrement count since a comparison is always done
		in the last return statement. */
	while( --count && *s1 && *s1 == *s2 )
		{
		++s1;
		++s2;
		}

	return ( nsint )( *( ( nsuchar* )s1 ) ) - ( nsint )( *( ( nsuchar* )s2 ) );	
	}


nsint ns_ascii_strnicmp( const nschar *s1, const nschar *s2, nssize count )
	{
	nsint ch1, ch2;

	ns_assert( NULL != s1 && NULL != s2 );

	if( ! count )
		return 0;

	do
		{
		ch1 = ( nsint )( *s1++ );
		ch2 = ( nsint )( *s2++ );

      ch1 = ns_ascii_tolower( ch1 );
		ch2 = ns_ascii_tolower( ch2 );
		}
	while( --count && ch1 && ( ch1 == ch2 ) );

	return ch1 - ch2;
	}


nsboolean ns_ascii_streq( const nschar *s1, const nschar *s2 )
	{  return 0 == ns_ascii_strcmp( s1, s2 );  }


nsboolean ns_ascii_strieq( const nschar *s1, const nschar *s2 )
	{  return 0 == ns_ascii_stricmp( s1, s2 );  }


nsboolean ns_ascii_strneq( const nschar *s1, const nschar *s2, nssize count )
	{  return 0 == ns_ascii_strncmp( s1, s2, count );  }


nsboolean ns_ascii_strnieq( const nschar *s1, const nschar *s2, nssize count )
	{  return 0 == ns_ascii_strnicmp( s1, s2, count );  }


nschar* ns_ascii_strlwr( nschar *string )
	{
	nschar *base = string;

	ns_assert( NULL != string );

	for( ; *string; ++string )
		*string = ( nschar )ns_ascii_tolower( ( nsint )( *string ) );

	return base;
	}


nschar* ns_ascii_strupr( nschar *string )
	{
	nschar *base = string;

	ns_assert( NULL != string );

	for( ; *string; ++string )
		*string = ( nschar )ns_ascii_toupper( ( nsint )( *string ) );

	return base;
	}


nschar* ns_ascii_strdup( const nschar *string )
	{
	nschar *dup;

	ns_assert( NULL != string );

	if( NULL != ( dup = ns_malloc( ns_ascii_strlen( string ) + 1 ) ) )
		ns_ascii_strcpy( dup, string );

	return dup;
	}


nsuint ns_ascii_hash( const nschar *string )
	{
   nsuint hash = 0;
   nsuint g;

   while( *string )
      {
      hash = ( hash << 4 ) + *string++;
         g = hash & 0xF0000000;

      if( g )
         hash ^= ( g >> 24 );

      hash &= ~g;
      }

   return hash;
	}


nschar* ns_ascii_chomp( const nschar *string )
	{
	ns_assert( NULL != string );

	while( ns_ascii_isspace( ( nsint )*string ) )
		++string;

	return NS_ASCII_NULL != *string ? ( nschar* )string : NULL;
	}


nschar* ns_ascii_strrev( nschar *string )
	{
	nschar  *base;
	nschar  *end;
	nschar   temp;


	ns_assert( NULL != string );

	base = string;
	end  = ( string + ns_ascii_strlen( string ) ) - 1;

	while( string < end )
		{
		temp    = *string;
		*string = *end;
		*end    = temp;

		++string;
		--end;
		}

	return base;
	}


nschar* ns_ascii_chrpre( nschar *dest, nsint src )
	{
	nschar *base = dest;

	ns_assert( NULL != dest );
	
	ns_memmove(
		dest + 1,
		dest,
		( ns_ascii_strlen( dest ) + 1 ) * sizeof( nschar )
		);

	*dest = ( nschar )src;
	return base;
	}


nschar* ns_ascii_chrarep( nschar *string, nsint dest, nsint src )
	{
	nschar *base = string;

	ns_assert( NULL != string );

	for( ; *string; ++string )
		if( dest == *string )
			*string = ( nschar )src;

	return base;
	}


nschar* ns_ascii_chrins( nschar *dest, nssize at, nsint src )
	{
	nschar *base = dest;

	ns_assert( NULL != dest );
	ns_ascii_chrpre( dest + at, src );

	return base;
	}


nschar* ns_ascii_strarep( nschar *string, const nschar *dest, const nschar *src )
	{
	nschar  *pstr;
	nssize   dest_length;
	nssize   src_length;
	nssize    count;


	ns_assert( NULL != string && NULL != dest && NULL != src );

	if( ! *dest )
		return string;

	dest_length = ns_ascii_strlen( dest );
	src_length  = ns_ascii_strlen( src );

	pstr = string;

	while( NULL != ( pstr = ns_ascii_strstr( pstr, dest ) ) )
		{
		count = ns_ascii_strlen( pstr + dest_length ) + 1;

		ns_memmove( pstr + src_length, pstr + dest_length, count * sizeof( nschar ) );
		ns_memmove( pstr, src, src_length * sizeof( nschar ) );

		pstr += src_length;
		}

	return string;
	}


nschar* ns_ascii_strntab( nschar *dest, const nschar *src, nssize count, nssize size )
	{
	nssize   n, t;
	nschar  *base = dest;


	ns_assert( NULL != src );
	ns_assert( NULL != dest );

	for( n = 0; n < count && *src; ++src )
		if( NS_ASCII_CHARACTER_TABULATION == *src )
			{
			for( t = 0; n < count && t < size; ++t )
				{
				*dest++ = NS_ASCII_SPACE;
				++n;
				}
			}
		else
			{
			*dest++ = *src;
			++n;
			}

	if( n < count )
		*dest = NS_ASCII_NULL;

	return base;
	}


nsboolean ns_ascii_isspace( nsint c )
	{  return ( 0x09 <= c && c <= 0x0D ) || ( 0x20 == c );  }


nsboolean ns_ascii_isdigit( nsint c )
	{  return ( '0' <= c && c <= '9' );  }


nsboolean ns_ascii_isxdigit( nsint c )
	{
	return ( '0' <= c && c <= '9' ) ||
			 ( 'a' <= c && c <= 'f' ) ||
			 ( 'A' <= c && c <= 'F' );
	}


nsboolean ns_ascii_islower( nsint c )
	{  return ( 'a' <= c && c <= 'z' );  }


nsboolean ns_ascii_isupper( nsint c )
	{  return ( 'A' <= c && c <= 'Z' );  }


nsboolean ns_ascii_isprint( nsint c )
	{  return 0x20 <= c && c <= 0x7E;  }
