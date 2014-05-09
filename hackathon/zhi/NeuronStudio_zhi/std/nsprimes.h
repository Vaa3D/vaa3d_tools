#ifndef __NS_STD_PRIMES_H__
#define __NS_STD_PRIMES_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_PRIME0,
	NS_PRIME1,
	NS_PRIME2,
	NS_PRIME3,
	NS_PRIME4,
	NS_PRIME5,
	NS_PRIME6,
	NS_PRIME7,
	NS_PRIME8,
	NS_PRIME9,
	NS_PRIME10,
	NS_PRIME11,
	NS_PRIME12,
	NS_PRIME13,
	NS_PRIME14,
	NS_PRIME15,
	NS_PRIME16,
	NS_PRIME17,
	NS_PRIME18,
	NS_PRIME19,
	NS_PRIME20,
	NS_PRIME21,
	NS_PRIME22,
	NS_PRIME23,
	NS_PRIME24,
	NS_PRIME25
	}
	NsPrimeType;

#define NS_PRIME_MIN  NS_PRIME0
#define NS_PRIME_MAX  NS_PRIME25


NS_IMPEXP nssize ns_prime( NsPrimeType type );

NS_IMPEXP NsPrimeType ns_prime_nearest( nssize value );

NS_DECLS_END

#endif/* __NS_STD_PRIMES_H__ */
