#ifndef __MARKER_RADIUS_H__
#define __MARKER_RADIUS_H__

#include <cmath>
#include "v3d_basicdatatype.h"
#include "my_surf_objs.h"

#ifndef MAX
#define MAX(x,y) (x > y ? (x) : (y))
#endif

#define DEFAULT_MARKER_RADIUS_METHOD 2  // 0: for accurate , 1: for fast. accurate is not the best, 2: for hanchuan's code

using namespace std;

//========================== 3D ==========================
template<class T> double markerRadius(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh, int method = DEFAULT_MARKER_RADIUS_METHOD)
{
    if(sz[2] == 1) return markerRadiusXY(inimg1d, sz, marker, thresh,0.001);
	if(method == 0) return markerRadius_accurate(inimg1d, sz, marker, thresh);
	if(method == 1) return markerRadius_fast(inimg1d, sz, marker, thresh);
	if(method == 2) return markerRadius_hanchuan(inimg1d, sz, marker, thresh);
}

template<class T> double markerRadius_accurate(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh)
{
	int max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
	int r;
	double tol_num, bak_num;
	int mx = marker.x + 0.5;
	int my = marker.y + 0.5;
	int mz = marker.z + 0.5;
	//cout<<"mx = "<<mx<<" my = "<<my<<" mz = "<<mz<<endl;
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

// only judge very small area
template<class T> double markerRadius_fast(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh)
{
	double max_r = MAX(MAX(sz[0]/2.0, sz[1]/2.0), sz[2]/2.0);
	double ir;
	double tol_num, bak_num;
	double mx = marker.x;
	double my = marker.y;
	double mz = marker.z;
	//cout<<"mx = "<<mx<<" my = "<<my<<" mz = "<<mz<<endl;
	V3DLONG x[2], y[2], z[2];
	V3DLONG sz01 = sz[0] * sz[1];

	double factor = 1.0/sqrt(3.0);
	tol_num = bak_num = 0.0;
	for(ir = 0; ir <= max_r; ir++)
	{
		double r1 = (ir -1)*factor;
		double r2 = ir;
		for(V3DLONG k = r1 ; k < r2; k++)
		{
			for(V3DLONG j = r1; j < r2; j++)
			{
				for(V3DLONG i = r1; i < r2; i++)
				{
					double dist = i * i + j * j + k * k;
					if(dist >= ir * ir || dist < (ir -1 )*(ir -1)) continue;
					x[0] = mx - i, x[1] = mx + i;
					y[0] = my - j, y[1] = my + i;
					z[0] = mz - k, z[1] = mz + k;
					for(char b = 0; b < 8; b++)
					{
						char ii = b & 0x01, jj = (b >> 1) & 0x01, kk = (b >> 2) & 0x01;
						if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1] || z[kk]<0 || z[kk] >= sz[2]) return ir;
						else
						{
							tol_num++;
							long pos = z[kk]*sz01 + y[jj] * sz[0] + x[ii];
							if(inimg1d[pos] < thresh){bak_num++;}
							if((bak_num / tol_num) > 0.0001) return ir;
						}
					}
				}
			}
		}
	}
	return ir;
}

template<class T> double markerRadius_hanchuan_XY(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh, double thresh_stop)
{
    //printf("markerRadius_hanchuan   XY 2D\n");

	long sz0 = sz[0];
	long sz01 = sz[0] * sz[1];
    double max_r = sz[0]/2;
    if (max_r > sz[1]/2) max_r = sz[1]/2;

    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir++)
    {
        total_num = background_num = 0;

        double dz, dy, dx;
        double zlower = 0, zupper = 0;
        for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; ++dy)
                for (dx= -ir; dx <= +ir; ++dx)
                {
                    total_num++;

                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                    if (r>ir-1 && r<=ir)
                    {
                        V3DLONG i = marker.x+dx;   if (i<0 || i>=sz[0]) goto end1;
                        V3DLONG j = marker.y+dy;   if (j<0 || j>=sz[1]) goto end1;
                        V3DLONG k = marker.z+dz;   if (k<0 || k>=sz[2]) goto end1;

                        if (inimg1d[k * sz01 + j * sz0 + i] <= thresh)
                        {
                            background_num++;

                            if ((background_num/total_num) > thresh_stop) goto end1; //change 0.01 to 0.001 on 100104
                        }
                    }
                }
    }
