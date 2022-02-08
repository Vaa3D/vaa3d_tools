// q_warp_stps.cpp
// warp pointset and image based on given matched pairs
// by Lei Qu
// 2010-03-22

#include <QtGui>

#include "q_warp_stps.h"
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "basic_surf_objs.h"

bool q_stps_cd(const vector<Coord3D_PCM> &vec_sub, const vector<Coord3D_PCM> &vec_tar,  const double d_lamda,
	Matrix &x4x4_d, Matrix &xnx4_c, Matrix &xnxn_K)
{
	//check parameters
	if (vec_sub.size()<4 || vec_sub.size() != vec_tar.size())
	{
		printf("ERROR: Invalid input parameters! \n");
		return false;
	}
	V3DLONG n_pts = vec_sub.size();
	
	if (xnx4_c.nrows() != n_pts || xnx4_c.ncols() != 4)
		xnx4_c.ReSize(n_pts, 4);
	if (x4x4_d.nrows() != 4 || xnx4_c.ncols() != 4)
		x4x4_d.ReSize(4, 4);
	if (xnxn_K.nrows() != n_pts || xnxn_K.ncols() != n_pts)
		xnxn_K.ReSize(n_pts, n_pts);

	//generate TPS kernel matrix K=-r=-|xi-xj|
	if (!q_stps_k(vec_sub, vec_sub, xnxn_K))
	{
		printf("ERROR: q_TPS_k() return false! \n");
		return false;
	}

	//------------------------------------------------------------------
	//compute the QR decomposition of x
	Matrix X(n_pts, 4), Y(n_pts, 4);
	Matrix Q(n_pts, n_pts); Q = 0.0;
	for (V3DLONG i = 0; i<n_pts; i++)
	{
		Q(i + 1, 1) = X(i + 1, 1) = 1;
		Q(i + 1, 2) = X(i + 1, 2) = vec_sub[i].x;
		Q(i + 1, 3) = X(i + 1, 3) = vec_sub[i].y;
		Q(i + 1, 4) = X(i + 1, 4) = vec_sub[i].z;

		Y(i + 1, 1) = 1;
		Y(i + 1, 2) = vec_tar[i].x;
		Y(i + 1, 3) = vec_tar[i].y;
		Y(i + 1, 4) = vec_tar[i].z;
	}

	UpperTriangularMatrix R;
	QRZ(Q, R);
	extend_orthonormal(Q, 4);//otherwise q2=0

	Matrix q1 = Q.columns(1, 4);
	Matrix q2 = Q.columns(5, n_pts);
	Matrix r = R.submatrix(1, 4, 1, 4);

	//------------------------------------------------------------------
	//compute non-affine term c which decomposed from TPS
	Matrix A = q2.t()*xnxn_K*q2 + IdentityMatrix(n_pts - 4)*d_lamda;
	xnx4_c = q2*(A.i()*q2.t()*Y);
	//compute affine term d (normal)
	x4x4_d = r.i()*q1.t()*(Y - xnxn_K*xnx4_c);
	//	//compute affine term d (regulized)
	//	//small lamda2 can not avoid flip, big lamba can will prevent rotation(not good!)
	//	double d_lamda2=0.01;
	//	Matrix xRtR=r.t()*r;
	//	x4x4_d=((xRtR+d_lamda2*IdentityMatrix(4)).i()) * (xRtR*x4x4_d-xRtR) + IdentityMatrix(4);

	//------------------------------------------------------------------------------------------------------------------------------------
	//suppress affine reflection and 90 degree rotate
	A.resize(3, 3);
	A = x4x4_d.submatrix(2, 4, 2, 4).t();

	DiagonalMatrix D(3);
	Matrix U(3, 3), V(3, 3);
	try
	{
		SVD(A, D, U, V);	//A = U * D * V.t()
	}
	catch (BaseException)
	{
		cout << BaseException::what() << endl;
		printf("ERROR: SVD() exception!\n");
		return false;
	}
	Matrix UV = U*V.t();
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
	if (A.determinant()<-1e-10)
	{

		//detect upside-down flip
		bool b_flipdetected = 0;
		if (UV(2, 2)<0)
		{
			printf("WARNING: ||||||||||||||||||||||||||||||||||upside down flip detected||||||||||||||||||||||||||||||||||||\n\n");
			b_flipdetected = 1;
		}
		//detect left-right flip
		if (UV(3, 3)<0)
		{
			printf("WARNING: -----------------------------------left right flip detected------------------------------------\n\n");
			b_flipdetected = 1;
		}
		//		if(!b_flipdetected)
		//		{
		//			v3d_msg("miss detect flip\n");
		//		}

		//surpress reflection
		D(3, 3) *= -1;

		A = U*D*V.t();
		x4x4_d.submatrix(2, 4, 2, 4) = A.t();
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
bool q_stps_k(const vector<Coord3D_PCM> &vec_sub, const vector<Coord3D_PCM> &vec_basis, Matrix &xmxn_K)
{
	//check parameters
	if (vec_sub.empty() || vec_basis.empty())
	{
		printf("ERROR: q_TPS_k: The input subject or basis marker vector is empty.\n");
		return false;
	}

	xmxn_K.resize(vec_sub.size(), vec_basis.size());

	//compute K=-r=-|xi-xj|
	double d_x, d_y, d_z;
	for (unsigned V3DLONG i = 0; i<vec_sub.size(); i++)
		for (unsigned V3DLONG j = 0; j<vec_basis.size(); j++)
		{
			d_x = vec_sub[i].x - vec_basis[j].x;
			d_y = vec_sub[i].y - vec_basis[j].y;
			d_z = vec_sub[i].z - vec_basis[j].z;
			xmxn_K(i + 1, j + 1) = -sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
		}

	return true;
}

bool q_compute_ptwarped_from_stps_3D(
	const Coord3D_PCM &pt_sub, const vector <Coord3D_PCM> &vec_ctlpt_sub, Matrix &x4x4_d, Matrix &xnx4_c,
	Coord3D_PCM &pt_sub2tar)
{
	//check parameters
	if (vec_ctlpt_sub.size()<3)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_3D: Are you sure the input control points are right?.\n");
		return false;
	}

	int n_marker = vec_ctlpt_sub.size();
	double tx, ty, tz, x, y, z;
	Matrix x_pt(1, 4);
	//calculate distance vector
	//for (long n = 0; n<n_marker; n++)
	//{
	//	x = 0, y = 0, z = 0;
	//	tx = pt_sub.x - vec_ctlpt_sub.at(n).x;	x += tx*tx;
	//	ty = pt_sub.y - vec_ctlpt_sub.at(n).y;	y += ty*ty;
	//	tz = pt_sub.z - vec_ctlpt_sub.at(n).z;	z += tz*tz;
	//}
	x_pt(1, 1) = 1.0;
	x_pt(1, 2) = pt_sub.x;
	x_pt(1, 3) = pt_sub.y;
	x_pt(1, 4) = pt_sub.z;

	Matrix xmxn_K;
	xmxn_K.resize(1, vec_ctlpt_sub.size());

	//compute K=-r=-|xi-xj|
	double d_x, d_y, d_z;
	for (unsigned V3DLONG j = 0; j<vec_ctlpt_sub.size(); j++)
		{
			d_x = pt_sub.x - vec_ctlpt_sub[j].x;
			d_y = pt_sub.y - vec_ctlpt_sub[j].y;
			d_z = pt_sub.z - vec_ctlpt_sub[j].z;
			xmxn_K(1, j + 1) = -sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
		}

	Matrix x_stps(1, 4);
	
	x_stps = x_pt*x4x4_d + xmxn_K*xnx4_c;
	pt_sub2tar.x = x_stps(1, 2);
	pt_sub2tar.y = x_stps(1, 3);
	pt_sub2tar.z = x_stps(1, 4);

	return true;
}



//STPS image warp
bool q_imagewarp_stps(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	Matrix &x4x4_affine, Matrix &xnx4_c, Matrix &xnxn_K,
	unsigned char *&p_img_stps)
{
	//check parameters
	if (vec_ctlpt_tar.size() == 0 || vec_ctlpt_sub.size() == 0 || vec_ctlpt_tar.size() != vec_ctlpt_sub.size())
	{
		printf("ERROR: target or subject control points is invalid!\n");
		return false;
	}
	if (p_img_sub == 0 || sz_img_sub == 0)
	{
		printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
		return false;
	}
	if (p_img_stps)
	{
		printf("WARNNING: output image pointer is not null, original memeroy it point to will lost!\n");
		p_img_stps = 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//assign output/warp image size
	long long sz_img_output[4] = { 0 };
	if (sz_img_output[0] == 0)		sz_img_output[0] = sz_img_stps[0];
	if (sz_img_output[1] == 0)		sz_img_output[1] = sz_img_stps[1];
	if (sz_img_output[2] == 0)		sz_img_output[2] = sz_img_stps[2];
	sz_img_output[3] = sz_img_sub[3];

	//allocate memory
	p_img_stps = new unsigned char[sz_img_output[0] * sz_img_output[1] * sz_img_output[2] * sz_img_output[3]]();
	if (!p_img_stps)
	{
		printf("ERROR: Fail to allocate memory for p_img_sub2tar.\n");
		return false;
	}
	unsigned char ****p_img_sub_4d = 0, ****p_img_sub2tar_4d = 0;
	if (!new4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3], p_img_sub) ||
		!new4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3], p_img_stps))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img_stps) 			{ delete[]p_img_stps;		p_img_stps = 0; }
		if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
		if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//compute sub2tar tps warp parameters
	if (!q_stps_cd(vec_ctlpt_tar, vec_ctlpt_sub, 0.2, x4x4_affine, xnx4_c, xnxn_K))
	{
		printf("ERROR: q_compute_tps_paras_3D() return false.\n");
		if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
		if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }
		if (p_img_stps) 			{ delete[]p_img_stps;		p_img_stps = 0; }
		return false;
	}

