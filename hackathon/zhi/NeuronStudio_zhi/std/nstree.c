#include "nstree.h"


enum{ NS_TREE_RED, NS_TREE_BLACK };


/* NOTE: This node is similar to a NULL but is necessary to
	satisfy the definition of a red-black tree. */

NS_PRIVATE NsTreeNode _ns_tree_nil = { NULL, NULL, NULL, NS_TREE_BLACK, NULL };

#define NS_TREE_NIL\
	( &_ns_tree_nil )


/* NOTE: The head's left child pointer always points to the deepest, left-most
   node in the tree while the head's right pointer always points to the 
   deepest, right-most node in the tree. The head's parent pointer always
   points to the root node of the tree. */

#define NS_TREE_RIGHT_MOST( tree )\
	( ( tree )->head.right_child )

#define NS_TREE_ROOT( tree )\
	( ( tree )->head.parent )


NS_PRIVATE void _ns_tree_init( NsTree *tree )
	{
	ns_assert( NULL != tree );

	/* NOTE: If the tree is empty then the heads pointers are
		set as the following. */

   tree->head.parent      = NS_TREE_NIL;
   tree->head.left_child  = &tree->head;
   tree->head.right_child = &tree->head;
   tree->head.color       = NS_TREE_RED;

	tree->size = 0;
	}


NS_PRIVATE void _ns_tree_rotate_left( NsTree *tree, nstreeiter P )
   {
   nstreeiter C = P->right_child;

   P->right_child = C->left_child;

   if( C->left_child != NS_TREE_NIL )
      C->left_child->parent = P;

   C->parent = P->parent;

   if( P == NS_TREE_ROOT( tree ) )
      NS_TREE_ROOT( tree ) = C;
   else if( P == P->parent->left_child )
      P->parent->left_child = C;
   else
      P->parent->right_child = C;

   C->left_child = P;
   P->parent     = C;
   }


NS_PRIVATE void _ns_tree_rotate_right( NsTree *tree, nstreeiter P )
   {
   nstreeiter C = P->left_child;
          
   P->left_child = C->right_child;
                
   if( C->right_child != NS_TREE_NIL )
      C->right_child->parent = P;

   C->parent = P->parent;

   if( P == NS_TREE_ROOT( tree ) )
      NS_TREE_ROOT( tree ) = C;
   else if( P == P->parent->right_child )
      P->parent->right_child = C;
   else
      P->parent->left_child = C;
                
   C->right_child = P;
   P->parent      = C;
   }


void ns_tree_construct
	(
	NsTree          *tree,
	NsCompareFunc    compare_func,
	NsFinalizeFunc   finalize_func
	)
	{
   ns_assert( NULL != tree );
	ns_assert( NULL != compare_func );

	_ns_tree_init( tree );

	tree->compare_func  = compare_func;
	tree->finalize_func = finalize_func;
	}


void ns_tree_destruct( NsTree *tree )
	{
	ns_assert( NULL != tree );
	ns_tree_clear( tree );
	}


nssize ns_tree_size( const NsTree *tree )
	{
	ns_assert( NULL != tree );
	return tree->size;
	}


nsboolean ns_tree_is_empty( const NsTree *tree )
	{
   ns_assert( NULL != tree );
	return 0 == ns_tree_size( tree );
	}


NsCompareFunc ns_tree_get_compare_func( const NsTree *tree )
	{
	ns_assert( NULL != tree );
	return tree->compare_func;
	}


void ns_tree_set_compare_func( NsTree *tree, NsCompareFunc compare_func )
	{
	ns_assert( NULL != tree );
	ns_assert( NULL != compare_func );

	tree->compare_func = compare_func;
	}


NsFinalizeFunc ns_tree_get_finalize_func( const NsTree *tree )
	{
	ns_assert( NULL != tree );
	return tree->finalize_func;
	}


void ns_tree_set_finalize_func( NsTree *tree, NsFinalizeFunc finalize_func )
	{
	ns_assert( NULL != tree );
	tree->finalize_func = finalize_func;
	}


