

#include "mBrainAligner.h"
#include "calHogFeature.h"
#include "preprocessing.h"
#include "landmarker_deal.h"
#include "until.h"

//linear interpolate the SubDFBlock to DFBlock
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming


//same as hanchuan's jab method (assume invalid pixel intensity<0)

bool q_mutualinformation2(float *&I1, float *&I2, long long npixels, float &nmi)
{
	int nbin = 16;

	//histogram and joint histogram
	float *hist1 = new(std::nothrow) float[nbin]();
	float *hist2 = new(std::nothrow) float[nbin]();
	float *hist12 = new(std::nothrow) float[nbin*nbin]();
	int pos1, pos2;
	for (long long i = 0; i < npixels; i++)
	{
		pos1 = int(floor(I1[i] * nbin)); if (pos1 < 0) pos1 = 0; if (pos1 >= nbin) pos1 = nbin - 1;
		pos2 = int(floor(I2[i] * nbin)); if (pos2 < 0) pos2 = 0; if (pos2 >= nbin) pos2 = nbin - 1;

		hist1[pos1]++;
		hist2[pos2]++;
		hist12[pos1 + nbin*pos2]++;
	}
	//normalize histogram and joint histogram
	for (long long i = 0; i < nbin; i++) hist1[i] /= npixels;
	for (long long i = 0; i < nbin; i++) hist2[i] /= npixels;
	for (long long i = 0; i < nbin*nbin; i++) hist12[i] /= npixels*npixels;

	//entropy and joint entropy
	double HA = .0, HB = .0, HAB = .0;
	for (long long i = 0; i < nbin; i++)
		if (hist1[i] > 10e-10)	HA += -hist1[i] * log(hist1[i]);
	for (long long i = 0; i < nbin; i++)
		if (hist2[i] > 10e-10)	HB += -hist2[i] * log(hist2[i]);
	for (long long i = 0; i < nbin*nbin; i++)
		if (hist12[i] > 10e-10)	HAB += -hist12[i] * log(hist12[i]);

	//mutual information
	nmi = (HA + HB) / HAB;//NMI(Normalization Mutual Information),越大越匹配
	double mi = HA + HB - HAB;//MI(Mutual Information)互信息越大，说明相互包含的信息多，即越匹配
	double ecc = 2 * mi / (HA + HB);//ECC(Entropy Corrleation Coefficient)，越大越匹配
	nmi = ecc;

	if (hist1) 	{ delete[]hist1;	hist1 = 0; }
	if (hist2) 	{ delete[]hist2;	hist2 = 0; }
	if (hist12) { delete[]hist12;	hist12 = 0; }
	return true;
}
bool q_mutualinformation3(float *&I1, float *&I2, int kernel_radius, float &nmi)
{
#define NBIN 25
	double P_ST[NBIN][NBIN], P_S[NBIN], P_T[NBIN];
	for (int j = 0; j < NBIN; j++)
	{
		P_S[j] = P_T[j] = 0;
		for (int i = 0; i < NBIN; i++)
			P_ST[j][i] = 0;
	}

	int kernel_width = 2 * kernel_radius + 1;
	double cnt = 0;
	for (int k = -kernel_radius; k <= kernel_radius; k++)
		for (int j = -kernel_radius; j <= kernel_radius; j++)
			for (int i = -kernel_radius; i <= kernel_radius; i++)
			{
				double tdis = k*k + j*j + i*i;
				if (tdis > kernel_radius*kernel_radius) continue;

				long long index = kernel_width*kernel_width*(k + kernel_radius) + kernel_width*(j + kernel_radius) + (i + kernel_radius);
				if (I1[index] < 0 || I2[index] < 0) continue;//assume out boundary tar and sub patch value are <0

				double tdiswei = exp(-tdis / (kernel_radius*kernel_radius) * 3);

				double cur_t = I1[index];
				double cur_s = I2[index];
				double tmp_s1 = tdiswei* cur_t, tmp_s2 = tdiswei* cur_s;

				int ind_t = int(ceil(cur_t*NBIN)); if (ind_t < 0) ind_t = 0; if (ind_t >= NBIN) ind_t = NBIN - 1;
				int ind_s = int(ceil(cur_s*NBIN)); if (ind_s < 0) ind_s = 0; if (ind_s >= NBIN) ind_s = NBIN - 1;
				P_ST[ind_t][ind_s] += tdiswei;
				P_S[ind_s] += tdiswei;
				P_T[ind_t] += tdiswei;
				cnt += tdiswei;
			}
	if (cnt == 0)
	{
		nmi = 0;
		return true;
	}
	double s_mi = 0, h_T = 0, h_S = 0;
	for (int j = 0; j < NBIN; j++)
	{
		P_T[j] /= cnt;	P_S[j] /= cnt;
		for (int i = 0; i < NBIN; i++)	P_ST[j][i] /= cnt;
	}

	for (int j = 0; j < NBIN; j++)
	{
		if (P_T[j]) h_T -= P_T[j] * log(P_T[j]);
		if (P_S[j]) h_S -= P_S[j] * log(P_S[j]);

		if (P_T[j] <= 0) continue;
		for (int i = 0; i < NBIN; i++)
		{
			if (P_S[i] <= 0) continue;
			if (P_ST[j][i] <= 0) continue;
			s_mi += double(P_ST[j][i])*log(double(P_ST[j][i]) / (double(P_T[j])*P_S[i]));
		}
	}
	if ((h_T + h_S) == 0.0)
		nmi = 0;
	else
		nmi = s_mi*2.0 / (h_T + h_S);

	return true;
}

//normalized Cross Correlation [0,1] (assume invalid pixel intensity<0)
bool q_ncc(float *&I1, float *&I2, long long npixels, float &ncc)
{
	long long npixel_valid = 0;
	float mean1 = 0.0, mean2 = 0.0;
	for (long long m = 0; m < npixels; m++)
	{
		if (I1[m] < 0 || I2[m] < 0) continue;//since invalid pixels are initialized as -1
		npixel_valid++;
		if (isnan(I1[m]) || isnan(I2[m]))
			cout << I1[m] << " " << I2[m] << endl;
		mean1 += (float)I1[m];
		mean2 += (float)I2[m];
	}

	if (npixel_valid == 0 || mean1 == 0 || mean2 == 0)
	{
		ncc = 0;
		return true;
	}
	mean1 = mean1 / npixel_valid;
	mean2 = mean2 / npixel_valid;
	float sigma1 = 0.0, sigma2 = 0.0, cc = 0.0;
	for (long long m = 0; m < npixels; m++)
	{
		if (I1[m] < 0 || I2[m] < 0) continue;//since invalid pixels are initialized as -1
		float diff1, diff2;
		diff1 = I1[m] - mean1;	sigma1 += diff1 * diff1;
		diff2 = I2[m] - mean2;	sigma2 += diff2 * diff2;
		cc += diff1*diff2;
	}
	sigma1 = sqrt(sigma1 / npixel_valid);
	sigma2 = sqrt(sigma2 / npixel_valid);
	cc = cc / npixel_valid;


	if (sigma1 == 0 & sigma2 == 0)
	{
		ncc = 1;
		return true;
	}


	if (sigma1 == 0 || sigma2 == 0)
		ncc = 0;
	else
		ncc = cc / (sigma1 * sigma2);//[-1,1]

	//normalized from [-1,1] to [0,1]
	//ncc = (ncc + 1) / 2;
	if (ncc < 0) ncc = -ncc;

	if (isnan(ncc))
		cout << "sigma1" << sigma1 << "sigma2" << sigma2 << "cc" << cc << endl;

	return true;
}