#pragma omp parallel for
	for (long long x = 0; x < sz_img_output[0]; x++)
	{
		printf("\tSTPS image warp: [%d/%d]\n", sz_img_output[0], x);
		for (long long y = 0; y < sz_img_output[1]; y++)
			for (long long z = 0; z < sz_img_output[2]; z++)
			{
				Coord3D_PCM pt_tar, pt_tar2sub;

				pt_tar.x = x;
				pt_tar.y = y;
				pt_tar.z = z;

				if (!q_compute_ptwarped_from_stps_3D(pt_tar, vec_ctlpt_tar, x4x4_affine, xnx4_c, pt_tar2sub))
				{
					printf("ERROR: q_compute_ptwarped_from_stps_3D() return false.\n");
					if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
					if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }
					if (p_img_stps) 			{ delete[]p_img_stps;		p_img_stps = 0; }
					//return false;
				}

				//------------------------------------------------------------------
				//linear interpolate
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0] = pt_tar2sub.x;
				cur_pos[1] = pt_tar2sub.y;
				cur_pos[2] = pt_tar2sub.z;

				//if interpolate pixel is out of subject image region, set to -inf
				if (cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0] - 1 ||
					cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1] - 1 ||
					cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2] - 1)
				{
					p_img_sub2tar_4d[0][z][y][x] = 0.0;
					continue;
				}

				//find 8 neighbor pixels boundary
				long long x_s, x_b, y_s, y_b, z_s, z_b;
				x_s = floor(cur_pos[0]);		x_b = ceil(cur_pos[0]);
				y_s = floor(cur_pos[1]);		y_b = ceil(cur_pos[1]);
				z_s = floor(cur_pos[2]);		z_b = ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w, r_w, t_w, b_w;
				l_w = 1.0 - (cur_pos[0] - x_s);	r_w = 1.0 - l_w;
				t_w = 1.0 - (cur_pos[1] - y_s);	b_w = 1.0 - t_w;
				//compute weight for higher slice and lower slice
				double u_w, d_w;
				u_w = 1.0 - (cur_pos[2] - z_s);	d_w = 1.0 - u_w;

				//linear interpolate each channel
				for (V3DLONG c = 0; c < sz_img_output[3]; c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice = t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s] + r_w*p_img_sub_4d[c][z_s][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s] + r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice = t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s] + r_w*p_img_sub_4d[c][z_b][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s] + r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					p_img_sub2tar_4d[c][z][y][x] = u_w*higher_slice + d_w*lower_slice;
				}

			}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. free memory. \n");
	if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
	if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }

	return true;
}

bool q_imagewarp_stps(const vector<Coord3D_PCM> &vec_ctlpt_tar, const vector<Coord3D_PCM>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	unsigned char *&p_img_stps)
{
	Matrix x4x4_affine, xnx4_c, xnxn_K;
	if (!q_imagewarp_stps(vec_ctlpt_tar, vec_ctlpt_sub,
		p_img_sub, sz_img_sub, sz_img_stps,
		x4x4_affine, xnx4_c, xnxn_K,
		p_img_stps))
	{
		printf("ERROR: q_imagewarp_stps() return false.\n");
		return false;
	}

	return true;
}
