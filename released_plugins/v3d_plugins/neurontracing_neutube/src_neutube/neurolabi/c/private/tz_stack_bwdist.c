/* 
 * linear time euclidean distance transform
 * Modified form F. Long's code
 */

#include <math.h>

#define FLOAT_INF (float)1E20

#define square(x) ((x) * (x))

/* 
 * dt of 1d function using squared distance 
 * user is in charge of allocating memory for label outside
 */

float *dt1d(float *f, long int * label, const long int n)
{
  /* alloc <d> */
  float *d = farray_malloc(n);
  /* alloc <v> */
  int *v = iarray_malloc(n);
  /* alloc <z> */
  float *z = farray_malloc(n + 1);
 
  int k = 0;
  v[0] = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;
  int q;
  float tmp_q;
  for (q = 1; q <= n-1; q++) 
    {
      tmp_q = f[q]+square(q);
      float s  = (tmp_q-(f[v[k]]+square(v[k])))/(float)(q-v[k]) / 2.0;
      while (s <= z[k]) 
	{
	  k--;
	  s  = (tmp_q-(f[v[k]]+square(v[k])))/(float)(q-v[k])/2.0;
	}
      k++;
      v[k] = q;
      z[k] = s;
      z[k+1] = +FLOAT_INF;
    }


  k = 0;
  
  for (q = 0; q <= n-1; q++) {
    while (z[k+1] < q)
      k++;
    d[q] = q-v[k];
    d[q] = (square(d[q])) + f[v[k]];
    if (label != NULL) {
      label[q] = v[k];
    }
  }

  /* free <v> */
  free(v);
  /* free <z> */
  free(z);

  /* return <d> */
  return d;
}