//mean square diffence error [0,1] (assume invalid pixel intensity<0)
bool q_mse(float *&I1, float *&I2, long long npixels, float &mse)
{
	long long npixel_valid = 0;
	mse = 0.0;
	float mean1 = 0.0, mean2 = 0.0;
	for (long long m = 0; m < npixels; m++)
	{
		if (I1[m] < 0 || I2[m] < 0) continue;//since invalid pixels are initialized as -1
		npixel_valid++;
		mean1 += (float)I1[m];
		mean2 += (float)I2[m];
	}
	mean1 = mean1 / npixel_valid;
	mean2 = mean2 / npixel_valid;
	for (long long m = 0; m < npixels; m++)
	{
		if (I1[m] < 0 || I2[m] < 0) continue;//since invalid pixels are initialized as -1
		float diff = (I1[m] - mean1) - (I2[m] - mean2);
		mse += diff*diff;
	}
	//make it the bigger the better
	if (npixel_valid == 0 || mean1 == 0 || mean2==0)
		mse = 0;
	else
		mse = exp(-sqrt(mse / npixel_valid) / 10);

	if (isnan(mse))
		cout << mse << endl;

	return true;
}




bool q_TPS_cd(const vector<point3D64F> &vec_sub, const vector<point3D64F> &vec_tar, const double d_lamda,
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
	if (!q_TPS_k(vec_sub, vec_sub, xnxn_K))
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

	return true;
}

//generate TPS kernel matrix
bool q_TPS_k(const vector<point3D64F> &vec_sub, const vector<point3D64F> &vec_basis, Matrix &xmxn_K)
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

