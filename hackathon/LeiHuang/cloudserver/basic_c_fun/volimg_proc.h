/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//volimg_proc.h
//
//Separate from laff project so that these functions will be common
//
//by Hanchuan Peng
//2008-04-14
//2008-05-03

#ifndef __BASIC_VOLUME_IMG_PROCESSING__
#define __BASIC_VOLUME_IMG_PROCESSING__

#include "volimg_proc_declare.h"

#include <math.h>
#include <stdio.h>

//the actual definitions are below

template <class T> T hardLimit(T a, T lowerBound, T upperBound)
{
	T b = (a<lowerBound) ? lowerBound : a;
	return ((b>upperBound) ? upperBound : b);
}

template <class T> void swapValue(T & a, T & b)
{
	T tmp = a; a = b; b=tmp;
	return;
}


template <class T> bool vol3d_assign(T *** res, T v, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = v;
			}
		}
	}

	return true;
}

template <class T> bool vol3d_assign(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = sa[k][j][i];
			}
		}
	}

	return true;
}

template <class T> bool vol3d_negative(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(-double(sa[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_inverse(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(1.0/double(sa[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_square(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double tmp;
	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				tmp = double(sa[k][j][i]);
				res[k][j][i] = (T)(tmp*tmp);
			}
		}
	}

	return true;
}

template <class T> bool vol3d_root(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)sqrt(double(sa[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_exp(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)exp(double(sa[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_log(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)log(double(sa[k][j][i]));
			}
		}
	}

	return true;
}


template <class T> bool vol3d_min(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double v;
	V3DLONG i,j,k;
	v = (double)sa[0][0][0];
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				v = (double(sa[k][j][i]) < v) ? double(sa[k][j][i]) : v;
			}
		}
	}
	res = (T)v;

	return true;
}

template <class T> bool vol3d_max(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double v;
	V3DLONG i,j,k;
	v = (double)sa[0][0][0];
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				v = (double(sa[k][j][i]) > v) ? double(sa[k][j][i]) : v;
			}
		}
	}
	res = (T)v;

	return true;
}


template <class T> bool vol3d_scale_01(T *** res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double vm, vM;
	V3DLONG i,j,k;
	vm = (double)sa[0][0][0];
	vM = (double)sa[0][0][0];
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				vm = (double(sa[k][j][i]) < vm) ? double(sa[k][j][i]) : vm;
				vM = (double(sa[k][j][i]) > vM) ? double(sa[k][j][i]) : vM;
			}
		}
	}

	double L = vM-vm;

	if (L==0.0)
	{
		for (k=0;k<d2;k++)
		{
			for (j=0;j<d1;j++)
			{
				for (i=0;i<d0;i++)
				{
					res[k][j][i] = (T)0;
				}
			}
		}
	}
	else
	{
		for (k=0;k<d2;k++)
		{
			for (j=0;j<d1;j++)
			{
				for (i=0;i<d0;i++)
				{
					res[k][j][i] = (T)((double(res[k][j][i])-vm)/L);
				}
			}
		}
	}

	return true;
}

template <class T> bool vol3d_sum(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double v;
	V3DLONG i,j,k;
	v = (double)0;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				v += double(sa[k][j][i]);
			}
		}
	}
	res = (T)v;

	return true;
}


template <class T> bool vol3d_mean(T & res, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	double v;
	V3DLONG i,j,k;
	v = (double)0;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				v += double(sa[k][j][i]);
			}
		}
	}
	res = (T)(v/(double(d0)*d1*d2));

	return true;
}

