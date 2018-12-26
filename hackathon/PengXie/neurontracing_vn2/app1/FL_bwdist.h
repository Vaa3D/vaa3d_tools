// linear time euclidean distance transform
// note the real distance should be the sqare root of the output data 

// reference: Pedro F. Felzenszwalb, Daniel P. Hutenlocher, Distance transforms of Sampled Functions 
// F. Long
// 20080601

#ifndef __FL_BWDIST__
#define __FL_BWDIST__

#include <algorithm>
//#include <math>

#define INF 1E20

template <class T> inline T square(const T &x) { return x*x; };

// dt of 1d function using squared distance 
// user is in charge of allocating memory for label outside

//int data type might do

float *dt1d(float *f, V3DLONG * label, const V3DLONG n)
{
	float *d = new float[n];
	int *v = new int[n];
	float *z = new float[n+1];
    
	int k = 0;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;
	for (int q = 1; q <= n-1; q++) 
	{
		float s  = ((f[q]+square(q))-(f[v[k]]+square(v[k])))/(2*q-2*v[k]);
		while (s <= z[k]) 
		{
			k--;
			s  = ((f[q]+square(q))-(f[v[k]]+square(v[k])))/(2*q-2*v[k]);
		}
		k++;
		v[k] = q;
		z[k] = s;
		z[k+1] = +INF;
	}
	
	k = 0;
	
	for (int q = 0; q <= n-1; q++) {
		while (z[k+1] < q)
			k++;
		d[q] = float(square(q-v[k])) + f[v[k]];
		
		label[q] = v[k];
		
	}
	
	if (v) {delete [] v; v=0;}
	if (z) {delete [] z; z=0;}
	return d;
}

// dt of general 2d function using squared euclidean distance 
// user is in charge of allocating memory for label outside

void dt2d(float *data, V3DLONG * label, const V3DLONG *sz) 
{
	V3DLONG len = std::max(sz[0],sz[1]);
	float *f = new float[len];
	V3DLONG tmp_j;
	V3DLONG i,j,k;
	V3DLONG *lab1 = new V3DLONG [sz[0]*sz[1]];
	V3DLONG *lab2 = new V3DLONG [sz[0]*sz[1]];
	
	// transform along columns
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
		if (d) {delete [] d; d = 0;}
	}
	
	// transform along rows
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
			V3DLONG tmp = tmp_j + i;
			*(data + tmp) = d[i];
			V3DLONG ii,jj;
			
			ii = *(lab2 + j*sz[0] + i);
			jj = *(lab1 + ii*sz[1] +j);
			
			*(label + tmp) = jj*sz[0] + ii;
		}
		
		if (d) {delete [] d; d = 0;}
	}
	
	if (f) {delete []f; f = 0;}
	if (lab1) {delete []lab1; lab1 = 0;}
	if (lab2) {delete []lab2; lab2 = 0;}
	
	return;
}

// dt of general 3d function using squared euclidean distance
// user is in charge of allocating memory for label outside

