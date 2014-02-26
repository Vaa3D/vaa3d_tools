#include "nsprimes.h"


#define _NS_PRIME_TABLE_SIZE ( ( nssize )( NS_PRIME_MAX - NS_PRIME_MIN ) + 1 )


NS_PRIVATE nssize _ns_prime_table[ _NS_PRIME_TABLE_SIZE ] =
	{
	53,
	97,
	193,
	389,
	769,
	1543,
	3079,
	6151,
	12289,
	24593,
	49157,
	98317,
	196613,
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457,
	1610612741
	};


nssize ns_prime( NsPrimeType type )
   {
   ns_assert( type >= NS_PRIME_MIN && type <= NS_PRIME_MAX );
   return _ns_prime_table[ type ];
   }


NsPrimeType ns_prime_nearest( nssize value )
   {
	NsPrimeType  i;
   nssize       lower_diff;
   nssize       upper_diff;


   for( i = NS_PRIME_MIN; i <= NS_PRIME_MAX; ++i )
      if( value < _ns_prime_table[ i ] )
         {
         if( i == NS_PRIME_MIN )
            return NS_PRIME_MIN;

         lower_diff = value - _ns_prime_table[ i - 1 ];
         upper_diff = _ns_prime_table[ i ] - value;

         return ( lower_diff < upper_diff ) ? i - 1 : i;
         }

   return NS_PRIME_MAX;
   }
