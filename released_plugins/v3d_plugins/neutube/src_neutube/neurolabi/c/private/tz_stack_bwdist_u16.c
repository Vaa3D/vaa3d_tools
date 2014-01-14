/* 
 * linear time euclidean distance transform
 * Modified form F. Long's code
 */

#include <math.h>

/*
 * f - mask
 * d - place to store data
 * v, z - workspace
 */
static void dt1d_first_m_u16(uint16 *d, const long int n, uint16 *f, int *v, float *z)
{ 
  int k = 0;
  v[0] = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;
  int q;
  float s;
  for (q = 1; q <= n - 1; q++) {
    if (f[v[k]] == f[q]) {
      s = (float)(q + v[k]) / 2.0;
    } else if (f[v[k]] == 0) { /* background */
      s  = (FLOAT_INF/(q-v[k]) + q + v[k]) / 2.0;
    } else {
      s = (-FLOAT_INF/(q-v[k]) + q + v[k]) / 2.0;
    }
    
    /* to do here */
    while (s <= z[k]) {
      k--;	
      if (f[v[k]] == f[q]) {
	s = (float)(q + v[k]) / 2.0;
      } else if (f[v[k]] == 0) { /* background */
	s  = (FLOAT_INF/(q-v[k]) + q + v[k]) / 2.0;
      } else {
	s = (-FLOAT_INF/(q-v[k]) + q + v[k]) / 2.0;
      }
    }

    k++;
    v[k] = q;
    z[k] = s;
    z[k+1] = +FLOAT_INF;
  }


  k = 0;
  for (q = 0; q <= n - 1; q++) {
    while (z[k+1] < q) {
      k++;
    }
    if (f[q] > 0) {
      d[q] = abs(q-v[k]);
      if (d[q] == 0) {
	d[q] = 1;
      }
    }
  }
}

static void dt1d_second_m_u16(uint16 *f, const long int n, uint16 *d, int *v, float *z)
{ 
  int k = 0;
  v[0] = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;
  int q;
  float s;
  for (q = 1; q <= n - 1; q++) {
    s  = (((float) f[q] - f[v[k]]) * (f[q] + f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;

    while (s <= z[k]) {
      k--;	
      s  = (((float) f[q] - f[v[k]]) * (f[q] + f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;
    }

    k++;
    v[k] = q;
    z[k] = s;
    z[k+1] = +FLOAT_INF;
  }


  k = 0;
  int tmp_q;
  for (q = 0; q <= n -1; q++) {
    while (z[k+1] < q)
      k++;
    if (f[q] > 0) {
      tmp_q = q-v[k];
      tmp_q = tmp_q * tmp_q + f[v[k]] * f[v[k]];
      if (tmp_q > 65535) {
	printf("here");
	TZ_WARN(ERROR_DATA_VALUE);
	d[q] = 65535;
      } else {
	d[q] = tmp_q;
      }
    } else {
      d[q] = 0;
    }
    /*
    if (label != NULL) {
      label[q - 1] = v[k];
    }
    */
  }
}

static void dt1d_m_u16(uint16 *f, const long int n, uint16 *d, int *v, float *z)
{ 
  int k = 0;
  v[0] = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;
  int q;
  float s;
  for (q = 1; q < n; q++) {
    s  = (((float) f[q] - f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;

    while (s <= z[k]) {
      k--;	
      s  = (((float) f[q] - f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;
    }

    k++;
    v[k] = q;
    z[k] = s;
    z[k+1] = +FLOAT_INF;
  }


  k = 0;
  int tmp_q;
  for (q = 0; q < n; q++) {
    while (z[k+1] < q)
      k++;
    if (f[q] > 0) {
      tmp_q = q-v[k];
      tmp_q = tmp_q * tmp_q + f[v[k]];
      if (tmp_q > 65535) {
	TZ_WARN(ERROR_DATA_VALUE);
	d[q] = 65535;
      } else {
	d[q] = tmp_q;
      }
    } else {
      d[q] = 0;
    }
  }
}

 
// dt of general 3d function using squared euclidean distance
// user is in charge of allocating memory for label outside

void dt3d_u16(uint16 *data, const long int *sz, int pad) 
{
	
  long int i,j,k;
	
  long int sz10 = sz[1]*sz[0];
  //long int sz20 = sz[2]*sz[0];
  //long int sz12 = sz[1]*sz[2];
  //  long int count;
  long int tmp_k, tmp_j;
	
  long len = lmax3(sz[0], sz[1], sz[2]); //std::max(std::max(sz[0], sz[1]),sz[2]);
  //long len2 = sz[0]*sz[1]*sz[2];
	
  /*
  float *f = new float [len];
  long int *lab1 = new long int [len2];
  long int *lab2 = new long int [len2];
  long int *lab3 = new long int [len2];
  */

  len += pad * 2;

  uint16 *f = u16array_malloc(len);
  uint16 *d = u16array_malloc(len);
  int *v = iarray_malloc(len);
  float *z = farray_malloc(len + 1);

  /*
  long int *lab1 = NULL;
  long int *lab2 = NULL;
  long int *lab3 = NULL;
  
  if (label != NULL) {
    lab1 = (long int *) malloc(len2 * sizeof(long int));
    lab2 = (long int *) malloc(len2 * sizeof(long int));
    lab3 = (long int *) malloc(len2 * sizeof(long int));
  }
  */
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

	  /*	
	  for (i = 0; i < sz[0]; i++) 
	    {
	      f[i] = *(data + tmp_k + tmp_j + i); 
	    }
	  */
		
	  memcpy(d + pad, data + tmp_k + tmp_j, sizeof(uint16) * sz[0]);
	  dt1d_first_m_u16(d, sz[0] + pad * 2, f, v, z);  
	  memcpy(data + tmp_k + tmp_j, d + pad, sizeof(uint16) * sz[0]);
	  
	  /*
	  for (i = 0; i < sz[0]; i++) 
	    {					
	      *(data + tmp_k + tmp_j + i) = d[i];
	    }
	  */
	}	
    }
	
  if (pad == 1) {
    f[0] = 0;
    f[sz[1] + 1] = 0;
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
	    }		

	    dt1d_second_m_u16(f, sz[1] + pad * 2, d, v, z);
			
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
	    }
	
	  dt1d_m_u16(f, sz[2] + pad * 2, d, v, z);
	  
	  for (k = pad; k < sz[2] + pad; k++) 
			
	    {

	      *(data + (k - pad)*sz10 + tmp_j + i) = d[k]; 
	    }			
	}	
    }
  
  
  free(f);
		
  // assign pixel index
  /*
  if (label != NULL) {
    long int ii,jj,kk;
	
    for (i = 0; i<sz[0]; i++)
      {		
	for (j = 0; j < sz[1]; j++) 
	  {			
	    for (k = 0; k < sz[2]; k++) 
	      {
		kk = *(lab3+j*sz20+i*sz[2]+k);
		jj = *(lab2+kk*sz10+i*sz[1]+j);
		ii = *(lab1+kk*sz10+jj*sz[0]+i);
		
		*(label + k*sz10+j*sz[0]+i) = kk*sz10 + jj*sz[0] + ii;
		
	      }
	  }	
	
      }
 
    free(lab1);
    free(lab2);
    free(lab3);
    }
  */
  free(d);
  free(v);
  free(z);

  /*
  delete [] f; f = 0;
  delete [] lab1; lab1 = 0;
  delete [] lab2; lab2 = 0;
  delete [] lab3; lab3 = 0;
  */


  return;
  
}

// distance transform of binary 3d using squred distance
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel
