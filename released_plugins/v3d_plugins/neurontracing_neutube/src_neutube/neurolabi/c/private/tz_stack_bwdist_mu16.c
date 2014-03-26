/* 
 * linear time euclidean distance transform
 * Modified form F. Long's code
 */

#include <math.h>
#include "../tz_u8array.h"

#ifndef UINT16_INF
#define UINT16_INF 0xFFFF
#endif

#ifndef FLOAT_INF
#define FLOAT_INF 1E20
#endif

/*
 * f - mask
 * d - place to store data
 * v, z - workspace
 */
static void dt1d_first_m_mu16(uint16 *d, const long int n, uint16 *f, int *v, float *z)
{ 
  int q;
  for (q = 1; q < n; q++) { 
    if (f[q - 1] != f[q]) {
      break;
    }
  }

  if (q == n) { /* if all 1s or 0s */
    return;
  }

  if (f[0] == 0) {
    v[0] = 0;
  } else {
    v[0] = q;
  }

  int k = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;

  float s;
  
  for (q = v[0] + 1; q <= n - 1; q++) {
    if (f[q] == 0) {  
      s = (float)(q + v[k]) / 2.0;
      while (s <= z[k]) {
	k--;
	s = (float)(q + v[k]) / 2.0;
      } /* pop seeds */
      
      k++;
      v[k] = q;
      z[k] = s;
      z[k+1] = +FLOAT_INF;
    }
  }

  k = 0;
  for (q = 0; q <= n - 1; q++) {
    while (z[k+1] < q) {
      k++;
    }

    if (f[q] > 0) {
      d[q] = abs(q-v[k]);
    }
  }
}

static void dt1d_second_m_mu16(uint16 *f, const long int n, uint16 *d, int *v, float *z, uint8 *m, int sqr_field)
{ 
  int q;
  for (q = 0; q < n; q++) { 
    if (m[q] == 0) {
      break;
    }
  }

  if (q == n) { /* if all 1s or 0s */
    return;
  }

  v[0] = q;

  int k = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;

  float s;

  for (q = v[0] + 1; q <= n - 1; q++) {
    if (m[q] == 0) {     
      s = f[q] - f[v[k]];
      if (sqr_field == 0) {
	s *= f[q] + f[v[k]];
      }      
      s = (s / (q - v[k]) + (float)(q + v[k])) / 2.0;
      while (s <= z[k]) {
	k--;
	s = f[q] - f[v[k]];
	if (sqr_field == 0) {
	  s *= f[q] + f[v[k]];
	}	
	s = (s / (q - v[k]) + (float)(q + v[k])) / 2.0;
      } /* pop seeds */
      
      k++;
      v[k] = q;
      z[k] = s;
      z[k+1] = +FLOAT_INF;
    }
  }

  k = 0;
  int df;
  for (q = 0; q <= n - 1; q++) {
    while (z[k+1] < q) {
      k++;
    }

    if (f[q] > 0) {
      df = q-v[k];
      if (sqr_field == 0) {
	df = df * df + f[v[k]] * f[v[k]];
      } else {
	df = df * df + f[v[k]];
      }
      if (df > UINT16_INF) {
	d[q] = UINT16_INF - 1;
      } else {
	d[q] = df;
      }
    } else {
      d[q] = 0;
    }
  }
}

 
// dt of general 3d function using squared euclidean distance
// user is in charge of allocating memory for label outside

void dt3d_mu16(uint16 *data, const long int *sz, int pad) 
{
	
  long int i,j,k;
	
  long int sz10 = sz[1]*sz[0];
  //long int sz20 = sz[2]*sz[0];
  //long int sz12 = sz[1]*sz[2];
  //  long int count;
  long int tmp_k, tmp_j;
	
  long len = lmax3(sz[0], sz[1], sz[2]); 
  len += pad * 2;

  /* alloc <f> */
  uint16 *f = u16array_malloc(len);
  /* alloc <d> */
  uint16 *d = u16array_malloc(len);
  /* alloc <m> */
  uint8 *m = u8array_malloc(len);
  /* alloc <v> */
  int *v = iarray_malloc(len);
  /* alloc <z> */
  float *z = farray_malloc(len + 1);

  if (pad == 1) {
    f[0] = 0;
    f[sz[0] + 1] = 0;
    d[0] = 0;
    d[sz[0] + 1] = 0;
  }
  for (k = 0; k<sz[2]; k++)
    {
      tmp_k =  k*sz10;
		
      for (j = 0; j < sz[1]; j++) 
	{
	  tmp_j = j*sz[0];
	  memcpy(f + pad, data + tmp_k + tmp_j, sizeof(uint16) * sz[0]);
	
	  memcpy(d + pad, data + tmp_k + tmp_j, sizeof(uint16) * sz[0]);
	  dt1d_first_m_mu16(d, sz[0] + pad * 2, f, v, z);  
	  memcpy(data + tmp_k + tmp_j, d + pad, sizeof(uint16) * sz[0]);
	}	
    }
   
  if (pad == 1) {
    f[0] = 0;
    f[sz[1] + 1] = 0;
    m[0] = 0;
    m[sz[1] + 1] = 0;
    d[0] = 0;
    d[sz[1] + 1] = 0;
  }  
  for (k = 0; k < sz[2]; k++)
    {
      tmp_k =  k*sz10;
		
      for (i = 0; i < sz[0]; i++) 
	{
	  for (j = pad; j < sz[1] + pad; j++) 
	    {				
	      f[j] = *(data + tmp_k + (j - pad) *sz[0] + i); 
	      m[j] = (f[j] == UINT16_INF);
	    }		

	  dt1d_second_m_mu16(f, sz[1] + pad * 2, d, v, z, m, 0);
			
	  for (j = pad; j < sz[1] + pad; j++) 
	    {			
	      *(data + tmp_k + (j - pad)*sz[0] + i) = d[j];			
	    }
	  
	}	
    }
	
  
  if (pad == 1) {
    f[0] = 0;
    f[sz[2] + 1] = 0;
    d[0] = 0;
    d[sz[2] + 1] = 0;
  }
  for (j = 0; j<sz[1]; j++)
    {		
      tmp_j = j*sz[0];
		
      for (i = 0; i < sz[0]; i++) 
	{
	  for (k = pad; k < sz[2] + pad; k++) 
	    {
	      f[k] = *(data + (k - pad)*sz10 + tmp_j + i); 
	      m[k] = (f[k] == UINT16_INF);
	    }
	
	  dt1d_second_m_mu16(f, sz[2] + pad * 2, d, v, z, m, 1);
	  
	  for (k = pad; k < sz[2] + pad; k++) 
			
	    {

	      *(data + (k - pad)*sz10 + tmp_j + i) = d[k]; 
	    }			
	}	
    }
  
  /* free <f> */
  free(f);
		
  /* free <d> */
  free(d);
  /* free <v> */
  free(v);
  /* free <z> */
  free(z);
  /* free <m> */
  free(m);

}