template <class T> bool vol3d_threshold(T *** res, V3DLONG d0, V3DLONG d1, V3DLONG d2, T thres, bool b_set_to_binary)
{
	if (!res || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	if (b_set_to_binary==false)
	{
		for (k=0;k<d2;k++)
		{
			for (j=0;j<d1;j++)
			{
				for (i=0;i<d0;i++)
				{
					if (res[k][j][i]<thres)
					  res[k][j][i] = (T)0;
				}
			}
		}
	}
	else
	{
		for (k=0;k<d2;k++)
		{
			for (j=0;j<d1;j++)
			{
				for (i=0;i<d0;i++)
				{
					if (res[k][j][i]<thres)
					  res[k][j][i] = (T)0;
					else
					  res[k][j][i] = (T)1;
				}
			}
		}
	}

	return true;
}

template <class T> bool vol3d_nnz(V3DLONG &nnz, T *** sa, V3DLONG d0, V3DLONG d1, V3DLONG d2, V3DLONG d0b, V3DLONG d0e, V3DLONG d1b, V3DLONG d1e, V3DLONG d2b, V3DLONG d2e)
{
	if (!sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	if (d0b<0 || d0b>=d0 || d0e<0 || d0e>=d0 || d0b>d0e ||
	    d1b<0 || d1b>=d1 || d1e<0 || d1e>=d1 || d1b>d1e ||
	    d2b<0 || d2b>=d2 || d2e<0 || d2e>=d2 || d2b>d2e)
		return false;

	V3DLONG i,j,k;
	nnz=0;
	for (k=d2b;k<=d2e;k++)
	{
		for (j=d1b;j<=d1e;j++)
		{
			for (i=d0b;i<=d0e;i++)
			{
				if (sa[k][j][i]) nnz++;
			}
		}
	}

	return true;
}


template <class T> bool vol3d_plus(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || !sb || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i]) + double(sb[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_plus_constant(T *** res, T *** sa, double c, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i]) + c);
			}
		}
	}

	return true;
}

template <class T> bool vol3d_minus(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || !sb || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i]) - double(sb[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_time(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || !sb || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i]) * double(sb[k][j][i]));
			}
		}
	}

	return true;
}

template <class T> bool vol3d_time_constant(T *** res, T *** sa, double c, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i]) * c);
			}
		}
	}

	return true;
}

template <class T> bool vol3d_divide(T *** res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!res || !sa || !sb || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				res[k][j][i] = (T)(double(sa[k][j][i])/double(sb[k][j][i]));
			}
		}
	}

	return true;
}


template <class T> bool vol3d_square_root_diff(double & res, T *** sa, T *** sb, V3DLONG d0, V3DLONG d1, V3DLONG d2)
{
	if (!sa || !sb || d0<=0 || d1<=0 || d2<=0)
		return false;

	V3DLONG i,j,k;
	double tmp;
	res = 0.0;
	for (k=0;k<d2;k++)
	{
		for (j=0;j<d1;j++)
		{
			for (i=0;i<d0;i++)
			{
				tmp = (double(sa[k][j][i]) - double(sb[k][j][i]));
				res += tmp*tmp;
			}
		}
	}
	res = sqrt(res);

	return true;
}

template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
{
    if (!data || n<=0)
	  return false;

	int j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = data[0];
	  sdev = (T2)0;
	  return true; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	double ave_double=(T2)(s/n); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(T2)(sqrt(var));
	ave=(T2)ave_double; //use ave_double for the best accuracy

	return true;
}

template <class T> bool mean_and_std(T *data, V3DLONG n, T & ave, T & sdev, T maskval)
{
    if (!data || n<=0)
	  return false;

	int j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = data[0];
	  sdev = (T)0;
	  return true; //do nothing
	}

	V3DLONG n_use;

	s=0.0;
	for (j=0, n_use=0;j<n;j++) {if (data[j]!=maskval) {s += data[j]; n_use++;}}
	if (n_use<=0)
	{
	  ave = data[0];
	  sdev = (T)0;
	  return true; //do nothing if everything is equal maskval
	}
	if (n_use==1)
	{
	  ave=s;
	  sdev = T(0);
	  return true; //do nothing if there is only one 1 value
	}

	double ave_double=(T)(s/n_use); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++)
	{
	    if (data[j]!=maskval)
		{
			s=data[j]-(ave_double);
			var += (p=s*s);
		}
	}
	var=(var-ep*ep/n_use)/(n_use-1);
	sdev=(T)(sqrt(var));
	ave=(T)ave_double; //use ave_double for the best accuracy

	return true;
}

