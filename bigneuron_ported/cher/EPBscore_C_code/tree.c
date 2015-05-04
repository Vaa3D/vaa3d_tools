/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"
#include "tree.h"
#include "ma_prototype.h"

void	set_2D_restricted_stencil(int nx, int nxy, int *sten)
{
	int	i, nxm1, nxp1;

	nxm1 = nx-1;		nxp1 = nx+1;

	for( i = 0;  i < 9;  i++ ) sten[i] = 0;
	sten[ 9] = -nxp1;      sten[10] = -nx;      sten[11] = -nxm1;
	sten[12] = -1;                              sten[13] =  1;
	sten[14] =  nxm1;      sten[15] =  nx;      sten[16] =  nxp1;
	for( i = 17;  i < 26;  i++ ) sten[i] = 0;
}


NODE	*set_child_node(int ind, unsigned char burn_num, NODE *parent)
{
	NODE	*child;

	int	Nsz = sizeof(NODE);

	child = (NODE *)MALLOC(Nsz);
	if(MEMCHECK(child,"child in set_child_node()",Nsz)) clean_up(0);
	
	child->ind      = ind;
	child->burn_num = burn_num;
	child->parent   = parent;
	child->children = NULL;
	child->num_c    = 0;

	return child;
}


short	num_children(int ind, short *pma, int *stencil)
{
	int	i;
	int	cind;
	short	num_c;

	num_c = 0;
	for( i = 0;  i < 26;  i++ )
	{
		cind = ind+stencil[i];
		if( pma[cind] == 99 || pma[cind] == 101) num_c++;
	}
	return num_c;
}


void	add_children_to_tree(NODE *par, short *pma, unsigned char *burn_dat,
							 int *stencil)
{
	int	i;
	int	pind, cind;
	short	num_c;
	NODE	**ppc;

	int	pNsz = sizeof(NODE *);

	pind = par->ind;
	
	num_c = num_children(pind,pma,stencil);		par->num_c = num_c;
	if( num_c == 0 ) return;

	par->children = (NODE **)MALLOC(num_c*pNsz);
	if( MEMCHECK(par->children,"par->children in ad_children_to_tree()",
							num_c*pNsz) ) clean_up(0);

	ppc = par->children;
	for( i = 0;  i < 26;  i++ )
	{
		cind = pind+stencil[i];
		if( pma[cind] == 99 )
		{
			*ppc = set_child_node(cind,burn_dat[cind],par);
			ppc++;

			pma[cind] = 1;	/* Prevents loops in tree */
		}
		else if( pma[cind] == 101 )
		{
			*ppc = set_child_node(cind,burn_dat[cind],par);
			ppc++;

			pma[cind] = 2;	/* Prevents loops in tree */
		}
	}
}


NODE	*make_BF_tree(short *pma, unsigned char *burn_dat, int rind,
					  int *stencil, int NXY, int NX)
{
	NODE	*root, *par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	i, j, k;

	int	pNsz = sizeof(NODE *);

		/* set root */
	root = set_child_node(rind,burn_dat[rind],(NODE *)NULL);
	     if( pma[rind] ==  99 ) pma[rind] = 1;
	else if( pma[rind] == 101 ) pma[rind] = 2;

		/* set up for recursive tree formation */
	level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in make_BF_tree()",
					level_n_children*pNsz) ) clean_up(0);
	
	level_n_nodes[0] = root;

	/*******************
	PRINTF("\n\t\tNew Tree\n\n");
	print_level_nodes(level_n_children,level_n_nodes,NXY,NX);
	*******************/

		/* recursive BF tree formation */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		add_children_to_tree(par,pma,burn_dat,stencil);
		level_np1_children += par->num_c;
	}

	if( level_np1_children == 0 ) goto done_tree;

	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,"level_np1_nodes in make_BF_tree()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}

	/*******************
	print_level_nodes(level_np1_children,level_np1_nodes,NXY,NX);
	*******************/

	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes    = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
	return root;
}