bool q_imagewarp_stps(const vector<point3D64F> &vec_ctlpt_tar, const vector<point3D64F>  &vec_ctlpt_sub,
	const unsigned char *p_img_sub, const long long *sz_img_sub, const long long *sz_img_stps,
	unsigned char *p_img_stps)
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
	Matrix x4x4_affine, xnx4_c, xnxn_K;
	if (!q_TPS_cd(vec_ctlpt_tar, vec_ctlpt_sub, 0.5, x4x4_affine, xnx4_c, xnxn_K))
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
		printf("STPS: [%d/%d]\n", sz_img_output[0], x);
		for (long long y = 0; y < sz_img_output[1]; y++)
			for (long long z = 0; z < sz_img_output[2]; z++)
			{
				point3D64F pt_tar, pt_tar2sub;

				pt_tar.x = x;
				pt_tar.y = y;
				pt_tar.z = z;

				if (!q_compute_ptwarped_from_stps_3D(pt_tar, vec_ctlpt_tar, x4x4_affine, xnx4_c, pt_tar2sub))
				{
					printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
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

bool q_compute_ptwarped_from_stps_3D(
	const point3D64F &pt_sub, const vector <point3D64F> &vec_ctlpt_sub, Matrix &x4x4_d, Matrix &xnx4_c,
	point3D64F &pt_sub2tar)
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

bool get_tar_patch(int Select_modal, vector<float *> &vec_pkernels, vector<float *> &vec_pkernels_Hog, vector<float *> &vec_pkernels_map, vector<float *> &vec_pkernels_Hog_map,
	vector<point3D64F>& vec_corners, long long & npixels_kernel, int & kernel_radius, int & kernel_width, long long * & sz_img_tar, float **** &p_img_label,
	 float **** & p_img32f_tar_4d, long long &patch_size_all)

{
	for (int i = 0; i < vec_corners.size(); i++)
	{
		vec_pkernels[i] = new(std::nothrow) float[npixels_kernel]();
		
		    vec_pkernels_map[i] = new(std::nothrow) float[npixels_kernel]();
	}

	for (int i = 0; i < vec_corners.size(); i++)
	{
		float  *p_kernel = vec_pkernels[i];
		float  *p_kernel_map;
		
		p_kernel_map = vec_pkernels_map[i];
		long long ind_kernel = 0;
		vector<neighborinfo> patch_label;
		float xx, yy, zz, dis, label;

		for (int zz = -kernel_radius; zz <= kernel_radius; zz++)
			for (int yy = -kernel_radius; yy <= kernel_radius; yy++)
				for (int xx = -kernel_radius; xx <= kernel_radius; xx++)
				{
					int x = vec_corners[i].x + xx;
					int y = vec_corners[i].y + yy;
					int z = vec_corners[i].z + zz;
					/*printf("%d %d %d ", x, y, z);*/
					if (x < 0 || y < 0 || z < 0 || x >= sz_img_tar[0] || y >= sz_img_tar[1] || z >= sz_img_tar[2])
					{
						p_kernel[ind_kernel] = 0;
					
						p_kernel_map[ind_kernel] = 0;//NOTE: set outboundary value  < 0
					}

					else
					{
						if (Select_modal ==0)
						{
							if (vec_corners[i].label == p_img_label[0][z][y][x])
							{
								
								p_kernel_map[ind_kernel] = p_img32f_tar_4d[0][z][y][x];
								p_kernel[ind_kernel] = p_img32f_tar_4d[0][z][y][x];
							}

							else
							{	
								p_kernel_map[ind_kernel] = 0;
											
								if (vec_corners[i].outline == 1)
									p_kernel[ind_kernel] = 0;
								else
									p_kernel[ind_kernel] = p_img32f_tar_4d[0][z][y][x];
								
							}
						}
						else
						{
							p_kernel[ind_kernel] = p_img32f_tar_4d[0][z][y][x];
							p_kernel_map[ind_kernel] = p_img32f_tar_4d[0][z][y][x];
						}
							
					}

					ind_kernel++;
				}
		
		vector<float> HogFeature, map_HogFeature;

		int cell_size = floor(kernel_width / 3.0);

		calHogFeature(p_kernel, kernel_width, cell_size, 9, 2, HogFeature);
		vec_pkernels_Hog[i] = new(std::nothrow) float[HogFeature.size()]();

		calHogFeature(p_kernel_map, kernel_width, cell_size, 9, 2, map_HogFeature);
		vec_pkernels_Hog_map[i] = new(std::nothrow) float[map_HogFeature.size()]();
		
		for (int index_i = 0; index_i < HogFeature.size(); index_i++)
		{
			vec_pkernels_Hog[i][index_i] = HogFeature[index_i];
			
				vec_pkernels_Hog_map[i][index_i] = map_HogFeature[index_i];
		}
		patch_size_all = HogFeature.size();
	}
	return true;
}

bool Compute_source_fMOST(float * & p_kernel_map, float *p_kernel, int & kernel_radius, long long *& sz_img_tar, vector<point3D64F>& vec_corners, vector<point3D64F> & aver_corner,
	vector<point3D64F> &or_sub, float **** & p_img32f_sub_4d, float **** & p_img_label, int  ind_landmark, bool select,
	long long  npixels_kernel, long long  index_fetmap, long long  index_source, int X, int  Y, int  Z, vector<float *> & Source, vector< float * > & vec2d_pfeatmaps,
	float  *&p_kernel_Hog_map, int  kernel_width, float  *& p_kernel_Hog, long long & patch_size_all,
	vector<point3D64F> &vec_corners_last, map <int, float ****> density_map_sub_4d, double outline)
{
	float  *p_patch = new(std::nothrow) float[kernel_width * kernel_width * kernel_width]();
	float  *p_patch_H = new(std::nothrow) float[patch_size_all]();
	float  *p_patch_map = new(std::nothrow) float[kernel_width * kernel_width * kernel_width]();
	float  *p_patch_H_map = new(std::nothrow) float[patch_size_all]();
	long long index_patch = 0;
	for (int zz = -kernel_radius; zz <= kernel_radius; zz++)
		for (int yy = -kernel_radius; yy <= kernel_radius; yy++)
			for (int xx = -kernel_radius; xx <= kernel_radius; xx++)
			{
				int x = X + xx;
				int y = Y + yy;
				int z = Z + zz;
				if (x < 0 || y < 0 || z < 0 || x >= sz_img_tar[0] || y >= sz_img_tar[1] || z >= sz_img_tar[2])
				{
					p_patch_map[index_patch] = 0;
					p_patch[index_patch] = 0;//note: set outboundary value  < 0
				}

				else
				{
					float ****map_label = density_map_sub_4d[vec_corners[ind_landmark].label];
					if (map_label[0][z][y][x] < 0.00000001)
						map_label[0][z][y][x] = 0;
					if (outline == 1)
					{
						p_patch[index_patch] = p_img32f_sub_4d[0][z][y][x] * map_label[0][z][y][x];
						
					}		
					else
						p_patch[index_patch] = p_img32f_sub_4d[0][z][y][x];
					p_patch_map[index_patch] = p_img32f_sub_4d[0][z][y][x] * map_label[0][z][y][x];
				}
				index_patch++;
			}

	vector<float> HogFeature_p;
	vector<float> HogFeature_p_map;
	int cell_size = floor(kernel_width / 3.0);
	calHogFeature(p_patch_map, kernel_width, cell_size, 9, 2, HogFeature_p_map);
	calHogFeature(p_patch, kernel_width, cell_size, 9, 2, HogFeature_p);
	for (int index_i = 0; index_i < HogFeature_p.size(); index_i++)
	{
		p_patch_H_map[index_i] = HogFeature_p_map[index_i];
		p_patch_H[index_i] = HogFeature_p[index_i];
	}

	//mean square diffence error [0,1], bigger is better
	float mse_H , mse_map;
	q_mse(p_kernel_Hog, p_patch_H, patch_size_all, mse_H);
	q_mse(p_kernel_Hog_map, p_patch_H_map, patch_size_all, mse_map);

	//normalized Cross Correlation [0,1], bigger is better
	float ncc_H , ncc_map ;
	q_ncc(p_kernel_Hog, p_patch_H, patch_size_all, ncc_H);
	q_ncc(p_kernel_Hog_map, p_patch_H_map, patch_size_all, ncc_map);

	//normalized mutual information [0,1], bigger is better
	float nmi_G, nmi_map;
	q_mutualinformation3(p_kernel, p_patch, kernel_radius, nmi_G);
	q_mutualinformation3(p_kernel_map, p_patch_map, kernel_radius, nmi_map);


	vec2d_pfeatmaps[ind_landmark][index_fetmap] = mse_map*ncc_map*nmi_map*nmi_G* mse_H*ncc_H;
	if (select)
	{
		Source[ind_landmark][index_source] = mse_map*ncc_map*nmi_map*nmi_G* mse_H*ncc_H;
	}

	if (p_patch) 			{ delete[]p_patch;		p_patch = 0; }
	if (p_patch_H) 			{ delete[]p_patch_H;		p_patch_H = 0; }
	if (p_patch_map) 			{ delete[]p_patch_map;		p_patch_map = 0; }
	if (p_patch_H_map) 			{ delete[]p_patch_H_map;		p_patch_H_map = 0; }
	return true;
}
bool Compute_source(float * & p_kernel_map, float *p_kernel, int & kernel_radius, long long *& sz_img_tar, vector<point3D64F>& vec_corners, vector<point3D64F> & aver_corner,
	vector<point3D64F> &or_sub, float **** & p_img32f_sub_4d, float **** & p_img_label, int  ind_landmark, bool select,
	long long  npixels_kernel, long long  index_fetmap, long long  index_source, int  X, int  Y, int  Z, vector<float *> & Source, vector< float * > & vec2d_pfeatmaps, 
	float  *&p_kernel_Hog_map,int & kernel_width, float  *& p_kernel_Hog, long long & patch_size_all, vector<point3D64F> &vec_corners_last)
{
	float  *p_patch = new(std::nothrow) float[kernel_width * kernel_width * kernel_width]();
	float  *p_patch_H = new(std::nothrow) float[patch_size_all]();
	long long index_patch = 0;
	for (int zz = -kernel_radius; zz <= kernel_radius; zz++)
		for (int yy = -kernel_radius; yy <= kernel_radius; yy++)
			for (int xx = -kernel_radius; xx <= kernel_radius; xx++)
			{
				int x = X + xx;
				int y = Y + yy;
				int z = Z + zz;
				if (x < 0 || y < 0 || z < 0 || x >= sz_img_tar[0] || y >= sz_img_tar[1] || z >= sz_img_tar[2])
				{
					p_patch[index_patch] = 0;//note: set outboundary value  < 0
				}

				else
				{
					p_patch[index_patch] = p_img32f_sub_4d[0][z][y][x] ;	
				}
				index_patch++;
			}

	long long sz_img32f_nmi[4] = { kernel_width, kernel_width, kernel_width, 1 };

	vector<float> HogFeature_p;
	int cell_size = floor(kernel_width / 3.0);
	calHogFeature(p_patch, kernel_width, cell_size, 9, 2, HogFeature_p);
	for (int index_i = 0; index_i < HogFeature_p.size(); index_i++)
	{
		p_patch_H[index_i] = HogFeature_p[index_i];
	}

	//mean square diffence error [0,1], bigger is better
	float mse_H = 0.0, mse_G = 0.0;
	q_mse(p_kernel_map, p_patch, patch_size_all, mse_G);
	q_mse(p_kernel_Hog_map, p_patch_H, patch_size_all, mse_H);

	//normalized Cross Correlation [0,1], bigger is better
	float ncc_H = 0.0, ncc_G = 0.0;
	q_ncc(p_kernel_map, p_patch, patch_size_all, ncc_G);
	q_ncc(p_kernel_Hog_map, p_patch_H, patch_size_all, ncc_H);

	//normalized mutual information [0,1], bigger is better
	float nmi_H = 0.0, nmi_G = 0.0;
	q_mutualinformation3(p_kernel_map, p_patch, kernel_radius, nmi_G);
	
	vec2d_pfeatmaps[ind_landmark][index_fetmap] = mse_H*ncc_H*nmi_G/** mse_G*ncc_G*/;
	if (select)
	{
		Source[ind_landmark][index_source] = mse_H*ncc_H*nmi_G/** mse_G*ncc_G*/;
	}

	if (p_patch) 			{ delete[]p_patch;		p_patch = 0; }
	if (p_patch_H) 			{ delete[]p_patch_H;		p_patch_H = 0; }
	return true;
}

bool Dic_brain(unsigned char * p_img_test, QString &dic_brain_file, QString &based_brain, long long * sz_img_test)
{
	float ncc_value = 0;
	QString match_brain, same_brain;
	unsigned char  *p_img_norm = 0;
	long long  *sz_img_norm = 0;
	int datatype_tar = 0, datatype_sub = 0;

	long long l_npixels = sz_img_test[0] * sz_img_test[1] * sz_img_test[2];

	float *test_norm_float;

	test_norm_float = new(std::nothrow) float[l_npixels]();
	for (int i = 0; i < l_npixels; i++)
		test_norm_float[i] = p_img_test[i];

	float  *grand_tar = 0;
	if (!Calculate_gradient_img(grand_tar, sz_img_test, test_norm_float, p_img_test))
	{
		printf("Error tar Calculate_gradient_img() is wrong!");
		return false;
	}

	float d_maxintensity_tar = 0.0, d_maxintensity_sub = 0.0;
	for (long i = 0; i < l_npixels; i++)
	{
		if (grand_tar[i]>d_maxintensity_tar)
			d_maxintensity_tar = grand_tar[i];
	}


	for (long i = 0; i < l_npixels; i++)
	{
		grand_tar[i] = grand_tar[i] / d_maxintensity_tar*255.0;
	}


	QDir dir(dic_brain_file);
	if (!dir.exists()) {
		return -1;
	}

	//取到所有的文件和文件名，但是去掉.和..的文件夹（这是QT默认有的）
	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

	//文件夹优先
	dir.setSorting(QDir::DirsFirst);

	//转化成一个list
	QFileInfoList list_norm = dir.entryInfoList();
	if (list_norm.size()< 1) {
		return -1;
	}
	int J = 0;

	do{
		QFileInfo fileInfo_norm = list_norm.at(J);

		//bool isDll = fileInfo.fileName().endsWith(".dll");
		qDebug() << fileInfo_norm.filePath() << ":" << fileInfo_norm.fileName();

		QString file_norm_path = fileInfo_norm.filePath() + "/" + fileInfo_norm.fileName()+"_outline.v3draw";
		QString marker_file = fileInfo_norm.filePath() + "/sub.marker";
		qDebug() << file_norm_path << " " << marker_file << endl;

		QFile Fout(marker_file);
		QFile Fout_v(file_norm_path);

		if (Fout.exists() && Fout_v.exists())
		{
			unsigned char *p_img_test = 0, *p_img_norm = 0;
			long long *sz_img_test = 0, *sz_img_norm = 0;
			int datatype_tar = 0, datatype_sub = 0;
			if (!file_norm_path.isNull())
			{
				if (!loadImage((char *)qPrintable(file_norm_path), p_img_norm, sz_img_norm, datatype_tar))
				{
					printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(file_norm_path));
					return false;
				}
				printf("\t>>read image file [%s] complete.\n", qPrintable(file_norm_path));
				printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_norm[0], sz_img_norm[1], sz_img_norm[2], sz_img_norm[3]);
				printf("\t\tdatatype: %d\n", datatype_tar);
				if (datatype_tar != 1)
				{
					printf("ERROR: Input image datatype is not UINT8.\n");
					return false;
				}
			}

			float *norm_float;

			norm_float = new(std::nothrow) float[l_npixels]();
			for (int i = 0; i < l_npixels; i++)
				norm_float[i] = p_img_norm[i];

			float  *grand_sub = 0;
			if (!Calculate_gradient_img(grand_sub, sz_img_norm, norm_float, p_img_norm))
			{
				printf("Error tar Calculate_gradient_img() is wrong!");
				return false;
			}

			float  d_maxintensity_sub = 0.0;
			for (long i = 0; i < l_npixels; i++)
			{
				if (grand_sub[i]>d_maxintensity_sub)
					d_maxintensity_sub = grand_sub[i];
			}


			for (long i = 0; i < l_npixels; i++)
			{
				grand_sub[i] = grand_sub[i] / d_maxintensity_sub*255.0;
			}


			float ncc = 0.0;
			q_ncc(grand_tar, grand_sub, l_npixels, ncc);
			float mse = 0.0;
			q_mse(grand_tar, grand_sub, l_npixels, mse);
			if (ncc*mse > ncc_value && ncc*mse<1)
			{
				match_brain = fileInfo_norm.fileName();
				ncc_value = ncc*mse;
			}
			else if (ncc*mse == 1)
			{
				same_brain = fileInfo_norm.fileName();
			}

			cout << ncc << " " << mse << endl;
			if (norm_float) 		{ delete[]norm_float;			norm_float = 0; }
			if (grand_sub) 		{ delete[]grand_sub;			grand_sub = 0; }
			if (p_img_norm) 		{ delete[]p_img_norm;			p_img_norm = 0; }
		}	
		J++;
	} while (J < list_norm.size());
	if (match_brain.isNull())
		match_brain = same_brain;
	based_brain = match_brain.section(".", 0, 0);
}

