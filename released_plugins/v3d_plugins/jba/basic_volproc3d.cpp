//
// Some basic support functions for simple volume image computation
//
// by Hanchuan Peng
// 2006-2011
// 

#ifndef __BASIC_VOLPROC3D_CPP__
#define __BASIC_VOLPROC3D_CPP__

#include <cmath>
#include "v3d_basicdatatype.h"

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
	
	T v;
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

template <class T> bool vol3d_log(T *** res, T *** sa, double V3DLONG d0, V3DLONG d1, V3DLONG d2)
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

template <class T> bool mean_and_std(T *data, V3DLONG n, T & ave, T & sdev)
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
	  
	s=0.0;
	for (j=0;j<n;j++) s += data[j];
	double ave_double=(T)(s/n); //use ave_double for the best accuracy
	
	double var=0.0;
	for (j=0;j<n;j++) {
		s=data[j]-(ave_double);
		var += (p=s*s);
	}
	var=(var-ep*ep/n)/(n-1);
	sdev=(T)(sqrt(var));
	ave=(T)ave_double; //use ave_double for the best accuracy
	
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
	  ave = data[0];
	  sdev = (T)0;
	  return true; 
	}
	
	V3DLONG n_use;
	  
	s=0.0;
	for (j=0, n_use=0;j<n;j++) {if (data[j]!=maskval) {s += data[j]; n_use++;}}
	if (n_use<=0)
	{
	  ave = data[0];
	  sdev = (T)0;
	  return true; 
	}
	if (n_use==1)
	{
	  ave=s;
	  sdev = T(0);
	  return true; 
	}
	
	double ave_double=(T)(s/n_use); 
    
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
	ave=(T)ave_double; 
	
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
	  ave = data[0];
	  sdev = (T)0;
	  return true; 
	}
	
	V3DLONG n_use;
	  
	if (maskval_lowerbound>maskval_upperbound) {T tmp=maskval_lowerbound; maskval_lowerbound=maskval_upperbound; maskval_upperbound=tmp;} 
	  
	s=0.0;
	for (j=0, n_use=0;j<n;j++) {if (data[j]<maskval_lowerbound || data[j]>maskval_upperbound) {s += data[j]; n_use++;}}
	if (n_use<=0)
	{
	  ave = data[0];
	  sdev = (T)0;
	  return true;
	}
	if (n_use==1)
	{
	  ave=s;
	  sdev = T(0);
	  return true; 
	}
	
	double ave_double=(T)(s/n_use); 
	
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
	ave=(T)ave_double; 
	
	return true;
}


#endif

