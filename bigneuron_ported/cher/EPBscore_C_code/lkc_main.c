/* #include "C:\MATLAB6p1\extern\include\mex.h" */

// call thin_driver('F',file_basename);
// assume non-timeseries files;
// no file output?

/*
 *	Copyrighted, Research Foundation of SUNY, 1998
 */

#include "stdafx.h"

#include <string.h>
#include "data.h"
#include "lkc_prototype.h"
#include "ma_prototype.h"

#define	Set_voids()\
	for( j = 0;  j < num_array;  j++ )\
	{\
	    PLC = bord_array[j];\
	    if( (PLC != -1) && (DD[PLC] != MATERIAL) && (DD[PLC] != EDGE_VOID) )\
		DD[PLC] = VOIDVOX;\
	}


mxArray * mexthinmain(const mxArray *input)
{


	mxArray * input_cube; 
	mxArray * fout;
	mxArray * mx_ma_loc;
	mwSize nx,ny,nz,number_of_dims;
	const mwSize * pind;
	unsigned char *ma_bno;
	UINT32	nloc, *ma_loc;

	input_cube = mxGetCell(input,0);
	number_of_dims = mxGetNumberOfDimensions(input_cube);
	if (number_of_dims==2)
	{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=1;
	}
		else if (number_of_dims==3)
		{
		 pind = mxGetDimensions(input_cube);
		 nx=pind[0];
		 ny=pind[1];
		 nz=pind[2];
		}
		else
		mexErrMsgTxt("Input must be 2 or 3 dimensional");
	if (mxIsUint8(input_cube) == 0 )
		mexErrMsgTxt("Input must be uint8 type.");

	thin_driver(nx,ny,nz,mxGetData(input_cube), &nloc, &ma_loc, &ma_bno);

	mx_ma_loc = mxCreateNumericMatrix(nloc, 1, mxUINT32_CLASS, mxREAL);
	mxSetData(mx_ma_loc,ma_loc);
	fout = mxCreateCellMatrix(1, 1);
	mxSetCell(fout,0,mx_ma_loc);
	return fout;
}
void	thin_driver(
					int nx, int ny, int nz,
					unsigned char *burn_cube,
					int *nloc, UINT32 **ma_loc, unsigned char **ma_bno)
{
	data_info   *datainfo;
	point_info  *ptinfo;
	char tp;

	char	conn_type;

	int	num_vox_removed, tot_vox_removed;
	int	itno, done;
	int	surface, bdry_alg, iso_or_dir, end_cond, clean_ends;
	int	ok;

	int	task = ERODE;

	int	csz = sizeof(char);

	tp = 'F';		// CSHL parameter
	surface = 0;
	bdry_alg = 1;
	iso_or_dir = 2;	// new condition: retain boundary voxel
	conn_type = (char)26;
	end_cond   = 1;
	clean_ends = 1;

	thin_input_obj_data(&datainfo,tp,conn_type,bdry_alg,nx,ny,nz,burn_cube);

    /* Program terminates if at any iteration the number of voxels */
    /* deleted is 0. A voxel is deleted if the corresponding entry */
    /* in datainfo->data is set to MATERIAL			   */

    ptinfo = point_init();  /* Set voxel and octant look up arrays */
	init_deuler_table();

	itno = 0;		tot_vox_removed = 0;
	done = 0;

	ok = ( iso_or_dir == 2 && end_cond == 2 && clean_ends == 2 ) ? 1:0;

	while( !done )
	{
		if( iso_or_dir == 1 )
		{
			num_vox_removed = thin_object_dir(datainfo,ptinfo,
											  surface,bdry_alg,0,
											  itno,conn_type,1);
		}
		else
		{
			num_vox_removed = thin_object_iso(datainfo,ptinfo,
											  surface,bdry_alg,0,
											  itno,conn_type,1,end_cond);
		}
		tot_vox_removed += num_vox_removed;
		itno++;
		if( num_vox_removed == 0 )
		{
			if( ok == 1 )
			{
				ok = 0;
				end_cond = 1;
			}
			else done = 1;
		}
    }
    free_point_info(ptinfo);
	pma_output(datainfo,nloc,ma_loc,ma_bno,burn_cube);

	PRINTF("tot_vox_removed %d, times_loop %d\n", tot_vox_removed,itno);
	free_data_info(datainfo);
}


#define PE(_strng) fprintf(stderr,_strng)
#define PO(_strng) fprintf(stdout,_strng)