end1:
    return ir;

}
template<class T> double markerRadius_hanchuan(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh)
{
    //printf("markerRadius_hanchuan   3D\n");
    
	long sz0 = sz[0];
	long sz01 = sz[0] * sz[1];
    double max_r = sz[0]/2;
    if (max_r > sz[1]/2) max_r = sz[1]/2;
	if (max_r > (sz[2])/2) max_r = (sz[2])/2;

    double total_num, background_num;
    double ir;
    for (ir=1; ir<=max_r; ir++)
    {
        total_num = background_num = 0;

        double dz, dy, dx;
        double zlower = -ir, zupper = +ir;
        for (dz= zlower; dz <= zupper; ++dz)
            for (dy= -ir; dy <= +ir; ++dy)
                for (dx= -ir; dx <= +ir; ++dx)
                {
                    total_num++;

                    double r = sqrt(dx*dx + dy*dy + dz*dz);
                    if (r>ir-1 && r<=ir)
                    {
                        V3DLONG i = marker.x+dx;   if (i<0 || i>=sz[0]) goto end2;
                        V3DLONG j = marker.y+dy;   if (j<0 || j>=sz[1]) goto end2;
                        V3DLONG k = marker.z+dz;   if (k<0 || k>=sz[2]) goto end2;

                        if (inimg1d[k * sz01 + j * sz0 + i] <= thresh)
                        {
                            background_num++;

                            if ((background_num/total_num) > 0.001) goto end2; //change 0.01 to 0.001 on 100104
                        }
                    }
                }
    }
end2:
    return ir;
}

template<class T> double markerRadius(T* &inimg1d, V3DLONG * sz, V3DLONG x, V3DLONG y , V3DLONG z, double thresh, int method = DEFAULT_MARKER_RADIUS_METHOD)
{
	MyMarker marker(x,y,z);
    return markerRadius(inimg1d, sz, marker, thresh, method);
}
//========================== 2D ==========================
template<class T> double markerRadiusXY(T* &inimg1d, V3DLONG * sz, MyMarker & marker, double thresh, double thresh_stop)
{
    return markerRadius_hanchuan_XY(inimg1d, sz, marker, thresh, thresh_stop);
	/*
	double max_r = MAX(sz[0]/2.0, sz[1]/2.0);
	double ir;
	double tol_num, bak_num;
	double mx = marker.x;
	double my = marker.y;
	V3DLONG x[2], y[2];

	double factor = 1.0/sqrt(2.0);
	tol_num = bak_num = 0.0;
	for(ir = 0; ir <= max_r; ir++)
	{
		double r1 = (ir -1)*factor;
		double r2 = ir;
		for(V3DLONG j = r1; j < r2; j++)
		{
			for(V3DLONG i = r1; i < r2; i++)
			{
				double dist = i * i + j * j;
				if(dist >= ir * ir || dist < (ir -1 )*(ir -1)) continue;
				x[0] = mx - i, x[1] = mx + i;
				y[0] = my - j, y[1] = my + i;
				for(char b = 0; b < 4; b++)
				{
					char ii = b & 0x01, jj = (b >> 1) & 0x01;
					if(x[ii]<0 || x[ii] >= sz[0] || y[jj]<0 || y[jj] >= sz[1]) return ir;
					else
					{
						tol_num++;
						long pos = y[jj] * sz[0] + x[ii];
						if(inimg1d[pos] < thresh){bak_num++;}
						if((bak_num / tol_num) > 0.0001) return ir;
					}
				}
			}
		}
	}
	return ir;
	*/
}
#endif
