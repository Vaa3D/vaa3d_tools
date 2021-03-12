// q_tps.cpp
// by Lei Qu
// 2019-05-01

#include <math.h>
#include "stackutil.h"

#include "q_imresize.cpp"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "q_tps.h"



bool q_normalize_points_3D(const vector<point3D64F> &vec_input,vector<point3D64F> &vec_output,Matrix &x4x4_normalize)
{
	//check parameters
	if(vec_input.size()<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(!vec_output.empty())
		vec_output.clear();
	vec_output=vec_input;
	if(x4x4_normalize.nrows()!=4 || x4x4_normalize.ncols()!=4)
	{
		x4x4_normalize.ReSize(4,4);
	}

	//compute the centriod of input point set
	point3D64F cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
		cord_centroid.z+=vec_input[i].z;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	cord_centroid.z/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
		vec_output[i].z-=cord_centroid.z;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y+vec_output[i].z*vec_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
		vec_output[i].z*=d_scale_factor;
	}

	//compute the transformation matrix
	// 1 row
	x4x4_normalize(1,1)=d_scale_factor;
	x4x4_normalize(1,2)=0;
	x4x4_normalize(1,3)=0;
	x4x4_normalize(1,4)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x4x4_normalize(2,1)=0;
	x4x4_normalize(2,2)=d_scale_factor;
	x4x4_normalize(2,3)=0;
	x4x4_normalize(2,4)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x4x4_normalize(3,1)=0;
	x4x4_normalize(3,2)=0;
	x4x4_normalize(3,3)=d_scale_factor;
	x4x4_normalize(3,4)=-d_scale_factor*cord_centroid.z;
	// 4 row
	x4x4_normalize(4,1)=0;
	x4x4_normalize(4,2)=0;
	x4x4_normalize(4,3)=0;
	x4x4_normalize(4,4)=1;

	return true;
}

//compute the rigid(actually is similar) transform matrix B=T*A
//	B=T*A
bool q_compute_rigidmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_rigidmatrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_rigidmatrix.nrows()!=4 || x4x4_rigidmatrix.ncols()!=4)
	{
		x4x4_rigidmatrix.ReSize(4,4);
	}
	int n_point=vec_A.size();

	//normalize point set
	vector<point3D64F> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//format
	Matrix x3xn_A(3,n_point),x3xn_B(3,n_point);
	for(V3DLONG i=0;i<n_point;i++)
	{
		x3xn_A(1,i+1)=vec_A_norm[i].x;	x3xn_A(2,i+1)=vec_A_norm[i].y;	x3xn_A(3,i+1)=vec_A_norm[i].z;
		x3xn_B(1,i+1)=vec_B_norm[i].x;	x3xn_B(2,i+1)=vec_B_norm[i].y;	x3xn_B(3,i+1)=vec_B_norm[i].z;
	}

	//compute rotation matrix
	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(x3xn_B*x3xn_A.t(),D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix R=V*U.t();

	//reshape the transformation matrix
	x4x4_rigidmatrix(1,1)=R(1,1);	x4x4_rigidmatrix(1,2)=R(1,2);	x4x4_rigidmatrix(1,3)=R(1,3);	x4x4_rigidmatrix(1,4)=0;
	x4x4_rigidmatrix(2,1)=R(2,1);	x4x4_rigidmatrix(2,2)=R(2,2);	x4x4_rigidmatrix(2,3)=R(2,3);	x4x4_rigidmatrix(2,4)=0;
	x4x4_rigidmatrix(3,1)=R(3,1);	x4x4_rigidmatrix(3,2)=R(3,2);	x4x4_rigidmatrix(3,3)=R(3,3);	x4x4_rigidmatrix(3,4)=0;
	x4x4_rigidmatrix(4,1)=0.0;		x4x4_rigidmatrix(4,2)=0.0;		x4x4_rigidmatrix(4,3)=0.0;		x4x4_rigidmatrix(4,4)=1.0;

	//denormalize
	x4x4_rigidmatrix=x4x4_normalize_B.i()*x4x4_rigidmatrix*x4x4_normalize_A;

	return true;
}