template <class T> bool mean_and_std(T *data, V3DLONG n, T & ave, T & sdev, T maskval_lowerbound, T maskval_upperbound)
{
    if (!data || n<=0)
	  return false;

	int j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
	  //printf("len must be at least 2 in mean_and_std\n");
	  ave = data[0];
	  sdev = (T)0;
	  return true; //do nothing
	}

	V3DLONG n_use;

	if (maskval_lowerbound>maskval_upperbound) {T tmp=maskval_lowerbound; maskval_lowerbound=maskval_upperbound; maskval_upperbound=tmp;} //070528: assure the range is valid

	s=0.0;
	for (j=0, n_use=0;j<n;j++) {if (data[j]<maskval_lowerbound || data[j]>maskval_upperbound) {s += data[j]; n_use++;}}
	if (n_use<=0)
	{
	  ave = data[0];
	  sdev = (T)0;
	  return true; //do nothing if everything is equal maskval
	}
	if (n_use==1)
	{
	  ave=s;
	  sdev = T(0);
	  return true; //do nothing if there is only one 1 value
	}

	double ave_double=(T)(s/n_use); //use ave_double for the best accuracy

	double var=0.0;
	for (j=0;j<n;j++)
	{
	    if (data[j]<maskval_lowerbound || data[j]>maskval_upperbound)
		{
			s=data[j]-(ave_double);
			var += (p=s*s);
		}
	}
	var=(var-ep*ep/n_use)/(n_use-1);
	sdev=(T)(sqrt(var));
	ave=(T)ave_double; //use ave_double for the best accuracy

	return true;
}

template <class T> void moment(T *data, V3DLONG n, double & ave, double & adev, double & sdev, double & var, double & skew, double & curt)
{
	int j;
	double ep=0.0,s,p;

	if (n <= 1)
	{
		printf("len must be at least 2 in moment\n");
		return; //do nothing
	}

	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	ave=s/n;
	adev=var=skew=curt=0.0;
	for (j=0;j<n;j++) {
		adev += fabs(s=data[j]-(ave));
		var += (p=s*s);
		skew += (p *= s);
		curt += (p *= s);
	}
	adev /= n;
	var=(var-ep*ep/n)/(n-1);
	sdev=sqrt(var);
	if (var) {
		skew /= (n*var*sdev);
		curt=curt/(n*var*var)-3.0;
	}
	else //No skew/kurtosis defined when variance = 0
	{
		skew = 0;
		curt = 0;
	}
}

template <class T> T minInVector(T * p, V3DLONG len, V3DLONG &pos)
{
	T a = p[0];
	pos = 0;
	for (V3DLONG i=1;i<len;i++)
	{
		if (p[i]<a)
		{
			a = p[i];
			pos = i;
		}
	}
	return a;
}

template <class T> T maxInVector(T * p, V3DLONG len, V3DLONG &pos)
{
	T a = p[0];
	pos = 0;
	for (V3DLONG i=1;i<len;i++)
	{
		if (p[i]>a)
		{
			a = p[i];
			pos = i;
		}
	}
	return a;
}

template <class T> bool minMaxInVector(T * p, V3DLONG len, V3DLONG &pos_min, T &minv, V3DLONG &pos_max, T &maxv)
{
	if (!p || len <= 0)
		return false;
	
	minv = maxv = p[0];
	pos_min = pos_max = 0;
	for (V3DLONG i=1;i<len;i++)
	{
		if (p[i]>maxv)
		{
			maxv = p[i];
			pos_max = i;
		}
		else if (p[i]<minv)
		{
			minv = p[i];
			pos_min = i;
		}
	}
	
	return true;
}