int	set_thin_connectivity(data_info *datainfo, point_info *ptinfo,
						  char conn_type, int iso_or_dir)
{
	char	deuler;
	int	connectivity;

	initialize_point_neighborhood(datainfo,ptinfo);
	if( conn_type == 26 && iso_or_dir == 1 )
		connectivity = local_connectivity(ptinfo);
	else if( conn_type == 26 && iso_or_dir == 2 )
	{
		deuler = calc_deuler(ptinfo->node_value,conn_type);
		if( deuler == 0 ) connectivity = local_connectivity(ptinfo);
		else		  connectivity = -1;
	}
	else
	{
		deuler = calc_deuler(ptinfo->node_value,conn_type);
		if( deuler == 0 ) connectivity = local_6_connectivity(ptinfo);
		else		  connectivity = -1;
	}
	return connectivity;
}


#define PLC ptinfo->place
#define DD datainfo->data
#define DS datainfo->stencil

#define	Reset_DD()\
{\
	if( DD[PLC] == EDGE_VOID) continue;\
	if( DD[PLC] > 30 )\
	{\
		val = DD[PLC] - 30;\
		if( val != dir )\
		{\
		    if( val == 8 ) DD[PLC] = EDGE_VOID;\
		    continue;\
		}\
		was_edge = 1;\
		ok = 1;\
	}\
	else if( DD[PLC] > 20 )\
	{\
		val = DD[PLC] - 20;\
		if( val != dir )\
		{\
		    if( val == 8 ) DD[PLC] = EDGE_VOID;\
		    continue;\
		}\
		was_edge = 1;\
	}\
	else if( DD[PLC] > 10 )\
	{\
		val = DD[PLC] - 10;\
		if( val != dir ) continue;\
		ok = 1;\
	}\
	else if( DD[PLC] > MATERIAL )\
	{\
		val = DD[PLC];\
		if( val != dir ) continue;\
	}\
	DD[PLC] = val;\
}


void	surface_thin(data_info *datainfo, point_info *ptinfo,
					 int *bord_array, int num_array, char conn_type,
					 int *num_vox_removed, int bdry_alg, unsigned char dir,
					 int iso_or_dir, int end_cond)
{
	unsigned char 	val;
	char	ok;
	int	j, connectivity, was_edge;
	
	for( j = 0;  j < num_array;  j++ )
	{
	    PLC = bord_array[j];
	    was_edge = 0;
	    if( PLC != -1 )
	    {
		if( iso_or_dir == 2 ) Reset_DD()

		connectivity = set_thin_connectivity(datainfo,ptinfo,conn_type,
							iso_or_dir);
		
		     if( (connectivity == -1) && was_edge ) DD[PLC] = EDGE_VOID;
		else if( (connectivity ==  1) && (end_cond == 1) )
		{
			if( !check_surface_edge(ptinfo) )
			{
				DD[PLC] = MATERIAL;
				bord_array[j] = -1;
				(*num_vox_removed)++;
			}
			else if( was_edge ) DD[PLC] = EDGE_VOID;
		}
		else if( (connectivity == 1) && (end_cond == 2) )
		{
			if( !mod_check_surface_edge(ptinfo,datainfo,
						    dir,bdry_alg) )
			{
				DD[PLC] = MATERIAL;
				bord_array[j] = -1;
				(*num_vox_removed)++;
			}
			else if( was_edge ) DD[PLC] = EDGE_VOID;
		}
	    }
	}
}


void	axis_thin(data_info *datainfo, point_info *ptinfo, int *bord_array,
				  int num_array, char conn_type, int *num_vox_removed,
				  int num_keep_pts, unsigned char dir, int iso_or_dir,
				  int end_cond)
{
	if( num_keep_pts > 0 )		/* axis end points given */
	{
		keep_points_axis_thin(datainfo,ptinfo,bord_array,num_array,
				      conn_type,num_vox_removed,dir,iso_or_dir);
	}
	else				/* find axis end points */
	{
	    if( end_cond == 2 )		/* arun's end point condition */
	    {
		new_end_point_axis_thin(datainfo,ptinfo,bord_array,num_array,
						conn_type,num_vox_removed,dir,
								iso_or_dir);
	    }
	    else
	    {
		lkc_end_pt_axis_thin(datainfo,ptinfo,bord_array,num_array,
						conn_type,num_vox_removed,dir,
							iso_or_dir,end_cond);
	    }
	}
}


