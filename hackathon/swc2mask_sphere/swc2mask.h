#ifndef __SWC2MASK_H__
#define __SWC2MASK_H__

#include <cmath>
#include <set>
#include <vector>

#include "my_surf_objs.h"

using namespace std;

bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2)
{
    double A = m2.x - m1.x;
    double B = m2.y - m1.y;
    double C = m2.z - m1.z;
	double R = m2.radius - m1.radius;
    double D = sqrt(A*A + B*B + C*C);
    A = A/D; B = B/D; C = C/D; R = R/D;

	double ctz = A/sqrt(A*A + B*B);
    double stz = B/sqrt(A*A + B*B);

    double cty = C/sqrt(A*A + B*B + C*C);
    double sty = sqrt(A*A + B*B)/sqrt(A*A + B*B + C*C);
 
    double x0 = m1.x;
    double y0 = m1.y;
    double z0 = m1.z;
	double r0 = m1.radius;

	set<MyMarker> marker_set;
    
    for(double t = 0.0; t <= dist(m1, m2); t += 1.0)                                                                                                                                                              
    {
        MyMarker marker;                                                                                                                                                                                                                 
        int cx = x0 + A*t + 0.5;                                                                                                                                                                                                             
        int cy = y0 + B*t + 0.5;                                                                                                                                                                                                             
        int cz = z0 + C*t + 0.5;          
		int radius = r0 + R*t + 0.5;
		int radius2 = radius * radius;
        
		for(int k = -radius; k <= radius; k++)
		{
			for(int j = -radius; j <= radius; j++)
			{
				for(int i = -radius; i <= radius; i++)
				{
					if(i * i + j * j + k * k > radius2) continue;
					double x = i, y = j, z = k;
					double x1, y1, z1;

					
					//rotate_coordinate_z_clockwise(ctz, stz, x, y, z);
					//rotate_along_y_clockwise     (cty, sty, x, y, z);
					//rotate_coordinate_x_anticlock(ctz, stz, x, y, z);
					//translate_to(cx, cy, cz, x, y, z);
					y1 = y * ctz - x * stz; x1 = x * ctz + y * stz; y = y1; x = x1;
					x1 = x * cty + z * sty; z1 = z * cty - x * sty; x = x1; z = z1;
					z1 = z * ctz + y * stz; y1 = y * ctz - z * stz; z = z1; y = y1;
					x += cx; y += cy; z += cz;
					x = (int)(x+0.5);
					y = (int)(y+0.5);
					z = (int)(z+0.5);
					marker_set.insert(MyMarker(x, y, z));
				}
			}
		}
    }   
   
	allmarkers.insert(allmarkers.end(), marker_set.begin(), marker_set.end());
    return true;
} 

template<class T> bool swc2mask(T * inimg1d, T * &outimg1d, vector<MyMarker * > & inswc, long sz0, long sz1, long sz2)
{
	long tol_sz = sz0 * sz1 * sz2;
	long sz01 = sz0 * sz1;
	if(outimg1d == 0) outimg1d = new T[tol_sz]; for(long i = 0; i < tol_sz; i++) outimg1d[i] = 0;
	vector<MyMarker*> leaf_markers = getLeaf_markers(inswc);
	set<MyMarker*> visited_markers;
	for(int i = 0; i < leaf_markers.size(); i++)
	{
		MyMarker * leaf = leaf_markers[i];
		MyMarker * p = leaf;
		while(visited_markers.find(p) == visited_markers.end() && p->parent != 0)
		{
			MyMarker * par = p->parent;
			vector<MyMarker> tmp_markers;
			getMarkersBetween(tmp_markers, *p, *par);
			for(int j = 0; j < tmp_markers.size(); j++)
			{
				int x = tmp_markers[j].x;
				int y = tmp_markers[j].y;
				int z = tmp_markers[j].z;
				if(x < 0 || x >= sz0 || y < 0 || y >= sz1 || z < 0 || z >= sz2) continue;
				outimg1d[z*sz01 + y * sz0 + x] = 255;
			}
			visited_markers.insert(p);
			p = par;
		}
	}
	return true;
}

#endif