bool downsapmle_3Dmarker(vector<point3D64F> & out_marker, vector<point3D64F> input_marker, int resample)
{
	double *dfactor;
	dfactor = new double[3];

	for (int i = 0; i < 3; i++)
	{
		dfactor[i] = resample;
	}

	out_marker.clear();
	out_marker = input_marker;
	for (int i = 0; i < input_marker.size(); i++)
	{
		out_marker[i].x = double(input_marker[i].x) / double(dfactor[0]);
		out_marker[i].y = double(input_marker[i].y) / double(dfactor[1]);
		out_marker[i].z = double(input_marker[i].z) / double(dfactor[2]);
	}
	return true;
}


bool mul_scale_mBrainAligner(Parameter input_Parameter, vector<point3D64F>vec_corners, vector<point3D64F> fine_sub_corner, 
	vector<point3D64F> aver_corner, vector<int> label, long long * sz_img , float * p_img32f_tar, 
	float * p_img32f_sub_bk, float * p_img32_sub_label, unsigned char * p_img_sub, map <int, float *> & density_map_sub)
{
	if (input_Parameter.resample != 1)
	{
		cout << "Downsample registration:" << input_Parameter.resample << endl;
		long long * sz_img_tar_resample, *sz_img_sub_resample;
		sz_img_tar_resample = new long long[4];
		sz_img_sub_resample = new long long[4];
		float * p_img32f_tar_resample = 0, *p_img32f_sub_bk_resample = 0, *p_img32_sub_label_resample = 0;
		unsigned char * p_img_sub_resample;
		map <int, float *>  density_map_sub_resample;
		vector<point3D64F>vec_corners_resample, fine_sub_corner_resample, aver_corner_resample;

		unsigned char tag = 0;
		downsapmle_3Dmarker(vec_corners_resample, vec_corners, input_Parameter.resample);
		downsapmle_3Dmarker(fine_sub_corner_resample, fine_sub_corner, input_Parameter.resample);
		downsapmle_3Dmarker(aver_corner_resample, aver_corner, input_Parameter.resample);
		downsample3dvol(p_img32f_tar_resample, p_img32f_tar, sz_img, sz_img_tar_resample, input_Parameter.resample, tag);
		downsample3dvol(p_img32f_sub_bk_resample, p_img32f_sub_bk, sz_img, sz_img_sub_resample, input_Parameter.resample, tag);
		downsample3dvol(p_img_sub_resample, p_img_sub, sz_img, sz_img_sub_resample, input_Parameter.resample, tag);

		tag = 1;
		if (input_Parameter.Select_modal < 2)
		{
			downsample3dvol(p_img32_sub_label_resample, p_img32_sub_label, sz_img, sz_img_sub_resample, input_Parameter.resample, tag);
		}
		
		float * fmost_label_edge_resample = 0;
		int array[10] = { 62, 75, 80, 100, 145, 159, 168, 249 };
		if (input_Parameter.Select_modal == 0)
		{
			for (int i = 0; i<8; i++)
			{
				float * map_resample = 0;
				downsample3dvol(map_resample, density_map_sub[array[i]], sz_img, sz_img_sub_resample, input_Parameter.resample, tag);
				density_map_sub_resample.insert(pair<int, float*>(array[i], map_resample));
			}
			
		}
		
		QList<ImageMarker> ql_marker_tar, ql_marker_sub, ql_marker_aver;
		for (long long i = 0; i < vec_corners.size(); i++)
		{

			ImageMarker tmp;
			tmp.x = vec_corners_resample[i].x;	tmp.y = vec_corners_resample[i].y;	tmp.z = vec_corners_resample[i].z;
			tmp.radius = 2, tmp.shape = 1;ql_marker_tar.push_back(tmp);
			tmp.x = fine_sub_corner_resample[i].x;	tmp.y = fine_sub_corner_resample[i].y;	tmp.z = fine_sub_corner_resample[i].z; 
			tmp.radius = 2, tmp.shape = 1;ql_marker_sub.push_back(tmp);
			tmp.x = aver_corner_resample[i].x;	tmp.y = aver_corner_resample[i].y;	tmp.z = aver_corner_resample[i].z;
			tmp.radius = 2, tmp.shape = 1;ql_marker_aver.push_back(tmp);

		}
		
		if (mBrainAligner(input_Parameter, vec_corners_resample, fine_sub_corner_resample, aver_corner_resample, label, sz_img_tar_resample, 
			p_img32f_tar_resample, p_img32f_sub_bk_resample, p_img32_sub_label_resample, p_img_sub_resample, density_map_sub_resample))
		{
			fine_sub_corner = fine_sub_corner_resample;
		}
		else
		{
			printf("Error dowmsample registration is wrong!!!!");
			return false;
		}

		printf("Dowmsample registration is complete and free memory. \n");
		if (p_img32f_sub_bk_resample) 		{ delete[]p_img32f_sub_bk_resample;		p_img32f_sub_bk_resample = 0; }
		if (p_img32f_tar_resample) 			{ delete[]p_img32f_tar_resample;			p_img32f_tar_resample = 0; }
		if (p_img_sub_resample) 			{ delete[]p_img_sub_resample;			p_img_sub_resample = 0; }
		if (p_img32_sub_label_resample) 		{ delete[]p_img32_sub_label_resample;		p_img32_sub_label_resample = 0; }
		if (fmost_label_edge_resample) 		{ delete[]fmost_label_edge_resample;		fmost_label_edge_resample = 0; }
		if (input_Parameter.Select_modal == 0)
		{
			for (int i = 0; i < 9; i++)
			{
				if (density_map_sub_resample[label[i]]) 		{ delete[]density_map_sub_resample[label[i]];		density_map_sub_resample[label[i]] = 0; }
			}
		}
		input_Parameter.star_iter = 1;
	}	

	for (int i = 0; i < 3; i++)
	{
		input_Parameter.resample = 1;
	}
	input_Parameter.save_path = input_Parameter.save_path + "ori_";
	printf("<< Raw sacle registration ... \n");

	if (!mBrainAligner(input_Parameter, vec_corners, fine_sub_corner, aver_corner, label, sz_img,p_img32f_tar,p_img32f_sub_bk, 
		p_img32_sub_label, p_img_sub, density_map_sub))
	{
		printf("ERROR::  Raw sacle registration is wrong!!!!");
		return false;
	}

	return true;
}