void	keep_points_axis_thin(data_info *datainfo, point_info *ptinfo,
							  int *bord_array, int num_array,
							  char conn_type, int *num_vox_removed,
							  unsigned char dir, int iso_or_dir)
{
	int	j, connectivity, was_edge, val;
	char	ok;

	for( j = 0;  j < num_array;  j++ )
	{
	    PLC = bord_array[j];
	    was_edge = 0;
	    if( PLC != -1 )
	    {
		if( iso_or_dir == 2 ) Reset_DD()
		else
		{
			if( DD[PLC] == EDGE_VOID ) was_edge = 1;
		}
			   
		connectivity = set_thin_connectivity(datainfo,ptinfo,
						     conn_type,iso_or_dir);

		if( connectivity == 1 )
		{
			DD[PLC] = MATERIAL;
			bord_array[j] = -1;
			(*num_vox_removed)++;
		}
		else if( (connectivity == -1) && was_edge )
			DD[PLC] = EDGE_VOID;
	    }
	}
}


void	new_end_point_axis_thin(data_info *datainfo, point_info *ptinfo,
								int *bord_array, int num_array,
								char conn_type,int *num_vox_removed,
								unsigned char dir, int iso_or_dir)
{
	int	j, f_cnt, cnt, ecnt, connectivity, index, was_edge, val;
	char	boole1, boole2, boole3, ok;

	int	*sten;

	sten = (conn_type == 6) ? datainfo->sten6 : datainfo->stencil;

	for( j = 0;  j < num_array;  j++ )
	{
		PLC = bord_array[j];
		if( PLC == -1 ) continue;

		ok = was_edge = 0;
		if( iso_or_dir == 2 ) Reset_DD()

		    /* value of 2 for iso_or_dir forces correct */
		    /*    usage of set_thin_connectivity()      */

	        connectivity = set_thin_connectivity(datainfo,ptinfo,
							conn_type,2);
		
		if( (connectivity == 1) && (ok == 0) )
		{
		    /* this case will occur in the early iterations  */
		    /* of axis thinning when there are border voxels */
		    /* which have neighboring interior voxels        */
		    
		    cnt = index = 0;
		    while( (cnt < 2) && (index < (int)conn_type) )
		    {
			boole1 = ( DD[PLC+sten[index]] >  MATERIAL );
			boole2 = ( DD[PLC+sten[index]] != EXTERIOR );
			if( boole1 && boole2 ) cnt++;
			index++;
		    }
		    
		    if( cnt >= 2 )
		    {
			ecnt = 1;
			if( was_edge )
			{
			    ecnt = index = 0;
			    while( (index < 26) && (ecnt == 0) )
			    {
				boole2 = ( DD[PLC+DS[index]] != EXTERIOR );
				boole3 = ( DD[PLC+DS[index]] >= 20 );
				if( boole2 && boole3 )
				{
				    if(    index ==  4 || index == 10
					|| index == 12 || index == 13
					|| index == 15 || index == 21   )
				    			       ecnt++;
				    else if( conn_type == 26 ) ecnt++;
				}
				index++;
			    }
			}
			if( ecnt )
			{
			    DD[PLC] = MATERIAL;
			    bord_array[j] = -1;
			    (*num_vox_removed)++;
			}
		    }
		    else if( was_edge) DD[PLC] = EDGE_VOID;
		}
		else if( (connectivity == 1) && (ok == 1) )
		{
		    cnt = f_cnt = ecnt = 0;
		    
		    for( index = 0;  index < 26;  index++ )
		    {
			boole1 = ( DD[PLC+DS[index]] >  MATERIAL );
			boole2 = ( DD[PLC+DS[index]] != EXTERIOR );
			boole3 = ( DD[PLC+DS[index]] >= 20 );
				
			if( boole1 && boole2 )
			{
			    if(    index ==  4 || index == 10
				|| index == 12 || index == 13
				|| index == 15 || index == 21)
			    {
				f_cnt++;
				cnt++;
				if( boole3 ) ecnt++;
			    }
			    else
			    {
				if( conn_type == 26 )
				{
				      cnt++;
				      if( boole3 ) ecnt++;
				}
			    }
			}
		    }
		    if( (f_cnt >= 2) || ( (f_cnt < 2) && (cnt > 4) ) )
		    {
			  /* this end condition can be fiddled with ? */
			if( (was_edge == 1 && ecnt > 0) || (was_edge == 0) )
			{
				DD[PLC] = MATERIAL;
				(*num_vox_removed)++;
				bord_array[j] = -1;
			}
			else if( was_edge ) DD[PLC] = EDGE_VOID;
		    }
		    else if( was_edge ) DD[PLC] = EDGE_VOID;
		}
		else if( (connectivity = -1) && was_edge )
			DD[PLC] = EDGE_VOID;
	}
}


