// define the class of Coord2D, Coord3D
// F. Long
// 20080507

#ifndef __COORD_DEFINE__
#define __COORD_DEFINE__

#include "FL_defType.h"

class Coord2D
{
public:
    FLOAT32 x,y; //should be fine because the real coordinates should between 0~2047
	static INT32 dim;

	Coord2D(FLOAT32 x0,FLOAT32 y0) {x=x0;y=y0;}
	Coord2D() {x=y=0;}
};


class Coord3D
{
public:
    FLOAT32 x,y,z; //should be fine because the real coordinates should between 0~2047
    static INT32 dim;	
	
	Coord3D(FLOAT32 x0, FLOAT32 y0, FLOAT32 z0) {x=x0;y=y0;z=z0;}
	Coord3D() {x=y=z=0;}
	
};

#endif