bool mBrainAligner(Parameter input_Parameter, vector<point3D64F>vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> aver_corner,
	vector<int> label, long long * sz_img_tar, float * p_img32f_tar, float * p_img32f_sub_bk, float * p_img32_sub_label,
	unsigned char * p_img_sub, map <int, float *> & density_map_sub)
{
	float  ****p_img32f_tar_4d = 0, ****p_img32f_sub_4d = 0, ****p_img_label = 0;
	new4dpointer(p_img32f_tar_4d, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3], p_img32f_tar);
	new4dpointer(p_img32f_sub_4d, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3], p_img32f_sub_bk);
	if (input_Parameter.Select_modal < 2)
	{
		new4dpointer(p_img_label, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3], p_img32_sub_label);
	}

	map <int, float ****>  density_map_sub_4d;
	if (input_Parameter.Select_modal == 0)
	{
		int array[10] =  { 62, 75, 80, 100, 145, 159, 168, 249 };
		for (int i = 0; i < 8; i++)
		{
			float **** map_4d = 0;
			new4dpointer(map_4d, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3], density_map_sub[array[i]]);
			density_map_sub_4d.insert(pair<int, float****>(array[i], map_4d));
		}
	}
	
	
	//paras
	int nfeature = 3;
	int search_radius = input_Parameter.search_radius;
	int nneighbors = 1;//+1 for include itself
	int kernel_width = 2 * input_Parameter.kernel_radius + 1;
	int featmap_width = 2 * search_radius + 1;
	long long npixels_kernel = kernel_width * kernel_width * kernel_width;
	long long npixels_featmap = featmap_width * featmap_width * featmap_width;
	long long save_with = 60;
	int max_iteration = input_Parameter.iterations_number;

	if (input_Parameter.resample != 1 )
	{
		max_iteration = input_Parameter.iterations_number / 2;
	}

	//allocate memory for local featuremaps of each landmark (one landmark may have 'nfeature' featuremaps)
	vector< float * > vec2d_pfeatmaps(vec_corners.size());
	for (int i = 0; i < vec_corners.size(); i++)
	{
		vec2d_pfeatmaps[i] = new(std::nothrow) float[npixels_featmap]();//ini to 0
		for (int j = 0; j < npixels_featmap; j++)
		{
			vec2d_pfeatmaps[i][j] = -1;
		}

	}

	vector< float * > Source(vec_corners.size());
	long long save_source = pow(save_with * 2 + 1, 3);
	for (int i = 0; i < vec_corners.size(); i++)

	{
		Source[i] = new(std::nothrow) float[save_source]();//ini to 0
		for (int j = 0; j < save_source; j++)
			Source[i][j] = -1.0;
	}

	//get kernel patch of current target landmark
	vector<float *> vec_pkernels(vec_corners.size(), NULL);
	vector<float *> vec_pkernels_Hog(vec_corners.size(), NULL);
	vector<float *> vec_pkernels_map(vec_corners.size(), NULL);
	vector<float *> vec_pkernels_Hog_map(vec_corners.size(), NULL);

	long long patch_size_all;

	if (!get_tar_patch(input_Parameter.Select_modal, vec_pkernels, vec_pkernels_Hog, vec_pkernels_map, vec_pkernels_Hog_map, vec_corners, npixels_kernel,
		input_Parameter.kernel_radius, kernel_width,
		sz_img_tar, p_img_label, p_img32f_tar_4d, patch_size_all))
	{
		printf("Error get_tar_patch() is wrong!");
		return false;
	}

	//precompute the ind2dis and index2pos of each featuremap
	vector<float> vec_ind2dis_featmap(npixels_featmap);//distance of each featuremap element to center
	vector<point3D64F> vec_ind2pos_featmap(npixels_featmap);//position of each featuremap element
	{
		long long ind_fetmap = 0;
		for (int zz = -search_radius; zz <= search_radius; zz++)
			for (int yy = -search_radius; yy <= search_radius; yy++)
				for (int xx = -search_radius; xx <= search_radius; xx++)
				{
					float dis = sqrt(xx*xx + yy*yy + zz*zz);
					vec_ind2dis_featmap[ind_fetmap] = dis;
					vec_ind2pos_featmap[ind_fetmap].x = xx;
					vec_ind2pos_featmap[ind_fetmap].y = yy;
					vec_ind2pos_featmap[ind_fetmap].z = zz;
					ind_fetmap++;
				}
	}

	//find N nearst landmarks for each landmark (include itself)
	vector< vector<neighborinfo> > vec2d_neighborinfo;
	find_nearst_landmarks(vec2d_neighborinfo, vec_corners, nneighbors);

	//---------------------------------------------------
	//enter SRS registration iteration
	//---------------------------------------------------
	long long time_total_start = clock();
	vector<point3D64F> vec_corners_last = vec_corners, or_sub = fine_sub_corner, fix_corners = fine_sub_corner;

	vec_corners_last = fine_sub_corner;
	vector<point3D64F> vec_corner_new = vec_corners_last;
	
	int interval_search_star = search_radius / 4, interval_search=1;

	for (int iter = input_Parameter.star_iter; iter <= max_iteration; iter++)
	{
		printf("iter:[%d]....\n", iter);
		
		/*if (iter>iterations_number / 2)
		{
			interval_search = ceil(interval_search_star * pow(0.998, iter - max_iteration / 2));
		}
		else
		{
			interval_search = ceil(interval_search_star * pow(0.998, iter));
		}*/

		//search around each landmark and fill its feature map
		int featuremap_pgsz_y = featmap_width;
		int featuremap_pgsz_xy = featmap_width*featmap_width;
		long long time_total_start1 = clock();
		#pragma omp parallel for
		for (int ind_landmark = 0; ind_landmark < vec_corners.size(); ind_landmark++)
		{
			if (input_Parameter.Select_modal < 2)
			{
				if ((fine_sub_corner[ind_landmark].outline == 0 && iter<input_Parameter.iterations_number / 2) ||
					(fine_sub_corner[ind_landmark].outline == 1 && iter >= input_Parameter.iterations_number / 2))
					continue;
			}
			
			int X_c = fix_corners[ind_landmark].x - save_with;
			int Y_c = fix_corners[ind_landmark].y - save_with;
			int Z_c = fix_corners[ind_landmark].z - save_with;
			for (int ZZ = -search_radius; ZZ <= search_radius; ZZ = ZZ + interval_search)
				for (int YY = -search_radius; YY <= search_radius; YY = YY + interval_search)
					for (int XX = -search_radius; XX <= search_radius; XX = XX + interval_search)
					{
						int X = vec_corners_last[ind_landmark].x + XX;
						int Y = vec_corners_last[ind_landmark].y + YY;
						int Z = vec_corners_last[ind_landmark].z + ZZ;
						int fetmappos_x = XX + search_radius;
						int fetmappos_y = YY + search_radius;
						int fetmappos_z = ZZ + search_radius;
						long long index_fetmap = featuremap_pgsz_xy*fetmappos_z + featuremap_pgsz_y*fetmappos_y + fetmappos_x;
						
						long long index_source = (2 * save_with + 1) *(2 * save_with + 1) * (Z - Z_c) + (2 * save_with + 1) * (Y - Y_c) + (X - X_c);

						bool select = (index_source<save_source && (Z - Z_c) >= 0 && (Y - Y_c) >= 0 && (X - X_c) >= 0 && (Z - Z_c) < (2 * save_with + 1) && (Y - Y_c) < (2 * save_with + 1) && (X - X_c) < (2 * save_with + 1));

						if (select)
						{
							if (Source[ind_landmark][index_source] != (-1.0))
							{
								vec2d_pfeatmaps[ind_landmark][index_fetmap] = Source[ind_landmark][index_source];
							}
							else
							{
								float  *p_kernel = vec_pkernels[ind_landmark];
								float  *p_kernel_Hog = vec_pkernels_Hog[ind_landmark];
								float  *p_kernel_map = vec_pkernels_map[ind_landmark];
								float  *p_kernel_Hog_map = vec_pkernels_Hog_map[ind_landmark];

								if (input_Parameter.Select_modal == 0)
								{
									if (!Compute_source_fMOST(p_kernel_map, p_kernel, input_Parameter.kernel_radius, sz_img_tar, vec_corners, aver_corner,
										or_sub, p_img32f_sub_4d, p_img_label, ind_landmark,select,
										 npixels_kernel,  index_fetmap, index_source, X, Y, Z,Source,vec2d_pfeatmaps,
										p_kernel_Hog_map,kernel_width, p_kernel_Hog, patch_size_all,
										vec_corners_last, density_map_sub_4d, fine_sub_corner[ind_landmark].outline))
									{
										printf("Error tar Compute_source() is wrong!");
										/*return false;*/
									}
								}
								else
								{												
									if (!Compute_source(p_kernel_map, p_kernel, input_Parameter.kernel_radius, sz_img_tar, vec_corners, aver_corner,
									or_sub, p_img32f_sub_4d, p_img_label, ind_landmark, select,
										npixels_kernel, index_fetmap, index_source, X, Y, Z, Source, vec2d_pfeatmaps,
										p_kernel_Hog_map, kernel_width, p_kernel_Hog, patch_size_all, vec_corners_last))
									{
										printf("Error tar Compute_source() is wrong!");
										/*return false;*/
									}
								}
							}
						}
						else
						{//get sliding patch and compare
							float  *p_kernel = vec_pkernels[ind_landmark];
							float  *p_kernel_Hog = vec_pkernels_Hog[ind_landmark];
							float  *p_kernel_map = vec_pkernels_map[ind_landmark];
							float  *p_kernel_Hog_map = vec_pkernels_Hog_map[ind_landmark];

							if (input_Parameter.Select_modal == 0)
							{
								if (!Compute_source_fMOST(p_kernel_map, p_kernel, input_Parameter.kernel_radius, sz_img_tar, vec_corners, aver_corner,
									or_sub, p_img32f_sub_4d, p_img_label, ind_landmark, select,
									npixels_kernel, index_fetmap, index_source, X, Y, Z, Source, vec2d_pfeatmaps,
									p_kernel_Hog_map, kernel_width, p_kernel_Hog, patch_size_all,
									vec_corners_last, density_map_sub_4d, fine_sub_corner[ind_landmark].outline))
								{
									printf("Error tar Compute_source() is wrong!");
									/*return false;*/
								}
							}
							else
							{
								if (!Compute_source(p_kernel_map, p_kernel, input_Parameter.kernel_radius, sz_img_tar, vec_corners, aver_corner,
									or_sub, p_img32f_sub_4d, p_img_label, ind_landmark, select,
									npixels_kernel, index_fetmap, index_source, X, Y, Z, Source, vec2d_pfeatmaps,
									p_kernel_Hog_map, kernel_width, p_kernel_Hog, patch_size_all, vec_corners_last))
								{
									printf("Error tar Compute_source() is wrong!");
									/*return false;*/
								}
							}
						}
					}
		}

		//combine featuremaps corresponding to the same landmark
		vector< vector<float> > vec2d_featmap_combine;

		time_total_start1 = clock();
		for (int i = 0; i < vec_corners.size(); i++)
		{
			long long ind_fetmap = 0;
			vector<float> vec_featmap_combine(npixels_featmap, 0.0);
			for (int z = 0; z < featmap_width; z++)
				for (int y = 0; y < featmap_width; y++)
					for (int x = 0; x < featmap_width; x++)
					{
						double matchscore;
						matchscore = vec2d_pfeatmaps[i][ind_fetmap];
						vec_featmap_combine[ind_fetmap] = matchscore;
						ind_fetmap++;
					}

			vec2d_featmap_combine.push_back(vec_featmap_combine);
		}
		vector<vector<double> > Source_weight;
		//given landmark position update labeling probability
		vector<vector<vector<double> > > vec3d_prob;//corner -> cur+nneighbor -> 1dfeaturemap
		{
			for (int i = 0; i < vec_corners.size(); i++)
			{
				vector<vector<double> > vec2d_prob;
				vector<double> source;
				for (int j = 0; j < nneighbors; j++)
				{
					vector<double> vec_prob(npixels_featmap + 1, 0.0);
					int ind_neighbor = vec2d_neighborinfo[i][j].ind;
					for (long long k = 0; k < npixels_featmap; k++)
					{
						double matchscore = vec2d_featmap_combine[ind_neighbor][k];
						if (k == (npixels_featmap - 1) / 2)
							source.push_back(matchscore);

						double dis = vec_ind2dis_featmap[k];
						double d_cost = (1 - matchscore)*dis / search_radius;// *vec_confidence[j][k];

						vec_prob[k] = exp(-d_cost / 0.3);
						//vec_prob[k] = exp(-d_cost / 0.3 - 1);//exp(-d_cost/d_T-1);
						if (matchscore == -1)
							vec_prob[k]=0;
					}
					double index = ind_neighbor;
					vec_prob[npixels_featmap] = index;
					vec2d_prob.push_back(vec_prob);
				}
				Source_weight.push_back(source);
				vec3d_prob.push_back(vec2d_prob);
			}

		}

		for (int i = 0; i < vec_corners.size(); i++)
		{
			vector<vector<double> > nneighbors_all;

			for (int j = 0; j < nneighbors; j++)
			{
				double d_probsum = 0, d_sum_x = 0, d_sum_y = 0, d_sum_z = 0;
				vector<double> nneighbors1;
				for (long long k = 0; k < (npixels_featmap); k++)
				{
					d_probsum += vec3d_prob[i][j][k];
					d_sum_x += vec3d_prob[i][j][k] * vec_ind2pos_featmap[k].x;
					d_sum_y += vec3d_prob[i][j][k] * vec_ind2pos_featmap[k].y;
					d_sum_z += vec3d_prob[i][j][k] * vec_ind2pos_featmap[k].z;
				}
				nneighbors1.push_back(d_probsum);
				nneighbors1.push_back(d_sum_x);
				nneighbors1.push_back(d_sum_y);
				nneighbors1.push_back(d_sum_z);
				nneighbors1.push_back(vec3d_prob[i][j][npixels_featmap]);
				nneighbors_all.push_back(nneighbors1);
			}

			double d_probsum = 0, d_sum_x = 0, d_sum_y = 0, d_sum_z = 0;
			for (int j = 0; j < nneighbors; j++)
			{
				d_probsum += nneighbors_all[j][0];
				d_sum_x += nneighbors_all[j][1];
				d_sum_y += nneighbors_all[j][2];
				d_sum_z += nneighbors_all[j][3];
			}
			if (input_Parameter.Select_modal < 2)
			{
				if ((fine_sub_corner[i].outline == 0 && iter >= input_Parameter.iterations_number / 2) ||
					(fine_sub_corner[i].outline == 1 && iter < input_Parameter.iterations_number / 2))
				{
					vec_corner_new[i].x = vec_corner_new[i].x + (d_sum_x / d_probsum);
					vec_corner_new[i].y = vec_corner_new[i].y + (d_sum_y / d_probsum);
					vec_corner_new[i].z = vec_corner_new[i].z + (d_sum_z / d_probsum);
				}
			}
			else
			{
				vec_corner_new[i].x = vec_corner_new[i].x + (d_sum_x / d_probsum);
				vec_corner_new[i].y = vec_corner_new[i].y + (d_sum_y / d_probsum);
				vec_corner_new[i].z = vec_corner_new[i].z + (d_sum_z / d_probsum);
			}
			
		}

		//update landmark's last position
		vec_corners_last = vec_corner_new;
		int fre_region_constraint = input_Parameter.fre_region_constraint;

		if (input_Parameter.Select_modal <2)
		{
			if ((iter) % fre_region_constraint == 0)
			{

				//STPS all point, but update interior point
				Matrix x4x4_affine, xnx4_c, xnxn_K;
				int number = 0;
				if (iter > input_Parameter.iterations_number / 2)
				{
					double lam = input_Parameter.star_lamda_inner*pow(0.99, iter);// 5000 * pow(0.95, iter);
					if (lam < input_Parameter.lam_end_inner)
						lam = input_Parameter.lam_end_inner;

					for (int label_i = 0; label_i < label.size(); label_i++)
					{
						vector<point3D64F> stps_vec_sub, stps_vec_tar, stps_vec_sub_warp;

						for (long i = 0; i < vec_corners.size(); i++)
						{
							if (vec_corners[i].label == label[label_i])
							{
								stps_vec_tar.push_back(aver_corner[i]);
								stps_vec_sub.push_back(vec_corner_new[i]);
							}
						}

						float lam1 = lam;
						stps_vec_sub_warp = stps_vec_tar;
						auto_warp_marker(lam1, stps_vec_tar, stps_vec_sub, stps_vec_sub_warp);
					
						for (unsigned V3DLONG i = 0; i < stps_vec_tar.size(); i++)
						{
							if (vec_corner_new[number + i].outline == 0)
							{
								vec_corner_new[number + i].x = stps_vec_sub_warp[i].x;
								vec_corner_new[number + i].y = stps_vec_sub_warp[i].y;
								vec_corner_new[number + i].z = stps_vec_sub_warp[i].z;
							}
						}

						number = number + stps_vec_tar.size();
					}
				}
				else
				{
					double lam = input_Parameter.star_lamda_outline*pow(0.99, iter);// 5000 * pow(0.95, iter);
					if (lam < input_Parameter.lam_end_out)
						lam = input_Parameter.lam_end_out;

					vector<point3D64F> stps_vec_sub, stps_vec_tar;
					vector<int> outline_index;

					for (long i = 0; i < vec_corners.size(); i++)
					{
						if (vec_corners[i].outline == 1)
						{
							stps_vec_tar.push_back(vec_corners[i]);
							stps_vec_sub.push_back(vec_corner_new[i]);
							outline_index.push_back(i);
						}
					}

					if (!q_TPS_cd(stps_vec_tar, stps_vec_sub, lam, x4x4_affine, xnx4_c, xnxn_K))
					{
						printf("ERROR: (outline) q_TPS_cd() return false!\n");
						return false;
					}
					Matrix x_ori(stps_vec_sub.size(), 4), x_tps(stps_vec_sub.size(), 4);
					for (unsigned V3DLONG i = 0; i < stps_vec_sub.size(); i++)
					{
						x_ori(i + 1, 1) = 1.0;
						x_ori(i + 1, 2) = stps_vec_tar[i].x;
						x_ori(i + 1, 3) = stps_vec_tar[i].y;
						x_ori(i + 1, 4) = stps_vec_tar[i].z;
					}
					x_tps = x_ori*x4x4_affine + xnxn_K*xnx4_c;
					for (unsigned V3DLONG i = 0; i < stps_vec_tar.size(); i++)
					{
						if (vec_corner_new[outline_index[i]].outline == 1)
						{
							vec_corner_new[outline_index[i]].x = x_tps(i + 1, 2);
							vec_corner_new[outline_index[i]].y = x_tps(i + 1, 3);
							vec_corner_new[outline_index[i]].z = x_tps(i + 1, 4);
						}
					}
				}
			}
		}

		if (input_Parameter.Select_modal <2)
		{
			if ((iter) % int(input_Parameter.fre_global_constraint) == 0 && iter> input_Parameter.iterations_number / 2)
			{

				//STPS all point, but update interior point
				Matrix x4x4_affine, xnx4_c, xnxn_K;
				double lam = (input_Parameter.star_lamda_inner)*pow(0.99, iter);// 5000 * pow(0.95, iter);

				if (lam < input_Parameter.lam_end_inner)
					lam = input_Parameter.lam_end_inner;

				if (!q_TPS_cd(vec_corners, vec_corner_new, lam, x4x4_affine, xnx4_c, xnxn_K))
				{
					printf("ERROR: (inner all) q_TPS_cd() return false!\n");
					return false;
				}
				Matrix x_ori(vec_corner_new.size(), 4), x_tps(vec_corner_new.size(), 4);
				for (unsigned V3DLONG i = 0; i < vec_corner_new.size(); i++)
				{
					x_ori(i + 1, 1) = 1.0;
					x_ori(i + 1, 2) = vec_corners[i].x;
					x_ori(i + 1, 3) = vec_corners[i].y;
					x_ori(i + 1, 4) = vec_corners[i].z;
				}
				x_tps = x_ori*x4x4_affine + xnxn_K*xnx4_c;

				for (unsigned V3DLONG i = 0; i < vec_corner_new.size(); i++)
				{
					if (vec_corner_new[i].outline == 0)
					{
						vec_corner_new[i].x = x_tps(i + 1, 2);
						vec_corner_new[i].y = x_tps(i + 1, 3);
						vec_corner_new[i].z = x_tps(i + 1, 4);
					}
				}

			}

		}
		else 
		{
			//STPS all point, but update interior point
			Matrix x4x4_affine, xnx4_c, xnxn_K;
			double lam = (input_Parameter.star_lamda_outline)*pow(0.99, iter);// 5000 * pow(0.95, iter);


			if (lam < input_Parameter.lam_end_inner)
				lam = input_Parameter.lam_end_inner;

			if (!q_TPS_cd(vec_corners, vec_corner_new, lam, x4x4_affine, xnx4_c, xnxn_K))
			{
				printf("ERROR: (inner all) q_TPS_cd() return false!\n");
				return false;
			}
			Matrix x_ori(vec_corner_new.size(), 4), x_tps(vec_corner_new.size(), 4);
			for (unsigned V3DLONG i = 0; i < vec_corner_new.size(); i++)
			{
				x_ori(i + 1, 1) = 1.0;
				x_ori(i + 1, 2) = vec_corners[i].x;
				x_ori(i + 1, 3) = vec_corners[i].y;
				x_ori(i + 1, 4) = vec_corners[i].z;
			}
			x_tps = x_ori*x4x4_affine + xnxn_K*xnx4_c;

			for (unsigned V3DLONG i = 0; i < vec_corner_new.size(); i++)
			{
					vec_corner_new[i].x = x_tps(i + 1, 2);
					vec_corner_new[i].y = x_tps(i + 1, 3);
					vec_corner_new[i].z = x_tps(i + 1, 4);
			}
		}





		//warp subject image based on updated landmark position

		/*if (iter == max_iteration)*/
		if ((iter) % int(input_Parameter.fre_save) == 0)
		{
			QList<ImageMarker> ql_marker_tar, ql_marker_sub;
			for (long long i = 0; i < vec_corners.size(); i++)
			{
				ImageMarker tmp;
				tmp.x = vec_corners[i].x; tmp.y = vec_corners[i].y; tmp.z = vec_corners[i].z; tmp.radius = 5, tmp.shape = 1; ql_marker_tar.push_back(tmp);
				tmp.x = vec_corner_new[i].x; tmp.y = vec_corner_new[i].y; tmp.z = vec_corner_new[i].z; tmp.radius = 5, tmp.shape = 1;	ql_marker_sub.push_back(tmp);
			}

			//warp float type image for next iteration
			V3DLONG szBlock_x, szBlock_y, szBlock_z;
			szBlock_x = szBlock_y = szBlock_z = 4;
			int		i_interpmethod_df = 1;		//default B-spline
			int		i_interpmethod_img = 0;		//default trilinear

			//warp uint8 type image for save
			{
				unsigned char *p_img_warp = 0;
				imgwarp_smallmemory(p_img_sub, sz_img_tar, ql_marker_tar, ql_marker_sub,
					szBlock_x, szBlock_y, szBlock_z, i_interpmethod_df, i_interpmethod_img,
					p_img_warp);
				char filename[2000];
				QString save_file = input_Parameter.save_path + "local_registered_image.v3draw";
				sprintf(filename, "%s", qPrintable(save_file));
				saveImage(filename, (unsigned char *)p_img_warp, sz_img_tar, 1);
				if (p_img_warp) 		{ delete[]p_img_warp;			p_img_warp = 0; }

				//save as marker files
				sprintf(filename, "%slocal_registered_tar.marker", qPrintable(input_Parameter.save_path));	writeMarker_file(filename, ql_marker_tar);
				sprintf(filename, "%slocal_registered_sub.marker", qPrintable(input_Parameter.save_path));	writeMarker_file(filename, ql_marker_sub);
				
				
			}
		}

		for (int i = 0; i < vec_corners.size(); i++)
		{
			for (int j = 0; j < npixels_featmap; j++)
			{
				vec2d_pfeatmaps[i][j] = -1;
			}
		}

	}

	fine_sub_corner = vec_corner_new;
	for (int i = 0; i < fine_sub_corner.size(); i++)
	{
		fine_sub_corner[i].x = fine_sub_corner[i].x*input_Parameter.resample;
		fine_sub_corner[i].y = fine_sub_corner[i].y*input_Parameter.resample;
		fine_sub_corner[i].z = fine_sub_corner[i].z*input_Parameter.resample;
	}

	//release memory
	{
		for (int i = 0; i < vec_corners.size(); i++)
			for (int j = 0; j < nfeature; j++)
			{
				if (vec2d_pfeatmaps[i])	{ delete[]vec2d_pfeatmaps[i]; vec2d_pfeatmaps[i] = 0; }
			}

		for (int i = 0; i < vec_corners.size(); i++)
		{
			if (Source[i])	{ delete[]Source[i]; Source[i] = 0; }
			if (vec_pkernels[i])	{ delete[]vec_pkernels[i]; vec_pkernels[i] = 0; }
			if (vec_pkernels_Hog[i])	{ delete[]vec_pkernels_Hog[i]; vec_pkernels_Hog[i] = 0; }
			if (vec_pkernels_map[i])	{ delete[]vec_pkernels_map[i]; vec_pkernels_map[i] = 0; }
			if (vec_pkernels_Hog_map[i])	{ delete[]vec_pkernels_Hog_map[i]; vec_pkernels_Hog_map[i] = 0; }
		}

		if (p_img32f_tar_4d)		{ delete4dpointer(p_img32f_tar_4d, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]); }
		if (p_img32f_sub_4d)		{ delete4dpointer(p_img32f_sub_4d, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]); }
		if (p_img_label)		{ delete4dpointer(p_img_label, sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]); }

		for (int i = 0; i < label.size(); i++)
		{
			if (density_map_sub_4d[label[i]]) 		{ delete4dpointer(density_map_sub_4d[label[i]], sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]); }
		}
	}
	
	return true;
}