void	lkc_end_pt_axis_thin(data_info *datainfo, point_info *ptinfo,
							 int *bord_array, int num_array, char conn_type,
							 int *num_vox_removed, unsigned char dir,
							 int iso_or_dir, int end_cond)
{
	int	j, cnt, ecnt, connectivity, index, was_edge, val;
	char	boole1, boole2, boole3, ok;
	int	*sten;

	sten = (conn_type == 6) ? datainfo->sten6 : datainfo->stencil;

	for( j = 0;  j < num_array;  j++ )
	{
	    PLC = bord_array[j];
	    if( PLC != -1 )
	    {
		was_edge = 0;
		if( iso_or_dir == 2 ) Reset_DD()
		else
		{
		      if( DD[PLC] == EDGE_VOID ) was_edge = 1;
		}

	        connectivity = set_thin_connectivity(datainfo,ptinfo,
					conn_type,iso_or_dir);

		if( connectivity == 1 )
		{
		    cnt = index = 0;
		    while( cnt < 2 && index < (int)conn_type )
		    {
			boole1 = ( DD[PLC+sten[index]] >  MATERIAL );
			boole2 = ( DD[PLC+sten[index]] != EXTERIOR );

			if( boole1 && boole2 ) cnt++;
			index++;
		    }

		    if( cnt >= 2 )
		    {
			ecnt = 1;
			if( was_edge )
			{
			    ecnt = index = 0;
			    while (index < 26 && ecnt == 0)
			    {
				boole2 = ( DD[PLC+DS[index]] != EXTERIOR );
				if( iso_or_dir == 2 )
				    boole3 = ( DD[PLC+DS[index]] >= 20 );
				else
				    boole3 = ( DD[PLC+DS[index]] == EDGE_VOID );

				if( boole2 && boole3 )
				{
				    if(    index ==  4 || index == 10
					|| index == 12 || index == 13
					|| index == 15 || index == 21  )
							       ecnt++;
				    else if( conn_type == 26 ) ecnt++;
				}
				index++;
			    }
			}
			if( ecnt )
			{
				DD[PLC] = MATERIAL;
				bord_array[j] = -1;
				(*num_vox_removed)++;
			}
		    }
		    else if( was_edge) DD[PLC] = EDGE_VOID;
		}
		else if( (connectivity == -1) && was_edge )
			DD[PLC] = EDGE_VOID;
	    }
	}
}


/*
*	Identify border points and create array lists of them for the
*	6 checking directions
*/

void	set_thin_border_points(data_info *datainfo, point_info *ptinfo,
							   int *bord_array, int num_array,
							   char conn_type)
{
	char	deuler;
	int	connectivity;
	int	j;

	for( j = 0;  j < num_array;  j++ )
	{
		ptinfo->place = bord_array[j];

		initialize_point_neighborhood(datainfo,ptinfo);
		deuler = calc_deuler(ptinfo->node_value,conn_type);

		if( deuler != 0 ) bord_array[j] = -1;
		else
		{
			if( conn_type == 26 )
				connectivity = local_connectivity(ptinfo);
			else
				connectivity = local_6_connectivity(ptinfo);

			if( ! connectivity ) bord_array[j] = -1;
		}
	}
}



int	thin_object_dir(data_info *datainfo, point_info *ptinfo, int surface,
					int bdry_alg, int num_keep_pts, int itno,
					char conn_type, int dbg_print)
{
	int	*bord_array;
	
	int	dir, dbg;
	int	j, num_array;
	int	num_vox_removed;
	
	int	isz = sizeof(int);

	dbg = ( debug && dbg_print ) ?  1 : 0;
	
	num_vox_removed = 0;

	for( dir = 0;  dir < 6;  dir++ )
	{
		get_border_array_dir(&bord_array,datainfo,&num_array,dir,
								bdry_alg);

		set_thin_border_points(datainfo,ptinfo,bord_array,num_array,
							conn_type);

			/* in *_thin() functions, the first `1' */
			/*     argument is a dummyi		*/

		if( surface )	/* find medial surface */
			surface_thin(datainfo,ptinfo,bord_array,num_array,
				     conn_type,&num_vox_removed,bdry_alg,
				     1,1,1);

		else		/* find medial axis */
			axis_thin(datainfo,ptinfo,bord_array,num_array,
				  conn_type,&num_vox_removed,num_keep_pts,
				  1,1,1);

		Set_voids()		

		if( num_array != 0) FREE(bord_array,num_array*isz);
	}
	return num_vox_removed;
}


#define Set_vval(_value)\
{\
	*ptr = i;\
	kount += 1;\
	if( kount == sz )\
	{\
		times_realloc += 1;\
\
fprintf(stderr,"realloc *bord %d times_realloc %d sz %d\n",*bord,times_realloc,sz);\
		*bord = (int *)REALLOC(*bord,times_realloc*sz*isz);\
		msg = "*bord realloc in get_border_array_dir()";\
		if( MEMCHECK(*bord,msg,sz*isz) ) clean_up(0); /* SIC ! */\
\
		ptr = *bord + ((times_realloc-1)*sz);\
		kount = 0;\
	}\
	else	ptr += 1;\
	vval[i] = _value;\
}


