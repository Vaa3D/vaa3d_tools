/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _arch_h
#define _arch_h

#define COMPRESS(fn) "compress -f %s",fn
#define UNZIP(raw_fn,uncmp_fn) "gunzip -cf %s > %s",raw_fn,uncmp_fn
#define ZIP(fn) "gzip -nf %s",fn

#define byte_swap_integers(x) \
((unsigned long int)((((unsigned long int)(x) & 0x000000ff) << 24) | \
		     (((unsigned long int)(x) & 0x0000ff00) <<  8) | \
		     (((unsigned long int)(x) & 0x00ff0000) >>  8) | \
		     (((unsigned long int)(x) & 0xff000000) >> 24)))
  
#define byte_swap_short_integers(x) \
  ((unsigned short int)((((unsigned short int)(x) & 0x00ff) << 8) | \
			(((unsigned short int)(x) & 0xff00) >> 8)))

#define NO_ENDIAN 0

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#ifdef sgi
#define	CHAR signed char
#else

#ifdef _AIX
#define	CHAR signed char
#else
#define	CHAR char
#endif

#endif

#endif	/* _arch_h */