TREE_LIST *set_pma_trees(short *pma, unsigned char *burn_dat, int nx,
						 int nxy, int nxyz)
{
	int	   TLsz = sizeof(TREE_LIST);

	int	   stencil[26];
	int	   i;
	TREE_LIST *new_tree;
	TREE_LIST *trees  = NULL;
	TREE_LIST *ptrees = NULL;

	set_stencil(nx,nxy,stencil);

	for( i = 0;  i < nxyz;  i++ )
	{
		if( pma[i] == 99 || pma[i] == 101)
		{
			new_tree = (TREE_LIST *)MALLOC(TLsz);
			if(MEMCHECK(new_tree,"new_tree in set_pma_trees()",TLsz))
				clean_up(0);
			
			new_tree->root = make_BF_tree(pma,burn_dat,i,stencil,
						      		     nxy,nx);
			new_tree->next = NULL;

			if( ptrees == NULL )
			{
				trees = new_tree;
				ptrees = trees;
			}
			else
			{
				ptrees->next = new_tree;
				ptrees = ptrees->next;
			}
		}
	}
	return trees;
}


void	print_node(NODE *nod, int NXY, int NX)
{
	int	ind, ix, iy, iz, i;

	ind = nod->ind;
	iz = ind/NXY;	ind = ind - iz*NXY;
	iy = ind/NX;	ix  = ind - iy*NX;
	PRINTF("\nNode: (%d,%d,%d)  burn_num %d,  ",ix,iy,iz,nod->burn_num);

	if( nod->parent == NULL ) PRINTF("par(0)");
	else
	{
		ind = nod->parent->ind;
		iz = ind/NXY;	ind = ind - iz*NXY;
		iy = ind/NX;	ix  = ind - iy*NX;
		PRINTF("par(%d,%d,%d)",ix,iy,iz);
	}

	if( nod->num_c == 0 ) PRINTF("    children: 0\n\n");
	else
	{
		PRINTF("\n    children: ");
		for( i = 0;  i < nod->num_c;  i++ )
		{
			ind = (nod->children[i])->ind;
			iz = ind/NXY;	ind = ind - iz*NXY;
			iy = ind/NX;	ix  = ind - iy*NX;
			PRINTF("(%d,%d,%d) ",ix,iy,iz);
		}
		PRINTF("\n");
	}
}


void	print_level_nodes(short num_nodes, NODE **node_list, int NXY,
						  int NX)
{
	short	i;
	int	ind, ix, iy, iz;

	PRINTF("\n\t\tNext level: %d nodes\n\n",num_nodes);
	for( i = 0;  i < num_nodes;  i++ )
	{
		ind = (node_list[i])->ind;
		iz = ind/NXY;	ind = ind - iz*NXY;
		iy = ind/NX;	ix  = ind - iy*NX;
		PRINTF("(%d,%d,%d) ",ix,iy,iz);
		if( i%8 == 0 ) PRINTF("\n");
	}
	PRINTF("\n");
}


void	print_tree_BF(NODE *root, int NXY, int NX)
{
	NODE	*par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	i, j, k;

	int	pNsz = sizeof(NODE *);

		/* set up for recursive printing by level */
	level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in print_tree_BF()",
					level_n_children*pNsz) ) clean_up(0);
	level_n_nodes[0] = root;

		/* recursive BF tree printing */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )	/* print level n nodes */
	{
		par = level_n_nodes[i];
		print_node(par,NXY,NX);
		level_np1_children += par->num_c;
	}

	if( level_np1_children == 0 ) goto done_tree;

 			/* set level np1 list */
	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,"level_np1_nodes in print_tree_BF()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}
	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
}


void	print_TREE_LIST(TREE_LIST *trees, int NXY, int NX)
{
	TREE_LIST *ptrees = trees;
	int	   tree_num = 0;

	while( ptrees != NULL )
	{
		PRINTF("\n\t\tTREE %d\n\n",tree_num);
		print_tree_BF(ptrees->root,NXY,NX);
		ptrees = ptrees->next;
		tree_num++;
	}
}


void	print_TREE_LIST_info(TREE_LIST *trees)
{
	TREE_LIST *ptrees = trees;
	int	   tree_num = 0;
	int	   num_nodes;

	PRINTF("\nTREE\tnum_nodes\n----------------\n");
	for( ptrees = trees;  ptrees != NULL;  ptrees = ptrees->next )
	{
		tree_num++;
		num_nodes = BF_tree_count_nodes(ptrees->root);
		PRINTF("%-4d\t%-4d\n",tree_num,num_nodes);
	}
	PRINTF("\n");
}