void	get_border_array_dir(int **bord, data_info *datainfo, int *num_array,
							 int dir, int bdry_alg)
{
	unsigned char *vval;

	char	*msg;

	int	*ptr;

	int	sten, sz;
	int	i, datasz;
	int	kount, times_realloc;
	int	xy, z, lastz;

	int	isz = sizeof(int);

	   xy = (datainfo->xsize)*(datainfo->ysize);
	lastz = (datainfo->zsize) - 2;

	datasz = xy*(datainfo->zsize);
	    sz = (int)(REALLOC_PARAM*datasz);

	*bord = (int *)MALLOC(sz*isz);
	msg = "*bord in get_border_array_dir()";
	if( MEMCHECK(*bord,msg,sz*isz) ) clean_up(0);

	ptr = *bord;
	kount = 0;
	times_realloc = 1;
	vval = datainfo->data;
  
	     if( dir == 0 ) sten = datainfo->stencil[ 4];
	else if( dir == 1 ) sten = datainfo->stencil[21];

	     if( dir == 2 ) sten = datainfo->stencil[10];
	else if( dir == 3 ) sten = datainfo->stencil[15];

	     if( dir == 4 ) sten = datainfo->stencil[12];
	else if( dir == 5 ) sten = datainfo->stencil[13];
  
	if( bdry_alg == 1 )
	{
	    for( i = 0;  i < datasz;  i++ )
	    {
		if(    (vval[i] > MATERIAL)
		    && (vval[i] != EXTERIOR)
		    && (vval[i] != KEEP_THIS_VOX) )
		{
		    /* ARUN - DOESN'T MAKE SENSE - Set_vval(vval[i]) */
		    /*	                  SETS vval[i] = vval[i] ??? */

		    if( vval[i + sten] == MATERIAL ) Set_vval(vval[i])
		}
	    }
	}
	else if( bdry_alg == 0 )
	{
	    for( i = 0;  i < datasz;  i++ )
	    {
		if(    (vval[i] > MATERIAL)
		    && (vval[i] != EXTERIOR)
		    && (vval[i] != KEEP_THIS_VOX) )
		{
		    if(    ( vval[i + sten] == MATERIAL )
			|| ( vval[i + sten] == EXTERIOR ) ) Set_vval(vval[i])
		}
	    }
	}
	else if( bdry_alg == 2 )
	{
	    for( i = 0;  i < datasz;  i++ )
	    {
		if(    (vval[i] > MATERIAL)
		    && (vval[i] != EXTERIOR)
		    && (vval[i] != KEEP_THIS_VOX) )
		{
		    if(    ( vval[i + sten] == MATERIAL )
			|| ( vval[i + sten] == EXTERIOR )    )
		    {
			z = i/xy;
			if( !((z == 1 && dir == 0) || (z == lastz && dir == 1)) )
				Set_vval(vval[i])
		    }
		}
	    }
	}
  
	*num_array = (times_realloc - 1)*sz + kount;

	if( *num_array == 0 )
	{
		FREE(*bord,times_realloc*sz*isz);
		*bord = (int *)NULL;
	}
	else
	{
		*bord = (int *)REALLOC(*bord,(*num_array)*isz);
		msg = "*bord realloc in get_border_array_dir()";
		if( MEMCHECK(*bord,msg,(kount-sz)*isz) ) clean_up(0);/* SIC ! */
	}
}


int	thin_object_iso(data_info *datainfo, point_info *ptinfo, int surface,
					int bdry_alg, int num_keep_pts, int itno,
					char conn_type, int dbg_print, int end_cond)
{
	int	*bord_array;

	int	dbg, ok;
	int	dir, dir_start, dir_stop;
	int	j, num_array;
	int	num_vox_removed, prev_vox_removed;
	
	int	isz = sizeof(int);

	dbg = ( debug && dbg_print ) ? 1 : 0;
	
	num_vox_removed = prev_vox_removed = dir = 0;
	
	get_border_array_iso(&bord_array,datainfo,&num_array,bdry_alg,end_cond,
							surface,conn_type);

	if( end_cond == 2 ) { dir_start = 7;	dir_stop = 2; }
	else		    { dir_start = 6;	dir_stop = 1; }

	ok = 1;
	while( ok )
	{
	    for( dir = dir_start;  dir >= dir_stop;  dir-- )
	    {
		if( surface ) surface_thin(datainfo,ptinfo,bord_array,num_array,
					   conn_type,&num_vox_removed,bdry_alg,
					   (unsigned char)dir,2,end_cond);
		else	      axis_thin(datainfo,ptinfo,bord_array,num_array,
					conn_type,&num_vox_removed,
					num_keep_pts,(unsigned char)dir,2,
								end_cond);
	    }
	    if( prev_vox_removed == num_vox_removed ) ok = 0;
	    else
	    {
		prev_vox_removed = num_vox_removed;
		rehash_bord_array(bord_array,datainfo,num_array,bdry_alg,
						end_cond,surface,conn_type);
	    }
	}

	Set_voids()
			
	if( num_array != 0 ) FREE(bord_array,num_array*isz);

	return num_vox_removed;
}


