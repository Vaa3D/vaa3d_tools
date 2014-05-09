#ifndef __NS_STD_SET_H__
#define __NS_STD_SET_H__

#include <nsconfigure.h>
#include <std/nsmacros.h>
#include <std/nstypes.h>
#include <std/nsdebug.h>
#include <std/nserror.h>
#include <std/nsfunc.h>
#include <std/nsmemory.h>
#include <std/nstree.h>

NS_DECLS_BEGIN

typedef NsTree         NsSet;
typedef nstreeiter     nssetiter;
typedef nstreereviter  nssetreviter;


#define ns_set_construct( set, compare_func, finalize_func )\
	ns_tree_construct( (set), (compare_func), (finalize_func) )

#define ns_set_destruct( set )\
	ns_tree_destruct( (set) )

#define ns_set_size( set )\
	ns_tree_size( (set) )

#define ns_set_is_empty( set )\
	ns_tree_is_empty( (set) )

#define ns_set_clear( set )\
	ns_tree_clear( (set) )

#define ns_set_assign( lhs, rhs, assign_func )\
	ns_tree_assign( (lhs), (rhs), (assign_func) )

#define ns_set_find( set, binder )\
	ns_tree_find( (set), (binder) )

#define ns_set_exists( set, binder )\
	ns_tree_exists( (set), (binder) )

#define ns_set_begin( set )\
	ns_tree_begin( (set) )

#define ns_set_end( set )\
	ns_tree_end( (set) )

#define ns_set_rev_begin( set )\
	ns_tree_rev_begin( (set) )

#define ns_set_rev_end( set )\
	ns_tree_rev_end( (set) )

/* Only inserts the 'object' is it isnt in the set already. 'R' can be NULL. */
NS_IMPEXP NsError ns_set_insert( NsSet *set, nspointer object, nssetiter *R );

#define ns_set_erase( set, I )\
	ns_tree_erase( (set), (I) )

#define ns_set_iter_get_object( I )\
	ns_tree_iter_get_object( (I) )

#define ns_set_iter_set_object( I, object )\
	ns_tree_iter_set_object( (I), (object) )

#define ns_set_iter_equal( I1, I2 )\
	ns_tree_iter_equal( (I1), (I2) )

#define ns_set_iter_not_equal( I1, I2 )\
	ns_tree_iter_not_equal( ( I1 ), ( I2 ) )

#define ns_set_rev_iter_get_object( I )\
	ns_tree_rev_iter_get_object( (I) )

#define ns_set_rev_iter_set_object( I, object )\
	ns_tree_rev_iter_set_object( (I), (object) )

#define ns_set_rev_iter_equal( I1, I2 )\
	ns_tree_rev_iter_equal( (I1), (I2) )

#define ns_set_rev_iter_not_equal( I1, I2 )\
	ns_tree_rev_iter_not_equal( ( I1 ), ( I2 ) )

#define ns_set_iter_next( I )\
	ns_tree_iter_next( (I) )

#define ns_set_iter_prev( I )\
	ns_tree_iter_prev( (I) )

#define ns_set_iter_offset( I, offset )\
	ns_tree_iter_offset( (I), (offset) )

#define ns_set_rev_iter_next( I )\
	ns_tree_rev_iter_next( (I) )

#define ns_set_rev_iter_prev( I )\
	ns_tree_rev_iter_prev( (I) )

#define ns_set_rev_iter_offset( I, offset )\
	ns_tree_rev_iter_offset( (I), (offset) )

#define ns_set_iter_to_rev_iter( I )\
	ns_tree_iter_to_rev_iter( (I) )

#define ns_set_rev_iter_to_iter( I )\
	ns_tree_rev_iter_to_iter( (I) )

NS_DECLS_END

#endif/* __NS_STD_SET_H__ */