/*
void	locate_trees(TREE_LIST *trees, FILE *fp, int NXY, int NX)
{
	TREE_LIST *ptrees = trees;
	int	   num_tree, i, num_nodes;
	int	   min_ix, max_ix, min_iy, max_iy, min_iz, max_iz;

	num_tree = 0;
	for( ptrees = trees;  ptrees != NULL;  ptrees = ptrees->next ) num_tree++;

	for( ptrees = trees, i = 1;  ptrees != NULL;  ptrees = ptrees->next, i++ )
	{
		num_nodes = BF_tree_limits(ptrees->root,&min_ix,&max_ix,
				&min_iy,&max_iy,&min_iz,&max_iz,NXY,NX);
		fPRINTF(fp,"nodes %d  x(%d -> %d)  y(%d -> %d)  z(%d ->%d)\n",
			num_nodes,min_ix,max_ix,min_iy,max_iy,min_iz,max_iz);
	}
	
}

*/
void	echo_int_list(int n, int *list, char *name)
{
	int	i;

	PRINTF("%s list\n",name);
	for( i = 0;  i < n;  i++ ) PRINTF("%d\n",list[i]);
	PRINTF("\n");
}


void	compress_int_list(int *n, int *ulist, int *clist)
{
	int	i, last_ind, last_val, count;

	last_ind = 0;	count = 1;
	last_val = ulist[0];
	for( i = 1;  i < *n;  i++ )
	{
		if( ulist[i] == last_val )
		{
			count++;
			continue;
		}
		ulist[last_ind] = last_val;
		clist[last_ind] = count;
		last_ind++;
		last_val = ulist[i];
		count = 1;
	}
	ulist[last_ind] = last_val;
	clist[last_ind] = count;
	*n = last_ind+1;
}


int	BF_tree_count_nodes(NODE *root)
{
	NODE	*par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	num_children;
	int	i, j, k;

	int	pNsz = sizeof(NODE *);

		/* set up for recursive access by level */
	num_children = level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in BF_tree_count_nodes()",
					level_n_children*pNsz) ) clean_up(0);
	level_n_nodes[0] = root;

		/* recursive BF tree access */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )	/* access level n nodes */
	{
		par = level_n_nodes[i];
		level_np1_children += par->num_c;
	}

	if( level_np1_children == 0 ) goto done_tree;
	num_children += level_np1_children;

 			/* set level np1 list */
	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,"level_np1_nodes in BF_tree_count_nodes()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}
	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
	return num_children;
}


int	BF_tree_limits(NODE *root, int *ix_min, int *ix_max, int *iy_min,
				   int *iy_max, int *iz_min, int *iz_max, int NXY, int NX)
{
	NODE	*par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	num_children;
	int	i, j, k, ind;
	int	ix, iy, iz;

	int	pNsz = sizeof(NODE *);

		/* set up for recursive access by level */
	num_children = level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in BF_tree_limits()",
					level_n_children*pNsz) ) clean_up(0);
	level_n_nodes[0] = root;

	par = level_n_nodes[0];
	ind = par->ind;
	iz = ind/NXY;   ind = ind - iz*NXY;
	iy = ind/NX;    ix  = ind - iy*NX;

	*ix_min = *ix_max = ix;
	*iy_min = *iy_max = iy;
	*iz_min = *iz_max = iz;

		/* recursive BF tree access */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )	/* access level n nodes */
	{
		par = level_n_nodes[i];

		ind = par->ind;
		iz = ind/NXY;   ind = ind - iz*NXY;
		iy = ind/NX;    ix  = ind - iy*NX;

		if     ( ix < *ix_min ) *ix_min = ix;
		else if( ix > *ix_max ) *ix_max = ix;
		if     ( iy < *iy_min ) *iy_min = iy;
		else if( iy > *iy_max ) *iy_max = iy;
		if     ( iz < *iz_min ) *iz_min = iz;
		else if( iz > *iz_max ) *iz_max = iz;

		level_np1_children += par->num_c;
	}

	if( level_np1_children == 0 ) goto done_tree;
	num_children += level_np1_children;

 			/* set level np1 list */
	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,"level_np1_nodes in BF_tree_limits()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}
	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
	return num_children;
}