#define GET_Set_vval(_fcount,_icount)\
	count = _fcount;\
	if( count )\
	{\
		if( end_cond == 2         ) count++;\
		if( vval[i]  == EDGE_VOID ) count += 20;\
		Set_vval((unsigned char)count)\
	}\
	else if( _icount )\
	{\
		if( vval[i] == EDGE_VOID ) Set_vval(28)\
		else			   Set_vval( 8)\
	}


void	get_border_array_iso(int **bord, data_info *datainfo,
							 int *num_array, int bdry_alg, int end_cond,
							 int surface, char conn_type)
{
	unsigned char *vval;

	char	*msg;

	int	*ptr;

	int	sz;
	int	i, j, i_dir, datasz;
	int	kount, times_realloc;
	int	xy, z, lastz;
	int	count, ok;
	int	exterior_count, exterior_face_count;
	int	interior_count, interior_face_count;

	int	isz = sizeof(int);

	   xy = (datainfo->xsize)*(datainfo->ysize);
	lastz = (datainfo->zsize) - 2;

	datasz = xy*(datainfo->zsize);
	    sz = (int)(REALLOC_PARAM*datasz);

	*bord = (int *)MALLOC(sz*isz);
	msg = "*bord in get_border_array_iso()";
	if( MEMCHECK(*bord,msg,sz*isz) ) clean_up(0);

	ptr = *bord;
	kount = 0;
	times_realloc = 1;
	vval = datainfo->data;
  
	for( i = 0;  i < datasz;  i++ )
	{
	    if(    (vval[i] > MATERIAL)
		&& (vval[i] != EXTERIOR)
		&& (vval[i] != KEEP_THIS_VOX) )
	    {
		interior_count = interior_face_count = 0;
		exterior_count = exterior_face_count = 0;

		for( j = 0;  j < 26;  j++ )
		{
		    i_dir = i + datainfo->stencil[j];

		    if(    j ==  4 || j == 10 || j == 12
			|| j == 13 || j == 15 || j == 21   )
		    {

			     if( vval[i_dir] == EXTERIOR ) exterior_face_count++;
			else if( vval[i_dir] == MATERIAL ) interior_face_count++;
		    }
		    else if( conn_type == 26 )
		    {

			     if( vval[i_dir] == EXTERIOR ) exterior_count++;
			else if( vval[i_dir] == MATERIAL ) interior_count++;
		    }
		}

		if( bdry_alg == 1 )
		{
		    GET_Set_vval(interior_face_count,interior_count)
		}
		else if( bdry_alg == 0 )
		{
		    GET_Set_vval((interior_face_count + exterior_face_count),
				 (interior_count + exterior_count))
		}
		else if( bdry_alg == 2 )
		{
		    z = i/xy;
		    if( z == 1 )
		    {
			exterior_face_count--;
			exterior_count -= 8;
		    }
		    if( z == lastz ) /* need both if() tests */
		    {
			exterior_face_count--;
			exterior_count -= 8;
		    }
		    GET_Set_vval((interior_face_count + exterior_face_count),
				 (interior_count + exterior_count))
		}
	    }
	}

	*num_array = (times_realloc - 1)*sz + kount;

	*bord = (int *)REALLOC(*bord,(*num_array)*isz);
	msg = "*bord realloc in get_border_array_iso()";
	if( MEMCHECK(*bord,msg,(kount-sz)*isz) ) clean_up(0);	/* SIC ! */

	if( end_cond == 2 )
	{
	    if( !surface )
	    {
		for( j = 0;  j < *num_array;  j++ )
		{
		    ok = 0;
		    for( i = 0;  i < 26;  i++ )
		    {
			/* New end_cond for axis imposed. A voxel is an end */
			/* pt only when every neighboring voxel of the same */
			/* phase is a border voxel. Note at this point only */
			/* interior voxels can be VOID or EDGE_VOID, the    */
			/* rest of the "VOID" voxels are now filled with #  */
			/* of faces touching  material or are set to 8 (in  */
			/* the 26 connected case)			    */
			/* A check against KEEP_THIS_VOX is not made here   */
			/* as it should not arise when (end_cond == 2)      */

			if(    (DD[(*bord)[j]+DS[i]] == VOIDVOX     )
			    || (DD[(*bord)[j]+DS[i]] == EDGE_VOID)   )
			{
				ok = 1;
				break;
			}
		    }
		    if( !ok ) DD[(*bord)[j]] += 10;
		}
	    }
	}
}


