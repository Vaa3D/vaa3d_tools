#ifndef __NS_STD_TREE_H__
#define __NS_STD_TREE_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>

NS_DECLS_BEGIN

/* Internal node class. DO NOT USE! */
typedef struct _NsTreeNode
	{
	struct _NsTreeNode  *parent;
	struct _NsTreeNode  *left_child;
	struct _NsTreeNode  *right_child;
	nsuint               color;
	nspointer            object;
	}
	NsTreeNode;

typedef struct _NsTree
	{
	NsTreeNode      head;
	NsCompareFunc   compare_func;
	NsFinalizeFunc  finalize_func;
	nssize          size;
	}
	NsTree;

typedef NsTreeNode*  nstreeiter;
typedef nstreeiter   nstreereviter;


#define NS_TREE_FOREACH( tree, iter )\
	NS_TYPE_FOREACH(\
		(tree),\
		(iter),\
		ns_tree_begin,\
		ns_tree_end,\
		ns_tree_iter_next,\
		ns_tree_iter_not_equal\
		)


/* The 'compare_func' is used to sort the tree. A return value LESS than zero indicates the
	object goes "to the left" of the object it is being compare to. A return value GREATER
	THAN zero indicates "to the right". A return value of 0 indicates the objects are equal
	which defaults to going "to the right"( doesnt matter which ). */
NS_IMPEXP void ns_tree_construct
	(
	NsTree          *tree,
	NsCompareFunc    compare_func,
	NsFinalizeFunc   finalize_func
	);

NS_IMPEXP void ns_tree_destruct( NsTree *tree );

NS_IMPEXP nssize ns_tree_size( const NsTree *tree );
NS_IMPEXP nsboolean ns_tree_is_empty( const NsTree *tree );

NS_IMPEXP NsCompareFunc ns_tree_get_compare_func( const NsTree *tree );
NS_IMPEXP void ns_tree_set_compare_func( NsTree *tree, NsCompareFunc compare_func );

NS_IMPEXP NsFinalizeFunc ns_tree_get_finalize_func( const NsTree *tree );
NS_IMPEXP void ns_tree_set_finalize_func( NsTree *tree, NsFinalizeFunc finalize_func );

NS_IMPEXP void ns_tree_clear( NsTree *tree );

/* Efficient O(logN) search function. Faster than iterating through whole tree.
	Returns ns_tree_end() if not found. */
NS_IMPEXP nstreeiter ns_tree_find( const NsTree *tree, const NsCompareBind2nd *binder );
NS_IMPEXP nsboolean ns_tree_exists( const NsTree *tree, const NsCompareBind2nd *binder );

#define ns_tree_begin( tree )\
   NS_TREE_LEFT_MOST( (tree) )

#define ns_tree_end( tree )\
   ( ( nstreeiter )( &(tree)->head ) )

NS_IMPEXP nstreereviter ns_tree_rev_begin( const NsTree *tree );
NS_IMPEXP nstreereviter ns_tree_rev_end( const NsTree *tree );

/* If no error, 'R' points to the new node if not null. */
NS_IMPEXP NsError ns_tree_insert( NsTree *tree, nspointer object, nstreeiter *R );
NS_IMPEXP void ns_tree_erase( NsTree *tree, nstreeiter I );

NS_IMPEXP nstreeiter ns_tree_iter_next( nstreeiter I );
NS_IMPEXP nstreeiter ns_tree_iter_prev( nstreeiter I );

#define ns_tree_iter_get_object( I )\
	( ( I )->object )

#define ns_tree_iter_set_object( I, obj )\
	( ( I )->object = ( obj ) )

#define ns_tree_iter_equal( I1, I2 )\
	( ( I1 ) == ( I2 ) )

#define ns_tree_iter_not_equal( I1, I2 )\
	( ! ns_tree_iter_equal( ( I1 ), ( I2 ) ) )

#define ns_tree_rev_iter_next( I )\
	ns_tree_iter_prev( ( I ) )

#define ns_tree_rev_iter_prev( I )\
	ns_tree_iter_next( ( I ) )

#define ns_tree_rev_iter_get_object( I )\
	ns_tree_iter_get_object( ( I ) )

#define ns_tree_rev_iter_set_object( I, obj )\
	ns_tree_iter_set_object( ( I ), ( obj ) )

#define ns_tree_rev_iter_equal( I1, I2 )\
	ns_tree_iter_equal( ( I1 ), ( I2 ) )

#define ns_tree_rev_iter_not_equal( I1, I2 )\
	ns_tree_iter_not_equal( ( I1 ), ( I2 ) )


/* Internal. DO NOT USE! */
#define NS_TREE_LEFT_MOST( tree )\
	( ( tree )->head.left_child )

NS_DECLS_END

#endif/* __NS_STD_TREE_H__ */