//fit to a data cube with a different 3D size
template <class T> bool fit_to_cube(T * & img, V3DLONG * sz, V3DLONG * sz_target)
{
	if (!img || !sz || !sz_target)
	{
		fprintf(stderr, "The input to fit_to_cube() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1 || sz[0]>2048 || sz[1]>2048 || sz[2]>1024 || sz[3]>10)
	{
		fprintf(stderr, "Input image size is not valid  [%s][%d], maybe because the sizes are too small or too large.\n", __FILE__, __LINE__);
		return false;
	}

	if (sz_target[0]<1 || sz_target[1]<1 || sz_target[2]<1 || sz_target[0]>2048 || sz_target[1]>2048 || sz_target[2]>300)
	{
		fprintf(stderr, "target image size is not valid  [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T * outimg = new T [sz_target[0] * sz_target[1] * sz_target[2] * sz[3]];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T **** out_tmp4d = 0;
	T **** in_tmp4d = 0;

	new4dpointer(out_tmp4d, sz_target[0], sz_target[1], sz_target[2], sz[3], outimg);
	new4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3], img);

	for (V3DLONG tmpi=0;tmpi<(sz_target[0] * sz_target[1] * sz_target[2] * sz[3]); tmpi++) //initialize to 0
	{
		outimg[tmpi] = 0;
	}

	//for k
	V3DLONG src_k0, dst_k0, kmax;
	if (sz_target[2]<sz[2])
	{
		src_k0=(sz[2]-sz_target[2])/2; dst_k0=0;
		kmax = sz_target[2]-1;
	}
	else
	{
		src_k0=0; dst_k0=(sz_target[2]-sz[2])/2;
		kmax = sz[2]-1;
	}

	//for j
	V3DLONG src_j0, dst_j0, jmax;
	if (sz_target[1]<sz[1])
	{
		src_j0=(sz[1]-sz_target[1])/2; dst_j0=0;
		jmax = sz_target[1]-1;
	}
	else
	{
		src_j0=0; dst_j0=(sz_target[1]-sz[1])/2;
		jmax = sz[1]-1;
	}

	//for i
	V3DLONG src_i0, dst_i0, imax;
	if (sz_target[0]<sz[0])
	{
		src_i0=(sz[0]-sz_target[0])/2; dst_i0=0;
		imax = sz_target[0]-1;
	}
	else
	{
		src_i0=0; dst_i0=(sz_target[0]-sz[0])/2;
		imax = sz[0]-1;
	}


    for (V3DLONG c=0;c<sz[3];c++)
	{
		for (V3DLONG k=0;k<=kmax;k++)
		{
			for (V3DLONG j=0;j<=jmax;j++)
			{
				for (V3DLONG i=0;i<=imax;i++)
				{
					out_tmp4d[c][k+dst_k0][j+dst_j0][i+dst_i0] = in_tmp4d[c][k+src_k0][j+src_j0][i+src_i0];
				}
			}
		}
	}

	//delete temprary 4d pointers

	delete4dpointer(out_tmp4d, sz_target[0], sz_target[1], sz_target[2], sz[3]);
	delete4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3]);

	//copy to output data

	delete []img;
	img = outimg;

	sz[0] = sz_target[0];
	sz[1] = sz_target[1];
	sz[2] = sz_target[2];
	sz[3] = sz[3];

	return true;
}


//down-sampling data volume
template <class T> bool downsample3dimg_1dpt(T * & img, V3DLONG * sz, double dfactor)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to downsample3dimg_1dpt() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (int(dfactor)<1 || int(dfactor)>16)
	{
		fprintf(stderr, "The downsampling factor must be >=1 and <= 16 [%s][%d] dfactor=%.2f.\n", __FILE__, __LINE__, dfactor);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1 || sz[0]>2048 || sz[1]>2048 || sz[2]>1024 || sz[3]>10)
	{
		fprintf(stderr, "Input image size is not valid [%s][%d][%5.3f].\n", __FILE__, __LINE__, dfactor);
		return false;
	}

	V3DLONG cur_sz0 = (V3DLONG)(floor(double(sz[0]) / double(dfactor)));
	V3DLONG cur_sz1 = (V3DLONG)(floor(double(sz[1]) / double(dfactor)));
	V3DLONG cur_sz2 = sz[2];
	V3DLONG cur_sz3 = sz[3];

	if (cur_sz0 <= 0 || cur_sz1 <=0 || cur_sz2<=0 || cur_sz3<=0)
	{
		fprintf(stderr, "The dfactor is not properly set, -- the resulted downsampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T * outimg = new T [cur_sz0 * cur_sz1 * cur_sz2 * cur_sz3];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T **** out_tmp4d = 0;
	T **** in_tmp4d = 0;

	new4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3, outimg);
	new4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3], img);

    for (V3DLONG c=0;c<cur_sz3;c++)
	{
		for (V3DLONG k=0;k<cur_sz2;k++)
		{
			//V3DLONG k2low=V3DLONG(floor(k*dfactor)), k2high=V3DLONG(floor((k+1)*dfactor-1));
			V3DLONG k2low=k, k2high=k; //do not downsampling z, 070927
			if (k2high>sz[2]-1) k2high = sz[2]-1;
			V3DLONG kw = k2high - k2low + 1;

			for (V3DLONG j=0;j<cur_sz1;j++)
			{
				//V3DLONG j2low=V3DLONG(floor(j*dfactor)), j2high=V3DLONG(floor((j+1)*dfactor-1));
				V3DLONG j2low=V3DLONG(floor(j*dfactor)); V3DLONG j2high=V3DLONG(floor((j+1)*dfactor-1));
				if (j2high>sz[1]-1) j2high = sz[1]-1;
				V3DLONG jw = j2high - j2low + 1;

				for (V3DLONG i=0;i<cur_sz0;i++)
				{
					//V3DLONG i2low=V3DLONG(floor(i*dfactor)), i2high=V3DLONG(floor((i+1)*dfactor-1));
					V3DLONG i2low=V3DLONG(floor(i*dfactor)); V3DLONG i2high=V3DLONG(floor((i+1)*dfactor-1));
					if (i2high>sz[0]-1) i2high = sz[0]-1;
					V3DLONG iw = i2high - i2low + 1;

					double cubevolume = double(kw) * jw * iw;
					//cout<<cubevolume <<" ";

					double s=0.0;
					for (V3DLONG k1=k2low;k1<=k2high;k1++)
					{
						for (V3DLONG j1=j2low;j1<=j2high;j1++)
						{
							for (V3DLONG i1=i2low;i1<=i2high;i1++)
							{
								s += in_tmp4d[c][k1][j1][i1];
							}
						}
					}

					out_tmp4d[c][k][j][i] = (T)(s/cubevolume);
				}
			}
		}
	}

	//delete temprary 4d pointers

	delete4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3);
	delete4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3]);

	//copy to output data

	delete []img;
	img = outimg;

	sz[0] = cur_sz0;
	sz[1] = cur_sz1;
	sz[2] = cur_sz2;
	sz[3] = cur_sz3;

	return true;
}