void	tree_2_pma_driver(TREE_LIST *tree_list, short *pma)
{
	TREE_LIST *ptree = tree_list;

	int	   num_nodes;

	for( ptree = tree_list;  ptree != NULL;  ptree = ptree->next )
	{
		num_nodes = BF_tree_count_nodes(ptree->root);

		if( num_nodes > 600 ) continue;

		tree_2_pma(ptree->root,pma);
	}
}


void	tree_2_pma(NODE *root, short *pma)
{
	NODE	*par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	i, j, k;

	int	pNsz = sizeof(NODE *);

		/* set up for recursive access by level */
	level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in tree_2_pma()",
					level_n_children*pNsz) ) clean_up(0);
	level_n_nodes[0] = root;

		/* recursive BF tree access */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )	/* access level n nodes */
	{
		par = level_n_nodes[i];
		level_np1_children += par->num_c;
		pma[par->ind] = 99;
	}

	if( level_np1_children == 0 ) goto done_tree;

 			/* set level np1 list */
	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,"level_np1_nodes in BF_tree_limits()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}
	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
	return;
}


int	BF_trim_tree_count_nodes(NODE *root, short *pma)
{
	NODE	*par;
	NODE	**level_n_nodes,  **level_np1_nodes;

	short	level_n_children, level_np1_children;
	int	num_children, num_trim_children;
	int	i, j, k;

	int	pNsz = sizeof(NODE *);

		/* set up for recursive access by level */
	num_children = level_n_children = 1;
	level_n_nodes = (NODE **)MALLOC(level_n_children*pNsz);
	if( MEMCHECK(level_n_nodes,"level_n_nodes in BF_trim_tree_count_nodes()",
					level_n_children*pNsz) ) clean_up(0);
	level_n_nodes[0] = root;
	num_trim_children = 0;
	
		/* recursive BF tree access */
    new_level:

	level_np1_children = 0;
	for( i = 0;  i < level_n_children;  i++ )    /* access level n nodes */
	{
		par = level_n_nodes[i];
		if(pma[par->ind] == 1) num_trim_children += 1;
		level_np1_children += par->num_c;
	}

	if( level_np1_children == 0 ) goto done_tree;
	num_children += level_np1_children;

 			/* set level np1 list */
	level_np1_nodes = (NODE **)MALLOC(level_np1_children*pNsz);
	if( MEMCHECK(level_np1_nodes,
			"level_np1_nodes in BF_trim_tree_count_nodes()",
					  level_np1_children*pNsz) ) clean_up(0);
	for( i = 0, k = 0;  i < level_n_children;  i++ )
	{
		par = level_n_nodes[i];
		for( j = 0;  j < par->num_c;  j++, k++ )
			level_np1_nodes[k] = (par->children)[j];
	}
	FREE(level_n_nodes,level_n_children*pNsz);
	level_n_nodes = level_np1_nodes;
	level_n_children = level_np1_children;
	goto new_level;

    done_tree:
	FREE(level_n_nodes,level_n_children*pNsz);
	return num_trim_children;
}


void	free_tree_list(TREE_LIST *tl_ptr)
{
	if( tl_ptr->next != NULL ) free_tree_list(tl_ptr->next);
	free_tree(tl_ptr);
}


void	free_tree(TREE_LIST *t_ptr)
{
	free_node(t_ptr->root);
	FREE(t_ptr,sizeof(TREE_LIST));
}


void	free_node(NODE *n_ptr)
{
	int	num_c = n_ptr->num_c;
  	int	i;
	int	pNsz = sizeof(NODE *);

	if( num_c > 0 )
	{
		for( i = 0;  i < num_c;  i++ ) free_node(n_ptr->children[i]);
		FREE(n_ptr->children,num_c*pNsz);
	}

	FREE(n_ptr,sizeof(NODE));
}