//compute the affine matraix
//	B=T*A
bool q_compute_affinmatrix_3D(const vector<point3D64F> &vec_A,const vector<point3D64F> &vec_B,Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	//normalize point set
	vector<point3D64F> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//fill matrix A
	//
	//	  | h1, h2, h3, h4 |    |x1| |x2|
	//	  | h5, h6, h7, h8 | *  |y1|=|y2| <=>
	//	  | h9, h10,h11,h12|    |z1| |z2|
	//	  | 0 ,  0,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, z1, 1,  0,  0,  0,  0,  0,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, 0, x1, y1, z1,  1,  0,  0,  0,  0, -y2 | * |h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,h13|=0
	//	  | 0,  0,  0, 0, 0, 0, 0, 0,  0, x1, y1, z1,  1, -z2 |
	int n_point=vec_A.size();
	Matrix A(3*n_point,13);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=vec_A_norm[i].z;	A(row,4)=1.0;
		A(row,5)=0.0;				A(row,6)=0.0;				A(row,7)=0.0;				A(row,8)=0.0;
		A(row,9)=0.0;				A(row,10)=0.0;				A(row,11)=0.0;				A(row,12)=0.0;
		A(row,13)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;				A(row+1,4)=0.0;
		A(row+1,5)=vec_A_norm[i].x;	A(row+1,6)=vec_A_norm[i].y;	A(row+1,7)=vec_A_norm[i].z;	A(row+1,8)=1.0;
		A(row+1,9)=0.0;				A(row+1,10)=0.0;			A(row+1,11)=0.0;			A(row+1,12)=0.0;
		A(row+1,13)=-vec_B_norm[i].y;

		A(row+2,1)=0.0;				A(row+2,2)=0.0;				A(row+2,3)=0.0;				A(row+2,4)=0.0;
		A(row+2,5)=0.0;				A(row+2,6)=0.0;				A(row+2,7)=0.0;				A(row+2,8)=0.0;
		A(row+2,9)=vec_A_norm[i].x;	A(row+2,10)=vec_A_norm[i].y;A(row+2,11)=vec_A_norm[i].z;A(row+2,12)=1.0;
		A(row+2,13)=-vec_B_norm[i].z;

		row+=3;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D(13);
	Matrix U(3*n_point,13),V(13,13);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		printf("ERROR: SVD() exception!\n");
		return false;
	}

	Matrix h=V.column(13);	//A*h=0
	if(D(12,12)==0)			//degenerate case
	{
		x4x4_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=13;i++)
	{
		h(i,1) /= h(13,1);
	}

	//reshape h:13*1 to 4*4 matrix
	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;

	//denormalize
	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}

