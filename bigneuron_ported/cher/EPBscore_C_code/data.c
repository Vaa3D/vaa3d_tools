/*
*	Copyrighted, Research Foundation of SUNY, 1998
*/

/* 
*	data.c - this file contains all the routines to initialize and
*	maintain information regarding the input data and the adjacency
*	tree of point v
*/

#include "stdafx.h"


#include "data.h"
#include "lkc_prototype.h"
#include "ma_prototype.h"


/*	INITIALIZE THE DATA STRUCTURE
*
*	Set the size of the data and initialize the stencil used for
*	referencing neighboring points.  The actual delegation of data
*	storage will be handled separately.
*/

data_info *data_init(int nx, int ny, int nz)
{
	data_info *dinfo;
	char	  *msg;

	int	i;
	int	isz = sizeof(int);
	int	usz = sizeof(unsigned char);
	int	disz = sizeof(data_info);

	dinfo = (data_info *) MALLOC(sizeof(data_info));
	if( MEMCHECK(dinfo,"dinfo in data_init()",disz) ) clean_up(0);

	dinfo->data = (unsigned char *) CALLOC(nx*ny*nz,usz); 
	msg = "dinfo->data in data_init()";
	if( MEMCHECK(dinfo->data,msg,nx*ny*nz*usz) ) clean_up(0);
  
	dinfo->xsize = nx;    dinfo->ysize = ny;    dinfo->zsize = nz;  

	for( i = 0;  i < nx*ny*nz;  i++ ) dinfo->data[i] = EXTERIOR;
  
	dinfo->stencil = (int *) MALLOC(NUM_NEIGHBORS*isz);
	msg = "dinfo->stencil in data_init()";
	if( MEMCHECK(dinfo->stencil,msg,NUM_NEIGHBORS*isz) ) clean_up(0);

	set_stencil(nx,nx*ny,dinfo->stencil);

	dinfo->sten6[0] = dinfo->stencil[ 4];
	dinfo->sten6[1] = dinfo->stencil[10];
	dinfo->sten6[2] = dinfo->stencil[12];
	dinfo->sten6[3] = dinfo->stencil[13];
	dinfo->sten6[4] = dinfo->stencil[15];
	dinfo->sten6[5] = dinfo->stencil[21];	

	return dinfo;
}


void	free_data_info(data_info *datainfo)
{
	int	x, y, z;

	x = datainfo->xsize;
	y = datainfo->ysize;
	z = datainfo->zsize;
	
	FREE(datainfo->stencil,NUM_NEIGHBORS*sizeof(int));
	FREE(datainfo->data,x*y*z*sizeof(unsigned char));
	FREE(datainfo,sizeof(data_info));
}


/*	INITIALIZE POINT INFORMATION 
*
*	Here we set up the data and initialize the pointers necessary to
*	examine the neighborhood of a specific point. The adjacency tree
*	information is filled in as well.
*/

point_info *point_init()
{
	point_info *newp;
	char	   *msg;

	int	i, j;

	int	pusz = sizeof(unsigned char *);
	int	 usz = sizeof(unsigned char);
  
	newp = (point_info *) MALLOC(sizeof(point_info));
	msg = "newp in point_init()";
	if( MEMCHECK(newp,msg,sizeof(point_info)) ) clean_up(0);

	newp->node_value = (unsigned char *) CALLOC(NUM_NEIGHBORS,usz);
	msg = "newp->node_value in point_init()";
	if( MEMCHECK(newp->node_value,msg,NUM_NEIGHBORS*usz) ) clean_up(0);
  
	newp->node_list = (unsigned char **) MALLOC(NUM_OCTANTS*pusz);
	msg ="newp->node_list in point_init()"; 
	if( MEMCHECK(newp->node_list,msg,NUM_OCTANTS*pusz) ) clean_up(0);

	for( i = 0;  i < NUM_OCTANTS;  i++ )
	{
	    newp->node_list[i] = (unsigned char *) MALLOC(NUM_OCTANT_NODES*usz);
	    msg = "newp->node_list[i] in point_init()";
	    if( MEMCHECK(newp->node_list[i],msg,NUM_OCTANT_NODES*usz) )
		clean_up(0);
	}

	for( i = 0;  i < NUM_OCTANTS;  i++ )	/* nlist is a global variable */
	{
		for( j = 0;  j < NUM_OCTANT_NODES;  j++ )
			newp->node_list[i][j] = nlist[i][j];
	}

	newp->octant_list = (unsigned char **) MALLOC(NUM_NEIGHBORS*pusz);
	msg = "newp->octant_list in point_init()";
	if( MEMCHECK(newp->octant_list,msg,NUM_NEIGHBORS*pusz) ) clean_up(0);

	for( i = 0;  i < NUM_NEIGHBORS;  i++ )
	{
	    newp->octant_list[i] = (unsigned char *)MALLOC(NUM_NODE_OCTANT*usz);
	    msg = "newp->octant_list[i] in point_init()";
	    if( MEMCHECK(newp->octant_list[i],msg,NUM_NODE_OCTANT*usz) )
		clean_up(0);
	}

	for ( i = 0;  i < NUM_NEIGHBORS;  i++ )	/* olist is a global variable */
	{
		for( j = 0;  j < NUM_NODE_OCTANT;  j++ )
			newp->octant_list[i][j] = olist[i][j];
	}

	return newp;
}


void	free_point_info(point_info *ptinfo)
{
	int	i;
	int	pusz = sizeof(unsigned char *);
	int	 usz = sizeof(unsigned char);
	
	for( i = 0;  i < NUM_NEIGHBORS;  i++ )
		FREE(ptinfo->octant_list[i],NUM_NODE_OCTANT*usz);
	FREE(ptinfo->octant_list,NUM_NEIGHBORS*pusz);

	for( i = 0;  i < NUM_OCTANTS;  i++ )
		FREE(ptinfo->node_list[i],NUM_OCTANT_NODES*usz);
	FREE(ptinfo->node_list,NUM_OCTANTS*pusz);

	FREE(ptinfo->node_value,NUM_NEIGHBORS*usz);

	FREE(ptinfo,sizeof(point_info));
}


/*	INITIALIZE NEIGHBORHOOD OF POINT FOR OBJECT STUDIES 
*
*	Here we fill in the status of the neighboring points using the
*	simple criteria S \intersect N(v) 
*/

void	initialize_point_neighborhood(data_info *input, point_info *point) 
{
	unsigned char	dval;

	int	i;

	for( i = 0;  i < NUM_NEIGHBORS;  i++ )
	{
		dval = input->data[ point->place+input->stencil[i] ];

		if( (dval  > MATERIAL) && (dval != EXTERIOR) )
			point->node_value[i] = VOIDVOX;
		else
			point->node_value[i] = MATERIAL;
	}
}


void	print_point_info(point_info *point)
{
	int	i;

	for( i = 0;  i < NUM_NEIGHBORS;  i++ )
	{
		if( point->node_value[i] != MATERIAL ) printf("%d ",i);
		
	}
	printf("\n");
}
