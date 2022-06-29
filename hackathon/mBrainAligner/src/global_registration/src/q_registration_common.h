// q_registration_common.h
// by Lei Qu
// 2011-04-08

#ifndef __Q_REGISTRATION_COMMON_H__
#define __Q_REGISTRATION_COMMON_H__

#include <vector>
using namespace std;

#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"

class Coord3D_PCM
{
public:
	double x, y, z;
	Coord3D_PCM(double x0, double y0, double z0) { x = x0; y = y0; z = z0; }
	Coord3D_PCM() { x = y = z = 0; }
};


class Point3D64f
{
public:
	double x,y,z;
	Point3D64f(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Point3D64f() {x=y=z=0;}
};

class CParas_reg
{
public:
	//common
	int		i_regtype;					//0:rigid; 1:affine; 2:FFD non-rigid
	bool 	b_alignmasscenter;
	long 	l_iter_max;
	double 	d_step_inimultiplyfactor;
	double 	d_step_annealingratio;
	double 	d_step_min;
	//nonrigid
	long	l_hierarchlevel;
	long 	l_gridwndsz;

	CParas_reg()
	{
		i_regtype=0;
		b_alignmasscenter=1;
		l_iter_max=500;
		d_step_inimultiplyfactor=2;
		d_step_annealingratio=0.95;
		d_step_min=0.1;
		l_hierarchlevel=3;
		l_gridwndsz=30;
	}
};


double q_round(double r);

//save 64 bit range [0~1] image
bool q_save64f01_image(const double *p_img64f,const long long sz_img[4],const char *filename);

//extract image channel
bool q_extractchannel(const unsigned char *p_img8u,const long sz_img[4],const long l_refchannel,unsigned char *&p_img8u_1c);

//generate gradient image
bool q_gradientnorm(const double *p_img64f,const long sz_img[4],const bool b_norm01,
		double *&p_img64f_gradnorm);

//align mass center of subject image to that of target
//two input images should have same size and 1c
bool q_align_masscenter(const double *p_img64f_tar,const double *p_img64f_sub,const long sz_img[4],
		double *&p_img64f_sub2tar,long l_masscenteroffset[4]);

//normalize 3D points
bool q_normalize_points_3D(const vector<Point3D64f> vec_input,vector<Point3D64f> &vec_output,Matrix &x4x4_normalize);


#endif

