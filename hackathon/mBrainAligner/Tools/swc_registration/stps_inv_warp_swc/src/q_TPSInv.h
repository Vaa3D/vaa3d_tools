// q_TPSinv.h
// by wang tao
// 2019-09-26

//#include "../../v3d_main/jba/newmat11/newmatap.h"
//#include "../../v3d_main/jba/newmat11/newmatio.h"
#include <math.h>
#include <vector>
#include "stackutil.h"
#include"q_warp_common.h"

#include <Eigen/Core>
#include <iostream>
#include <LBFGS.h>
using Eigen::VectorXd;
using namespace LBFGSpp;


#ifndef __Q_TPSInv_H__
#define __Q_TPSInv_H__

//since TPS is not inverse consistent, that means when map a point to target, then map back to subject, is not same postion
//to solove this, we use Newton's optimization to approximate its inverse projected position
//this class compute the Loss and gradient of TPS for Newton's optimization
class TPSInv
{
public:
private:
	int n;
	Matrix _x4x4_d, _xnx4_c; //TPS parameters
	vector <Coord3D_PCM> _vec_ctlpts; //TPS control points, sub(this)-->tar
	Coord3D_PCM _pt;//pos in subject want to be inverse mapped
public:
	TPSInv(int n_) : n(n_) {}
	void setTPSPara(Matrix x4x4_d, Matrix xnx4_c, vector <Coord3D_PCM> vec_ctlpts)
	{
		_x4x4_d.ReSize(4, 4);
		_xnx4_c.ReSize(vec_ctlpts.size(), 4);
		_x4x4_d = x4x4_d; _xnx4_c = xnx4_c;
		for (long i = 0; i < vec_ctlpts.size(); i++)
			_vec_ctlpts.push_back(vec_ctlpts[i]);
	}
	void setTargetPos(Coord3D_PCM pt) { _pt.x = pt.x; _pt.y = pt.y; _pt.z = pt.z; }
	void tpsWarpPt(Coord3D_PCM &pt_input, Coord3D_PCM &pt_warp)
	{
		Matrix x_pt(1, 4);
		x_pt(1, 1) = 1.0;
		x_pt(1, 2) = pt_input.x;
		x_pt(1, 3) = pt_input.y;
		x_pt(1, 4) = pt_input.z;

		Matrix xmxn_K;
		xmxn_K.resize(1, _vec_ctlpts.size());

		//compute K=-r=-|xi-xj|
		double d_x, d_y, d_z;
		for (unsigned V3DLONG j = 0; j < _vec_ctlpts.size(); j++)
		{
			d_x = pt_input.x - _vec_ctlpts[j].x;
			d_y = pt_input.y - _vec_ctlpts[j].y;
			d_z = pt_input.z - _vec_ctlpts[j].z;
			xmxn_K(1, j + 1) = -sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
		}

		Matrix x_stps(1, 4);
		x_stps = x_pt*_x4x4_d + xmxn_K*_xnx4_c;
		pt_warp.x = x_stps(1, 2);
		pt_warp.y = x_stps(1, 3);
		pt_warp.z = x_stps(1, 4);
	}
	void cptLoss(double x, double y, double z, double& fx)
	{
		Coord3D_PCM pt_input, pt_warp;
		double x_dif, y_dif, z_dif;
		//fx
		pt_input.x = x;
		pt_input.y = y;
		pt_input.z = z;
		tpsWarpPt(pt_input, pt_warp);
		x_dif = pt_warp.x - _pt.x;
		y_dif = pt_warp.y - _pt.y;
		z_dif = pt_warp.z - _pt.z;
		fx = sqrt(x_dif*x_dif + y_dif*y_dif + z_dif*z_dif);
	}
	double operator()(const VectorXd& x, VectorXd& grad)
	{
		//fx
		double fx = 0.0;
		cptLoss(x[0], x[1], x[2], fx);
		//grad
		double delta = 1.0e-6;//default 1.0e-6
		double fx_dx, fx_dy, fx_dz;
		cptLoss(x[0] + delta, x[1], x[2], fx_dx);
		cptLoss(x[0], x[1] + delta, x[2], fx_dy);
		cptLoss(x[0], x[1], x[2] + delta, fx_dz);
		grad[0] = (fx_dx - fx) / delta;
		grad[1] = (fx_dy - fx) / delta;
		grad[2] = (fx_dz - fx) / delta;
		return fx;
	}
};


#endif