NS_PRIVATE void _ns_tree_clear( NsTree *tree, NsTreeNode *node )
	{
	/* NOTE: Recursive depth-first traversal probably faster than
		calling erase on all the nodes. */

	if( NS_TREE_NIL != node )
		{
		_ns_tree_clear( tree, node->left_child );
		_ns_tree_clear( tree, node->right_child );

		if( NULL != tree->finalize_func )
			( tree->finalize_func )( node->object );

		ns_delete( node );
		}
	}


void ns_tree_clear( NsTree *tree )
	{
   ns_assert( NULL != tree );

	_ns_tree_clear( tree, NS_TREE_ROOT( tree ) );
	_ns_tree_init( tree );
	}


nstreeiter ns_tree_find( const NsTree *tree, const NsCompareBind2nd *binder )
	{
	nstreeiter  I;
	nsint       compare_value;


	ns_assert( NULL != tree );
	ns_assert( NULL != binder );

	I = NS_TREE_ROOT( tree );

	while( NS_TREE_NIL != I )
		{
		compare_value = ns_compare_bind_2nd( binder, I->object );

		if( 0 == compare_value )
			return I;
		else if( compare_value < 0 )
			I = I->right_child;
		else
			I = I->left_child;
		}

	return ns_tree_end( tree );
	}


nsboolean ns_tree_exists( const NsTree *tree, const NsCompareBind2nd *binder )
	{
	ns_assert( NULL != tree );
	ns_assert( NULL != binder );

	return ns_tree_iter_not_equal(
				ns_tree_find( tree, binder ),
				ns_tree_end( tree )
				);
	}


nstreereviter ns_tree_rev_begin( const NsTree *tree )
	{
	ns_assert( NULL != tree );
   return NS_TREE_RIGHT_MOST( tree );
	}


nstreereviter ns_tree_rev_end( const NsTree *tree )
	{
	ns_assert( NULL != tree );
   return ( nstreeiter )( &tree->head );
	}


NsError ns_tree_insert( NsTree *tree, nspointer object, nstreeiter *R )
	{
   nstreeiter   parent;
	nstreeiter   current;
	NsTreeNode  *insert;


   ns_assert( NULL != tree );

   current = NS_TREE_ROOT( tree );
   parent  = &tree->head;

   /* Traverse the tree and find where to insert the new node. */
   while( NS_TREE_NIL != current )
      {
      parent  = current;
      current = ( ( tree->compare_func )( object, current->object ) < 0 ) 
                ? current->left_child : current->right_child;
      }

	/* Allocate a new node. */
   if( NULL == ( insert = ns_new( NsTreeNode ) ) )
		return ns_error_nomem( NS_ERROR_LEVEL_CRITICAL, NS_MODULE );

   insert->parent      = parent;
   insert->color       = NS_TREE_RED;
   insert->left_child  = NS_TREE_NIL;
   insert->right_child = NS_TREE_NIL;
   insert->object      = object;

   ++tree->size;

   /* Insert the node as a child of the parent. The parent being the head node
      is a special case, i.e. the tree is empty, so need to initalize the root,
      right-most, and left-most pointers( which are really just the head nodes
      parent, right child, and left child pointers respectively ). */

   if( parent == &tree->head )
      {
      NS_TREE_ROOT( tree )       = insert;
      NS_TREE_RIGHT_MOST( tree ) = insert;
      NS_TREE_LEFT_MOST( tree )  = insert;
      }
   else if( ( tree->compare_func )( object, parent->object ) < 0 )
      {
      parent->left_child = insert;
                      
      if( parent == NS_TREE_LEFT_MOST( tree ) )
         NS_TREE_LEFT_MOST( tree ) = insert;
      }
   else
      {
      parent->right_child = insert;

      if( parent == NS_TREE_RIGHT_MOST( tree ) )
         NS_TREE_RIGHT_MOST( tree ) = insert;
      }

   /* Have to keep the tree black-height balanced.

		NOTE: A red node which has a child who is also red indicates the tree
      needs to be updated to maintain height balance.

      A critical cluster is defined as a black colored node and descendant 
      red nodes that can be directly "reached" from this node, i.e cant 
      "go through" any black colored nodes.

      A critical cluster involving 4 nodes( 1 black and 3 red )
      can be fixed with simple color changes.

      A critical cluster involving 3 nodes( 1 black and 2 red )
      needs to be fixed with a rotation.
   */

   for( current = insert; current != NS_TREE_ROOT( tree ) && NS_TREE_RED == current->parent->color; )
      {
      if( current->parent == current->parent->parent->left_child )
         {
         if( NS_TREE_RED == current->parent->parent->right_child->color )/* 4 node cluster */
            {
            current->parent->color                      = NS_TREE_BLACK;
            current->parent->parent->right_child->color = NS_TREE_BLACK;
            current->parent->parent->color              = NS_TREE_RED;

            current = current->parent->parent;
            }
         else/* 3 node cluster */
            {
            if( current == current->parent->right_child )
               {
               current = current->parent;
               _ns_tree_rotate_left( tree, current );
               }
             
            current->parent->color         = NS_TREE_BLACK;
            current->parent->parent->color = NS_TREE_RED;

            _ns_tree_rotate_right( tree, current->parent->parent );
            }
         }
      else
         {
         if( NS_TREE_RED == current->parent->parent->left_child->color )/* 4 node cluster */
            {
            current->parent->color                     = NS_TREE_BLACK;
            current->parent->parent->left_child->color = NS_TREE_BLACK;
            current->parent->parent->color             = NS_TREE_RED;

            current = current->parent->parent;
            }
         else/* 3 node cluster */
            {
            if( current == current->parent->left_child )
               {
               current = current->parent;
               _ns_tree_rotate_right( tree, current );
               }
                      
            current->parent->color         = NS_TREE_BLACK;
            current->parent->parent->color = NS_TREE_RED;
                            
            _ns_tree_rotate_left( tree, current->parent->parent );
            }
         }
      }

   /* The root node should be black after every insert. */
   NS_TREE_ROOT( tree )->color = NS_TREE_BLACK;

	if( NULL != R )
		*R = insert;

   return ns_no_error();
	}


