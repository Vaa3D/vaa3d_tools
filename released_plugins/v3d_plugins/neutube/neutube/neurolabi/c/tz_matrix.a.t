/* tz_matrix.a.t
 * 
 * 14-Aug-2007  Initial write:  Ting Zhao
 */

/*
 * Template functions for matrix operation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "tz_error.h"
#include "utilities.h"
#include "tz_utilities.h"
#include "tz_math.h"
#include "tz_<2t>.h"
#include "tz_<3t>.h"

static void <2t>_error(const char *msg,const char *arg)
{
  fprintf(stderr,"\nError in tz_<2t>:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}


// Awk-generated matrix memory management

static inline dim_type <2t>_asize(const <2T>* dm)
{ return (matrix_size(dm->dim,dm->ndim)*sizeof(<1t>)); }

MANAGER -r <2T> array:asize

void Reset_<2T>()
{
  reset_<2t>();
}

/* Make_<2T>(): Create a <1t> matrix.
 *
 * Args: dim - dimensions of the matrix
 *       ndim - number of dimensions
 *
 * Return: a <1t> matrix.
 */
<2T>* Make_<2T>(const dim_type dim[],ndim_type ndim)
{
  check_matrix(dim,ndim);

  <2T> *dm;
  ndim_type i;

  dm = new_<2t>(matrix_size(dim,ndim)*sizeof(<1t>),"Make_<2T>");
  dm->ndim = ndim;
  for(i=0;i<ndim;i++)
    dm->dim[i] = dim[i];

  return dm;
}

<2T>* Make_3d_<2T>(dim_type width, dim_type height, dim_type depth)
{
  dim_type dim[3];
  dim[0] = width;
  dim[1] = height;
  dim[2] = depth;
  
  return Make_<2T>(dim,  3);
}

void <2T>_Copy(<2T> *des, const <2T> *src)
{
  memcpy(des->array, src->array, <2t>_asize(src));
}

/* Crop_<2T>(): Crop matrix.
 * 
 * Note: The caller is responsible for clearing the returned matix.
 *
 * Args: dm - input matrix
 *       offset - the offset from the starting corner of the cropped matrix to 
 *                the starting corner of the original matrix.
 *       dim - size of the output matrix
 *
 * Return: a <1t> matrix.
 */
<2T>* Crop_<2T>(const <2T>* dm,const int offset[],const dim_type dim[],
		<2T> *out)
{
  int crop = 0;
  tz_uint32 i;

  for(i=0;i<dm->ndim;i++) {
    if(offset[i]!=0) {
      crop = 1;
      break;
    }

    if(dm->dim[i]!=dim[i]) {
      crop = 1;
      break;
    }
  }

  if (out == NULL) {
    out = Make_<2T>(dim, dm->ndim);
  }

  if(!crop) {
    <2T>_Copy(out, dm);
    return out;
  }

  dim_type src_start[MAX_DIM],des_start[MAX_DIM],
    src_end[MAX_DIM],des_end[MAX_DIM];
  
  dim_type src_plane_offset[MAX_DIM],des_plane_offset[MAX_DIM],
    src_offset[MAX_DIM],des_offset[MAX_DIM],src_pos,des_pos;
  dim_type copy_length,total_rows,copy_dim[MAX_DIM];
  int cur_dim,status[MAX_DIM];
  
  src_plane_offset[0] = 1;
  des_plane_offset[0] = 1;

  src_pos = 0;
  des_pos = 0;
  src_offset[0] = dm->dim[0];
  des_offset[0] = dim[0];
  total_rows = 1;
  for(i=0;i<dm->ndim;i++) {
    src_start[i] = imax2(offset[i],0);
    des_start[i] = imax2(0,-offset[i]);
    src_end[i] = imin2(dm->dim[i]-1,dim[i]+offset[i]-1);
    des_end[i] = imin2(dim[i]-1,dm->dim[i]-offset[i]-1);
    copy_dim[i] = des_end[i]-des_start[i]+1;
    if(i>0) {
      src_plane_offset[i] = src_plane_offset[i-1]*dm->dim[i-1];
      des_plane_offset[i] = des_plane_offset[i-1]*dim[i-1];
      src_offset[i] = src_offset[i-1]+(dm->dim[i]-1-src_end[i]+src_start[i])*src_plane_offset[i];
      des_offset[i] = des_offset[i-1]+(dim[i]-1-des_end[i]+des_start[i])*des_plane_offset[i];
      total_rows *= copy_dim[i];
    }
    src_pos += src_plane_offset[i]*src_start[i];
    des_pos += des_plane_offset[i]*des_start[i];
    
    status[i] = 0;
  }

  copy_length = sizeof(<1t>)*(des_end[0]-des_start[0]+1);

  <2T>_Set_Zero(out);
  <2T>* dm2 = out;

  cur_dim = 0;
  for(i=0;i<total_rows;i++) {
    memcpy(dm2->array+des_pos,dm->array+src_pos,copy_length);

    status[0]++; /* move one row */

    if(cur_dim>0)
      cur_dim = 0;
    else {
      while(status[cur_dim]==copy_dim[cur_dim+1]) {
	status[cur_dim] = 0;
	cur_dim++;	
	if(cur_dim==dm->ndim-1)
	  break;
	status[cur_dim]++;
      }
    }

#if 0
    iarray_print2(status,dm->ndim,1);
#endif

    /* cur_dim>0: jump; cur_dim==0: move one row */
    src_pos += src_offset[cur_dim];
    des_pos += des_offset[cur_dim];
  }

  return out;
}