//simple nearest neighbor reslicing of a 3D stack

template <class T> bool reslice_Z(T * & invol1d, V3DLONG * sz, double xy_rez, double z_rez, int interp_method)
{
  //check if parameters are correct

  V3DLONG i,j,k,c;

  //get image

  if (!invol1d || sz[0]*sz[1]*sz[2]*sz[3]<=0 || xy_rez<=0 || z_rez<=0)
  {
    fprintf(stderr,"You have provided illgeal parameters to reslice_Z().\n");
	return false;
  }

  if (interp_method!=0 && interp_method!=1) //0 for nearest neighbor interp and 1 for linear
  {
    fprintf(stderr,"You have provided illgeal interp_method parameters to reslice_Z() [you pass a code %d].\n", interp_method);
	return false;
  }


  //==================== set up the input and output volume matrix =====================

  V3DLONG xlen_out = sz[0];
  V3DLONG ylen_out = sz[1];
  V3DLONG zlen_out = V3DLONG((double(sz[2]) * z_rez)/xy_rez + 0.5); //if use ceil() then rish having no value at the border //remove sz[2]-1 on 100831, by PHC
  double z_rez_new = xy_rez;
  V3DLONG clen_out = sz[3];

  T * outvol1d = 0;
  T **** outvol4d=0, ****invol4d=0;

  try
  {
  outvol1d =  new T [xlen_out*ylen_out*zlen_out*clen_out];
  if (!outvol1d)
  {
    fprintf(stderr,"Fail to allocate memory for the output volume 1d in reslice_Z().\n");
	return false;
  }

  new4dpointer(invol4d, sz[0], sz[1], sz[2], sz[3], invol1d);
  new4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out, outvol1d);
  if (!invol4d || !outvol4d)
  {
    if (invol4d) {delete4dpointer(invol4d, sz[0], sz[1], sz[2], sz[3]); invol4d=0;}
	if (outvol4d) {delete4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out); outvol4d=0;}
    fprintf(stderr,"Fail to allocate memory for the input/output volumes 4d in reslice_Z().\n");
	return false;
  }
  }
  catch(...)
  {
  fprintf(stderr, "Unable to allocate mmeory in reslice_Z().\n");
  return false;
  }

  printf("#original slice=%ld original rez=%6.5f -> #output slices=%ld new rez=%6.5f\n", sz[2], z_rez, zlen_out, z_rez_new);
  //return;

  //============ generate linear interpolation ===================

  if (interp_method==1) //linear interp
  {
	  for (c=0; c<clen_out; c++)
	  {
		  for (i=0; i<zlen_out; i++)
		  {
			  double curpz = double(i)*z_rez_new/z_rez;
			  if (curpz>=sz[2]-1) curpz=sz[2]-1; //100831, by PHC
			  
			  V3DLONG cpz0 = (V3DLONG)(floor(curpz)), cpz1 = (V3DLONG)(ceil(curpz));

			  if (cpz0==cpz1)
			  {
				  for (j=0;j<ylen_out; j++)
				  {
					for (k=0; k<xlen_out; k++)
					{
						outvol4d[c][i][j][k] = invol4d[c][cpz0][j][k];
					}
				  }
			  }
			  else
			  {
				  double w0z = (cpz1-curpz);
				  double w1z = (curpz-cpz0);

				  for (j=0;j<ylen_out; j++)
				  {
					for (k=0; k<xlen_out; k++)
					{
						outvol4d[c][i][j][k] = (T)(w0z * double(invol4d[c][cpz0][j][k]) + w1z * double(invol4d[c][cpz1][j][k]));
					}
				  }
			  }

		  }
	  }
  }
  else //nearest neighbor interp
  {
	  for (c=0; c<clen_out; c++)
	  {
		  for (i=0; i<zlen_out; i++)
		  {
			  double curpz = double(i)*z_rez_new/z_rez;
			  if (curpz>=sz[2]-1) curpz=sz[2]-1; //100831, by PHC

			  V3DLONG cpz0 = (V3DLONG)(floor(curpz)), cpz1 = (V3DLONG)(ceil(curpz));

			  if (cpz0==cpz1)
			  {
				  for (j=0;j<ylen_out; j++)
				  {
					for (k=0; k<xlen_out; k++)
					{
						outvol4d[c][i][j][k] = invol4d[c][cpz0][j][k];
					}
				  }
			  }
			  else
			  {
				  double w0z = (cpz1-curpz);
				  double w1z = (curpz-cpz0);

				  if (w0z>=w1z) //note >= condition so that to handle the case of x2 zoom-in
				  {
					  for (j=0;j<ylen_out; j++)
					  {
						for (k=0; k<xlen_out; k++)
						{
							outvol4d[c][i][j][k] = invol4d[c][cpz0][j][k];
						}
					  }
				  }
				  else
				  {
					  for (j=0;j<ylen_out; j++)
					  {
						for (k=0; k<xlen_out; k++)
						{
							outvol4d[c][i][j][k] = invol4d[c][cpz1][j][k];
						}
					  }
				  }
			  }
		  }
	  }
  }

  // ====free memory=============
  if (invol4d) {delete4dpointer(invol4d, sz[0], sz[1], sz[2], sz[3]); invol4d=0;}
  if (outvol4d) {delete4dpointer(outvol4d, xlen_out, ylen_out, zlen_out, clen_out); outvol4d=0;}

  //update the output pointer and size of Z
  delete []invol1d;
  invol1d = outvol1d;

  sz[2] = zlen_out;

  return true;
}

