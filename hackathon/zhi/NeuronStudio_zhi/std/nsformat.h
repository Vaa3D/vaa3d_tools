#ifndef __NS_STD_FORMAT_H__
#define __NS_STD_FORMAT_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>

NS_DECLS_BEGIN

/* 'flags_width_precision' must be a string literal, e.g. "+04.2" */
#define NS_MAKE_FMT( flags_width_precision, type )\
	"%" flags_width_precision NS_STRINGIZE( type )

/* 'flags', 'width', and 'precision' must be string literals. 
	Pass "" for any of these if not necessary. */
#define NS_MAKE_FMT_EX( flags, width, precision, type )\
	"%" flags width precision NS_STRINGIZE( type )


/* Flags to be passed to the NS_MAKE_FMT... macros. */
#define NS_FMT_FLAG_LEFT_JUSTIFY  "-"
#define NS_FMT_FLAG_FORCE_SIGN    "+"
#define NS_FMT_FLAG_ALT_FORM      "#"
#define NS_FMT_FLAG_LEAD_SPACE    " "
#define NS_FMT_FLAG_ZERO_FILL     "0"


/* Add to string before the precision. */
#define NS_FMT_PRECISION  "."


#define NS_FMT_ARGUMENT  "*"


/* Complete example...
NS_MAKE_FMT(
	NS_FMT_FLAG_FORCE_SIGN
	NS_FMT_FLAG_LEAD_SPACE
	"24"
	NS_FMT_PRECISION
	NS_FMT_ARGUMENT,
	NS_FMT_TYPE_INT
	)
*/


#define NS_FMT_PERCENT_SIGN  NS_MAKE_FMT( "", % )

/* NOTE: Use the NS_FMT_TYPE... macros only in the above
	NS_MAKE_FMT... macros as the 'type' parameter. */
#define NS_FMT_TYPE_SHORT          hd
#define NS_FMT_SHORT               NS_MAKE_FMT( "", NS_FMT_TYPE_SHORT )
#define NS_FMT_TYPE_SHORT_PTR      hn
#define NS_FMT_SHORT_PTR           NS_MAKE_FMT( "", NS_FMT_TYPE_SHORT_PTR )
#define NS_FMT_TYPE_USHORT         hu
#define NS_FMT_USHORT              NS_MAKE_FMT( "", NS_FMT_TYPE_USHORT )
#define NS_FMT_TYPE_USHORT_OCT     ho
#define NS_FMT_USHORT_OCT          NS_MAKE_FMT( "", NS_FMT_TYPE_USHORT_OCT )
#define NS_FMT_TYPE_USHORT_HEX     hx
#define NS_FMT_USHORT_HEX          NS_MAKE_FMT( "", NS_FMT_TYPE_USHORT_HEX )
#define NS_FMT_TYPE_USHORT_HEX_UC  hX
#define NS_FMT_USHORT_HEX_UC       NS_MAKE_FMT( "", NS_FMT_TYPE_USHORT_HEX_UC )

#define NS_FMT_TYPE_INT            d
#define NS_FMT_INT                 NS_MAKE_FMT( "", NS_FMT_TYPE_INT )
#define NS_FMT_TYPE_INT_PTR        n
#define NS_FMT_INT_PTR             NS_MAKE_FMT( "", NS_FMT_TYPE_INT_PTR )
#define NS_FMT_TYPE_UINT           u
#define NS_FMT_UINT                NS_MAKE_FMT( "", NS_FMT_TYPE_UINT )
#define NS_FMT_TYPE_UINT_OCT       o
#define NS_FMT_UINT_OCT            NS_MAKE_FMT( "", NS_FMT_TYPE_UINT_OCT )
#define NS_FMT_TYPE_UINT_HEX       x
#define NS_FMT_UINT_HEX            NS_MAKE_FMT( "", NS_FMT_TYPE_UINT_HEX )
#define NS_FMT_TYPE_UINT_HEX_UC    X
#define NS_FMT_UINT_HEX_UC         NS_MAKE_FMT( "", NS_FMT_TYPE_UINT_HEX_UC )

#if 32 == NS_POINTER_BITS
	#define NS_FMT_TYPE_LONG           ld
	#define NS_FMT_LONG                NS_MAKE_FMT( "", NS_FMT_TYPE_LONG )
	#define NS_FMT_TYPE_LONG_PTR       ln
	#define NS_FMT_LONG_PTR            NS_MAKE_FMT( "", NS_FMT_TYPE_LONG_PTR )
	#define NS_FMT_TYPE_ULONG          lu
	#define NS_FMT_ULONG               NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG )
	#define NS_FMT_TYPE_ULONG_OCT      lo
	#define NS_FMT_ULONG_OCT           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_OCT )
	#define NS_FMT_TYPE_ULONG_HEX      lx
	#define NS_FMT_ULONG_HEX           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX )
	#define NS_FMT_TYPE_ULONG_HEX_UC   lX
	#define NS_FMT_ULONG_HEX_UC        NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX_UC )