#define MATRIX_BLOCK_STATE_INC(state, inc_dim, ndim, dim)	\
  {								\
    state[1]++;							\
    inc_dim = 1;						\
    ndim_type k;						\
    for (k = 1; k < ndim - 1; k++) {				\
      if (state[k] == dim[k]) {					\
	state[k] = src_offset[k];				\
	state[k+1]++;						\
	inc_dim = k + 1;					\
      }								\
    }								\
  }


void <2T>_Copy_Block(<2T> *des, const dim_type des_offset[], const <2T> *src,
		     const dim_type src_offset[])
{
  ASSERT(des->ndim == src->ndim, "Dimension unmatched");
  
  size_t row_size = sizeof(<1t>) * (src->dim[0] - src_offset[0]);

  ndim_t i;
  dim_t state[MAX_DIM];
  for (i = 0; i < MAX_DIM; i++) {
    state[i] = src_offset[i];
  }

  int src_block_offset[MAX_DIM];
  int src_area[MAX_DIM];
  int des_area[MAX_DIM];
  dim_t src_jump[MAX_DIM];
  dim_t des_jump[MAX_DIM];
  
  src_jump[0] = 0;
  des_jump[0] = 0; 
  src_block_offset[0] = 1;
  src_area[0] = src->dim[0];
  des_area[0] = des->dim[0];

  for (i = 1; i < src->ndim; i++) {
    src_block_offset[i] = src->dim[i] - src_offset[i];
    src_area[i] = src_area[i-1] * src->dim[i];
    des_area[i] = des_area[i-1] * des->dim[i];
    if (i == 1) {
      src_jump[i] = 0;
      des_jump[i] = 0;
    } else {
      src_jump[i] = src_jump[i-1] + (src_block_offset[i-1] - 1) * src_area[i-2];
      des_jump[i] = des_jump[i-1] + (src_block_offset[i-1] - 1) * des_area[i-2];
    }
  }

  for (i = 1; i < src->ndim; i++) {
    /* jump on the i^th dimension */
    src_jump[i] = src_area[i-1] - src_jump[i];
    des_jump[i] = des_area[i-1] - des_jump[i];
  }

  int offset1, offset2;

  offset1 = Sub_To_Ind(src->dim, src->ndim, src_offset);
  offset2 = Sub_To_Ind(des->dim, des->ndim, des_offset);

  ndim_t inc_dim;

  while (state[src->ndim-1] < src->dim[src->ndim-1]) {
    memcpy(des->array + offset2, src->array + offset1, row_size);
    MATRIX_BLOCK_STATE_INC(state, inc_dim, src->ndim, src->dim);    
    offset1 += src_jump[inc_dim];
    offset2 += des_jump[inc_dim];
  }
}

/* <2T>_Blocksum(): Local sum of a matrix.
 *
 * Note: Calculate the sum of each block defined by the template size. The 
 *       returned matrix will have size 
 *       (dm->dim[0]+bdim[0]-1) x ... x (dm->dim[n]+bdim[n]-1)
 *       for n+1 dimensional matrix. 
 *       The caller is resposible for clearing the returned matrix.
 *
 * Args: dm - input matrix
 *       bdim - size of the template or block
 *       dm2 - where the result is stored. If it is NULL, a new matrix will be
 *             created. Otherwise its size must be the same as expected.
 *  
 * Return: the matrix of block sum. The (t1,t2,...,tn)th element of the output 
 *         is the sum of the block with the rightmost corner 
 *         (t1-1,t2-1,...,tn-1).
 */
<2T>* <2T>_Blocksum(const <2T> *dm, const dim_type bdim[], <2T> *dm2)
{
  if(dm->ndim > 3)
    <2t>_error("<2T>_Blocksum does not support dimension greater than 3 currently.",NULL);

  check_matrix(dm->dim,dm->ndim);
  check_matrix(bdim,dm->ndim);

  //int org_width,org_height,width,height,bwidth,bheight;
  dim_type org_offset=0, offset = 0;
  ndim_type idim;
  dim_type i,plane_offset,org_plane_offset;
  tz_int32 zero_size;
  dim_type new_dim[MAX_DIM];
 
  for(idim=0;idim<dm->ndim;idim++)
    new_dim[idim] = dm->dim[idim]+bdim[idim]-1;
  
  if (dm2 == NULL)
    dm2 = Make_<2T>(new_dim,dm->ndim);
  else {
    if (dm2->ndim != dm->ndim)
      <2t>_error("Matrix dimension error.",NULL);

    for(idim = 0; idim < dm->ndim; idim++) {
      if (dm2->dim[idim] != new_dim[idim])
	<2t>_error("Matrix size error.",NULL);
    }
  }

  if(dm->ndim==1) { //1D array
    <3t>_linsum1(dm->array,dm2->array, 
		   dm->dim[0],1,bdim[0],1);
    return dm2;
  }

  zero_size = (bdim[dm->ndim-1]-1)*matrix_subsize(dm2->dim,0,dm->ndim-2);
  for(i=0;i<zero_size;i++) {
    dm2->array[offset] = 0;
    offset++;
  }

  if(dm->ndim==2) { //2D matrix
    <3t>_linsum1(dm->array,dm2->array, 
		   dm->dim[0],dm->dim[1],bdim[0],bdim[1]);
    <3t>_linsum2(dm2->array,dm2->dim[0],dm->dim[1],bdim[1]);
  } else {
    plane_offset = dm2->dim[0]*dm2->dim[1];
    org_plane_offset = dm->dim[0]*dm->dim[1];
    
    /*sum up each slice*/
    for(i=0;i<dm->dim[2];i++) {
      <3t>_linsum1(dm->array+org_offset,dm2->array+offset, 
		     dm->dim[0],dm->dim[1],bdim[0],bdim[1]);
      <3t>_linsum2(dm2->array+offset,dm2->dim[0],dm->dim[1],bdim[1]);
      offset += plane_offset;
      org_offset += org_plane_offset;
    }
  
    /*sum up Z axis*/
    <3t>_linsum2(dm2->array,dm2->dim[0]*dm2->dim[1],dm->dim[2],bdim[2]);
  }

  return dm2;
  
  /* for future development
  check_dmatrix(dm->dim,dm->ndim);
  check_dmatrix(bdim,dm->ndim);

  for(i=0;i<dm->ndim;i++)
    new_dim[i] = dm->dim[i]+bdim[i]-1;
    
  dm2 = Make_DMatrix(new_dim,dm->ndim);

  //first dimension
  int org_width,org_height,width,height,bwidth,bheight;
  int offset = 0;
  int i,j,plane_offset;
  int zero_size;
  int ndim_left;

  plane_offset = 0;

  zero_size = bdim[dm->ndim-1]*dmatrix_subsize(dm2->dim,0,dm->ndim-2);
  for(i=0;i<zero_size;i++) {
    dm2->array[offset] = 0;
    offset++;
  }
  
  //process first dimension
  if(dm->ndim>2) {
    for(i=dm->ndim-2;i==1;i--) {
      for(j=0;j<dm2->dim[i];j++) {
	if(i==1)
	  darray_linsum1(dm->array+offset1,dm2->array+offset,dm->dim[0],dm->dim[1],bdim[0],bdim[1]);
	else {	  
	  zero_size = dmatrix_subsize(dm2->dim,0,dm->ndim-i-1);
	  for(j=0;j<zero_size;j++) {
	    dm2->array[offset+j] = 0;
	  }
	}
	plane_offset = dmatrix_subsize(dm2->dim,0,dm->ndim-i-2);
	offset += plane_offset; 	
      }
    }
  }
    
  org_width = dm->dim[0];
  org_height = dmatrix_subsize(dm->dim,1,dm->ndim-1);
  width = dm2->dim[0];
  height = dmatrix_subsize(dm2->dim,1,dm2->ndim-1);  
  bwidth = bdim[0];
  bheight = dmatrix_subsize(bdim,1,dm->ndim-1);
  
  int ndim_left = dm->ndim-1;

  for(i=1;i<ndim_left;i++) {
    org_width = dmatrix_subsize(dm->dim,0,i-1);
    org_height = dm->dim[i];
   
    bwidth = dmatrix_subsize(bdim,0,i-1);
    bheight = bdim[i];

    width = dmatrix_subsize(dm2->dim,0,i-1);
    height = dm2->dim[i];
    depth = dmatrix_subsize(dm2->dim,i+1,dm2->ndim-1);
    
    plane_offset = width*height;

    for(j=0;j<depth;j++) {
      darray_linsum2(dm2->array+offset,org_width,org_height,bwidth,bheight);
      offset += plane_offset;
    }
  }
  
  darray_linsum2(dm2->array,org_width,org_height,bwidth,bheight);
  */
}

/* <2T>_Blocksum_Part: Partial local sum.
 *
 * Note: This function is similar to <2T>_Blocksum, except that it only
 *       calculates a part of the matrix.
 *       The caller is responsible for clearing the returned matrix.
 *  
 * Args: dm - input matrix
 *       bdim - size of the block
 *       start - starting corner of the partial matrix
 *       end - ending corner of the partial matrix
 *
 * Return: the matrix of local sum.
 */
