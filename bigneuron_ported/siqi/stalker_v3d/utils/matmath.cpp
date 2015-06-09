// Some C++ Implementations for the matlab classic calls
#include <iostream>
#include <fstream>
#include "matmath.h"
#include "assert.h"
#include <cmath>
#include "stackutil.h"

#ifndef MAX
#define MAX(x,y) (x > y ? (x) : (y))
#endif




using namespace std;

vectype linspace(const double a, const double b, const int n) {
// Equals to linspace in matlab
    vectype outvec (n);
    double step = (b-a) / (n-1);
    double t = a;
    vectype::iterator outitr = outvec.begin();
    while(t <= b && outitr != outvec.end()) {
        *outitr = t;
        outitr++;
        t += step;           // could recode to better handle rounding errors
    }
    return outvec;
}


vectype repmat1d(vectype invec, int repeat, int dim){
	assert(invec.size() != 0);
	const int nvar = repeat * invec.size();
	vectype outvec (nvar);
    vectype::iterator outitr = outvec.begin();
	int out_szrow, out_szcol;

	switch (dim){
	    case 1:
	        {
	        out_szrow = repeat;
	        out_szcol = invec.size();
			// Assume the caller has allocated the memory for outarray
			for (int col = 0; col < out_szcol; col++)
			    for(int row = 0; row < out_szrow; row++, outitr++)
			    {
		            *outitr = invec[col];
			    }
		    break;
			}
	    case 2:
	        {
	        out_szrow = invec.size();
	        out_szcol = repeat;
			// Assume the caller has allocated the memory for outarray
			for (int col = 0; col < out_szcol; col++)
			    for(int row = 0; row < out_szrow; row++, outitr++)
			    {
		            *outitr = invec[row];
			    }
		    break;
		    }
	    default: throw 1; break;
	}

	return outvec;
}


vectype transpose(const vectype invec, const int N, const int M) {
    vectype outvec(invec.size());
    vectype::iterator outitr = outvec.begin();
    int i,j;

	for (int n=0; n<M*N; n++, outitr++)
	{
        i = n/N;
        j = n%N;
        *outitr = invec[M*j + i];
    }

    return outvec;
}


void sph2cart(vectype thvec, vectype phivec, vectype rvec, vectype * xvec, vectype * yvec, vectype * zvec ){
	assert(thvec.size() == phivec.size() && phivec.size() == rvec.size());
	assert(thvec.size() == xvec->size() && thvec.size() == yvec->size() && thvec.size() == zvec->size());
	vectype::iterator xitr, yitr, zitr, thitr, phiitr, ritr;

	for (xitr = xvec->begin(), 
		 yitr = yvec->begin(), 
		 zitr = zvec->begin(), 
		 thitr = thvec.begin(),
		 phiitr = phivec.begin(),
		 ritr = rvec.begin();
		 thitr != thvec.end();
		 xitr++, yitr++, zitr++,
		 thitr++, phiitr++, ritr++)
	{
	    *xitr = *ritr * cos(*thitr)	* sin(*phiitr);
	    *yitr = *ritr * sin(*thitr)	* sin(*phiitr);
	    *zitr = *ritr * cos(*phiitr);
	}
}