static void dt1d_m(float *f, long int * label, const long int n, float *d, int *v, float *z)
{ 
  int k = 0;
  v[0] = 0;
  z[0] = -FLOAT_INF;
  z[1] = +FLOAT_INF;
  int q;
  for (q = 1; q <= n-1; q++) 
    {
      float s  = ((f[q] - f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;
      while (s <= z[k]) 
	{
	  k--;
	  s  = ((f[q] - f[v[k]])/(q-v[k]) + q + v[k]) / 2.0;//(tmp_q-(f[v[k]]+square(v[k])))/(float)(q-v[k])/2.0;
	}
      k++;
      v[k] = q;
      z[k] = s;
      z[k+1] = +FLOAT_INF;
    }


  k = 0;
  
  for (q = 0; q <= n-1; q++) {
    while (z[k+1] < q)
      k++;
    d[q] = q-v[k];
    d[q] = (square(d[q])) + f[v[k]];
    if (label != NULL) {
      label[q] = v[k];
    }
  }
}

// dt of general 2d function using squared euclidean distance 
// user is in charge of allocating memory for label outside

void dt2d(float *data, long int * label, const long int *sz) 
{
  long len = lmax2(sz[0], sz[1]); //std::max(sz[0],sz[1]);
  float *f = farray_malloc(len); //new float[len];
  long int tmp_j;
  long int i,j;
  long int *lab1 = (long int *) malloc(sz[0] * sz[1] * sizeof(long int));//new long int [sz[0]*sz[1]];
  long int *lab2 = (long int *) malloc(sz[0] * sz[1] * sizeof(long int)); //new long int [sz[0]*sz[1]];
	
	
  for (i = 0; i < sz[0]; i++) 
    {
      for (j = 0; j < sz[1]; j++) 
	{
	  f[j] = *(data + j*sz[0] + i); 
	}
		
      float *d = dt1d(f, lab1 + i*sz[1], sz[1]);
		
      for (j = 0; j < sz[1]; j++) 
	{
	  *(data + j*sz[0] + i) = d[j];
	}
      //delete [] d; 
      free(d);
      d = 0;
    }

  for (j = 0; j < sz[1]; j++) 
    {
      tmp_j = j*sz[0];
		
      for (i = 0; i < sz[0]; i++) 
	{
	  f[i] = *(data + tmp_j + i); 
	}
		
      float *d = dt1d(f, lab2 + j*sz[0], sz[0]);
		
      for (i = 0; i < sz[0]; i++) 
	{
	  long tmp = tmp_j + i;
	  *(data + tmp) = d[i];
	  long ii,jj;
		  
	  ii = *(lab2 + j*sz[0] + i);
	  jj = *(lab1 + ii*sz[1] +j);
		  
	  *(label + tmp) = jj*sz[0] + ii;
	}
		
      //delete [] d;
      free(d);
      d = 0;
    }
  /*	  
  delete []f; f = 0;
  delete []lab1; lab1 = 0;
  delete []lab2; lab2 = 0;
  */
  free(f);
  free(lab1);
  free(lab2);
	  
  return;
	  
}
 
// dt of general 3d function using squared euclidean distance
// user is in charge of allocating memory for label outside

void dt3d(float *data, long int * label, const long int *sz) 
{
	
  long int i,j,k;
	
  long int sz10 = sz[1]*sz[0];
  long int sz20 = sz[2]*sz[0];
  //long int sz12 = sz[1]*sz[2];
  //  long int count;
  long int tmp_k, tmp_j;
	
  long len = lmax3(sz[0], sz[1], sz[2]); //std::max(std::max(sz[0], sz[1]),sz[2]);
  long len2 = sz[0]*sz[1]*sz[2];
	
  /*
  float *f = new float [len];
  long int *lab1 = new long int [len2];
  long int *lab2 = new long int [len2];
  long int *lab3 = new long int [len2];
  */
  float *f = farray_malloc(len);
  
  float *d = farray_malloc(len);
  int *v = iarray_malloc(len);
  float *z = farray_malloc(len + 1);

  long int *lab1 = NULL;
  long int *lab2 = NULL;
  long int *lab3 = NULL;
  
  if (label != NULL) {
    lab1 = (long int *) malloc(len2 * sizeof(long int));
    lab2 = (long int *) malloc(len2 * sizeof(long int));
    lab3 = (long int *) malloc(len2 * sizeof(long int));
  }

  for (k = 0; k<sz[2]; k++)
    {
      tmp_k =  k*sz10;
		
      for (j = 0; j < sz[1]; j++) 
	{
	  tmp_j = j*sz[0];
			
	  for (i = 0; i < sz[0]; i++) 
	    {
	      f[i] = *(data + tmp_k + tmp_j + i); 
	    }
			
	  if (lab1 == NULL) {
	    dt1d_m(f, NULL, sz[0], d, v, z);  
	  } else {
	    dt1d_m(f, lab1+tmp_k+tmp_j, sz[0], d, v, z);  
	  }

	  for (i = 0; i < sz[0]; i++) 
	    {					
	      *(data + tmp_k + tmp_j + i) = d[i];
	    }
	}	
    }
	
  
  for (k = 0; k < sz[2]; k++)
    {
      tmp_k =  k*sz10;
		
      for (i = 0; i < sz[0]; i++) 
	{
			
	  for (j = 0; j < sz[1]; j++) 
	    {
				
	      f[j] = *(data + tmp_k + j*sz[0] + i); 
	    }
			
	  if (lab2 == NULL) {
	    dt1d_m(f, NULL, sz[1], d, v, z);
	  } else {
	    dt1d_m(f, lab2+k*sz10+i*sz[1], sz[1], d, v, z);
	  }
			
	  for (j = 0; j < sz[1]; j++) 
	    {			
	      *(data + tmp_k + j*sz[0] + i) = d[j];			
	    }

	}	
    }
	


  for (j = 0; j<sz[1]; j++)
    {		
      tmp_j = j*sz[0];
		
      for (i = 0; i < sz[0]; i++) 
	{			
	  for (k = 0; k < sz[2]; k++) 
	    {
	      f[k] = *(data + k*sz10 + tmp_j + i); 
	    }
	
	  if (lab3 == NULL) {
	    dt1d_m(f, NULL, sz[2], d, v, z);
	  } else {
	    dt1d_m(f, lab3+j*sz20+i*sz[2], sz[2], d, v, z);
	  }

	  for (k = 0; k < sz[2]; k++) 
			
	    {

	      *(data + k*sz10 + tmp_j + i) = d[k]; 
	    }			
	}	
    }


  free(f);
		
  // assign pixel index
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

  /*
  delete [] f; f = 0;
  delete [] lab1; lab1 = 0;
  delete [] lab2; lab2 = 0;
  delete [] lab3; lab3 = 0;
  */

  free(d);
  free(v);
  free(z);
  
}


// distance transform of binary 2d using squared distance 
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel

void dt2d_binary(float *data, long int * label, const long int *sz, unsigned char tag) 
{
  long int i,j;
  float * ptr;
  long int tmp_j;
	
  for (j = 0; j < sz[1]; j++)
    {
      tmp_j = j*sz[0];
		
      for (i = 0; i < sz[0]; i++)
	{
	  ptr = data + tmp_j + i;
			
	  if (tag ==0)
	    if ( *ptr > 0 )
	      *ptr = 0;
	    else
	      *ptr = FLOAT_INF;
	  else
	    if ( *ptr > 0 )
	      *ptr = FLOAT_INF;
	    else
	      *ptr = 0;
				
	}
    }
  dt2d(data, label, sz);
	
}

// distance transform of binary 3d using squred distance
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel

void dt3d_binary(float *data, long int * label, const long int *sz, unsigned char tag) 
{
  long int i,j,k;
  float *ptr;
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
				
	      if (tag == 0) //calculate the distance transform for zero value pixels
		if ( *ptr > 0 )
		  *ptr = 0;
		else
		  *ptr = FLOAT_INF;
	      else //calculate the distance transform for non-zero value pixels
		if ( *ptr > 0 )
		  *ptr = FLOAT_INF;
		else
		  *ptr = 0;

	    }
	}
    }
	
  dt3d(data, label, sz);
	
}