<2T>* <2T>_Blocksum_Part(const <2T>* dm,const dim_type bdim[],const dim_type start[],const dim_type end[])
{
  if(dm->ndim != 3)
    <2t>_error("<2T>_Blocksum_Part does not support dimension other than 3 currently.",NULL);

  dim_type i,j,k;
  dim_type i1,j1,k1;
  dim_type dim2[3];
  for(i=0;i<3;i++)
    dim2[i] = end[i]-start[i]+1;
  
  <2T>* dm2 = <2T>_Zeros(dim2,3);
  dim_type offset,offset2,tmpoffset,tmpoffset2;
  dim_type suboffset[3],tmpsuboffset[3],start2[3],end2[3];
  dim_type plane_offset = dm->dim[0]*dm->dim[1];
  for(i=0;i<3;i++) {
    suboffset[i] = start[i];
    tmpsuboffset[i] = suboffset[i]; //initial offset
  }

  offset2 = 0;

  <1t> inc,dec;
  int leftin,rightin,orgleft,orgright;

  for(k=0;k<dim2[2];k++) {
    suboffset[2] = tmpsuboffset[2]+k; //current depth offset
    //block depth start
    start2[2] = imax2(0,suboffset[2]-bdim[2]+1);
    //block depth end
    end2[2] = imin2(suboffset[2]+1,dm->dim[2]);
    for(j=0;j<dim2[1];j++) {
      suboffset[1] = tmpsuboffset[1]+j;
      //block column start
      start2[1] = imax2(0,suboffset[1]-bdim[1]+1); 
      //block column end
      end2[1] = imin2(suboffset[1]+1,dm->dim[1]);
      inc = 0;
      dec = 0;
      for(i=0;i<dim2[0];i++) {
	suboffset[0] = tmpsuboffset[0]+i;
	orgleft = suboffset[0]-bdim[0]+1;
	if(orgleft<0) {
	  leftin = 0;
	  start2[0] = 0;
	} else {
	  leftin = 1;
	  start2[0] = orgleft;
	}

	orgright = suboffset[0]+1;
	if(orgright>=dm->dim[0]) {
	  rightin = 0;
	  end2[0] = dm->dim[0];
	} else {
	  rightin = 1;
	  end2[0] = orgright;
	}

	offset = sub2ind(dm->dim,3,start2);

	if(i==0) {
	  inc = 0;
	  dec = 0;
	  for(k1=start2[2];k1<end2[2];k1++) {	 
	    tmpoffset2 = offset;
	    for(j1=start2[1];j1<end2[1];j1++) {
	      if(leftin) dec += dm->array[offset];
	      if(rightin) inc += dm->array[offset+end2[0]-start2[0]];

	      tmpoffset = offset;
	      <1t> inc2 = 0;
	      for(i1=start2[0];i1<end2[0];i1++) {
		//dm2->array[offset2] += dm->array[offset];
		inc2 += dm->array[offset];
		offset++;
	      }
	      dm2->array[offset2] += inc2;

	      offset = tmpoffset+dm->dim[0];
	    }
	    offset = tmpoffset2+plane_offset;
	  }
	}
	else {
	  dm2->array[offset2] = dm2->array[offset2-1]+inc-dec;
	  inc = 0;
	  dec = 0;
	  //if(!leftin) leftin = (orgleft+i==0);
	  //if(rightin) rightin = (orgright+i<dm->dim[0]);

	  for(k1=start2[2];k1<end2[2];k1++) {
	    tmpoffset2 = offset;
	    for(j1=start2[1];j1<end2[1];j1++) {
	      if(leftin)
		dec += dm->array[offset];

	      if(rightin)
		inc += dm->array[offset+end2[0]-start2[0]];

	      offset += dm->dim[0];
	    }
	    offset = tmpoffset2+plane_offset;
	  }
	}

	offset2++;
      }
    }
  }

  return dm2;
}

/* <2T>_Blocksum_Part2(): Another version of  <2T>_Blocksum_Part
 */
<2T>* <2T>_Blocksum_Part2(const <2T>* dm,const dim_type bdim[],const dim_type start[],const dim_type end[])
{
  dim_type offset[MAX_DIM];
  dim_type offset2[MAX_DIM];
  dim_type dim_part[MAX_DIM];
  dim_type final_dim[MAX_DIM]; /* dimensions of the returned matrix */
  ndim_type i;
  
  for (i=0; i<dm->ndim; i++) {
    final_dim[i] = end[i] - start[i] + 1;
    dim_part[i] = final_dim[i] + bdim[i] * 2 - 2;
    offset[i] = start[i] - bdim[i] + 1;
    offset2[i] = bdim[i] - 1;
  }
  <2T> *matrix_part = Crop_<2T>(dm, offset, dim_part, NULL);
  <2T> *matrix_sum = <2T>_Blocksum(matrix_part, bdim, NULL);
  <2T> *matrix_final = Crop_<2T>(matrix_sum, offset2, final_dim, NULL);

  Kill_<2T>(matrix_part);
  Kill_<2T>(matrix_sum);

  return matrix_final;
}

/**
 * Calcualte the block sum of a certain dimension
 *
 */
/*
DMatrix* DMatrix_Linsum(DMatrix* dm,int idim,int bdim)
{
  int offset_size = 1,rest_size=1;
  int i,j;
  int new_dim[MAX_DIM];

  for(i=0;i<dm->ndim;i++)
    new_dim[i] = dm->dim[i]+bdim[i]-1;
    
  dm2 = Make_DMatrix(new_dim,dm->ndim);
0
  for(i=0;i<idim-1;i++) {
    offset_size *= dm->dim[i];
  }

  for(i=idim;i<dm->ndim;i++) {
    rest_size *= dm->dim[i];
  }
  
  if(bdim<=dm->dim[idim]) {
    for(j=0;j<rest_size;j++) {
      
    }
  } else {
  }
}
*/

/** 
 * Calculate the mean of each block defined by the template size bdim.
 * The calculation is performed out place if itype is 0. If itype is 
 * not 0, dm should already be the block sum and the calculation will be done
 * in place.
 */
<2T>* <2T>_Blockmean(<2T>* dm,const dim_type bdim[],int itype)
{
  <2T>* dm2;
  ndim_type idim;
  dim_type i,j,k;
  dim_type dim[MAX_DIM];

  if(itype) {
    dm2 = dm;
    for(idim=0;idim<dm->ndim;idim++)
      dim[idim] = dm->dim[idim]-bdim[idim]+1;
  }
  else {
    dm2 = <2T>_Blocksum(dm, bdim, NULL);
    for(idim=0;idim<dm->ndim;idim++)
      dim[idim] = dm->dim[idim];    
  }

  dim_type mindim,maxdim;
  dim_type* valid_size[3];


  for(idim=0;idim<3;idim++) {
    mindim = imin2(dim[idim],bdim[idim]);
    maxdim = imax2(dim[idim],bdim[idim]);
    valid_size[idim] = (dim_type*)malloc(dm2->dim[idim]*sizeof(dim_type));

    for(j=0;j<dm2->dim[idim];j++) {
      if(j<mindim)
	valid_size[idim][j] = j+1;
      else { 
	if(j>=maxdim)
	  valid_size[idim][j] = dm2->dim[idim]-j;
	else
	  valid_size[idim][j] = mindim;
      }
    }
  }
  
  dim_type offset=0;
  for(k=0;k<dm2->dim[2];k++)
    for(j=0;j<dm2->dim[1];j++)
      for(i=0;i<dm2->dim[0];i++) {
	dm2->array[offset] /= valid_size[0][i]*valid_size[1][j]*valid_size[2][k];
	offset++;
      }

  for(idim=0;idim<3;idim++)
    free(valid_size[idim]);

  return dm2;
}


