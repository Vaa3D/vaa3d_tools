//jba_affine_xform.cpp
// by Hanchuan Peng
// 2006-2011

#include "jba_affine_xform.h"

#include <fstream> 
#include <iostream>
using namespace std;

#define WANT_STREAM       
#include "newmatap.h"
#include "newmatio.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

Matrix jba_pseudoinverse(const Matrix & X) 
{
	Matrix Xinv;
	DiagonalMatrix D;
	Matrix U,V;
	
	SVD(X,D,U,V);
	Matrix Dinv = D.i();
	Xinv = V * Dinv * U.t();
	return Xinv;
}

bool compute_affine_xform_parameters_from_landmarks(
	const vector<Coord3D_JBA> & matchTargetPos,
	const vector<Coord3D_JBA> & matchSubjectPos,
	WarpParameterAffine3D * p,
	bool b_auto_recenter
	)
{	
	if (matchTargetPos.size()!=matchSubjectPos.size() || matchTargetPos.size()<=0 || !p)
		return false;

	V3DLONG i,j;
	int ndims=3;
	int num_cpts = matchTargetPos.size();

	Matrix pos_target(num_cpts, ndims);
	Matrix pos_subject(num_cpts, ndims+1);
	Matrix T(ndims+1, ndims);
	
	double xc_subject=0, yc_subject=0, zc_subject=0, xc_target=0, yc_target=0, zc_target=0;
	for (j=0; j<num_cpts; j++)	
	{
		pos_subject(j+1,1) = matchSubjectPos.at(j).x;
		pos_subject(j+1,2) = matchSubjectPos.at(j).y;
		pos_subject(j+1,3) = matchSubjectPos.at(j).z;
		pos_subject(j+1,4) = 1;
		
		if (b_auto_recenter)
		{
			xc_subject += pos_subject(j+1,1);
			yc_subject += pos_subject(j+1,2);
			zc_subject += pos_subject(j+1,3);
		}
		
		pos_target(j+1,1) = matchTargetPos.at(j).x;
		pos_target(j+1,2) = matchTargetPos.at(j).y;
		pos_target(j+1,3) = matchTargetPos.at(j).z;

		if (b_auto_recenter)
		{
			xc_target += pos_target(j+1,1);
			yc_target += pos_target(j+1,2);
			zc_target += pos_target(j+1,3);
		}
	}
	
	if (b_auto_recenter)
	{
		if (num_cpts>0)
		{
			xc_target /= num_cpts;
			yc_target /= num_cpts;
			zc_target /= num_cpts;
			xc_subject /= num_cpts;
			yc_subject /= num_cpts;
			zc_subject /= num_cpts;
		}

		for (j=0; j<num_cpts; j++)	
		{
			pos_subject(j+1,1) -= xc_subject;
			pos_subject(j+1,2) -= yc_subject;
			pos_subject(j+1,3) -= zc_subject;
			
			pos_target(j+1,1) -= xc_target;
			pos_target(j+1,2) -= yc_target;
			pos_target(j+1,3) -= zc_target;
		}
	}
	
	Matrix pos_subject_inv = jba_pseudoinverse(pos_subject);

	T = pos_subject_inv * pos_target;
	
	p->mxx = T(1,1); p->mxy = T(2,1); p->mxz = T(3,1); p->sx = T(4,1);
	p->myx = T(1,2); p->myy = T(2,2); p->myz = T(3,2); p->sy = T(4,2);
	p->mzx = T(1,3); p->mzy = T(2,3); p->mzz = T(3,3); p->sz = T(4,3);
	
	return true;
}