NS_PRIVATE nstreeiter _ns_tree_iter_max( nstreeiter I )
	{
	while( I->right_child != NS_TREE_NIL )
		I = I->right_child;

	return I;
	}


NS_PRIVATE nstreeiter _ns_tree_iter_min( nstreeiter I )
	{
	while( I->left_child != NS_TREE_NIL )
		I = I->left_child;

	return I;
	}


void ns_tree_erase( NsTree *tree, nstreeiter I )
	{
	nstreeiter current, sibling, erase, lock;


	lock = erase = I;

	if( NS_TREE_NIL == erase->left_child )
		current = erase->right_child;
	else if( NS_TREE_NIL == erase->right_child )
		current = erase->left_child;
	else
		{
		erase   = _ns_tree_iter_min( erase->right_child );
		current = erase->right_child;
		}

	if( erase != lock )
		{
		lock->left_child->parent = erase;
		erase->left_child        = lock->left_child;

		if( erase == lock->right_child )
			current->parent = erase;
		else
			{
			current->parent            = erase->parent;
			erase->parent->left_child  = current;
			erase->right_child         = lock->right_child;
			lock->right_child->parent  = erase;
			}

		if( NS_TREE_ROOT( tree ) == lock )
			NS_TREE_ROOT( tree ) = erase;
		else if( lock->parent->left_child == lock )
			lock->parent->left_child = erase;
		else
			lock->parent->right_child = erase;
			
		erase->parent = lock->parent;

		NS_SWAP( nsuint, erase->color, lock->color );

		erase = lock;
		}
	else
		{
		current->parent = erase->parent;

		if( NS_TREE_ROOT( tree ) == lock )
			NS_TREE_ROOT( tree ) = current;
		else if( lock->parent->left_child == lock )
			lock->parent->left_child = current;
		else
			lock->parent->right_child = current;

		if( NS_TREE_LEFT_MOST( tree ) != lock )
			;
		else if( NS_TREE_NIL == lock->right_child )
			NS_TREE_LEFT_MOST( tree ) = lock->parent;
		else
			NS_TREE_LEFT_MOST( tree ) = _ns_tree_iter_min( current );

		if( NS_TREE_RIGHT_MOST( tree ) != lock )
			;
		else if( NS_TREE_NIL == lock->left_child )
			NS_TREE_RIGHT_MOST( tree ) = lock->parent;
		else
			NS_TREE_RIGHT_MOST( tree ) = _ns_tree_iter_max( current );
		}

	if( NS_TREE_BLACK == erase->color )
		{
		while( current != NS_TREE_ROOT( tree ) && NS_TREE_BLACK == current->color )
			if( current == current->parent->left_child )
				{
				sibling = current->parent->right_child;
				
				if( NS_TREE_RED == sibling->color )
					{
					sibling->color         = NS_TREE_BLACK;
					current->parent->color = NS_TREE_RED;

					_ns_tree_rotate_left( tree, current->parent );

					sibling = current->parent->right_child;
					}

				if( NS_TREE_BLACK == sibling->left_child->color && NS_TREE_BLACK == sibling->right_child->color )
					{
					sibling->color = NS_TREE_RED;
					current        = current->parent;
					}
				else
					{
					if( NS_TREE_BLACK == sibling->right_child->color )
						{
						sibling->left_child->color = NS_TREE_BLACK;
						sibling->color             = NS_TREE_RED;

						_ns_tree_rotate_right( tree, sibling );

						sibling = current->parent->right_child;
						}

					sibling->color              = current->parent->color;
					current->parent->color      = NS_TREE_BLACK;
					sibling->right_child->color = NS_TREE_BLACK;

					_ns_tree_rotate_left( tree, current->parent );
					break;
					}
				}
			else
				{
				sibling = current->parent->left_child;

				if( NS_TREE_RED == sibling->color )
					{
					sibling->color         = NS_TREE_BLACK;
					current->parent->color = NS_TREE_RED;

					_ns_tree_rotate_right( tree, current->parent );

					sibling = current->parent->left_child;
					}

				if( NS_TREE_BLACK == sibling->right_child->color && NS_TREE_BLACK == sibling->left_child->color )
					{
					sibling->color = NS_TREE_RED;
					current        = current->parent;
					}
				else
					{
					if( NS_TREE_BLACK == sibling->left_child->color )
						{
						sibling->right_child->color = NS_TREE_BLACK;
						sibling->color              = NS_TREE_RED;

						_ns_tree_rotate_left( tree, sibling );

						sibling = current->parent->left_child;
						}

					sibling->color             = current->parent->color;
					current->parent->color     = NS_TREE_BLACK;
					sibling->left_child->color = NS_TREE_BLACK;

					_ns_tree_rotate_right( tree, current->parent );
					break;
					}
				}

		current->color = NS_TREE_BLACK;
		}

   if( NULL != tree->finalize_func )
		( tree->finalize_func )( erase->object );

	ns_delete( erase );
	--tree->size;
	}