<2T>* <2T>_Blockmean_Part(<2T>* dm,const dim_type dim[],const dim_type bdim[],const dim_type start[],const dim_type end[],int itype)
{
  <2T>* dm2;
  ndim_type idim;
  dim_type i,j,k;

  if(itype)
    dm2 = dm;
  else 
    dm2 = <2T>_Blocksum_Part(dm,bdim,start,end);  

  dim_type* valid_size[3];
  const dim_type* cur_dim;
  if(itype)
    cur_dim = dim;
  else
    cur_dim = dm->dim;

  for(idim=0;idim<3;idim++) {
    valid_size[idim] = (dim_type*)malloc(dm2->dim[idim]*sizeof(dim_type));
    for(j=start[idim];j<=end[idim];j++) {
      valid_size[idim][j-start[idim]] = 
	imin2(j+bdim[idim]-1,cur_dim[idim]+bdim[idim]-2) -
	imax2(bdim[idim]-1,j)+1;
    }
  }  
  
  dim_type offset=0;
  for(k=0;k<dm2->dim[2];k++)
    for(j=0;j<dm2->dim[1];j++)
      for(i=0;i<dm2->dim[0];i++) {
	dm2->array[offset] /= valid_size[0][i]*valid_size[1][j]*valid_size[2][k];
	offset++;
      }

  for(idim=0;idim<3;idim++)
    free(valid_size[idim]);

  return dm2;
}

void <2T>_Print(const <2T>* dm)
{
  check_matrix(dm->dim,dm->ndim);

  if(dm->ndim==1)
    <3t>_print2(dm->array,dm->dim[0],1);

  if(dm->ndim==2)
    <3t>_print2(dm->array,dm->dim[0],dm->dim[1]);

  if(dm->ndim==3)
    <3t>_print3(dm->array,dm->dim[0],dm->dim[1],dm->dim[2]);

  if(dm->ndim>3) {
    ndim_type idim;
    dim_type j;
    dim_type plane_offset = dm->dim[0]*dm->dim[1];
    dim_type length=1;
    for(idim=2;idim<dm->ndim;idim++)
      length *= dm->dim[idim];

    dim_type offset=0;
    dim_type sub[MAX_DIM];
    int k;

    for(j=0;j<length;j++) {
      ind2sub(dm->dim+2,dm->ndim-2,j,sub);
      printf("Plane: ");
      for(k=0;k<dm->ndim-2;k++)
	printf("%d ",sub[k]);
      printf("\n");
      <3t>_print2(dm->array+offset,dm->dim[0],dm->dim[1]);
      offset += plane_offset;
    } 
  }
}

void <2T>_Print_Part(const <2T>* dm,const dim_type start[],const dim_type end[])
{
  if(dm->ndim != 3)
    <2t>_error("<2T>_Print_Part does not support dimension other than 3 currently.",NULL);

  dim_type i,j,k;
    
  dim_type offset,tmpoffset,tmpoffset2;
  dim_type suboffset[3],end2[3];
  dim_type plane_offset = dm->dim[0]*dm->dim[1];
  for(i=0;i<3;i++) {
    suboffset[i] = imax2(0,start[i]);
    end2[i] = imin2(dm->dim[i],end[i]+1);
  }

  offset = sub2ind(dm->dim,3,suboffset);
  for(k=suboffset[2];k<end2[2];k++) {
    printf("plane %d\n",k);
    tmpoffset2 = offset;
    for(j=suboffset[1];j<end2[1];j++) {
      tmpoffset = offset;
      for(i=suboffset[0];i<end2[0];i++) {
	<1t=float,double>
	printf("%.3f ",dm->array[offset]);
	</t>
	<1t=int,uint8>
	printf("%d ",dm->array[offset]);
	</t>    
	offset++;
      }
      printf("\n");
      offset = tmpoffset+dm->dim[0];
    }
    offset = tmpoffset2+plane_offset;
    printf("\n");
  }
}


/**
 * Write <1t> matrix. The first 4 bytes is for the number of dimension.
 * Then the following bytes are the dimensions.
 */
void <2T>_Write(const char* filename,const <2T>* dm)
{
  FILE* fp;
  if( !(fp=fopen(filename,"w+")) )
    <2t>_error("Unable to open the file %s for writing.",filename);

  fwrite(&(dm->ndim),sizeof(ndim_type),1,fp);
  fwrite(dm->dim,sizeof(dim_type),dm->ndim,fp);
  fwrite(dm->array,sizeof(<1t>),matrix_size(dm->dim,dm->ndim),fp);
  
  fclose(fp);
}

<2T>* <2T>_Read(const char* filename)
{
  FILE* fp;
  if( !(fp=fopen(filename,"r")) )
    <2t>_error("Unable to open the file %s for reading.",filename);
  
  ndim_type ndim;
  dim_type dim[MAX_DIM];

  fread(&(ndim),sizeof(ndim_type),1,fp);
  fread(dim,sizeof(dim_type),ndim,fp);

  <2T>* dm = Make_<2T>(dim,ndim);
  fread(dm->array,sizeof(<1t>),matrix_size(dm->dim,dm->ndim),fp);

  fclose(fp);

  return dm;
}


