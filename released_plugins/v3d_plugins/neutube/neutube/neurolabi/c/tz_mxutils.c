#include <stdio.h>
#include <stdlib.h>
#include "tz_error.h"
#include "tz_mxutils.h" 


void matrix_error(const char *msg,const char *arg)
{
  fprintf(stderr,"\nError in tz_mxutils:\n");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

void check_matrix(const dim_type dim[],ndim_type ndim)
{
  if(ndim<=0 || ndim>TZ_MATRIX_MAX_DIM) {
    TZ_WARN(ERROR_DATA_VALUE);
    matrix_error("Invalid dimension number",NULL);
  }

  ndim_type i;
  for(i=0;i<ndim;i++)
    if(dim[i]<=0)
      matrix_error("Invalid dimension",NULL);
}

size_t matrix_size(const dim_type dim[],ndim_type ndim)
{
  check_matrix(dim,ndim);

  size_t size = 1;
  ndim_type i;
  for(i=0;i<ndim;i++)
    size *= dim[i];

  return size;
}

size_t matrix_subsize(const dim_type dim[],ndim_type start,ndim_type end)
{
  ndim_type i;
  size_t subsize=1;
  for(i=start;i<=end;i++)
    subsize *= dim[i];
  
  return subsize;
}

/**
 * Convert subscripts to index
 */
size_t sub2ind(const dim_type dim[],ndim_type ndim,const dim_type sub[])
{
  ndim_type i;
  size_t idx;

  if(sub[ndim-1]+1 > dim[ndim-1] || sub[ndim-1]<0)
    matrix_error("Subscript out of range",NULL);

  idx = sub[ndim-1];
  for(i=ndim-1;i>0;i--) {
    if(sub[i-1]>dim[i-1] || sub[i-1]<0)
      matrix_error("Subscript out of range",NULL);
    idx = idx*dim[i-1]+sub[i-1];
  }

  return idx;	    
}

/**
 * Convert index to subscripts
 */
void ind2sub(const dim_type dim[],ndim_type ndim,size_t index,dim_type sub[])
{
  dim_type length = 1;
  ndim_type i;

  for(i=0;i<ndim;i++)
    length *= dim[i];

  if(index>=length)
    matrix_error("Index out of range: %s","ind2sub");

  for (i = 0; i < ndim; i++) {
    sub[i] = index % dim[i];
    index /= dim[i];
  }
  /*  
  for(i=ndim-1;i>=1;i--) {
    length /= dim[i];
    sub[i] = I/length;
    I %= length;
  }

  sub[0] = I;
  */
}

void translate_intdim(const int idim[], dim_type dim[], int ndim)
{
  int i;
  for (i = 0; i < ndim; i++) {
    if (idim[i] < 0)
      dim[i] = 0;
    else
      dim[i] = idim[i];
  }
}

void Print_Dim(const dim_t dim[], ndim_t ndim)
{
  ndim_t i;
  for (i = 0; i < ndim; i++) {
    printf(DIM_T_FORMAT, dim[i]);
    printf(" ");
  }
  printf("\n");
}

void Matrix_Overlap(const dim_type dim[],const dim_type bdim[],const dim_type offset[],ndim_type ndim,dim_type op1[],dim_type op2[])
{
  ndim_type i;
  for(i=0;i<ndim;i++) {
    op1[i*2] = imax2(0,(int)(bdim[i])-1-offset[i]);
    op1[i*2+1] = imin2(bdim[i]-1,bdim[i]+dim[i]-2-offset[i]);
    op2[i*2] = imax2(0,(int)(offset[i])-bdim[i]+1);
    op2[i*2+1] = imin2(dim[i]-1,offset[i]);
  }
}

int Compare_Dim(const dim_type dim1[],ndim_type ndim1,
		const dim_type dim2[],ndim_type ndim2)
{
  if(ndim1!=ndim2)
    return 1;
  
  int i;
  for(i=0;i<ndim1;i++)
    if(dim1[i] != dim2[i]) return 2;

  return 0;
}

dim_type* Get_Area_Part(dim_type bdim[],dim_type dim[],dim_type start[],dim_type end[],ndim_type ndim)
{
  dim_type area_dim[TZ_MATRIX_MAX_DIM];
  dim_type i,j;
  for(i=0;i<ndim;i++) {
    if (end == NULL) {
      area_dim[i] = dim[i];
    } else {
      area_dim[i] = end[i] + 1;
    }
    if (start != NULL) {
      area_dim[i] -= start[i];
    }
  }

  dim_type length = matrix_size(area_dim,ndim);
  dim_type *array = (dim_type *)malloc(sizeof(dim_type)*length);
  dim_type sub[TZ_MATRIX_MAX_DIM];
  
  for(i=0;i<length;i++) {
    array[i] = 1;
    ind2sub(area_dim,ndim,i,sub);
    for(j=0;j<ndim;j++) {
      array[i] *= imin2(bdim[j]+dim[j]-2,sub[j]+start[j]+bdim[j]-1)
	-imax2(sub[j]+start[j],bdim[j]-2)+1;
    }
  }

  return array;
}
