// FL_registerAffine.cpp
// 20090203

#include "FL_registerAffine.h"

#define WANT_STREAM       // include iostream and iomanipulators
#include "../../../../v3d_external/v3d_main/jba/newmat11/newmatap.h"
#include "../../../../v3d_external/v3d_main/jba/newmat11/newmatio.h"

#ifdef use_namespace
using namespace RBD_LIBRARIES;
#endif

Matrix pseudoinverse(Matrix X);

// compute pseudo inverse of a matrix X
//Matrix pseudoinverse(const Matrix& X) //avoid unnecessary matrix copies
Matrix pseudoinverse(Matrix X) //avoid unnecessary matrix copies
{
	Matrix Xinv;
	DiagonalMatrix D;
	Matrix U,V;
	
	SVD(X,D,U,V);
	Matrix Dinv = D.i();
	Xinv = V * Dinv * U.t();
	return Xinv;
	
}


// affine transformation between point clouds
// ndims: number of dimensions
// num_pts: number of points in subject
// num_cpts: number of control point pairs
// the transformed result is save again in subject
void registerAffine(float *subject, long num_pts, float *cpt_subject, float *cpt_target, long num_cpts, unsigned char ndims)
{
	long i,j;
//	long len = (ndims+1)*num_cpts;
//	long len2 = 
//	float *U = new float [len];
//	float *X = new float [len];
//	float *A = new float [(ndims+1)*(ndims+1)];
//	float *S = new float [(ndims+1)*num_pts];
//	
//	// assign values to U, X, and S
//	
//	for (i=0; i<ndims, i++)
//	for (j=0; j<num_cpts, j++)
//	{
//		U[i*num_cpts+j] = cpt_target[j*ndims+i];
//		X[i*num_cpts+j] = cpt_subject[j*ndims+i];
//	}
//
//	for (i=0; i<ndims, i++)
//	for (j=0; j<num_pts, j++)
//		S[i*num_cpts+j] = subject[j*ndims+i];


	Matrix Y(num_cpts, ndims);
	Matrix X(num_cpts, ndims+1);
	Matrix T(ndims+1, ndims);
	Matrix S(num_pts, ndims+1);

// ---------------------------
// X = [x1, y1, z1, 1
//		x2, y2, z2, 1
//      ....
//		xm, ym, zm, 1]
//
// Y = [u1, v1, w1
//		u2, v2, w2
//      ....
//		um, vm, wm]
//
// Y = X*T
// T = INV(X)*Y
// ---------------------------
		
	// assign values to Y, X, and S

	for (j=0; j<num_cpts; j++)	
	{
		for (i=0; i<ndims; i++)
			X(j+1,i+1) = cpt_subject[j*ndims+i];	
		X(j+1,i+1) = 1;
	}

	for (j=0; j<num_cpts; j++)	
	for (i=0; i<ndims; i++)
		Y(j+1,i+1) = cpt_target[j*ndims+i];	

	for (j=0; j<num_pts; j++)
	{
		for (i=0; i<ndims; i++)
			S(j+1,i+1) = subject[j*ndims+i];
		S(j+1, i+1) = 1;
	}
	
	// compute inverse of X	
	Matrix Xinv = pseudoinverse(X);
	
	// compute T = Xinv*Y
	T = Xinv*Y;
	
	// compute new coordinate of subject, assigne the result of A*S to subject
	Matrix Snew = S*T;
	
	for (j=0; j<num_pts; j++)
	for (i=0; i<ndims; i++)
		subject[j*ndims+i] = Snew(j+1,i+1);
	
//	Y.release();
//	X.release();
//	T.release();
//	S.release();
//	Snew.release();
//	Xinv.release();

	
}