nstreeiter ns_tree_iter_next( nstreeiter I )
	{
   ns_assert( NULL != I );

   if( NS_TREE_NIL != I->right_child )
      {
      I = I->right_child;

      while( NS_TREE_NIL != I->left_child )
         I = I->left_child;
      }
   else
      {
      nstreeiter parent;

      NS_INFINITE_LOOP
         {
         parent = I->parent;
          
         if( parent->right_child != I )
            break;

         I = parent;
         }

      /* NOTE: I->right_child can equal the parent when the tree has only
         one node. I will be pointing to the head node when the following
         code is called and we want to leave it pointing to the head so the
         "next" iterations can terminate. */
                                
      if( I->right_child != parent )
         I = parent;
      }

   return I;
	}


nstreeiter ns_tree_iter_prev( nstreeiter I )
	{
   ns_assert( NULL != I );

   if( NS_TREE_RED == I->color && I->parent->parent == I )
		I = I->right_child;
   else if( NS_TREE_NIL != I->left_child )
		{
		I = I->left_child;

		while( NS_TREE_NIL != I->right_child )
			I = I->right_child;
		}
   else
		{
		nstreeiter parent;
           
		NS_INFINITE_LOOP
			{
			parent = I->parent;

			if( parent->left_child != I )
				break;

			I = parent;
			}

      /* NOTE: I->left_child can equal the parent when the tree has only
         one node. I will be pointing to the head node when the following
         code is called and we want to leave it pointing to the head so the
         "next" iterations can terminate. */

		if( I->left_child != parent ) 
			I = parent;
		}

	return I;
	}