void cart2sph(vectype xvec, vectype yvec, vectype zvec, vectype* thvec, vectype* phivec, vectype* rvec ){
// Convert cartisian coordinates to spherical coordinates
// Using the mathmatician definition of the S^2
//                                2*pi > th > 0
//                                pi > phi > 0
// Ref: http://en.wikipedia.org/wiki/List_of_common_coordinate_transformations
// Assume thvec, phivec, rvec are initialsed before callling this function

	assert(xvec.size() == yvec.size() && yvec.size() == zvec.size());
	assert(xvec.size() == thvec->size() && xvec.size() == phivec->size() && xvec.size() == rvec->size());
	vectype::iterator xitr, yitr, zitr, thitr, phiitr, ritr;

	for (xitr = xvec.begin(), 
		 yitr = yvec.begin(), 
		 zitr = zvec.begin(), 
		 thitr = thvec->begin(),
		 phiitr = phivec->begin(),
		 ritr = rvec->begin();
		 xitr != xvec.end();
		 xitr++, yitr++, zitr++,
		 thitr++, phiitr++, ritr++)
	{
	    if (*xitr == 0)	
	    {
	    	if (*yitr > 0) *thitr = M_PI;
	    	else if (*yitr < 0) *thitr = 3 * M_PI / 2;
    		else *thitr = 0;
	    }
	    else
	    {
	        *thitr = atan(abs(*yitr / *xitr));
	    }
        
        // Put th to the right domain (atan only outputs [0, PI/2])
        if (*xitr < 0.0 && *yitr > 0.0)
            *thitr = M_PI - *thitr; // --  QII
        else if (*xitr < 0.0 && *yitr < 0.0)
            *thitr += M_PI; // --  QIII
        else if (*xitr > 0.0 && *yitr < 0.0)  // -- QIV
        	*thitr = 2.0 * M_PI - *thitr;


        *ritr = pow(pow(*xitr, 2) + pow(*yitr, 2) + pow(*zitr, 2), 0.5);
        
        *phiitr = acos(*zitr / *ritr);
	}

}


void savepts2csv(vectype a, vectype b, vectype c, const char* filename){
// Save a list of points to a text file
// Each point occupies a line
// Since three coordinates are saved, this function works both for Cartisian and Spherical 
	ofstream f;
	f.open(filename);
	assert(a.size() == b.size() && b.size() == c.size());

	for (int i = 0; i < a.size(); i++)
	{
	   f<<a[i]<<","<<b[i]<<","<<c[i]<<endl;
	}

	f.close();
}


vectype eucdistance2center(const float x, const float y, const float z, const vectype lx, const vectype ly, const vectype lz)
{
	assert(lx.size() == ly.size() && ly.size() == lz.size());

    vectype vd(lx.size());

    for (int i = 0; i < lx.size(); i++)	
    {
        vd[i] = pow( pow(lx[i] - x, 2) + pow(ly[i] - y, 2) + pow(lz[i] - z, 2), 0.5);
    }

    return vd;
}


int appradius(unsigned char * inimg1d, V3DLONG * sz,  double thresh, int location_x, int location_y, int location_z){

    int max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    int r;
    double tol_num, bak_num;
    int mx = location_x + 0.5;
    int my = location_y+ 0.5;
    int mz = location_z + 0.5;
    V3DLONG x[2], y[2], z[2];

    tol_num = bak_num = 0.0;
    V3DLONG sz01 = sz[0] * sz[1];
    for(r = 1; r <= max_r; r++)
    {
        double r1 = r - 0.5;
        double r2 = r + 0.5;
        double r1_r1 = r1 * r1;
        double r2_r2 = r2 * r2;
        double z_min = 0, z_max = r2;
        for(int dz = z_min ; dz < z_max; dz++)
        {
            double dz_dz = dz * dz;
            double y_min = 0;
            double y_max = sqrt(r2_r2 - dz_dz);
            for(int dy = y_min; dy < y_max; dy++)
            {
                double dy_dy = dy * dy;
                double x_min = r1_r1 - dz_dz - dy_dy;
                x_min = x_min > 0 ? sqrt(x_min)+1 : 0;
                double x_max = sqrt(r2_r2 - dz_dz - dy_dy);
                for(int dx = x_min; dx < x_max; dx++)
                {
                    x[0] = mx - dx, x[1] = mx + dx;
                    y[0] = my - dy, y[1] = my + dy;
                    z[0] = mz - dz, z[1] = mz + dz;
                    for(char b = 0; b < 8; b++)
                    {
                        char ii = b & 0x01, jj = (b >> 1) & 0x01, kk = (b >> 2) & 0x01;
                        if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1] || z[kk]<0 || z[kk] >= sz[2]) return r;
                        else
                        {
                            tol_num++;
                            long pos = z[kk]*sz01 + y[jj] * sz[0] + x[ii];
                            if(inimg1d[pos] < thresh){bak_num++;}
                            if((bak_num / tol_num) > 0.0001) return r;
                        }
                    }
                }
            }
        }
    }
    return r;
}
