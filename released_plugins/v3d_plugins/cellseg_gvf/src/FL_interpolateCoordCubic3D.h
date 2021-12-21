// upsample a 3D volume by a factor along each dimension
// F. Long
// 20080507

#ifndef __INTERPOLATE_COORD_CUBIC3D__
#define __INTERPOLATE_COORD_CUBIC3D__

#include "volimg_proc.h"
#include "img_definition.h"
#include "stackutil.h"

#include "FL_coordDefinition.h"
#include "FL_defType.h"

template <class T> bool interpolate_coord_cubic(T * interpolatedVal, Coord3D *c, V3DLONG numCoord, 
					   T *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2, 
					   V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2)
{
	//check if parameters are correct
	if (!interpolatedVal || !c || numCoord<=0 || 
		!templateVol3d || tsz0<=0 || tsz1<=0 || tsz2<=0 ||
		tlow0<0 || tlow0>=tsz0 || tup0<0 || tup0>=tsz0 || tlow0>tup0 ||
		tlow1<0 || tlow1>=tsz1 || tup1<0 || tup1>=tsz1 || tlow1>tup1 ||
		tlow2<0 || tlow2>=tsz2 || tup2<0 || tup2>=tsz2 || tlow2>tup2)
	{
		fprintf(stderr, "Invalid parameters! [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}
	
	// generate cubic interpolation
		
	//V3DLONG i,j,k;
	
	double curpx, curpy, curpz;
	V3DLONG cpx0, cpx1, cpy0, cpy1, cpz0, cpz1;
	
	for (V3DLONG ipt=0;ipt<numCoord;ipt++)
	{
		curpx = c[ipt].x; curpx=(curpx<tlow0)?tlow0:curpx; curpx=(curpx>tup0)?tup0:curpx;
//#ifndef POSITIVE_Y_COORDINATE
//		curpy = tsz1-1-c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080114
//#else
		curpy = c[ipt].y; curpy=(curpy<tlow1)?tlow1:curpy; curpy=(curpy>tup1)?tup1:curpy; //080114
//#endif
		curpz = c[ipt].z; curpz=(curpz<tlow2)?tlow2:curpz; curpz=(curpz>tup2)?tup2:curpz;
		
		cpx0 = V3DLONG(floor(curpx)); cpx1 = V3DLONG(ceil(curpx));
		cpy0 = V3DLONG(floor(curpy)); cpy1 = V3DLONG(ceil(curpy));
		cpz0 = V3DLONG(floor(curpz)); cpz1 = V3DLONG(ceil(curpz));
		
		if (cpz0==cpz1)
		{
			if (cpy0==cpy1)
			{
				if (cpx0==cpx1)
				{
					interpolatedVal[ipt] = (T)(templateVol3d[cpz0][cpy0][cpx0]);
				}
				else
				{
					double w0x0y0z = (cpx1-curpx);
					double w1x0y0z = (curpx-cpx0);
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + 
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
					double w0x0y0z = (cpy1-curpy);
					double w0x1y0z = (curpy-cpy0);
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + 
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]));
				}
				else
				{
					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy);
					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0);
					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy);
					double w1x1y0z = (curpx-cpx0)*(curpy-cpy0);
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + 
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + 
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]));
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
					
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]));
				}
				else
				{
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//					
//					double w1x0y0z = (curpx-cpx0)*(cpy1-curpy)*(cpz1-curpz);
//					double w1x0y1z = (curpx-cpx0)*(cpy1-curpy)*(curpz-cpz0);

                    // FL 20120410
					double w0x0y0z = (cpx1-curpx)*(cpz1-curpz);
					double w0x0y1z = (cpx1-curpx)*(curpz-cpz0);
					
					double w1x0y0z = (curpx-cpx0)*(cpz1-curpz);
					double w1x0y1z = (curpx-cpx0)*(curpz-cpz0);
                    
					
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) + 
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]));
				}
			}
			else
			{
				if (cpx0==cpx1)
				{
//					double w0x0y0z = (cpx1-curpx)*(cpy1-curpy)*(cpz1-curpz);
//					double w0x0y1z = (cpx1-curpx)*(cpy1-curpy)*(curpz-cpz0);
//					
//					double w0x1y0z = (cpx1-curpx)*(curpy-cpy0)*(cpz1-curpz);
//					double w0x1y1z = (cpx1-curpx)*(curpy-cpy0)*(curpz-cpz0);
                    
                    // FL 20120410
					double w0x0y0z = (cpy1-curpy)*(cpz1-curpz);
					double w0x0y1z = (cpy1-curpy)*(curpz-cpz0);
					
					double w0x1y0z = (curpy-cpy0)*(cpz1-curpz);
					double w0x1y1z = (curpy-cpy0)*(curpz-cpz0);
                    
					
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) + 
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]));
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
					
					interpolatedVal[ipt] = (T)(w0x0y0z * double(templateVol3d[cpz0][cpy0][cpx0]) + w0x0y1z * double(templateVol3d[cpz1][cpy0][cpx0]) + 
													 w0x1y0z * double(templateVol3d[cpz0][cpy1][cpx0]) + w0x1y1z * double(templateVol3d[cpz1][cpy1][cpx0]) +
													 w1x0y0z * double(templateVol3d[cpz0][cpy0][cpx1]) + w1x0y1z * double(templateVol3d[cpz1][cpy0][cpx1]) + 
													 w1x1y0z * double(templateVol3d[cpz0][cpy1][cpx1]) + w1x1y1z * double(templateVol3d[cpz1][cpy1][cpx1]) );
				}
			}
		}
		
	}
	
	return true;
}



#endif
