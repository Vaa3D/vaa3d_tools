#ifndef __NS_STD_PROC_DB_H__
#define __NS_STD_PROC_DB_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nshashtable.h>
#include <std/nsclosure.h>
#include <std/nsenumeration.h>
#include <std/nsascii.h>
#include <std/nsutil.h>

NS_DECLS_BEGIN

typedef enum
	{
	NS_PROC_INVOKE_MARSHAL
	}
	NsProcInvokeType;


/* TODO: Other invocation types, plug-ins. etc... */
typedef struct _NsProc
	{
	const nschar      *name;
	const nschar      *title;
	const nschar      *author;
	const nschar      *version;
	const nschar      *copyright;
	const nschar      *help;
	NsProcInvokeType   invoke_type;
	NsClosureValue    *params;
	nssize             num_params;
	NsClosureValue     ret_value;
	NsClosureMarshal   marshal;
	}
	NsProc;


typedef struct _NsProcDb
	{
	NsHashTable entries;
	}
	NsProcDb;


NS_IMPEXP NsError ns_proc_db_construct( NsProcDb *db );
NS_IMPEXP void ns_proc_db_destruct( NsProcDb *db );

NS_IMPEXP nssize ns_proc_db_size( const NsProcDb *db );

/* NOTE: 'proc' is saved by pointer only! */
NS_IMPEXP NsError ns_proc_db_register( NsProcDb *db, NsProc *proc );

NS_IMPEXP void ns_proc_db_unregister( NsProcDb *db, const nschar *name );

NS_IMPEXP nsboolean ns_proc_db_is_registered( const NsProcDb *db, const nschar *name );

NS_IMPEXP NsProc* ns_proc_db_lookup( const NsProcDb *db, const nschar *name );

/* NOTE: For simplicity, 'params' can be NULL if the procedure
	takes no parameters and 'ret_value' can be NULL if the procedure
	returns void. */
NS_IMPEXP NsError ns_proc_db_invoke
	(
	NsProcDb         *db,
	const nschar     *name,
	NsValue          *params,
	nssize            num_params,
	NsValue          *ret_value,
	NsClosureRecord  *record
	);

NS_IMPEXP NsError ns_proc_db_run
	(
	NsProcDb         *db,
	const nschar     *name,
	NsValue          *ret_value,
	NsClosureRecord  *record,
	...
	);

NS_IMPEXP NsError ns_proc_db_vrun
	(
	NsProcDb         *db,
	const nschar     *name,
	ns_va_list        args,
	NsValue          *ret_value,
	NsClosureRecord  *record
	);

/* NOTE: Cast return value from 'ns_enumeration_next_element() to
	a const NsProc*. */
NS_IMPEXP NsEnumeration* ns_proc_db_enum( const NsProcDb *db, NsEnumeration *enumeration );

NS_DECLS_END

#endif/* __NS_STD_PROC_DB_H__ */