<1t> <2T>_Max(const <2T>* dm,dim_type* sub)
{
  size_t length = matrix_size(dm->dim,dm->ndim);
  size_t idx;
  <1t> max_value;
  max_value = <3t>_max(dm->array,length,&idx);
  if (sub != NULL) {
    ind2sub(dm->dim,dm->ndim,(dim_type)idx,sub);
  }

  return max_value;
}

<1t> <2T>_Max_P(const <2T> *dm, const dim_type *start, const dim_type *end,
		dim_type *sub)
{
  <1t> max_value;
  dim_t i[MAX_DIM];
  size_t jump_offset[MAX_DIM];
  size_t area[MAX_DIM];
  size_t subarea[MAX_DIM];
  size_t subdim[MAX_DIM];

  area[0] = 1;
  subarea[0] = 1;
  i[0] = start[0];
  sub[0] = start[0];
  jump_offset[0] = 1;
  subdim[0] = end[0] - start[0] + 1;

  ndim_t k;
  for (k = 1; k < dm->ndim; k++) {
    area[k] = area[k-1] * dm->dim[k-1];
    subdim[k] = end[k] - start[k] + 1;
    subarea[k] = subarea[k-1] * subdim[k-1];
    /* see the M document for jump offset calculation. */
    jump_offset[k] = jump_offset[k-1] + area[k] - subdim[k-1] * area[k-1];
    i[k] = start[k];
    sub[k] = start[k];
  }

  size_t offset = Sub_To_Ind(dm->dim, dm->ndim, start);
  ndim_t cur_dim = 0;
  ndim_t lastdim = dm->ndim - 1;
  BOOL overflow = FALSE;

  max_value = dm->array[offset];

  while (!overflow) {
    if (cur_dim == 0) {
      if (max_value < dm->array[offset]) {
	max_value = dm->array[offset];
	for (k = 0; k < dm->ndim; k++) {
	  sub[k] = i[k];
	}
      }
      offset++;
      i[0]++;
    } else {
      offset += jump_offset[cur_dim] - 1;
      cur_dim = 0;
    }

    while (i[cur_dim] > end[cur_dim]) {
      /* reset */
      i[cur_dim] = start[cur_dim];
      cur_dim++;
      if (cur_dim > lastdim) {
	overflow = TRUE;
	break;
      } else {
	i[cur_dim]++;
      }
    }
  }

  return max_value;
}

void <2T>_Clean_Edge(<2T>* dm)
{
  dim_type offset = 0;
  dim_type i,j,k;
  for(k=0;k<dm->dim[2];k++)
    for(j=0;j<dm->dim[1];j++)
      for(i=0;i<dm->dim[0];i++) {
	if(i==0 || j==0 || k==0)
	  dm->array[offset] = 0;
	offset++;
      }
}

<1t> <2T>_Scale(<2T>* dm)
{
  size_t length = matrix_size(dm->dim,dm->ndim);
  size_t idx;
  <1t> max_value = <3t>_max(dm->array,length,&idx);
  <3t>_divc(dm->array,max_value,length);
  return max_value;
}

/**
 * Convert an array to a matrix.
 */
<2T>* <3t>2<2t>(<1t>* array,ndim_type ndim,...)
{
  int idim;
  va_list ap;
  _<2T> *object;

  object = (_<2T> *) Guarded_Malloc(sizeof(_<2T>),"Copy_<2T>");
  <2T>_Offset = ((char *) &(object-><2t>)) - ((char *) object);
  object-><2t>.array = array;

  object-><2t>.ndim = ndim;

  va_start(ap,ndim);
  for(idim=0;idim<ndim;idim++)
    object-><2t>.dim[idim] = va_arg(ap,dim_type);
  va_end(ap);

  object->asize = matrix_size(object-><2t>.dim,object-><2t>.ndim)*sizeof(<1t>);

  <2T>_Inuse += 1;

  return (&(object-><2t>));
}

<2t=fmatrix,imatrix>
/**
 * Convert a double array to a matrix of different type
 */
<2T>* darray2<2t>(double* array,ndim_type ndim,...)
{
  int idim;
  va_list ap;
  _<2T> *object;

  object = (_<2T> *) Guarded_Malloc(sizeof(_<2T>),"Copy_<2T>");
  <2T>_Offset = ((char *) &(object-><2t>)) - ((char *) object);
  //object-><2t>.array = array;

  object-><2t>.ndim = ndim;

  va_start(ap,ndim);
  for(idim=0;idim<ndim;idim++)
    object-><2t>.dim[idim] = va_arg(ap,dim_type);
  va_end(ap);

  size_t length = matrix_size(object-><2t>.dim,object-><2t>.ndim);
  size_t i;
  object-><2t>.array = (<1t>*)malloc(length*sizeof(<1t>));
  for(i=0;i<length;i++)
    object-><2t>.array[i] = (<1t>)(array[i]<2t=imatrix>+.5</t>);


  object->asize = length*sizeof(<1t>);

  <2T>_Inuse += 1;

  return (&(object-><2t>));
}
</t>

<2T> <2T>_View_Array(<1t>* array, ndim_type ndim, ...)
{
  int idim;
  va_list ap;
  <2T> matrix;

  matrix.ndim = ndim;

  va_start(ap,ndim);
  for (idim=0; idim<ndim; idim++) {
    matrix.dim[idim] = va_arg(ap,dim_type);
  }
  va_end(ap);

  matrix.array = array;

  return matrix;  
}

