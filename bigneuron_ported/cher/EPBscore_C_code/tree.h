/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#ifndef _tree_h
#define _tree_h

struct _NODE
{
	int             ind;
	unsigned char	burn_num;
	struct _NODE  *parent;
	struct _NODE **children;
	short 	        num_c;
};

typedef struct _NODE NODE;


struct _TREE_LIST
{
	struct _NODE *root;
	struct _TREE_LIST *next;
};

typedef struct _TREE_LIST TREE_LIST;
#endif	/* _tree_h */
