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

vectype linspace(const PRECISION a, const PRECISION b, const int n) {
// Equals to linspace in matlab
    vectype outvec (n);
    PRECISION step = (b-a) / (n-1);
    PRECISION t = a;
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

void savepts2csvfourva(vectype a, vectype b, vectype c, vectype d, const char* filename){
// Save a list of points to a text file
// Each point occupies a line
// Since three coordinates are saved, this function works both for Cartisian and Spherical 
	ofstream f;
	f.open(filename);
	assert(a.size() == b.size() && b.size() == c.size() && c.size() == d.size());

	for (int i = 0; i < a.size(); i++)
	{
	   f<<a[i]<<","<<b[i]<<","<<c[i]<<","<<d[i]<<endl;
	}

	f.close();
}

vectype eucdistance2center(const PRECISION x, const PRECISION y, const PRECISION z, const vectype lx, const vectype ly, const vectype lz)
{
	assert(lx.size() == ly.size() && ly.size() == lz.size());

    vectype vd(lx.size());

    for (int i = 0; i < lx.size(); i++)	
    {
        vd[i] = pow( pow(lx[i] - x, 2) + pow(ly[i] - y, 2) + pow(lz[i] - z, 2), 0.5);
    }

    return vd;
}


// find the radius of the seed
// Same method used in APP2
int appradius(unsigned char * inimg1d, V3DLONG * sz,  PRECISION thresh, int location_x, int location_y, int location_z){

    int max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
    int r;
    PRECISION tol_num, bak_num;
    int mx = location_x + 0.5;
    int my = location_y+ 0.5;
    int mz = location_z + 0.5;
    V3DLONG x[2], y[2], z[2];

    tol_num = bak_num = 0.0;
    V3DLONG sz01 = sz[0] * sz[1];
    for(r = 1; r <= max_r; r++)
    {
        PRECISION r1 = r - 0.5;
        PRECISION r2 = r + 0.5;
        PRECISION r1_r1 = r1 * r1;
        PRECISION r2_r2 = r2 * r2;
        PRECISION z_min = 0, z_max = r2;
        for(int dz = z_min ; dz < z_max; dz++)
        {
            PRECISION dz_dz = dz * dz;
            PRECISION y_min = 0;
            PRECISION y_max = sqrt(r2_r2 - dz_dz);
            for(int dy = y_min; dy < y_max; dy++)
            {
                PRECISION dy_dy = dy * dy;
                PRECISION x_min = r1_r1 - dz_dz - dy_dy;
                x_min = x_min > 0 ? sqrt(x_min)+1 : 0;
                PRECISION x_max = sqrt(r2_r2 - dz_dz - dy_dy);
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


void vecproj(vectype * u, const vectype v)
{
	// Projv(u) = [ (u•v)/(v•v) ] · v project u ulong v
	double udotv = (*u)[0] * v[0] + (*u)[1] * v[1] + (*u)[2] * v[2]; 
	double vdotv = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	(*u)[0] = udotv / vdotv * v[0];
	(*u)[1] = udotv / vdotv * v[1];
	(*u)[2] = udotv / vdotv * v[2];
}


void vecnorm(vectype *u, const vectype v)
{
	// Projv(u) = [ (u•v)/(v•v) ] · v project u ulong v
	double udotv = (*u)[0] * v[0] + (*u)[1] * v[1] + (*u)[2] * v[2]; 
	double vdotv = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	(*u)[0] = (*u)[0] - udotv / vdotv * v[0];
	(*u)[1] = (*u)[1] - udotv / vdotv * v[1];
	(*u)[2] = (*u)[2] - udotv / vdotv * v[2];
}


float constrain(float in, float low, float high)
{
	if (in < low) return low;
	if (in > high) return high;
}


// Calculate the value of the cosine of the angle between two spherical vectors
// Larger cosine between these two angles means closer these two angles are
// Ref: http://math.stackexchange.com/questions/231221/great-arc-distance-between-two-points-on-a-unit-sphere
vectype sphveccos(vectype th1, vectype phi1, vectype th2, vectype phi2)
{
    assert(th1.size() == phi1.size() && th2.size() == phi2.size() && phi1.size() == phi2.size());

    vectype result(th1.size());

    for (int i = 0; i < th1.size(); i++)
    {
        result[i] = cos(th1[i]) * cos(th2[i]) + sin(th1[i]) * sin(th2[i]) * cos(phi1[i] - phi2[i]);
    }

    return result;
}

void seedadjust(vectype *seedx, vectype *seedy, vectype *seedz){
	//seedadjust is put in math for a reason. the seedadjust can also be used as a approximation to filter
	//redundant points similar to k-means
    vectype adseedx, adseedy, adseedz;
	assert((*seedx).size() == (*seedy).size() && (*seedy).size() == (*seedz).size());
	vectype::iterator xitr, yitr, zitr;
	int counter = 0;
	bool judge = true;
	bool curjudge = false;
	bool xjudge = true;
	bool yjudge = true;
	bool zjudge = true;
	float diffx = 0;
	float diffy = 0;
	float diffz = 0;
	for (xitr = seedx->begin(), 
		 yitr = seedy->begin(), 
		 zitr = seedz->begin(); 
		 xitr != seedx->end();
		 xitr++, yitr++, zitr++)
	{
		judge = true;
		curjudge = false;
		counter++;
		//cout<<"counter: "<<counter<<endl;
		//cout<<"xitr: "<<round(*xitr)<<"yitr: "<<round(*yitr)<<"zitr: "<<round(*zitr)<<endl;
		for(int i = 0; i < adseedx.size(); i++)
		{
			//cout<<"adseedx value is working: "<<(*adseedx)[i]<<endl;x
			diffx = abs((*xitr)-adseedx[i]);
			diffy = abs((*yitr)-adseedy[i]);
			diffz = abs((*zitr)-adseedz[i]);
			xjudge = (diffx > 3);
			yjudge = (diffy > 3);
			zjudge = (diffz > 3);
			curjudge = xjudge || yjudge || zjudge; 
			//cout<<"xdiff: "<<diffx<<"ydiff: "<<diffy<<"zdiff: "<<diffz<<endl;
			//cout<<"xjudge: "<<xjudge<<"yjudge: "<<yjudge<<"zjudge: "<<zjudge<<"curjudge: "<<curjudge<<endl;
			judge = judge && curjudge;
		}
		if (judge == true)
		{
			(adseedx).push_back(round(*xitr));
			(adseedy).push_back(round(*yitr));
			(adseedz).push_back(round(*zitr));
		}
		//cout<<"adseedx size: "<<adseedx.size()<<endl;
	}
	(*seedx).clear();
	(*seedy).clear();
	(*seedz).clear();
	(*seedx) = (adseedx);
	(*seedy) = (adseedy);
	(*seedz) = (adseedz);

}