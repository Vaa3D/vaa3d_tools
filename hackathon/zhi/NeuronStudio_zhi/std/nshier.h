#ifndef __NS_STD_HIER_H__
#define __NS_STD_HIER_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>

NS_DECLS_BEGIN

typedef struct _NsHierNode
	{
	struct _NsHierNode  *parent;
	struct _NsHierNode  *child;
	struct _NsHierNode  *sibling;
	nspointer            object;
	}
	NsHierNode;

/* Height unbalanced N-ary tree. */
typedef struct _NsHier
	{
   NsHierNode       root;
	nssize           size;
	NsFinalizeFunc   finalize_func;
	}
	NsHier;

typedef NsHierNode* nshieriter;


#define NS_HIER_FOREACH( hier, iter )\
	NS_TYPE_FOREACH(\
		(hier),\
		(iter),\
		ns_hier_begin,\
		ns_hier_end,\
		ns_hier_iter_next,\
		ns_hier_iter_not_equal\
		)


/* Pass NULL for 'finalize_func' if not needed. */
NS_IMPEXP void ns_hier_construct( NsHier *hier, NsFinalizeFunc finalize_func );
NS_IMPEXP void ns_hier_destruct( NsHier *hier );

NS_IMPEXP nssize ns_hier_size( const NsHier *hier );

#define ns_hier_is_empty( hier )\
	( 0 == ns_hier_size( (hier) ) )

NS_IMPEXP NsFinalizeFunc ns_hier_get_finalize_func( const NsHier *hier );
NS_IMPEXP void ns_hier_set_finalize_func( NsHier *hier, NsFinalizeFunc finalize_func );

NS_IMPEXP void ns_hier_clear( NsHier *hier );

/* NOTE: These begin/end pairs provide a depth-first type traversal. */
NS_IMPEXP nshieriter ns_hier_begin( const NsHier *hier );
NS_IMPEXP nshieriter ns_hier_end( const NsHier *hier );

/* Only safe to call 'insert_child' and 'has_child' on the root. */
NS_IMPEXP nshieriter ns_hier_root( const NsHier *hier );

/* The new node becomes the first child of 'I'. */
NS_IMPEXP NsError ns_hier_insert_child( NsHier *hier, nshieriter I, nspointer object );

/* The new node becomes the next sibling of 'I'. */
NS_IMPEXP NsError ns_hier_insert_sibling( NsHier *hier, nshieriter I, nspointer object );

/* NOTE: All nodes below I in the hier are erased as well! */
NS_IMPEXP void ns_hier_erase( NsHier *hier, nshieriter I );

#define ns_hier_iter_has_parent( I )\
	( NULL != (I)->parent )

#define ns_hier_iter_has_child( I )\
	( NULL != (I)->child )

#define ns_hier_iter_has_sibling( I )\
	( NULL != (I)->sibling )

#define ns_hier_iter_parent( I )\
	( (I)->parent )

#define ns_hier_iter_child( I )\
	( (I)->child )

#define ns_hier_iter_sibling( I )\
	( (I)->sibling )

#define ns_hier_iter_get_object( I )\
	( (I)->object )

#define ns_hier_iter_set_object( I, obj )\
	( (I)->object = (obj) )

#define ns_hier_iter_equal( I1, I2 )\
	( (I1) == (I2) )

#define ns_hier_iter_not_equal( I1, I2 )\
	( ! ns_hier_iter_equal( ( I1 ), ( I2 ) ) )

NS_IMPEXP nshieriter ns_hier_iter_next( nshieriter I );

NS_IMPEXP nshieriter ns_hier_iter_offset( nshieriter I, nsulong offset );

NS_DECLS_END

#endif/* __NS_STD_HIER_H__ */