//re-sampling data volume with different scaling factors of different axes
template <class T> bool resample3dimg_interp(T * & img, V3DLONG * sz, double dfactor_x, double dfactor_y, double dfactor_z, int interp_method)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to resample3dimg_interp() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (dfactor_x<1 || dfactor_y<1 || dfactor_z<1)
	{
		fprintf(stderr, "The resampling factor must be >1 in resample3dimg_linear_interp(), because now only DOWN-sampling is supported [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1)
	{
		fprintf(stderr, "Input image size is not valid in resample3dimg_interp() [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (interp_method!=1) //0 for nearest neighbor interp and 1 for linear
	{
		fprintf(stderr,"Invalid interpolation code. Now only linear interpolation is supported in  resample3dimg_linear_interp() [you pass a code %d].\n", interp_method);
		return false;
	}

	V3DLONG cur_sz0 = (V3DLONG)(floor(double(sz[0]) / double(dfactor_x)));
	V3DLONG cur_sz1 = (V3DLONG)(floor(double(sz[1]) / double(dfactor_y)));
	V3DLONG cur_sz2 = (V3DLONG)(floor(double(sz[2]) / double(dfactor_z)));
	V3DLONG cur_sz3 = sz[3];

	if (cur_sz0 <= 0 || cur_sz1 <=0 || cur_sz2<=0 || cur_sz3<=0)
	{
		fprintf(stderr, "The dfactors are not properly set, -- the resulted resampled size is too small. Do nothing. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T * outimg = new T [cur_sz0 * cur_sz1 * cur_sz2 * cur_sz3];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	T **** out_tmp4d = 0;
	T **** in_tmp4d = 0;

	new4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3, outimg);
	new4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3], img);

    for (V3DLONG c=0;c<cur_sz3;c++)
	{
		for (V3DLONG k=0;k<cur_sz2;k++)
		{
			V3DLONG k2low=(V3DLONG)(floor(k*dfactor_z)), k2high=(V3DLONG)(floor((k+1)*dfactor_z-1));
			if (k2high>sz[2]-1) k2high = sz[2]-1;
			V3DLONG kw = k2high - k2low + 1;

			for (V3DLONG j=0;j<cur_sz1;j++)
			{
				V3DLONG j2low=(V3DLONG)(floor(j*dfactor_y)), j2high=(V3DLONG)(floor((j+1)*dfactor_y-1));
				if (j2high>sz[1]-1) j2high = sz[1]-1;
				V3DLONG jw = j2high - j2low + 1;

				for (V3DLONG i=0;i<cur_sz0;i++)
				{
					V3DLONG i2low=(V3DLONG)(floor(i*dfactor_x)), i2high=(V3DLONG)(floor((i+1)*dfactor_x-1));
					if (i2high>sz[0]-1) i2high = sz[0]-1;
					V3DLONG iw = i2high - i2low + 1;

					double cubevolume = double(kw) * jw * iw;
					//cout<<cubevolume <<" ";

					double s=0.0;
					for (V3DLONG k1=k2low;k1<=k2high;k1++)
					{
						for (V3DLONG j1=j2low;j1<=j2high;j1++)
						{
							for (V3DLONG i1=i2low;i1<=i2high;i1++)
							{
								s += in_tmp4d[c][k1][j1][i1];
							}
						}
					}

					out_tmp4d[c][k][j][i] = (T)(s/cubevolume);
				}
			}
		}
	}

	//delete temprary 4d pointers

	delete4dpointer(out_tmp4d, cur_sz0, cur_sz1, cur_sz2, cur_sz3);
	delete4dpointer(in_tmp4d, sz[0], sz[1], sz[2], sz[3]);

	//copy to output data

	delete []img;
	img = outimg;

	sz[0] = cur_sz0;
	sz[1] = cur_sz1;
	sz[2] = cur_sz2;
	sz[3] = cur_sz3;

	return true;
}

template <class T> double calCorrelation(T * img1, T * img2, V3DLONG imglen)
{
  V3DLONG i,j;
  double s12=0, s11=0, s22=0;
  double m11=0, m22=0;

  V3DLONG n=0;
  for (i=0;i<imglen;i++)
  {
    m11 += double(img1[i]);
    m22 += double(img2[i]);
    n++;
  }
  m11 /= double(n);
  m22 /= double(n);

  double tmp1, tmp2;
  for (i=0;i<imglen;i++)
  {
    tmp1 = double(img1[i])-m11;
    tmp2 = double(img2[i])-m22;
    s12 += tmp1*tmp2;
    s11 += tmp1*tmp1;
    s22 += tmp2*tmp2;
  }

  double score = s12/sqrt(s11)/sqrt(s22);
  return score;
}


template <class T> void twopoints_lineprofile_3dimg(T *** img, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double x1, double y1, double z1, double x2, double y2, double z2, T * &profile, V3DLONG & profile_len)
{
	//first reset the output
	if (profile) {delete profile; profile=0;} //delete the space if it is no null
	profile_len = 0;

	//then check if the input coordinates are correct or not
	if (!img ||
		sz0<0 || x1<0 || x1>sz0-1 || x2<0 || x2>sz0-1 ||
		sz1<0 || y1<0 || y1>sz1-1 || y2<0 || y2>sz1-1 ||
		sz2<0 || z1<0 || z1>sz2-1 || z2<0 || z2>sz2-1)
	{
		return; //note in this case the return profile is null
	}

	double dx=x2-x1, dy=y2-y1, dz=z2-z1;
	double len = sqrt(dx*dx+dy*dy+dz*dz);
	profile_len = 1 + V3DLONG(len + 0.5); //+1 because there is at least one element in the profile, which is the starting point (x1,y1,z1)
	try
	{
		profile = new T [profile_len];
	}
	catch (...)
	{
		profile = 0;
		profile_len = 0;
		return;
	}

	//now do resampling
	double curpx,curpy,curpz; //the coordinate of the current point
	double sx=dx/profile_len, sy=dy/profile_len, sz=dz/profile_len; //step size
	for (V3DLONG n=0;n<profile_len;n++)
	{
		curpx = x1+sx*n; curpy = y1+sy*n; curpz = z1+sz*n;

		//now compute the interpolated value
		V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;
		{
			curpx=(curpx<0)?0:curpx; curpx=(curpx>=sz0-1)?sz0-1:curpx;
			curpy=(curpy<0)?0:curpy; curpy=(curpy>=sz1-1)?sz1-1:curpy;
			curpz=(curpz<0)?0:curpz; curpz=(curpz>=sz2-1)?sz2-1:curpz;

			cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
			cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
			cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));

			if (cpz0==cpz1)
			{
				if (cpy0==cpy1)
				{
					if (cpx0==cpx1)
					{
						profile[n] = (double)(img[cpz0][cpy0][cpx0]);
					}
					else
					{
						double w0x0y0z = (cpx1-curpx);
						double w1x0y0z = (curpx-cpx0);
						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) +
											  w1x0y0z * double(img[cpz0][cpy0][cpx1]));
					}
				}
				else
				{
					if (cpx0==cpx1)
					{
						double w0x0y0z = (cpy1-curpy);
						double w0x1y0z = (curpy-cpy0);
						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) +
											  w0x1y0z * double(img[cpz0][cpy1][cpx0]));
					}
					else
					{
						double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
						double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
						double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
						double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);
						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) +
											  w0x1y0z * double(img[cpz0][cpy1][cpx0]) +
											  w1x0y0z * double(img[cpz0][cpy0][cpx1]) +
											  w1x1y0z * double(img[cpz0][cpy1][cpx1]));
					}
				}
			}
			else
			{
				if (cpy0==cpy1)
				{
					if (cpx0==cpx1)
					{
						double w0x0y0z = (cpz1-curpz);
						double w0x0y1z = (curpz-cpz0);

						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) + w0x0y1z * double(img[cpz1][cpy0][cpx0]));
					}
					else
					{
						double w0x0y0z = (cpx1-curpx)*(cpz1-curpz); //remove mid term in (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz), which should be a bug causing 0 in certain place. 080504
						double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);

						double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
						double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);

						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) + w0x0y1z * double(img[cpz1][cpy0][cpx0]) +
											  w1x0y0z * double(img[cpz0][cpy0][cpx1]) + w1x0y1z * double(img[cpz1][cpy0][cpx1]));
					}
				}
				else
				{
					if (cpx0==cpx1)
					{
						double w0x0y0z = (cpy1-curpy)*(cpz1-curpz); //update 080504, remove the first (cpx1-curpx) term, which should be a bug
						double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);

						double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
						double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);

						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) + w0x0y1z * double(img[cpz1][cpy0][cpx0]) +
											  w0x1y0z * double(img[cpz0][cpy1][cpx0]) + w0x1y1z * double(img[cpz1][cpy1][cpx0]));
					}
					else
					{
						double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
						double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);

						double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
						double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);

						double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
						double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

						double w1x1y0z = (curpx-cpx0)*(curpy-cpy0)*(cpz1-curpz);
						double w1x1y1z = (curpx-cpx0)*(curpy-cpy0)*(curpz-cpz0);

						profile[n] = (double)(w0x0y0z * double(img[cpz0][cpy0][cpx0]) + w0x0y1z * double(img[cpz1][cpy0][cpx0]) +
											  w0x1y0z * double(img[cpz0][cpy1][cpx0]) + w0x1y1z * double(img[cpz1][cpy1][cpx0]) +
											  w1x0y0z * double(img[cpz0][cpy0][cpx1]) + w1x0y1z * double(img[cpz1][cpy0][cpx1]) +
											  w1x1y0z * double(img[cpz0][cpy1][cpx1]) + w1x1y1z * double(img[cpz1][cpy1][cpx1]) );
					}
				}
			}
		}
	}

	return;
}

template <class T> double twopoints_lineprofile_3dimg_sum(T *** img, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2, double x1, double y1, double z1, double x2, double y2, double z2)
{
	T *profile=0;
	V3DLONG profile_len=0;
	twopoints_lineprofile_3dimg(img, sz0, sz1, sz2, x1, y1, z1, x2, y2, z2, profile, profile_len);
	if (profile && profile_len)
	{
		double s=0.0;
		for (V3DLONG n=0;n<profile_len;n++)
			s += profile[n];
		if (profile) {delete []profile; profile=0;}
		return s;
	}
	else
		return 0;
}

template <class T> double cosangle_two_vectors(const T a[3], const T b[3]) //in case an error, return -2
{
    double vab=0,vaa=0,vbb=0;
    for (int i=0;i<3;i++)
    {
        vab += a[i]*b[i];
        vaa += a[i]*a[i];
        vbb += b[i]*b[i];
    }
    return (vaa*vbb<1e-10) ? -2 : vab/sqrt(vaa*vbb);
}

#endif

