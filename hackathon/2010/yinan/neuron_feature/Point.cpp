#include <iostream.h>
#include "Point.h"
#include <cmath>

Point::Point(double xx, double yy, double zz)
{
	x=xx;y=yy;z=zz;
}

Point::Point(){
	x=0;y=0;z=0;
}

Point::Point(Point* a){
	x=a->getX();
	y=a->getY();
	z=a->getZ();
}

double Point::distance(Point *b){
	
	double disX=(this->getX() - b->getX());
	double disY=(this->getY() - b->getY());
	double disZ=(this->getZ() - b->getZ());

	
	return sqrt( disX*disX + disY*disY + disZ*disZ );
}

;