void <2T>_Set_Zero(<2T> *dm)
{
  size_t size = matrix_size(dm->dim, dm->ndim);
  size_t i;
  
  for(i=0;i<size;i++) {
    dm->array[i] = 0;
  }
}

<2T>* Constant_<2T>(const dim_type dim[],ndim_type ndim,<1t> value)
{
  <2T>* dm = Make_<2T>(dim,ndim);
  size_t size = matrix_size(dim,ndim);
  size_t i;
  
  for(i=0;i<size;i++) {
    dm->array[i] = value;
  }

  return dm;  
}

<2T>* <2T>_Zeros(const dim_type dim[],ndim_type ndim)
{
  return Constant_<2T>(dim,ndim,0);

  /* Another way to make zeros. Not reliable. */
  /*
  DMatrix* dm = Make_DMatrix(dim,ndim);
  bzero(dm->array,dmatrix_asize(dm));
  return dm;
  */
}

<2T>* <2T>_Ones(const dim_type dim[],ndim_type ndim)
{
  return Constant_<2T>(dim,ndim,1);
}

<2T>* <2T>_Add(<2T>* dm1,const <2T>*  dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim))
    <2t>_error("Unmatched matrix size in %s","<2T>_Add");

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_add(dm1->array,dm2->array,length);
  
  return dm1;
}

<2T>* <2T>_Addc(<2T>* dm1,<1t> d)
{
  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_addc(dm1->array,d,length);
  
  return dm1;
}

<2T>* <2T>_Sub(<2T>* dm1,const <2T>*  dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim))
    <2t>_error("Unmatched matrix size in %s","<2T>_Add");

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_sub(dm1->array,dm2->array,length);
  
  return dm1;
}

<2T>* <2T>_Subc(<2T>* dm1,<1t> d)
{
  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_subc(dm1->array,d,length);
  
  return dm1;  
}

<2T>* <2T>_Mul(<2T>* dm1,const <2T>*  dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim))
    <2t>_error("Unmatched matrix size in %s","<2T>_Add");

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_mul(dm1->array,dm2->array,length);
  
  return dm1;
}

<2T>* <2T>_Div(<2T>* dm1,const <2T>*  dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim))
    <2t>_error("Unmatched matrix size in %s","<2T>_Add");

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  printf("%d\n", length);
  <3t>_div(dm1->array,dm2->array,length);
  
  return dm1;  
}

<2T>* <2T>_Sqr(<2T>* dm1)
{
  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_sqr(dm1->array,length);
  
  return dm1;  
}

<2T>* <2T>_Sqrt(<2T>* dm1)
{
  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_sqrt(dm1->array,length);
  
  return dm1;  
}

<2T>* <2T>_Negative(<2T> *dm)
{
  size_t length = matrix_size(dm->dim,dm->ndim);
  int i;
  for (i = 0; i < length; i++) {
    dm->array[i] = -dm->array[i];
  }

  return dm;
}

<2T>* <2T>_Max2(<2T> *dm1, const <2T> *dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim)) {
    <2t>_error("Unmatched matrix size in %s","<2T>_Max2");
  }

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_max2(dm1->array,dm2->array,length);
  
  return dm1;  
}

<2T>* <2T>_Min2(<2T> *dm1, const <2T> *dm2)
{
  if(Compare_Dim(dm1->dim,dm1->ndim,dm2->dim,dm2->ndim)) {
    <2t>_error("Unmatched matrix size in %s","<2T>_Min2");
  }

  size_t length = matrix_size(dm1->dim,dm1->ndim);
  <3t>_min2(dm1->array,dm2->array,length);
  
  return dm1;    
}

<2T>* <2T>_Partial_Diff(const <2T> *dm, ndim_type dim, <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(dm->dim, dm->ndim); 
  }

  size_t stride = 1;
  dim_type i, j;
  for (i = 0; i < dim; i++) {
    stride *= dm->dim[i];
  }

#ifdef _DEBUG_
  printf("stride: %d\n", stride);
#endif

  size_t nline = 1;
  for (i = 0; i < dm->ndim; i++) {
    if (i != dim) {
      nline *= dm->dim[i];
    }
  }
  
  size_t offset = 0;
  size_t offset2 = 0;
  size_t counter = 1;

  for (i = 0; i < nline; i++) {
    /* reset offset */
    offset = offset2;
    result->array[offset] = dm->array[offset + stride] - dm->array[offset];
    offset += stride;
    /* internal points */
    for (j = 2; j < dm->dim[dim]; j++) {
      result->array[offset] = 
	(dm->array[offset + stride] - dm->array[offset - stride]) / 2.0;
      offset += stride;
    }
    result->array[offset] = dm->array[offset] - dm->array[offset - stride];
    
    counter++;
    if (counter > stride) {
      counter = 1;
      offset2 += stride * (dm->dim[dim] - 1) + 1;
    } else {
      offset2++;
    }
  }

  return result;
}

void <2T>_Threshold(<2T> *dm, <1t> threshold)
{
  size_t length = Matrix_Size(dm->dim, dm->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    if (dm->array[i] <= threshold) {
      dm->array[i] = 0;
    }
  }
}

<1t=double,float>
void <2T>_Abs_Threshold(<2T> *dm, <1t> threshold)
{
  size_t length = Matrix_Size(dm->dim, dm->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    if (fabs(dm->array[i]) <= threshold) {
      dm->array[i] = 0;
    }
  }
}
</t>

#define MATRIX_ABS abs
<1t=double>
#undef MATRIX_ABS
#define MATRIX_ABS fabs
</t>
<1t=float>
#undef MATRIX_ABS
#define MATRIX_ABS fabsf
</t>


