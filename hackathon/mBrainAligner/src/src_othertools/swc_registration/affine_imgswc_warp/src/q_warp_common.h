// q_warp_common.h
// by Lei Qu
// 2019-09-26

#ifndef __Q_WARP_COMMON_H__
#define __Q_WARP_COMMON_H__


class Coord3D_PCM
{
public:
	double x,y,z;
	Coord3D_PCM(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCM() {x=y=z=0;}
};

#endif

