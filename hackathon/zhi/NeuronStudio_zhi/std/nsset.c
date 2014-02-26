#include "nsset.h"


NsError ns_set_insert( NsSet *set, nspointer object, nssetiter *R )
	{
	NsCompareBind2nd  pred;
	NsError           error;


	ns_assert( NULL != set );

	error = ns_no_error();

	pred.second = object;
	pred.func   = set->compare_func;

	/* Check if the 'object' is already in the set before adding it. */
	if( ns_set_iter_equal( ns_set_end( set ), ns_set_find( set, &pred ) ) )
		error = ns_tree_insert( (set), object, NULL != R ? R : NULL );

	return error;
	}