void dt3d_mu16_p(uint16 *data, const long int *sz, int pad) 
{

  long int i,j,k;

  long int sz10 = sz[1]*sz[0];
  //long int sz20 = sz[2]*sz[0];
  //long int sz12 = sz[1]*sz[2];
  //  long int count;
  long int tmp_k, tmp_j;

  long len = lmax3(sz[0], sz[1], sz[2]); 
  len += pad * 2;

  /* alloc <f> */
  uint16 *f = u16array_malloc(len);
  /* alloc <d> */
  uint16 *d = u16array_malloc(len);
  /* alloc <m> */
  uint8 *m = u8array_malloc(len);
  /* alloc <v> */
  int *v = iarray_malloc(len);
  /* alloc <z> */
  float *z = farray_malloc(len + 1);

  if (pad == 1) {
    f[0] = 0;
    f[sz[0] + 1] = 0;
    d[0] = 0;
    d[sz[0] + 1] = 0;
  }
  for (k = 0; k<sz[2]; k++)
  {
    tmp_k =  k*sz10;

    for (j = 0; j < sz[1]; j++) 
    {
      tmp_j = j*sz[0];
      memcpy(f + pad, data + tmp_k + tmp_j, sizeof(uint16) * sz[0]);

      memcpy(d + pad, data + tmp_k + tmp_j, sizeof(uint16) * sz[0]);
      dt1d_first_m_mu16(d, sz[0] + pad * 2, f, v, z);  
      memcpy(data + tmp_k + tmp_j, d + pad, sizeof(uint16) * sz[0]);
    }	
  }

  if (pad == 1) {
    f[0] = 0;
    f[sz[1] + 1] = 0;
    m[0] = 0;
    m[sz[1] + 1] = 0;
    d[0] = 0;
    d[sz[1] + 1] = 0;
  }  
  for (k = 0; k < sz[2]; k++)
  {
    tmp_k =  k*sz10;

    for (i = 0; i < sz[0]; i++) 
    {
      for (j = pad; j < sz[1] + pad; j++) 
      {				
	f[j] = *(data + tmp_k + (j - pad) *sz[0] + i); 
	m[j] = (f[j] == UINT16_INF);
      }		

      dt1d_second_m_mu16(f, sz[1] + pad * 2, d, v, z, m, 0);

      for (j = pad; j < sz[1] + pad; j++) 
      {			
	*(data + tmp_k + (j - pad)*sz[0] + i) = d[j];			
      }

    }	
  }

  /* free <f> */
  free(f);

  /* free <d> */
  free(d);
  /* free <v> */
  free(v);
  /* free <z> */
  free(z);
  /* free <m> */
  free(m);

}

// distance transform of binary 3d using squred distance
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel
void dt3d_binary_mu16(uint16 *data, const long int *sz, int pad) 
{
  long int i,j,k;
  uint16 *ptr;
  long int tmp_k, tmp_j;
  long int sz10 = sz[0]*sz[1];
	
  for  (k = 0; k < sz[2]; k++) 
    {
      tmp_k = k*sz10;
      for (j = 0; j < sz[1]; j++)
	{
	  tmp_j = j*sz[0];
			
	  for (i = 0; i < sz[0]; i++)
	    {
	      ptr = data + tmp_k + tmp_j + i;
				
	      if ( *ptr > 0 )
		*ptr = UINT16_INF;
	      else
		*ptr = 0;

	    }
	}
    }
	
  dt3d_mu16(data, sz, pad);
	
}

void dt3d_binary_mu16_p(uint16 *data, const long int *sz, int pad) 
{
  long int i,j,k;
  uint16 *ptr;
  long int tmp_k, tmp_j;
  long int sz10 = sz[0]*sz[1];
	
  for  (k = 0; k < sz[2]; k++) 
    {
      tmp_k = k*sz10;
      for (j = 0; j < sz[1]; j++)
	{
	  tmp_j = j*sz[0];
			
	  for (i = 0; i < sz[0]; i++)
	    {
	      ptr = data + tmp_k + tmp_j + i;
				
	      if ( *ptr > 0 )
		*ptr = UINT16_INF;
	      else
		*ptr = 0;

	    }
	}
    }
	
  dt3d_mu16_p(data, sz, pad);
	
}
