#ifndef __NS_STD_CLOSURE_H__
#define __NS_STD_CLOSURE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nsvalue.h>
#include <std/nsascii.h>

NS_DECLS_BEGIN

typedef struct _NsClosureValue
	{
	const nschar  *name;
	const nschar  *type;
	}
	NsClosureValue;


/* Forward declaration. */
struct _NsClosure;
typedef struct _NsClosure NsClosure;


typedef NsError ( *NsClosureMarshal )( NsClosure*, NsValue*, nssize, NsValue* );


struct _NsClosure
	{
	NsClosureMarshal   marshal;
	NsClosureValue    *formal_params;
	NsClosureValue     formal_ret_value;
	NsValue           *actual_params;
	nssize             num_params;
	nspointer          user_data;
	NsFunc             user_func;
	};


/* A record of one particular invokation of a closure. */
typedef struct _NsClosureRecord
	{
	nschar          *description;
	NsClosureValue  *formal_params;
	NsClosureValue   formal_ret_value;
	NsValue         *actual_params;
	NsValue          actual_ret_value;
	nssize           num_params;
	nspointer        user_data;
	}
	NsClosureRecord;

NS_IMPEXP NsError ns_closure_record_construct
	(
	NsClosureRecord  *record,
	const nschar     *description,
	nspointer         user_data
	);

NS_IMPEXP void ns_closure_record_destruct( NsClosureRecord *record );

NS_IMPEXP const nschar* ns_closure_record_description( const NsClosureRecord *record );

NS_IMPEXP nssize ns_closure_record_num_params( const NsClosureRecord *record );

NS_IMPEXP const NsClosureValue* ns_closure_record_formal_params( const NsClosureRecord *record );
NS_IMPEXP const NsClosureValue* ns_closure_record_formal_param( const NsClosureRecord *record, nssize i );
NS_IMPEXP const NsClosureValue* ns_closure_record_formal_param_by_name( const NsClosureRecord *record, const nschar *name );

NS_IMPEXP const NsValue* ns_closure_record_actual_params( const NsClosureRecord *record );
NS_IMPEXP const NsValue* ns_closure_record_actual_param( const NsClosureRecord *record, nssize i );
NS_IMPEXP const NsValue* ns_closure_record_actual_param_by_name( const NsClosureRecord *record, const nschar *name );

NS_IMPEXP const NsClosureValue* ns_closure_record_formal_ret_value( const NsClosureRecord *record );
NS_IMPEXP const NsValue* ns_closure_record_actual_ret_value( const NsClosureRecord *record );


/* NOTE: For all functions below, 'formal_params' can be NULL if
	the function takes no parameters. 'ret_value' can be
	NULL if the return type is void. */

NS_IMPEXP NsError ns_closure_construct
	(
	NsClosure             *closure,
	NsClosureMarshal       marshal,
	const NsClosureValue  *formal_params,
	nssize                 num_params,
	const NsClosureValue  *formal_ret_value
	);

NS_IMPEXP void ns_closure_destruct( NsClosure *closure );


/* 'actual_ret_value' can be NULL if type void. For the
	following functions 'record' can be NULL. Errors on
	creating the 'record' are ignored. */

NS_IMPEXP NsError ns_closure_invoke
	(
	NsClosure        *closure,
	NsValue          *actual_params,
	nssize            num_params,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record
	);

NS_IMPEXP NsError ns_closure_run
	(
	NsClosure        *closure,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record,
	...
	);

NS_IMPEXP NsError ns_closure_vrun
	(
	NsClosure        *closure,
	ns_va_list        args,
	NsValue          *actual_ret_value,
	NsClosureRecord  *record
	);

/* Careful with set_marshal()! */
NS_IMPEXP NsClosureMarshal ns_closure_get_marshal( const NsClosure *closure );
NS_IMPEXP void ns_closure_set_marshal( NsClosure *closure, NsClosureMarshal marshal );

NS_IMPEXP nspointer ns_closure_get_user_data( const NsClosure *closure );
NS_IMPEXP void ns_closure_set_user_data( NsClosure *closure, nspointer user_data );

NS_IMPEXP NsFunc ns_closure_get_user_func( const NsClosure *closure );
NS_IMPEXP void ns_closure_set_user_func( NsClosure *closure, NsFunc user_func );

NS_DECLS_END

#endif /* __NS_STD_CLOSURE_H__ */