#define REHASH_Set_vval(_fcount,_icount)\
	count = _fcount;\
	if( count )\
	{\
		if( end_cond == 2               ) count++;\
		if( vval[bord[i]]  == EDGE_VOID ) count += 20;\
		vval[bord[i]] = count;\
	}\
	else if( _icount )\
	{\
		if( vval[bord[i]] == EDGE_VOID ) vval[bord[i]] = 28;\
		else				 vval[bord[i]] =  8;\
	}


void	rehash_bord_array(int *bord, data_info *datainfo, int num_array,
						  int bdry_alg, int end_cond, int surface,
						  char conn_type)
{
	unsigned char *vval;

	int	i, j, i_dir;
	int	xy, z, lastz;
	int	count, ok;
	int	exterior_count, exterior_face_count;
	int	interior_count, interior_face_count;

	xy    = (datainfo->xsize)*(datainfo->ysize);
	lastz = (datainfo->zsize) - 2;
	vval  =  datainfo->data; 

	for( i = 0;  i < num_array;  i++ )
	{
	    if( bord[i] == -1 ) continue;

	    /* No check against KEEP_THIS_VOX as we are examining */
	    /*     bord[] voxels which cannot have this value     */

	    if( (vval[bord[i]] > MATERIAL) && (vval[bord[i]] != EXTERIOR) )
	    {
		interior_count = interior_face_count = 0;
		exterior_count = exterior_face_count = 0;

		for( j = 0;  j < 26;  j++ )
		{
		    i_dir = bord[i] + datainfo->stencil[j];

		    if(    j ==  4 || j == 10 || j == 12
			|| j == 13 || j == 15 || j == 21   )
		    {
	
			     if( vval[i_dir] == EXTERIOR ) exterior_face_count++;
			else if( vval[i_dir] == MATERIAL ) interior_face_count++;
		    }
		    else if( conn_type == 26 )
		    {
	
			     if( vval[i_dir] == EXTERIOR ) exterior_count++;
			else if( vval[i_dir] == MATERIAL ) interior_count++;
		    }
		}

		if( bdry_alg == 1 )
		{
		    REHASH_Set_vval(interior_face_count,interior_count)
		}
		else if( bdry_alg == 0 )
		{
		    REHASH_Set_vval((interior_face_count + exterior_face_count),
				    (interior_count + exterior_count))
		}
		else if( bdry_alg == 2 )
		{
		    z = i/xy;
		    if( z == 1 )
		    {
			exterior_face_count--;
			exterior_count -= 8;
		    }
			
		    if( z == lastz )
		    {
			exterior_face_count--;
			exterior_count -= 8;
		    }
				
		    REHASH_Set_vval((interior_face_count + exterior_face_count),
				    (interior_count + exterior_count))
		}
	    }
	}

	if( end_cond == 2 )
	{
	    if( !surface )
	    {
		for( j = 0;  j < num_array;  j++ )
		{
		    if( bord[j] == -1 ) continue;

		    ok = 0;
		    for( i = 0;  i < 26;  i++ )
		    {
			/* New end_cond for axis imposed. A voxel is an end */
			/* pt only when every neighboring voxel of the same */
			/* phase is a border voxel. Note at this point only */
			/* interior voxels can be VOID or EDGE_VOID, the    */
			/* rest of the "VOID" voxels are now filled with #  */
			/* of faces touching  material or are set to 8 (in  */
			/* the 26 connected case)			    */
			/* A check against KEEP_THIS_VOX is not made here   */
			/* as it should not arise when (end_cond == 2)      */

			if(    (vval[bord[j]+DS[i]] == VOIDVOX     )
			    || (vval[bord[j]+DS[i]] == EDGE_VOID)   )
			{
				ok = 1;
				break;
			}
		    }
		    if( !ok ) vval[bord[j]] += 10;
		}
	    }
	}
}
void	cross_link_ma_list_cube_entries(int *ma_list, int n_ma_vox,
										int *cube)
{
	int	n, ma_ind;

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		ma_ind = ma_list[n];
		cube[ma_ind] = n;
	}
}