#else
	#ifdef NS_OS_WINDOWS
		#define NS_FMT_TYPE_LONG           I64d
		#define NS_FMT_LONG                NS_MAKE_FMT( "", NS_FMT_TYPE_LONG )
		#define NS_FMT_TYPE_LONG_PTR       I64n
		#define NS_FMT_LONG_PTR            NS_MAKE_FMT( "", NS_FMT_TYPE_LONG_PTR )
		#define NS_FMT_TYPE_ULONG          I64u
		#define NS_FMT_ULONG               NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG )
		#define NS_FMT_TYPE_ULONG_OCT      I64o
		#define NS_FMT_ULONG_OCT           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_OCT )
		#define NS_FMT_TYPE_ULONG_HEX      I64x
		#define NS_FMT_ULONG_HEX           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX )
		#define NS_FMT_TYPE_ULONG_HEX_UC   I64X
		#define NS_FMT_ULONG_HEX_UC        NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX_UC )
	#else
		#define NS_FMT_TYPE_LONG           lld
		#define NS_FMT_LONG                NS_MAKE_FMT( "", NS_FMT_TYPE_LONG )
		#define NS_FMT_TYPE_LONG_PTR       lln
		#define NS_FMT_LONG_PTR            NS_MAKE_FMT( "", NS_FMT_TYPE_LONG_PTR )
		#define NS_FMT_TYPE_ULONG          llu
		#define NS_FMT_ULONG               NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG )
		#define NS_FMT_TYPE_ULONG_OCT      llo
		#define NS_FMT_ULONG_OCT           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_OCT )
		#define NS_FMT_TYPE_ULONG_HEX      llx
		#define NS_FMT_ULONG_HEX           NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX )
		#define NS_FMT_TYPE_ULONG_HEX_UC   llX
		#define NS_FMT_ULONG_HEX_UC        NS_MAKE_FMT( "", NS_FMT_TYPE_ULONG_HEX_UC )
	#endif
#endif

#define NS_FMT_TYPE_DOUBLE             f
#define NS_FMT_DOUBLE                  NS_MAKE_FMT( "", NS_FMT_TYPE_DOUBLE )
#define NS_FMT_TYPE_DOUBLE_SCI         e
#define NS_FMT_DOUBLE_SCI              NS_MAKE_FMT( "", NS_FMT_TYPE_DOUBLE_SCI )
#define NS_FMT_TYPE_DOUBLE_SCI_UC      E
#define NS_FMT_DOUBLE_SCI_UC           NS_MAKE_FMT( "", NS_FMT_TYPE_DOUBLE_SCI_UC )
#define NS_FMT_TYPE_DOUBLE_COMPACT     g
#define NS_FMT_DOUBLE_COMPACT          NS_MAKE_FMT( "", NS_FMT_TYPE_DOUBLE_COMPACT )
#define NS_FMT_TYPE_DOUBLE_COMPACT_UC  G
#define NS_FMT_DOUBLE_COMPACT_UC       NS_MAKE_FMT( "", NS_FMT_TYPE_DOUBLE_COMPACT_UC )

#define NS_FMT_TYPE_POINTER        p
#define NS_FMT_POINTER             NS_MAKE_FMT( "", NS_FMT_TYPE_POINTER )
#define NS_FMT_TYPE_CONSTPOINTER   NS_FMT_TYPE_POINTER
#define NS_FMT_CONSTPOINTER        NS_MAKE_FMT( "", NS_FMT_TYPE_CONSTPOINTER )

#define NS_FMT_TYPE_UNICHAR  c
#define NS_FMT_UNICHAR       NS_MAKE_FMT( "", NS_FMT_TYPE_UNICHAR )

#define NS_FMT_UNICHAR_QUOTED\
	NS_STRING_QUOTATION NS_FMT_UNICHAR NS_STRING_QUOTATION

#define NS_FMT_TYPE_STRING    s
#define NS_FMT_STRING         NS_MAKE_FMT( "", NS_FMT_TYPE_STRING )

#define NS_FMT_STRING_QUOTED\
	NS_STRING_QUOTATION NS_FMT_STRING NS_STRING_QUOTATION

#define NS_FMT_STRING_DOUBLE_QUOTED\
	NS_STRING_DOUBLE_QUOTATION NS_FMT_STRING NS_STRING_DOUBLE_QUOTATION

NS_DECLS_END

#endif/* __NS_STD_FORMAT_H__ */
