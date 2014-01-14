#include <math.h>
#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_<1t>.h"


/*************
 * nx * ny array
 *
 * ----------ny------->
 * |
 * |
 * nx
 * |
 *\|/
 * '
 * for fftw, the multi-dimensional arrays are expected to be stored as
 * a single contiguous block in row-major order, i.e. the first dimension's
 * index varies most slowly and the last dimension's index varies most quickly.
 */

/* <1t>_complex_comp(): Get components of a complex array.
 *
 * Args: array - the complex array for component extraction;
 *       length - length of the array;
 *       comp - which component to extract: 0 : real; 1 : imaginary; 
 *              2 : amplitude; 3 : phase (not implemented yet).
 * 
 * Return: the array of the extracted components.
 */
<1t>_comp* <1t>_complex_comp(<1t>_complex* array,long length,int comp) 
{
  ASSERT(array == NULL, "Null input array.");

  long i;
  <1t>_comp* comp_array = (<1t>_comp*)<1t>_malloc(length*(sizeof(<1t>_comp)));
  if(comp==0 || comp==1) {
    for(i=0;i<length;i++) {
      comp_array[i] = array[i][comp];
    }
  } else {
    switch(comp) {
    case 2: /* amplitude */
      for(i=0;i<length;i++)
	comp_array[i] = sqrt(array[i][0]*array[i][0]+array[i][1]*array[i][1]);
      break;
    case 3: /* phase */

    default:
      fprintf(stderr,"<1t>_complex_comp: Unsupported component type.");
    }
  }

  return comp_array;
}

/* <1t>_complete_2d(): Obselete.
 *
 * Pad r2c results. 
 */
void <1t>_complete_2d(<1t>_complex* array,int nx,int ny) 
{
  int length = nx*ny;
  int x,y;
  int lidx,ridx;
  lidx = ny+1;
  ridx = length-ny-1; 
  for(y=1;y<ny;y++) {
    for(x=1;x<nx;x++) {
      array[ridx][0] = array[lidx][0];
      array[ridx][1] = array[lidx][1];
      lidx++;
      ridx--;
    }
    lidx++;
    ridx--;
  }
} 


<1t>_complex* <1t>_malloc_r2c_1d(int nx) 
{
  <1t>_complex* out;
  out = (<1t>_complex*) <1t>_malloc(sizeof(<1t>_complex)*(nx/2+1));
  return out;
}

<1t>_complex* <1t>_malloc_r2c_2d(int nx,int ny) {
  <1t>_complex* out;
  out = (<1t>_complex*) <1t>_malloc(sizeof(<1t>_complex)*nx*(ny/2+1));
  return out;
}

<1t>_complex* <1t>_malloc_r2c_3d(int nx,int ny,int nz) {
  <1t>_complex* out;
  out = (<1t>_complex*) <1t>_malloc(sizeof(<1t>_complex)*nx*ny*(nz/2+1));
  return out;
}

void <1t>_pack_c2r_result(<2t> *in,int nlast,int nbefore)
{
  int npad = (nlast/2+1)*2;
  int copy_length = nlast*sizeof(<2t>);
  int i,offset1,offset2;
  offset1 = nlast;
  offset2 = npad;

  for(i=0;i<nbefore-1;i++) {
    memmove(in+offset1,in+offset2,copy_length);
    offset1 += nlast;
    offset2 += npad;
  }
}