void	unlink_cube_from_ma_list_entries(int *ma_list, int n_ma_vox,
										 int *cube)
{
	int	n, ma_ind;

	for( n = 0;  n < n_ma_vox;  n++ )
	{
		ma_ind = ma_list[n];
		cube[ma_ind] = 99;
	}
}
void	thin_input_obj_data(data_info **datainfo, char tp, char conn_type,
							int bdry_alg, int nx, int ny, int nz,
							unsigned char *burn_cube)
{
	unsigned char *data;

	int	csz = sizeof(char);

	*datainfo = data_init(nx+2,ny+2,nz+2);
	data = (*datainfo)->data;

	thin_read_binary_file(data,nx+2,ny+2,nz+2,burn_cube);

	mark_edge_void(data,(*datainfo)->stencil,1,nz,
				   nx+2,ny+2,nz+2,bdry_alg,conn_type); 

}
void	thin_read_binary_file(unsigned char *data,
							  int nx, int ny, int nz,
							  unsigned char *burn_cube)
{
	unsigned char *pdat, *ppd;

	unsigned char	val;
	int		i, j, k;
	int		nred;
	int		nxy;
	int		j1nx1;

	nxy  = nx*ny;

	pdat = data+nxy;
	nred = 0;
	for( k = 0;  k < (nz-2);  k++, pdat += nxy )
	{
		//read_burn_fp(fp,nx-2,ny-2,tmp_burn);
		/***
		read_burn_file(fn,is_cmprss,nx-2,ny-2,n_polys,bh,bh_count,tmp_burn);
		***/
		for( j = 0;  j < ny-2;  j++ )
		{
			j1nx1 = (j+1)*nx+1;		ppd = pdat + j1nx1;

			for( i = 0;  i < nx-2;  i++, ppd++ )
			{
				val = burn_cube[nred];

					 if( (val < BARRIER) && (val != 0    ) ) *ppd = VOIDVOX;
				else if( (val == 0     ) || (val == GRAIN) ) *ppd = MATERIAL;
				else if(  val == EXT_VAL				   ) *ppd = EXTERIOR;
				else
				{
					PRINTF("ERROR: wrong value in burnfile %d ",k+1);
					PRINTF("at x %d y %d burn # %d\n",i,j,val);
					PRINTF("ERROR: wrong value in burnfile %d ",k+1);
					PRINTF("at x %d y %d burn # %d\n",i,j,val);
					clean_up(0);
				}
				nred++;
			}
		}
	}
}


void	mark_edge_void(unsigned char *data, int *stencil, int zs, int ze,
					   int nx, int ny,
					   int nz, int bdry_alg, char conn_type)
{
	int	lastz, i, index;

	int	*tsten;
	int	sten6[6], same[2];
	int	top, bot;
	int	nxy, nxyz, z_coord, pos;

	if( bdry_alg == 0 ) return;

	if( conn_type == 6 )
	{
		sten6[0] = stencil[ 4];		sten6[1] = stencil[10];
		sten6[2] = stencil[12];		sten6[3] = stencil[13];
		sten6[4] = stencil[15];		sten6[5] = stencil[21];

		top = 0;	same[0] = 1;
				same[1] = 5;	bot = 6;
		tsten = sten6;
	}
	else
	{
		top = 0;	same[0] = 9;
				same[1] = 17;	bot = 26;
		tsten = stencil;
	}
	nxy = nx*ny;  nxyz = nxy*nz;
	lastz = nz-2;
   
	for( i = 0;  i < nxyz;  i++ )
	{
		if( data[i] == VOIDVOX )
		{
			z_coord = i/nxy;

			if( bdry_alg == 1 )
			{
				if( lastz == 1 )
				{
					for( index = same[0];  index < same[1];  index++ )
					{ 
						pos = i + tsten[index];
			
						if( data[pos] == EXTERIOR )
						{
							data[i] = EDGE_VOID;
							break;
						}
					}
				}
				else if( lastz != 1 )
				{
					if( z_coord == 1 )
					{
						for( index = same[0];  index < bot;  index++ )
						{
							pos = i + tsten[index];
				
							if( data[pos] == EXTERIOR )
							{
								data[i] = EDGE_VOID;
								break;
							}
						}
					}
					else if( z_coord == lastz )
					{
						for( index = top;  index < same[1];  index++ )
						{ 
							pos = i + tsten[index];

							if( data[pos] == EXTERIOR )
							{
								data[i] = EDGE_VOID;
								break;
							}
						}
					}
				}
			}
			else if( bdry_alg == 2 )
			{
				if( lastz != 1 )
				{
					if( z_coord == 1 )
					{
						data[i] = EDGE_VOID;
						break;
					}
					else if( z_coord == lastz )
					{
						data[i] = EDGE_VOID;
						break;
					}
				}
			}
		}
	}
}