void dt3d(float *data, V3DLONG * label, const V3DLONG *sz) 
{
	
	V3DLONG i,j,k;
	
	V3DLONG sz10 = sz[1]*sz[0];
	V3DLONG sz20 = sz[2]*sz[0];
	V3DLONG sz12 = sz[1]*sz[2];
	V3DLONG count;
	V3DLONG tmp_k, tmp_j;
	
	V3DLONG len = std::max(std::max(sz[0], sz[1]),sz[2]);
	V3DLONG len2 = sz[0]*sz[1]*sz[2];
	
	float *f = new float [len];
	V3DLONG *lab1 = new V3DLONG [len2];
	V3DLONG *lab2 = new V3DLONG [len2];
	V3DLONG *lab3 = new V3DLONG [len2];
	
	
	// transform along the i dimension
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
			
			float *d = dt1d(f, lab1+tmp_k+tmp_j, sz[0]);  
			
			for (i = 0; i < sz[0]; i++) 
			{					
				*(data + tmp_k + tmp_j + i) = d[i];
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	// transform along the j dimension
	
	for (k = 0; k < sz[2]; k++)
	{
		tmp_k =  k*sz10;
		
		for (i = 0; i < sz[0]; i++) 
		{
			
			for (j = 0; j < sz[1]; j++) 
			{
				
				f[j] = *(data + tmp_k + j*sz[0] + i); 
			}
			
			float *d = dt1d(f, lab2+k*sz10+i*sz[1], sz[1]);
			
			for (j = 0; j < sz[1]; j++) 
			{			
				*(data + tmp_k + j*sz[0] + i) = d[j];			
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	
	// transform along the k dimension
	
	for (j = 0; j<sz[1]; j++)
	{		
		tmp_j = j*sz[0];
		
		for (i = 0; i < sz[0]; i++) 
		{			
			for (k = 0; k < sz[2]; k++) 
			{
				f[k] = *(data + k*sz10 + tmp_j + i); 
			}
			
			float *d = dt1d(f, lab3+j*sz20+i*sz[2], sz[2]);
			
			for (k = 0; k < sz[2]; k++) 
			{
				*(data + k*sz10 + tmp_j + i) = d[k]; 
			}
			
			if (d) {delete [] d; d = 0;}
		}	
	}
	
	// assign pixel index
	V3DLONG ii,jj,kk;
	
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
	
	
	if (f) {delete [] f; f = 0;}
	if (lab1) {delete [] lab1; lab1 = 0;}
	if (lab2) {delete [] lab2; lab2 = 0;}
	if (lab3) {delete [] lab3; lab3 = 0;}
	
	return;
}


// distance transform of binary 2d using squared distance 
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel

//note input and output share the same array, thus input arrary will be changed after calling dt2d_binary
void dt2d_binary(float *data, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	
	V3DLONG len = sz[0]*sz[1];
	
	for (V3DLONG i=0; i<len; i++)
	{
		
		if (tag ==0)
			if (data[i]>0)
				data[i] = 0;
			else
				data[i] = INF;
		else
			if (data[i]>0)
				data[i] = INF;
		else
			data[i] = 0;
	}
	
	dt2d(data, label, sz);	
}


//note input and output use separate array, thus input arrary will not be changed after calling dt2d_binary
void dt2d_binary(float *indata, float *outdata, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	
	V3DLONG len = sz[0]*sz[1];
	
	for (V3DLONG i=0; i<len; i++)
	{
		
		if (tag ==0)
			if (indata[i]>0)
				outdata[i] = 0;
			else
				outdata[i] = INF;
		else
			if (indata[i]>0)
				outdata[i] = INF;
		else
			outdata[i] = 0;
	}
	
	dt2d(outdata, label, sz);	
}


// distance transform of binary 3d using squred distance
// user is in charge of allocating memory for label outside
// tag indicate whether to compute the distance transform for zero or non-zero values
// label returns the linear index of the nearest non-zero (when tag = 0) or zero (when tag = 1)  pixel

// note input and output share the same array, thus input arrary will be changed after calling dt3d_binary
void dt3d_binary(float *data, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	
	V3DLONG len = sz[0]*sz[1]*sz[2];
	
	for (V3DLONG i=0; i<len; i++)
	{
		
		if (tag ==0)
		{
			if (data[i]>0)
				data[i] = 0;
			else
				data[i] = INF;
		}
		else
		{
			if (data[i]>0)
				data[i] = INF;
			else
				data[i] = 0;
		}
	}
	
	dt3d(data, label, sz);
}

// note input and output use separate array, thus input arrary will not be changed after calling dt3d_binary
void dt3d_binary(float *indata, float *outdata, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	
	V3DLONG len = sz[0]*sz[1]*sz[2];
	
	for (V3DLONG i=0; i<len; i++)
	{
		
		//		outdata[i] = indata[i];
		
		if (tag ==0)
		{
			if (indata[i]>0)
				outdata[i] = 0;
			else
				outdata[i] = INF;
		}
		else
		{
			if (indata[i]>0)
				outdata[i] = INF;
			else
				outdata[i] = 0;
		}
	}
	
	dt3d(outdata, label, sz);
	
}



template <class T1, class T2> void dt2d_binary(T1 *indata, T2 *outdata, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	V3DLONG len = sz[0]*sz[1];
	V3DLONG i;
	
	float *tmpdata = new float [len];
	
	for (i=0; i<len; i++)
	{
		//		tmpdata[i] = (float)indata[i];
		
		if (tag ==0) // compute dt on background (zeros) pixels, with respect to foreground (non-zero) pixels
		{
			if (indata[i]>0)
				tmpdata[i] = 0;
			else
				tmpdata[i] = INF;
		}
		else // compute dt on foreground (non-zeros) pixels, with respect to background (zero) pixels
		{
			if (indata[i]>0)
				tmpdata[i] = INF;
			else
			tmpdata[i] = 0;
		}
	}
	
	dt2d(tmpdata, label, sz);
	
	for (i=0; i<len; i++)
		outdata[i] = (T2)tmpdata[i];
	
	if (tmpdata) {delete [] tmpdata; tmpdata =0;}
	
}

template <class T1, class T2> void dt3d_binary(T1 *indata, T2 *outdata, V3DLONG * label, const V3DLONG *sz, unsigned char tag) 
{
	V3DLONG len = sz[0]*sz[1]*sz[2];
	V3DLONG i;
	
	float *tmpdata = new float [len];
	
	float *tmpdatap = tmpdata, *endp=tmpdata+len;
	T1 *indatap = indata;
	for (tmpdatap=tmpdata; tmpdatap<endp; ++tmpdatap)
	{
		if (tag ==0)
			*tmpdatap = (*indatap++> 0) ? 0 : INF;
		else
			*tmpdatap = (*indatap++ > 0) ? INF : 0;
	}
	
	dt3d(tmpdata, label, sz);

	T2 *outdatap, *outdatap_end = outdata+len;
	for (outdatap=outdata, tmpdatap=tmpdata; outdatap<outdatap_end; ++outdatap)
		*outdatap = (T2)(*tmpdatap++);
	
	if (tmpdata) {delete [] tmpdata; tmpdata =0;}
}

#endif