//compute the affine term d and nonaffine term c which decomposed from TPS (tar=sub*d+K*c)
//more stable compare to the q_affine_compute_affinmatrix_3D()
bool q_TPS_cd(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_tar,const double d_lamda,
		Matrix &x4x4_d,Matrix &xnx4_c,Matrix &xnxn_K)
{
	//check parameters
	if(vec_sub.size()<4 || vec_sub.size()!=vec_tar.size())
	{
		printf("ERROR: Invalid input parameters! \n");
		return false;
	}
	V3DLONG n_pts=vec_sub.size();
	if(xnx4_c.nrows()!=n_pts || xnx4_c.ncols()!=4)
		xnx4_c.ReSize(n_pts,4);
	if(x4x4_d.nrows()!=4 || xnx4_c.ncols()!=4)
		x4x4_d.ReSize(4,4);
	if(xnxn_K.nrows()!=n_pts || xnxn_K.ncols()!=n_pts)
		xnxn_K.ReSize(n_pts,n_pts);

	//generate TPS kernel matrix K=-r=-|xi-xj|
	if(!q_TPS_k(vec_sub,vec_sub,xnxn_K))
	{
		printf("ERROR: q_TPS_k() return false! \n");
		return false;
	}

	//------------------------------------------------------------------
	//compute the QR decomposition of x
	Matrix X(n_pts,4),Y(n_pts,4);
	Matrix Q(n_pts,n_pts); Q=0.0;
	for(V3DLONG i=0;i<n_pts;i++)
	{
		Q(i+1,1)=X(i+1,1)=1;
		Q(i+1,2)=X(i+1,2)=vec_sub[i].x;
		Q(i+1,3)=X(i+1,3)=vec_sub[i].y;
		Q(i+1,4)=X(i+1,4)=vec_sub[i].z;

		Y(i+1,1)=1;
		Y(i+1,2)=vec_tar[i].x;
		Y(i+1,3)=vec_tar[i].y;
		Y(i+1,4)=vec_tar[i].z;
	}

	UpperTriangularMatrix R;
	QRZ(Q,R);
	extend_orthonormal(Q,4);//otherwise q2=0

	Matrix q1=Q.columns(1,4);
	Matrix q2=Q.columns(5,n_pts);
	Matrix r=R.submatrix(1,4,1,4);

	//------------------------------------------------------------------
	//compute non-affine term c which decomposed from TPS
	Matrix A=q2.t()*xnxn_K*q2+IdentityMatrix(n_pts-4)*d_lamda;
	xnx4_c=q2*(A.i()*q2.t()*Y);
	//compute affine term d (normal)
	x4x4_d=r.i()*q1.t()*(Y-xnxn_K*xnx4_c);
//	//compute affine term d (regulized)
//	//small lamda2 can not avoid flip, big lamba can will prevent rotation(not good!)
//	double d_lamda2=0.01;
//	Matrix xRtR=r.t()*r;
//	x4x4_d=((xRtR+d_lamda2*IdentityMatrix(4)).i()) * (xRtR*x4x4_d-xRtR) + IdentityMatrix(4);

	//------------------------------------------------------------------------------------------------------------------------------------
	//suppress affine reflection and 90 degree rotate
	A.resize(3,3);
	A=x4x4_d.submatrix(2,4,2,4).t();

	DiagonalMatrix D(3);
	Matrix U(3,3),V(3,3);
	try
	{
		SVD(A,D,U,V);	//A = U * D * V.t()
	}
	catch(BaseException)
	{
		cout<<BaseException::what()<<endl;
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix UV=U*V.t();
//	cout<<D<<endl;
//	cout<<UV<<endl;

	//------------------------------------------------------------------
	//suppress affine reflection
	//since the variation along ventral-dorsal dir is big than AP dir (we can initialize the atlas by multiply XYZ with 321 respectly)
	//after SVD of tmp, the dir with biggest sigular value must corresponding to x dir, the second one to y, last to z
	//if flip happened, the eigen vector will flip its direction(since the singular value is always >0)
	//since the affine can be decomposed into two rotation with scaling(T=(UV')*(VDV')), judge the flip dir we can use UV':
	//det(T)=det(UV')*det(VDV')=det(UV')*det(V)*det(D)*det(V')=det(UV')*(det(V))^2*det(D)=sign(det(UV'))*det(D) <=> sign(det(T))=sign(det(UV'))
	//strategy:
	//	1. if upside down flip appear first, the left right flip should be enforces
	//	2. if left right flip appear first,another left right flip should be enforces immediately(for compensating the first one)
	//	3. summary: if flip detected, we set D(3,3)=-D(3,3)
	if(A.determinant()<-1e-10)
	{

		//detect upside-down flip
		bool b_flipdetected=0;
		if(UV(2,2)<0)
		{
			printf("WARNING: ||||||||||||||||||||||||||||||||||upside down flip detected||||||||||||||||||||||||||||||||||||\n\n");
			b_flipdetected=1;
		}
		//detect left-right flip
		if(UV(3,3)<0)
		{
			printf("WARNING: -----------------------------------left right flip detected------------------------------------\n\n");
			b_flipdetected=1;
		}
//		if(!b_flipdetected)
//		{
//			v3d_msg("miss detect flip\n");
//		}

		//surpress reflection
		D(3,3)*=-1;

		A=U*D*V.t();
		x4x4_d.submatrix(2,4,2,4)=A.t();
	}

//	//------------------------------------------------------------------
//	//suppress 90 degree rotation
//	//change along Y should always > which along Z (since we have initialize the atlas by multiply XYZ with 321 respectly)
//	//this result in the second column shoudl alway correponding to Y instead of X
//	//shoud perform rotate, not only swap singular value
//	//90 degree have two possible direction, it is hard to make right choice, we only give warnning here, user will make the choice
//	if((fabs(UV(3,2))>fabs(UV(2,2))) && (fabs(UV(3,3))<fabs(UV(2,3))) &&
//        fabs(V(2,2))>0.86)
//	{
//		printf("WARNING: LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL90 degree rotation detectedLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL\n\n");
//
////		double tmp=D(3,3);
////		D(3,3)=D(2,2);
////		D(2,2)=tmp;
//
////		D(2,2)=D(3,3)*0.98;
////		D(3,3)=D(2,2)*1.02;
//
////		A=U*D*V.t();
////		x4x4_d.submatrix(2,4,2,4)=A.t();
//	}

	return true;
}

//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub,const vector<point3D64F> &vec_basis,Matrix &xmxn_K)
{
	//check parameters
	if(vec_sub.empty() || vec_basis.empty())
	{
		printf("ERROR: q_TPS_k: The input subject or basis marker vector is empty.\n");
		return false;
	}

	xmxn_K.resize(vec_sub.size(),vec_basis.size());

	//compute K=-r=-|xi-xj|
	double d_x,d_y,d_z;
	for(unsigned V3DLONG i=0;i<vec_sub.size();i++)
		for(unsigned V3DLONG j=0;j<vec_basis.size();j++)
		{
			d_x=vec_sub[i].x-vec_basis[j].x;
			d_y=vec_sub[i].y-vec_basis[j].y;
			d_z=vec_sub[i].z-vec_basis[j].z;
			xmxn_K(i+1,j+1)=-sqrt(d_x*d_x+d_y*d_y+d_z*d_z);
		}

	return true;
}

Matrix q_pseudoinverse(Matrix inputMat)
{
	Matrix pinv;

	//calculate SVD decomposition
	DiagonalMatrix D;
	Matrix U,V;
	SVD(inputMat,D,U,V);
	Matrix Dinv=D.i();
	pinv=V*Dinv*U.t();
	return pinv;
}

//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_3D(
		const vector <point3D64F> &vec_ctlpts_sub,const vector <point3D64F> &vec_ctlpts_tar,
		Matrix &wa)
{
	//check parameters
	if(vec_ctlpts_sub.empty() || vec_ctlpts_tar.empty())
	{
		printf("ERROR: q_compute_tps_paras_3D: The input subject or target marker vector is empty.\n");
		return false;
	}
	if(vec_ctlpts_sub.size()!=vec_ctlpts_tar.size())
	{
		printf("ERROR: q_compute_tps_paras_3D: The number of markers in subject and target vector is different.\n");
		return false;
	}

	int n_marker=vec_ctlpts_sub.size();

	//compute the tps transform parameters
	double tmp,s;

	Matrix wR(n_marker,n_marker);
	for(V3DLONG j=0;j<n_marker;j++)
	  for(V3DLONG i=0;i<n_marker;i++)
	  {
		s=0.0;
		tmp=vec_ctlpts_sub.at(i).x-vec_ctlpts_sub.at(j).x;	s+=tmp*tmp;
		tmp=vec_ctlpts_sub.at(i).y-vec_ctlpts_sub.at(j).y; 	s+=tmp*tmp;
		tmp=vec_ctlpts_sub.at(i).z-vec_ctlpts_sub.at(j).z; 	s+=tmp*tmp;
//	    wR(i+1,j+1)=2*s*log(s+1e-20);	//the control points can not be perfectly aligned
	    wR(i+1,j+1)=s*log(s+1e-20);
	  }

	Matrix wP(n_marker,4);
	for(V3DLONG i=0;i<n_marker;i++)
	{
	   wP(i+1,1)=1;
	   wP(i+1,2)=vec_ctlpts_sub.at(i).x;
	   wP(i+1,3)=vec_ctlpts_sub.at(i).y;
	   wP(i+1,4)=vec_ctlpts_sub.at(i).z;
	}

	Matrix wL(n_marker+4,n_marker+4);
	wL.submatrix(1,n_marker,1,n_marker)=wR;
	wL.submatrix(1,n_marker,n_marker+1,n_marker+4)=wP;
	wL.submatrix(n_marker+1,n_marker+4,1,n_marker)=wP.t();
	wL.submatrix(n_marker+1,n_marker+4,n_marker+1,n_marker+4)=0;

	Matrix wY(n_marker+4,3);
	for(V3DLONG i=0;i<n_marker;i++)
	{
	   wY(i+1,1)=vec_ctlpts_tar.at(i).x;
	   wY(i+1,2)=vec_ctlpts_tar.at(i).y;
	   wY(i+1,3)=vec_ctlpts_tar.at(i).z;
	}
	wY.submatrix(n_marker+1,n_marker+4,1,3)=0;

	Try
	{
//	    wa=wL.i()*wY;

//		wa=wL.i();
		wa=q_pseudoinverse(wL);

		wa=wa*wY;
	}
	CatchAll
	{
		printf("ERROR: q_compute_tps_paras_3d: Fail to find the (pseudo)inverse of the wL matrix, maybe too sigular.\n");
		return false;
	}

	return true;
}

