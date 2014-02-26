#include "nsrandom.h"


#define _NS_RANDI_KA  1103515245  /*  214013 */
#define _NS_RANDI_KC       12345  /* 2531011 */ 


void ns_randi_init( NsRandi *r, nsuint32 seed )
   {
   ns_assert( NULL != r );
   r->seed = seed;
   }


void ns_randi_seed( NsRandi *r, nsuint32 seed )
   {
   ns_assert( NULL != r );
   r->seed = seed;
   }


nsuint16 ns_randi( NsRandi *r )
   {
   ns_assert( NULL != r );

   r->seed = _NS_RANDI_KA * r->seed + _NS_RANDI_KC;

   return ( nsuint16 )( ( r->seed >> 16 ) & NS_RANDI_MAX );
   }


nsuint16 ns_randi_ranged( NsRandi *r, nsuint16 min, nsuint16 max )
   {
   ns_assert( NULL != r );
   ns_assert( min <= max );

   return ( ns_randi( r ) % ( max - min + 1 ) ) + min;
   }


NS_PRIVATE NsRandi _ns_randi;


void ns_srand( nsuint32 seed )
	{  ns_randi_seed( &_ns_randi, seed );  }


nsuint16 ns_rand( void )
	{  return ns_randi( &_ns_randi );  }


NS_PRIVATE nsulong _ns_randf_table[ _NS_RANDF_N ] = 
	{ 
	40,
	1387256442,  539505633,    7126687, 2115653676,  480642437,
	1403109719,  898019591, 1609472695,  742049136,  964528840,
	1774590149,  531014893, 1478060509,  224730595, 1413365137,
	1415397063,  370513614, 1981855272, 1672294721, 1559669404,
	1992066581,  440083042, 1552169384,  949029171, 1848294689,
	1014369863, 1226252978,  199445637,  552539314,  101995811,
	1795618857, 1468200845,  403608434,  466262418, 1783034892,
	2125486341, 1437171068,  839437811,  685760609,  311739045,
	1876584692,  223544964,  667792106, 1829604735,  887026472,
	 688815796, 1153871680, 1135467106, 1975710098, 1393037901,
	 330755675,  804762632,  393596594, 1695657725,   50479950,
	1039358666, 1885424316,  400881551,  142829986,  187416368,
	 821029919, 1292641081,  415120294, 1104581275, 1258423968,
	 304285054,  400491932, 2014625087, 1619263031,  750624285,
	1996732699,   97476312, 1250544934, 2145510054, 1510875684,
	 262891578,  616032534, 1316668730, 1500747974, 2138561534,
	 809719156, 1605036043,  510086967,  317411066,   54278455,
	2052774305,  439191668, 1881943474, 1397167115, 2046084812,
	 644321591,  328615697, 1004646018, 1110120728, 2007784487,
 	 992677826, 1756605308,  796797739
	};


void ns_randf_init( NsRandf *r, nssize shuffle )
   {
   ns_assert( NULL != r );
   ns_assert( sizeof( r->table ) == sizeof( _ns_randf_table ) );

   memcpy( r->table, _ns_randf_table, sizeof( _ns_randf_table ) );

	for( ; 0 < shuffle; --shuffle )
		ns_randf( r );
   }


nsfloat ns_randf( NsRandf *r )
   {
   nsulong *sp, k, p;

   ns_assert( NULL != r );

   sp = r->table;
   p  = sp[ 0 ];

   if( ++p > 98 )
      p = 1;

   k = p + 27;

   if( k > 98 )
      k -= 98;

   sp[ p ] ^= sp[ k ];
   sp[ 0 ] = p;

   return ( nsfloat )( sp[ p ] / 2147483647.0 );
   }


NS_PRIVATE NsRandi ____ns_global_randi;


void ns_global_randi_init( nsuint32 seed )
	{  ns_randi_init( &____ns_global_randi, seed );  }


nsuint16 ns_global_randi( void )
	{  return ns_randi( &____ns_global_randi );  }


nsuint16 ns_global_randi_ranged( nsuint16 min, nsuint16 max )
	{  return ns_randi_ranged( &____ns_global_randi, min, max );  }


NS_PRIVATE NsRandf ____ns_global_randf;


void ns_global_randf_init( nssize shuffle )
	{  ns_randf_init( &____ns_global_randf, shuffle );  }


nsfloat ns_global_randf( void )
	{  return ns_randf( &____ns_global_randf );  }
