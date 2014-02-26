#include "nshier.h"


void ns_hier_construct( NsHier *hier, NsFinalizeFunc finalize_func )
	{
	ns_assert( NULL != hier );

	hier->size          = 0;
	hier->finalize_func = finalize_func;

	hier->root.parent  = NULL;
	hier->root.child   = NULL;
	hier->root.sibling = NULL;
	}


void ns_hier_destruct( NsHier *hier )
   {
	ns_assert( NULL != hier );
	ns_hier_clear( hier );
	}


NS_PRIVATE nshieriter _ns_hier_left_most( nshieriter I )
	{
	ns_assert( NULL != I );

	while( NULL != I->child )
		I = I->child;

	return I;
	}


NS_PRIVATE void _ns_hier_erase_descendants( NsHier *hier, nshieriter I )
	{
   nshieriter current, next, end;


   ns_assert( NULL != hier );
	ns_assert( NULL != I );

	if( ! ns_hier_iter_has_child( I ) )
		return;

   current = _ns_hier_left_most( I );
   end     = I;

   while( ns_hier_iter_not_equal( current, end ) )
      {
      next = ns_hier_iter_next( current );

      if( NULL != hier->finalize_func )
         ( hier->finalize_func )( current->object );

      ns_delete( current );
		--(hier->size);

      current = next;
      }

	I->child = NULL;
	}


NS_PRIVATE NsHierNode* _ns_hier_new_node
	( 
	NsHier      *hier,
	NsHierNode  *parent,
	NsHierNode  *child,
	NsHierNode  *sibling,
	nspointer    object
	)
	{
	NsHierNode *node;

	ns_assert( NULL != hier );

   node = ns_new( NsHierNode );

   if( NULL == node )
      return NULL;

   node->parent  = parent;
   node->child   = child;
   node->sibling = sibling;
	node->object  = object;

   ++(hier->size);

	return node;
	}


nssize ns_hier_size( const NsHier *hier )
	{
	ns_assert( NULL != hier );
	return hier->size;
	}


NsFinalizeFunc ns_hier_get_finalize_func( const NsHier *hier )
	{
	ns_assert( NULL != hier );
	return hier->finalize_func;
	}


void ns_hier_set_finalize_func( NsHier *hier, NsFinalizeFunc finalize_func )
	{
	ns_assert( NULL != hier );
	hier->finalize_func = finalize_func;
	}


void ns_hier_clear( NsHier *hier )
	{
	ns_assert( NULL != hier );
	_ns_hier_erase_descendants( hier, &hier->root );
	}


nshieriter ns_hier_begin( const NsHier *hier )
	{
	ns_assert( NULL != hier );
	return _ns_hier_left_most( ( nshieriter )&hier->root );
	}


nshieriter ns_hier_end( const NsHier *hier )
	{
	ns_assert( NULL != hier );
	return ( nshieriter )&hier->root;
	}


nshieriter ns_hier_root( const NsHier *hier )
	{
	ns_assert( NULL != hier );
	return ( nshieriter )&hier->root;
	}


NsError ns_hier_insert_child( NsHier *hier, nshieriter I, nspointer object )
	{
   NsHierNode *node;

   ns_assert( NULL != hier );
	ns_assert( NULL != I );

   if( NULL == ( node =_ns_hier_new_node( hier, I, NULL, I->child, object ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   I->child = node;
   return ns_no_error();
	}


NsError ns_hier_insert_sibling( NsHier *hier, nshieriter I, nspointer object )
	{
   NsHierNode *node;

   ns_assert( NULL != hier );
	ns_assert( NULL != I );

   if( NULL == ( node =_ns_hier_new_node( hier, I->parent, NULL, I->sibling, object ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   I->sibling = node;
   return ns_no_error();
	}


void ns_hier_erase( NsHier *hier, nshieriter I )
	{
	ns_assert( NULL != hier );
	ns_assert( NULL != I );

	_ns_hier_erase_descendants( hier, I );

	ns_assert( NULL != I->parent );
	ns_assert( NULL != I->parent->child );

	/* Splice 'I' out of the hier. */
	if( I->parent->child == I )
		I->parent->child = I->sibling;
	else
		{
		nshieriter C = I->parent->child;
		
		while( C->sibling != I )
			{
			C = C->sibling;
			ns_assert( NULL != C );
			}

		C->sibling = I->sibling;
		}

   if( NULL != hier->finalize_func )
      ( hier->finalize_func )( I->object );

   ns_delete( I );
	--(hier->size);
	}


nshieriter ns_hier_iter_next( nshieriter I )
	{
   ns_assert( NULL != I );

   I = NULL != I->sibling ? _ns_hier_left_most( I->sibling ) : I->parent;
	return I;
	}


nshieriter ns_hier_iter_offset( nshieriter I, nsulong offset )
	{
	ns_assert( NULL != I );
	
	for( ; 0 < offset; --offset )
		I = ns_hier_iter_next( I );

	return I;
	}