<2T>* <2T>_Eigen3_Coeff2(<2T> *a, <2T> *b, <2T> *c, <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(a->dim, a->ndim);
  }

  size_t length = Matrix_Size(a->dim, a->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    result->array[i] = a->array[i] + b->array[i] + c->array[i];
  }  

  return result;
}

<2T>* <2T>_Eigen3_Coeff1(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(a->dim, a->ndim);
  }

  size_t length = Matrix_Size(a->dim, a->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    result->array[i] = a->array[i] * b->array[i] + b->array[i] * c->array[i] +
      a->array[i] * c->array[i] - e->array[i] * e->array[i] - 
      d->array[i] * d->array[i] - f->array[i] * f->array[i];
  }  

  return result;
}

<2T>* <2T>_Eigen3_Coeff0(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(a->dim, a->ndim);
  }

  size_t length = Matrix_Size(a->dim, a->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    result->array[i] = a->array[i] * b->array[i] * c->array[i] + 
      2.0 * d->array[i] * e->array[i] * f->array[i] -
      a->array[i] * f->array[i] * f->array[i] - 
      c->array[i] * d->array[i] * d->array[i] - 
      b->array[i] * e->array[i] * e->array[i];
  }  

  return result;
}

<2T>* <2T>_Eigen3_Coeffd(<2T> *a, <2T> *b, <2T> *c, <2T> *d, <2T> *e, <2T> *f,
			 <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(a->dim, a->ndim);
  }

  size_t length = Matrix_Size(a->dim, a->ndim);
  size_t i;
  for (i = 0; i < length; i++) {
    if (a->array[i] + b->array[i] + c->array[i] >= 0.0) {
      result->array[i] = 0.0;
    } else {
      result->array[i] =  a->array[i] * b->array[i] + 
	b->array[i] * c->array[i] +
	a->array[i] * c->array[i] - e->array[i] * e->array[i] - 
	d->array[i] * d->array[i] - f->array[i] * f->array[i];
      
      if (result->array[i] <= 0.0) {
	result->array[i] = 0.0;
      } else {
	result->array[i] /= 1.0 +
	  Cube_Root(MATRIX_ABS(a->array[i] * b->array[i] * c->array[i] + 
			       2.0 * d->array[i] * e->array[i] * f->array[i] -
			       a->array[i] * f->array[i] * f->array[i] - 
			       c->array[i] * d->array[i] * d->array[i] - 
			       b->array[i] * e->array[i] * e->array[i]));
	
      }
      
    }
  }

  return result;
}

#ifdef _DEBUG_
#include "tz_image_lib_defs.h"
#include "tz_stack_lib.h"
#endif

<2T>* <2T>_Eigen3_Solution_Score(<2T> *a, <2T> *b, <2T> *c, <2T> *d, 
				 <2T> *e, <2T> *f, <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(a->dim, a->ndim);
  }

  size_t length = Matrix_Size(a->dim, a->ndim);
  size_t i;
 
  for (i = 0; i < length; i++) {
    if (a->array[i] + b->array[i] + c->array[i] >= 0.0) {
      result->array[i] = 0.0;
    } else {
      double coeff[3];
      coeff[0] = -a->array[i] - b->array[i] - c->array[i];
      coeff[1] = a->array[i] * b->array[i] + 
	b->array[i] * c->array[i] +
	a->array[i] * c->array[i] - e->array[i] * e->array[i] - 
	d->array[i] * d->array[i] - f->array[i] * f->array[i];
      coeff[2] = -a->array[i] * b->array[i] * c->array[i] - 
	2.0 * d->array[i] * e->array[i] * f->array[i] +
	a->array[i] * f->array[i] * f->array[i] + 
	c->array[i] * d->array[i] * d->array[i] + 
	b->array[i] * e->array[i] * e->array[i];
      if (Solve_Cubic(1.0, coeff[0], coeff[1], coeff[2], coeff) > 0){
	double tmp;
	if (coeff[0] < coeff[1]) {
	  SWAP2(coeff[0], coeff[1], tmp);
	  if (coeff[0] < coeff[2]) {
	    SWAP2(coeff[0], coeff[2], tmp);
	  }
	}


	if ((coeff[1] >= 0) || (coeff[2] >= 0)) {
	  result->array[i] = 0.0;
	} else {
	  result->array[i] = sqrt(coeff[1] * coeff[2]);
	  if (coeff[0] > 0.0) {
	    coeff[0] = -coeff[0] / 2.0;
	  }
	}
	result->array[i] += coeff[0];
	if (result->array[i] < 0.0) {
	  result->array[i] = 0.0;
	}
      } else {
	result->array[i] = 0.0;
      }
    }
  }

  return result;
}

<2T>* <2T>_Eigen3_Curvature(<2T> *xx, <2T> *yy, <2T> *zz, 
			    <2T> *x, <2T> *y, <2T> *z, <2T> *result)
{
  if (result == NULL) {
    result = Make_<2T>(xx->dim, xx->ndim);
  }

  size_t length = Matrix_Size(xx->dim, xx->ndim);
  size_t i;
  double t;
  for (i = 0; i < length; i++) {
    t = sqrt(1.0 + x->array[i] * x->array[i]);
    result->array[i] = xx->array[i] / t /t /t;
    t = sqrt(1.0 + y->array[i] * y->array[i]);
    result->array[i] += yy->array[i] / t /t /t;
    t = sqrt(1.0 + z->array[i] * z->array[i]);
    result->array[i] += zz->array[i] / t /t /t;
  }  

  <2T>_Negative(result);

  return result;  
}