//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_3D(
		const point3D64F &pt_sub,const vector <point3D64F> &vec_ctlpts_sub,const Matrix &wa_sub2tar,
		point3D64F &pt_sub2tar)
{
	//check parameters
	if(vec_ctlpts_sub.size()<3)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_3D: Are you sure the input control points are right?.\n");
		return false;
	}

	//allocate memory
    double *Ua=new double[vec_ctlpts_sub.size()+4]();
	if(!Ua)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_3D: Fail to allocate memory dist for tps warping.\n");
		return false;
	}

	int n_marker=vec_ctlpts_sub.size();
	double tmp,s;

	//calculate distance vector
	for(V3DLONG n=0;n<n_marker;n++)
	{
		s=0;
		tmp=pt_sub.x-vec_ctlpts_sub.at(n).x;	s+=tmp*tmp;
		tmp=pt_sub.y-vec_ctlpts_sub.at(n).y;	s+=tmp*tmp;
		tmp=pt_sub.z-vec_ctlpts_sub.at(n).z;	s+=tmp*tmp;
		Ua[n]=s*log(s+1e-20);
	}

	Ua[n_marker]  =1;
	Ua[n_marker+1]=pt_sub.x;
	Ua[n_marker+2]=pt_sub.y;
	Ua[n_marker+3]=pt_sub.z;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,1);
	pt_sub2tar.x=s;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,2);
	pt_sub2tar.y=s;

	s=0;
	for(V3DLONG p=0;p<n_marker+4;p++)	s+=Ua[p]*wa_sub2tar(p+1,3);
	pt_sub2tar.z=s;

	//free memory
	if(Ua) 	{delete []Ua;	Ua=0;}

	return true;
}

