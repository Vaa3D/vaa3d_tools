#ifndef __NS_STD_FUNC_H__
#define __NS_STD_FUNC_H__

#include "nsconfigure.h"
#include "nsmacros.h"
#include "nstypes.h"
#include "nserror.h"

NS_DECLS_BEGIN

/* Can be used for generic function pointers similar to a void pointer */
typedef void ( *NsFunc )( void );

typedef nsuint ( *NsHashFunc )( nsconstpointer object );

typedef NsError ( *NsInitializeFunc )( nspointer object, nspointer user_data );
typedef void ( *NsFinalizeFunc )( nspointer object );

/* Returns < 0, 0, or > 0 */
typedef nsint ( *NsCompareFunc )( nsconstpointer l, nsconstpointer r );

/* bool = l op r */
typedef nsboolean ( *NsBinaryBooleanFunc )( nsconstpointer l, nsconstpointer r );

typedef NsBinaryBooleanFunc  NsEqualFunc;        /* == */
typedef NsBinaryBooleanFunc  NsNotEqualFunc;     /* != */
typedef NsBinaryBooleanFunc  NsLessFunc;         /*  < */
typedef NsBinaryBooleanFunc  NsGreaterFunc;      /*  > */
typedef NsBinaryBooleanFunc  NsLessEqualFunc;    /* <= */
typedef NsBinaryBooleanFunc  NsGreaterEqualFunc; /* >= */
typedef NsBinaryBooleanFunc  NsLogicalOrFunc;    /* || */
typedef NsBinaryBooleanFunc  NsLogicalAndFunc;   /* && */

/* *v = l op r */
typedef NsError ( *NsBinaryObjectFunc )( nsconstpointer l, nsconstpointer r, nspointer *v );

typedef NsBinaryObjectFunc  NsPlusFunc;       /* + */
typedef NsBinaryObjectFunc  NsMinusFunc;      /* - */
typedef NsBinaryObjectFunc  NsMultipliesFunc; /* * */
typedef NsBinaryObjectFunc  NsDividesFunc;    /* / */
typedef NsBinaryObjectFunc  NsModulusFunc;    /* % */
typedef NsBinaryObjectFunc  NsBitwiseXORFunc; /* ^ */
typedef NsBinaryObjectFunc  NsBitwiseOrFunc;  /* | */
typedef NsBinaryObjectFunc  NsBitwiseAndFunc; /* & */

/* *l = *l op r */
typedef NsError ( *NsBinaryCmpdObjectFunc )( nspointer *l, nsconstpointer r );

typedef NsBinaryCmpdObjectFunc  NsCmpdPlusFunc;       /* += */
typedef NsBinaryCmpdObjectFunc  NsCmpdMinusFunc;      /* -= */
typedef NsBinaryCmpdObjectFunc  NsCmpdMultipliesFunc; /* *= */
typedef NsBinaryCmpdObjectFunc  NsCmpdDividesFunc;    /* /= */
typedef NsBinaryCmpdObjectFunc  NsCmpdBitwiseXORFunc; /* ^= */
typedef NsBinaryCmpdObjectFunc  NsCmpdBitwiseOrFunc;  /* |= */
typedef NsBinaryCmpdObjectFunc  NsCmpdBitwiseAndFunc; /* &= */

/* bool = op o; */
typedef nsboolean ( *NsUnaryBooleanFunc )( nsconstpointer o );

typedef NsUnaryBooleanFunc  NsLogicalNotFunc;/* ! */

/* *o = op *o */
typedef NsError ( *NsUnaryObjectFunc )( nsconstpointer o, nspointer *v );

typedef NsUnaryObjectFunc  NsNegateFunc;     /* - */
typedef NsUnaryObjectFunc  NsBitwiseNotFunc; /* ~ */

/* *l = r */
typedef NsError ( *NsAssignFunc )( nspointer *l, nsconstpointer r );/* = */


typedef struct _NsCompareBind1st
	{
	nsconstpointer  first;
	NsCompareFunc   func;
	}
	NsCompareBind1st;

#define ns_compare_bind_1st( binder, object )\
	( ( ( binder )->func )( ( binder )->first, ( object ) ) )


typedef struct _NsCompareBind2nd
	{
	nsconstpointer  second;
	NsCompareFunc   func;
	}
	NsCompareBind2nd;

#define ns_compare_bind_2nd( binder, object )\
	( ( ( binder )->func )( ( object ), ( binder )->second ) )


typedef struct _NsBooleanBind1st
	{
	nsconstpointer       first;
	NsBinaryBooleanFunc  func;
	}
	NsBooleanBind1st;

#define ns_boolean_bind_1st( binder, object )\
	( ( ( binder )->func )( ( binder )->first, ( object ) ) )


typedef struct _NsBooleanBind2nd
	{
	nsconstpointer       second;
	NsBinaryBooleanFunc  func;
	}
	NsBooleanBind2nd;

#define ns_boolean_bind_2nd( binder, object )\
	( ( ( binder )->func )( ( object ), ( binder )->second ) )


NS_IMPEXP nsuint ns_pointer_hash_func( nsconstpointer );
NS_IMPEXP nsboolean ns_pointer_equal_func( nsconstpointer, nsconstpointer );

NS_DECLS_END

#endif/* __NS_STD_FUNC_H__ */
