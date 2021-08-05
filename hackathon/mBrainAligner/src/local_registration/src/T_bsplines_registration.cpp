
#include"T_bsplines_registration.h"

#include "basic_surf_objs.h"
#include "../../basic_c_fun/basic_memory.cpp"

bool T_bsplines_registration(const long long sz_img_sub[4],  vector<point3D64F> &vec_tar,  vector<point3D64F> &vec_sub, 
	vector<point3D64F> &vec_warp, vector< vector< vector< vector<double> > > > &vec4D_grid, long &New_Spacing, long long & number_outline_landmark)
{
	//check paras
	if (sz_img_sub[0] <= 0 || sz_img_sub[1] <= 0 || sz_img_sub[2] <= 0 || sz_img_sub[3] != 1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (vec_tar.size() == 0 || vec_sub.size() == 0 || vec_tar.size()!=vec_sub.size())
	{
		printf("ERROR: Invalid input target or subject control points!\n");
		return false;
	}
	/*if (vec_warp.size() != 0)
	{
		printf("ERROR: input update marker is not empty!\n");
		return false;
	}*/
	if (vec4D_grid.size() != 0)
	{
		printf("WARNING: Output vec4D_grid is not empty!\n");
		vec4D_grid.clear();
	}
	if (New_Spacing != 0)
	{
		printf("WARNING: input update Spacing is not empty! now setting Spacing to zero!\n");
		New_Spacing = 0;
	}

	//Initialize grid spacing
	long MaxTtt;
	MaxTtt = min(min(floor(log2(sz_img_sub[0] / 2)), floor(log2(sz_img_sub[1] / 2))), floor(log2(sz_img_sub[2] / 2)));
	New_Spacing = pow(2, MaxTtt);

	//Control point error
	vector<point3D64F> vec_update_d;
	point3D64F tmp;
	for (long long i = 0; i < vec_sub.size(); i++)
	{
		tmp.x = vec_sub[i].x - vec_tar[i].x; tmp.y = vec_sub[i].y - vec_tar[i].y; tmp.z = vec_sub[i].z - vec_tar[i].z;
		vec_update_d.push_back(tmp);
		vec_warp.push_back(tmp);
	}

	long sz_grid[3];//w,h,z
	for (long i = 0; i < 3; i++)
	{
		sz_grid[i] = sz_img_sub[i] / New_Spacing + 4;
	}

	vector< vector< vector< vector<double> > > > vec4D_grid_tmp;
	/*vector< vector< vector< vector<double> > > > vec4D_grid_ori;*/
	vector< vector< vector< vector<double> > > > vec4D_grid_df;
	vec4D_grid_df.assign(sz_grid[1], vector< vector< vector<double> > >(sz_grid[0], vector< vector<double> >(sz_grid[2], vector<double>(3, 0))));
	if (!q_nonrigid_inigrid_generator(sz_img_sub, New_Spacing, vec4D_grid))
	{
		printf("ERROR: q_inigrid_generator() return false!\n");
		return false;
	}
	printf("\t>>grid_size: %d,%d,%d; Spacing_size: %ld\n", vec4D_grid[0].size(), vec4D_grid.size(), vec4D_grid[0][0].size(), New_Spacing);
	
	
	//************************************************************************************************************************************
	//enter b-splines registration iteration
	//************************************************************************************************************************************
	printf("Enter b-splines registration iteration!\n ");
	int iter_num = log2(New_Spacing / 4);
	for (long iter = 0; iter < iter_num; iter++)
	{
		printf("\t----------------------------------------------------------\n");
		printf("\t         BSPLINES ITER: [%d]\n", iter);
		printf("\t----------------------------------------------------------\n");
		
		for (int i = 0; i < 3; i++)
		{
			sz_grid[i] = sz_img_sub[i] / New_Spacing + 4;
		}
		vec4D_grid_tmp.assign(sz_grid[1], vector< vector< vector<double> > >(sz_grid[0], vector< vector<double> >(sz_grid[2], vector<double>(3, 0))));
		for (long k = 0; k < 3; k++)
			for (long x = 0; x<sz_grid[0]; x++)
				for (long y = 0; y<sz_grid[1]; y++)
					for (long z = 0; z<sz_grid[2]; z++)
					{
						vec4D_grid_tmp[y][x][z][k] = vec4D_grid_df[y][x][z][k];
					}

		//Generate initial B-spline control points meshgrid
	/*	long sz_img_lowest[4] = { 1, 1, 1, 1 };
		for (long i = 0; i<3; i++)
			sz_img_lowest[i] = ceil(sz_img_sub[i]);*/
		//if (!q_nonrigid_inigrid_generator(sz_img_lowest, Spacing, vec4D_grid))
		//{
		//	printf("ERROR: q_inigrid_generator() return false!\n");
		//	return false;
		//}
		
		//backup the initial grid
		

		//------------------------------------------------------------------------------------------------------------------------------------
		// B-spline basis/blending functions update grid
		if (!T_bsplinebasis2grid_3D(sz_grid, New_Spacing, vec_tar, vec_update_d, vec4D_grid_tmp, vec4D_grid))
		{
			printf("ERROR: T_bsplinebasis2grid_3D() return false!\n");
			return false;
		}

		if (!T_bsplines_wrapmarker(New_Spacing, vec_tar, vec_sub, vec4D_grid, vec_warp, vec_update_d,number_outline_landmark))
		{
			printf("ERROR: q_bsplines_wrapmarker() return false!\n");
			return false;
		}
		if (iter < iter_num - 1)
		{
			long old_Spacing = New_Spacing;
			if (!T_bsplines_RefineGrid(old_Spacing, sz_grid, sz_img_sub, vec4D_grid_tmp, vec4D_grid_df, vec4D_grid, New_Spacing))
			{
				printf("ERROR: T_bsplines_RefineGrid() return false!\n");
				return false;
			}
		}
	}
}

bool bspline_coefficients(const double ax, const double ay, const double az, Matrix &BxBxB)
{
	//check paras
	if (ax < 0 || ay < 0 || az < 0)
	{
		printf("ERROR: n should > 0!\n");
		return false;
	}

	//cubic B-spline basis matrix
	Matrix B(4, 4);
	B.row(1) << -1 << 3 << -3 << 1;
	B.row(2) << 3 << -6 << 3 << 0;
	B.row(3) << -3 << 0 << 3 << 0;
	B.row(4) << 1 << 4 << 1 << 0;
	B /= 6.0;

	//construct T(i,:)=[t^3 t^2 t^1 1]
	Matrix T(1, 4), V(1, 4), U(1, 4);
	for (long j = 0; j <= 3; j++)
	{
		T(1, j + 1) = pow(ax, 3 - j);
		V(1, j + 1) = pow(ay, 3 - j);
		U(1, j + 1) = pow(az, 3 - j);
	}

	//construct B-spline basis/blending functions B=T*B
	Matrix Bu(1, 4), Bv(1, 4), Bw(1, 4);
	Bu = T*B; Bv = V*B; Bw = U*B;
	int n = 1;
	for (int i = 1; i <= 4; i++)
		for (int j = 1; j <= 4; j++)
			for (int k = 1; k <= 4; k++)
			{
				BxBxB(1, n) = Bu(1, k)*Bv(1, j)*Bw(1, i); n++;
			}

	return true;
}

bool q_nonrigid_warpimage_baseongrid(const unsigned char *p_img8u_sub, const long long sz_img_sub[4],const vector< vector< vector< vector<double> > > > &vec4D_grid, 
	const long Spacing, unsigned char *&p_img8u_sub_warp)
{
	//check paras
	if (p_img8u_sub == 0)
	{
		printf("ERROR: Invalid input image pointer !\n");
		return false;
	}
	if (sz_img_sub[0] <= 0 || sz_img_sub[1] <= 0 || sz_img_sub[2] <= 0 || sz_img_sub[3] <= 0 || sz_img_sub[3]>3)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (Spacing<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if (vec4D_grid.size() == 0)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if (p_img8u_sub_warp)
	{
		printf("WARNING: Output image pointer is not NULL, original data will be released!\n");
		delete[]p_img8u_sub_warp;		p_img8u_sub_warp = 0;
	}

	//Initialize B-spline basis/blending functions
	printf("q_ini_bsplinebasis_3D()....\n");
	Matrix x_bsplinebasis(pow(Spacing,3), pow(4, 3));
	if (!q_image_ini_bsplinebasis_3D(Spacing, x_bsplinebasis))
	{
		printf("ERROR: q_ini_bsplinebasis_3D() return false!\n");
		return false;
	}
	printf("\t>>x_bsplinebasis:[%d,%d]\n", x_bsplinebasis.nrows(), x_bsplinebasis.ncols());

	//long sz_gridwnd[3];
	//for (long i = 0; i < 3; i++)
	//{
	//	sz_gridwnd[i] = sz_grid[i];
	//}
	//B-Spline interpolate the meshgrid into transformation field
	printf("q_bspline_grid2field_3D()....\n");
	vector< vector< vector< vector<double> > > > vec4D_grid_int;
	if (!q_bspline_grid2field_3D(vec4D_grid, Spacing, x_bsplinebasis, vec4D_grid_int))
	{
		printf("ERROR: q_bspline_grid2field_3D() return false!\n");
		return false;
	}

	//get gird size = embed image size
	long sz_img_embed[4] = { 1, 1, 1, 1 };//w,h,z
	sz_img_embed[1] = vec4D_grid_int.size();		//nrow
	sz_img_embed[0] = vec4D_grid_int[0].size();	//ncol
	sz_img_embed[2] = vec4D_grid_int[0][0].size();//nz
	sz_img_embed[3] = sz_img_sub[3];				//nc

	//embedding input subject image
	unsigned char *p_img8u_embed = 0;
	p_img8u_embed = new unsigned char[sz_img_embed[0] * sz_img_embed[1] * sz_img_embed[2] * sz_img_embed[3]]();
	if (!p_img8u_embed)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_sub_embed!\n");
		return false;
	}
	unsigned char ****p_img_sub_4d = 0, ****p_img_embed_4d = 0;
	if (!new4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3], p_img8u_sub) ||
		!new4dpointer(p_img_embed_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3], p_img8u_embed))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img8u_embed) 		{ delete[]p_img8u_embed;		p_img8u_embed = 0; }
		if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
		if (p_img_embed_4d) 		{ delete4dpointer(p_img_embed_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }
		return false;
	}
	for (long c = 0; c<sz_img_sub[3]; c++)
		for (long x = 0; x<sz_img_embed[0]; x++)
			for (long y = 0; y<sz_img_embed[1]; y++)
				for (long z = 0; z<sz_img_embed[2]; z++)
					if (x >= sz_img_sub[0] || y >= sz_img_sub[1] || z >= sz_img_sub[2])
						p_img_embed_4d[c][z][y][x] = 0;
					else
						p_img_embed_4d[c][z][y][x] = p_img_sub_4d[c][z][y][x];
	if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
	if (p_img_embed_4d) 		{ delete4dpointer(p_img_embed_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }

	//warp embeded subject image base on the deformed meshgrid
	printf("q_nonrigid_interpolate_3D()....\n");
	unsigned char *p_img8u_embed_warp = 0;
	if (!q_nonrigid_interpolate_3D(p_img8u_embed, sz_img_embed, vec4D_grid_int, p_img8u_embed_warp))
	{
		printf("ERROR: q_interpolate_3D() return false!\n");
		if (p_img8u_embed) 		{ delete[]p_img8u_embed;		p_img8u_embed = 0; }
		if (p_img8u_sub_warp) 	{ delete[]p_img8u_sub_warp;		p_img8u_sub_warp = 0; }
		return false;
	}
	if (p_img8u_embed) 		{ delete[]p_img8u_embed;		p_img8u_embed = 0; }
	

	//crop the valid image region
	p_img8u_sub_warp = new unsigned char[sz_img_sub[0] * sz_img_sub[1] * sz_img_sub[2] * sz_img_sub[3]]();
	if (!p_img8u_sub_warp)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_output_sub!\n");
		if (p_img8u_embed) 		{ delete[]p_img8u_embed;		p_img8u_embed = 0; }
		if (p_img8u_embed_warp) 	{ delete[]p_img8u_embed_warp;	p_img8u_embed_warp = 0; }
		return false;
	}
	unsigned char ****p_img_sub_warp_4d = 0, ****p_img_embed_warp_4d = 0;
	if (!new4dpointer(p_img_sub_warp_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3], p_img8u_sub_warp) ||
		!new4dpointer(p_img_embed_warp_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3], p_img8u_embed_warp))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img8u_embed) 		{ delete[]p_img8u_embed;		p_img8u_embed = 0; }
		if (p_img8u_embed_warp) 	{ delete[]p_img8u_embed_warp;	p_img8u_embed_warp = 0; }
		if (p_img8u_sub_warp) 	{ delete[]p_img8u_sub_warp;		p_img8u_sub_warp = 0; }
		if (p_img_sub_warp_4d) 	{ delete4dpointer(p_img_sub_warp_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
		if (p_img_embed_warp_4d) { delete4dpointer(p_img_embed_warp_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }
		return false;
	}
	for (long c = 0; c<sz_img_sub[3]; c++)
		for (long x = 0; x<sz_img_sub[0]; x++)
			for (long y = 0; y<sz_img_sub[1]; y++)
				for (long z = 0; z<sz_img_sub[2]; z++)
					p_img_sub_warp_4d[c][z][y][x] = p_img_embed_warp_4d[c][z][y][x];
	if (p_img_sub_warp_4d) 	{ delete4dpointer(p_img_sub_warp_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
	if (p_img_embed_warp_4d) { delete4dpointer(p_img_embed_warp_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }

	//free memory
	if (p_img8u_embed) 		{ delete[]p_img8u_embed;			p_img8u_embed = 0; }
	if (p_img8u_embed_warp) 	{ delete[]p_img8u_embed_warp;		p_img8u_embed_warp = 0; }
	if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
	if (p_img_embed_4d) 		{ delete4dpointer(p_img_embed_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }
	if (p_img_sub_warp_4d) 	{ delete4dpointer(p_img_sub_warp_4d, p_img8u_sub[0], p_img8u_sub[1], p_img8u_sub[2], p_img8u_sub[3]); }
	if (p_img_embed_warp_4d) { delete4dpointer(p_img_embed_warp_4d, sz_img_embed[0], sz_img_embed[1], sz_img_embed[2], sz_img_embed[3]); }

	return true;

}

bool q_image_ini_bsplinebasis_3D(const long n, Matrix &BxBxB)
{
	//check paras
	if (n <= 0)
	{
		printf("ERROR: n should > 0!\n");
		return false;
	}

	//cubic B-spline basis matrix
	Matrix B(4, 4);
	B.row(1) << -1 << 3 << -3 << 1;
	B.row(2) << 3 << -6 << 3 << 0;
	B.row(3) << -3 << 0 << 3 << 0;
	B.row(4) << 1 << 4 << 1 << 0;
	B /= 6.0;

	//construct T(i,:)=[t^3 t^2 t^1 1]
	Matrix T(n, 4);
	double t_step = 1.0 / n;
	for (long i = 0; i<n; i++)
	{
		double t = t_step*i;
		for (long j = 0; j <= 3; j++)
			T(i + 1, j + 1) = pow(t, 3 - j);
	}

	//construct B-spline basis/blending functions B=T*B
	Matrix TB = T*B;//n x 4

	//construct B-spline basis/blending functions for 2D interpolation B=BxB
	Matrix BxB = KP(TB, TB);//n^2 x 4^2
	//construct B-spline basis/blending functions for 3D interpolation B=BxBxB
	BxBxB = KP(BxB, TB);//n^3 x 4^3

	return true;
}

//meshgrid generator
//for B_spline interpolation, we need to maintain a larger grid (one more gridnode out of each boundary)
bool q_nonrigid_inigrid_generator(const long long sz_img[4], const long Spacing,
	vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	//check paras
	if (sz_img[0] <= 0 || sz_img[1] <= 0 || sz_img[2] <= 0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (Spacing<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if (vec4D_grid.size() != 0)
	{
		printf("WARNNING: Output vec4D_grid is not empty, original data will be cleared!\n");
		vec4D_grid.clear();
	}

	//initial meshgrid size
	long sz_grid[3];//w,h,z
	for (long i = 0; i<3; i++)
		sz_grid[i] = sz_img[i] / Spacing + 4;

	//fill gridnode coordinate
	vec4D_grid.assign(sz_grid[1], vector< vector< vector<double> > >(sz_grid[0], vector< vector<double> >(sz_grid[2], vector<double>(3, 0))));
	for (long x = 0; x<sz_grid[0]; x++)
		for (long y = 0; y<sz_grid[1]; y++)
			for (long z = 0; z<sz_grid[2]; z++)
			{
				vec4D_grid[y][x][z][0] = (x - 1)*Spacing;//x coord
				vec4D_grid[y][x][z][1] = (y - 1)*Spacing;//y coord
				vec4D_grid[y][x][z][2] = (z - 1)*Spacing;//z coord
			}

	return true;
}

bool T_bsplinebasis2grid_3D(const long sz_grid[3], const long Spacing, const vector<point3D64F> &vec_tar, const vector<point3D64F> &vec_Diff,
	vector< vector< vector< vector<double> > > > &vec4D_grid_tmp,
	vector< vector< vector< vector<double> > > > &vec4D_grid)
{
	//check paras
	if (sz_grid[0] <= 0 || sz_grid[1] <= 0 || sz_grid[2] <= 0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (Spacing<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if (vec_tar.size() == 0 || vec_Diff.size() == 0)
	{
		printf("ERROR: input target, subject's marker or their difference is empty!\n");
		return false;
	}
	
	vector<vector< vector<double > > >S_WNx(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >S_WNy(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >S_WNz(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >S_W2(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >ux(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >uy(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));
	vector<vector< vector<double > > >uz(sz_grid[1], vector<vector<double> >(sz_grid[0], vector<double>(sz_grid[2], 0)));

	for (long i = 0; i < vec_tar.size(); i++)
	{
		int gx, gy, gz;
		double ax, ay, az;
		gx = floor(vec_tar[i].x / Spacing);
		gy = floor(vec_tar[i].y / Spacing);
		gz = floor(vec_tar[i].z / Spacing);

		ax = (vec_tar[i].x - gx*Spacing) / Spacing;
		ay = (vec_tar[i].y - gy*Spacing) / Spacing;
		az = (vec_tar[i].z - gz*Spacing) / Spacing;
		if (ax < 0 || ay < 0 || az < 0 || ax>1 || ay>1 || az>1){ printf("error grid"); return false; }


		Matrix W(1, 64);
		if (!bspline_coefficients(ax, ay, az, W))
		{
			printf("ERROR : bspline_coefficients return false!\n");
			return false;
		}

		Matrix W2 = SP(W, W);
		double S = sum(W2);
		Matrix WT = SP(W2, W);

		Matrix WNx = WT*(vec_Diff[i].x / S);
		Matrix WNy = WT*(vec_Diff[i].y / S);
		Matrix WNz = WT*(vec_Diff[i].z / S);

		long indx = 1, indy = 1, indz = 1, index = 1;
		for (long z = gz; z < 4 + gz; z++)
			for (long y = gy; y < 4 + gy; y++)
				for (long x = gx; x < 4 + gx; x++)
				{
					S_WNx[y][x][z] += WNx(1, indx); indx++;
					S_WNy[y][x][z] += WNy(1, indy); indy++;
					S_WNz[y][x][z] += WNz(1, indz); indz++;
					S_W2[y][x][z] += W2(1, index); index++;
				}

		for (long x = 0; x < sz_grid[0]; x++)
			for (long y = 0; y < sz_grid[1]; y++)
				for (long z = 0; z < sz_grid[2]; z++)
				{
					ux[y][x][z] = S_WNx[y][x][z] / (S_W2[y][x][z] + 2.2204e-16);
					uy[y][x][z] = S_WNy[y][x][z] / (S_W2[y][x][z] + 2.2204e-16);
					uz[y][x][z] = S_WNz[y][x][z] / (S_W2[y][x][z] + 2.2204e-16);
				}
	}
	for (long x = 0; x < sz_grid[0]; x++)
		for (long y = 0; y < sz_grid[1]; y++)
			for (long z = 0; z < sz_grid[2]; z++)
			{
				vec4D_grid_tmp[y][x][z][0] += ux[y][x][z];
				vec4D_grid_tmp[y][x][z][1] += uy[y][x][z];
				vec4D_grid_tmp[y][x][z][2] += uz[y][x][z];
			}

	for (long x = 0; x < sz_grid[0]; x++)
		for (long y = 0; y < sz_grid[1]; y++)
			for (long z = 0; z < sz_grid[2]; z++)
			{
				vec4D_grid[y][x][z][0] += vec4D_grid_tmp[y][x][z][0];
				vec4D_grid[y][x][z][1] += vec4D_grid_tmp[y][x][z][1];
				vec4D_grid[y][x][z][2] += vec4D_grid_tmp[y][x][z][2];
			}
	return true;
}

bool T_bsplines_wrapmarker(const long Spacing, const vector<point3D64F> &vec_tar, const vector<point3D64F> &vec_sub, vector< vector< vector< vector<double> > > > &vec4D_grid,
	vector<point3D64F> &vec_warp, vector<point3D64F> &vec_update , long long & number_outline_landmark)
{
	if (Spacing<1)
	{
		printf("ERROR: Invalid input grid window size!\n");
		return false;
	}
	if (vec_tar.size() == 0 || vec_tar.size()!=vec_sub.size())
	{
		printf("ERROR: target marker is NULL or tar and sub size is difference!\n");
	}
	if (vec4D_grid.size() == 0)
	{
		printf("ERROR: Iutput vec4D_grid is empty!\n");
	}

	point3D64F tp,t; 
	double err = 0;
	for (long long i = 0; i < vec_tar.size(); i++)
	{
		int gx, gy, gz;
		double ax, ay, az;
		gx = floor(vec_tar[i].x / Spacing);
		gy = floor(vec_tar[i].y / Spacing);
		gz = floor(vec_tar[i].z / Spacing);

		ax = (vec_tar[i].x - gx*Spacing) / Spacing;
		ay = (vec_tar[i].y - gy*Spacing) / Spacing;
		az = (vec_tar[i].z - gz*Spacing) / Spacing;
		if (ax < 0 || ay < 0 || az < 0 || ax>1 || ay>1 || az>1){ printf("error grid"); return false; }

		Matrix W(1, 64);
		if (!bspline_coefficients(ax, ay, az, W))
		{
			printf("ERROR : bspline_coefficients return false!\n");
			return false;
		}

		long index = 1, indx = 1, indy = 1, indz = 1;

		double TX = 0;
		double TY = 0;
		double TZ = 0;
		for (long z = gz; z < 4 + gz; z++)
			for (long y = gy; y < 4 + gy; y++)
				for (long x = gx; x < 4 + gx; x++)
				{
					TX += W(1, indx) * vec4D_grid[y][x][z][0]; indx++;
					TY += W(1, indy) * vec4D_grid[y][x][z][1]; indy++;
					TZ += W(1, indz) * vec4D_grid[y][x][z][2]; indz++;
				}
		tp.x = TX; tp.y = TY; tp.z = TZ;
		//\B8\FC\C4ڲ\BF\BA\CD\CDⲿ\B5\E3;
		if (i>= number_outline_landmark)
		    vec_warp[i] = tp;

		t.x = vec_sub[i].x - TX; t.y = vec_sub[i].y - TY; t.z = vec_sub[i].z - TZ;
		vec_update[i] = t;
		err = err + sqrt(t.x*t.x + t.y*t.y + t.z*t.z);
	}

	cout << "Mean Distance : " << err / vec_sub.size() << endl;
	return true;
}

bool T_bsplines_RefineGrid(const long Old_Spacing, long sz_grid[3], const long long sz_img_sub[4], const vector< vector< vector< vector<double> > > > &vec4D_grid_tmp,
	vector< vector< vector< vector<double> > > > &vec4D_grid_df, vector< vector< vector< vector<double> > > > &vec4D_grid, long &New_Spacing)
{
	New_Spacing = Old_Spacing / 2;
	long sz_grid_old[3];
	for (long i = 0; i < 3; i++)
	{
		sz_grid[i] = 2 * sz_grid[i] - 3;
		sz_grid_old[i] = (sz_grid[i] + 3) / 2;
	}

	vec4D_grid_df.assign(sz_grid[1], vector< vector< vector<double> > >(sz_grid[0], vector< vector<double> >(sz_grid[2], vector<double>(3, 0))));

	//--------------------------------------------------------
	vector<double > P0x;
	vector<double > P1x;
	vector<double > P2x;
	vector<double > P3x;
	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid_old[2]; z++)
			for (long y = 0; y < sz_grid_old[1]; y++)
				for (long x = 0; x < sz_grid_old[0]; x++)
				{
					if (x < (sz_grid_old[0] - 3))                { P0x.push_back(vec4D_grid_tmp[y][x][z][k]); }
					if (x > 0 && x < (sz_grid_old[0] - 2))       { P1x.push_back(vec4D_grid_tmp[y][x][z][k]); }
					if (x > 1 && x < (sz_grid_old[0] - 1))       { P2x.push_back(vec4D_grid_tmp[y][x][z][k]); }
					if (x > 2 && x < sz_grid_old[0])             { P3x.push_back(vec4D_grid_tmp[y][x][z][k]); }
				}


	long thx = (sz_grid_old[0] - 3) * sz_grid_old[1] * sz_grid_old[2];


	//Refine B-spline grid in the x-direction
	long a = 0; long b = 0; long c = 0; long d = 0; long e = 0;


	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid_old[2]; z++)
			for (long y = 0; y < sz_grid_old[1]; y++)
				for (long x = 0; x < sz_grid[0]; x++)
				{
					if (x % 2 == 0 && x < sz_grid[0] - 4)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P0x[a] + P1x[a]) / 8; a++;
					}

					if (x % 2 != 0 && x < sz_grid[0] - 3)
					{
						vec4D_grid_df[y][x][z][k] = 1 * ((P0x[b] + 6 * P1x[b]) + P2x[b]) / 8; b++;
					}

					if (x % 2 == 0 && x > 1 && x < sz_grid[0] - 2)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P1x[c] + P2x[c]) / 8; c++;
					}

					if (x % 2 != 0 && x > 2 && x < sz_grid[0] - 1)
					{
						vec4D_grid_df[y][x][z][k] = 1 * (P1x[d] + 6 * P2x[d] + P3x[d]) / 8; d++;
					}

					if (x % 2 == 0 && x > 3 && x < sz_grid[0])
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P2x[e] + P3x[e]) / 8; e++;
					}
				}
	//Refine B-spline grid in the y-direction
	vector<double > P0y;
	vector<double > P1y;
	vector<double > P2y;
	vector<double > P3y;
	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid_old[2]; z++)
			for (long y = 0; y < sz_grid_old[1]; y++)
				for (long x = 0; x < sz_grid[0]; x++)
				{
					if (y < (sz_grid_old[1] - 3))                           { P0y.push_back(vec4D_grid_df[y][x][z][k]); }
					if (y > 0 && y < (sz_grid_old[1] - 2))       { P1y.push_back(vec4D_grid_df[y][x][z][k]); }
					if (y > 1 && y < (sz_grid_old[1] - 1))       { P2y.push_back(vec4D_grid_df[y][x][z][k]); }
					if (y > 2 && y < sz_grid_old[1])                   { P3y.push_back(vec4D_grid_df[y][x][z][k]); }
				}



	long a1 = 0; long b1 = 0; long c1 = 0; long d1 = 0; long e1 = 0;

	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid_old[2]; z++)
			for (long y = 0; y < sz_grid[1]; y++)
				for (long x = 0; x < sz_grid[0]; x++)
				{
					if (y % 2 == 0 && y < sz_grid[1] - 4)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P0y[a1] + P1y[a1]) / 8; a1++;
					}
					if (y % 2 != 0 && y < sz_grid[1] - 3)
					{
						vec4D_grid_df[y][x][z][k] = 1 * (P0y[b1] + 6 * P1y[b1] + P2y[b1]) / 8; b1++;
					}
					if (y % 2 == 0 && y > 1 && y < sz_grid[1] - 2)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P1y[c1] + P2y[c1]) / 8; c1++;
					}
					if (y % 2 != 0 && y > 2 && y < sz_grid[1] - 1)
					{
						vec4D_grid_df[y][x][z][k] = 1 * (P1y[d1] + 6 * P2y[d1] + P3y[d1]) / 8; d1++;
					}
					if (y % 2 == 0 && y > 3 && y < sz_grid[1])
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P2y[e1] + P3y[e1]) / 8; e1++;
					}
				}
	//Refine B-spline grid in the z-direction
	vector<double > P0z;
	vector<double > P1z;
	vector<double > P2z;
	vector<double > P3z;
	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid_old[2]; z++)
			for (long y = 0; y < sz_grid[1]; y++)
				for (long x = 0; x < sz_grid[0]; x++)
				{
					if (z < (sz_grid_old[2] - 3))                           { P0z.push_back(vec4D_grid_df[y][x][z][k]); }
					if (z > 0 && z < (sz_grid_old[2] - 2))       { P1z.push_back(vec4D_grid_df[y][x][z][k]); }
					if (z > 1 && z < (sz_grid_old[2] - 1))       { P2z.push_back(vec4D_grid_df[y][x][z][k]); }
					if (z > 2 && z < sz_grid_old[2])                   { P3z.push_back(vec4D_grid_df[y][x][z][k]); }
				}

	long thz = sz_grid[0] * sz_grid[1] * (sz_grid_old[2] - 3);
	long a2 = 0; long b2 = 0; long c2 = 0; long d2 = 0; long e2 = 0;

	for (long k = 0; k < 3; k++)
		for (long z = 0; z < sz_grid[2]; z++)
			for (long y = 0; y < sz_grid[1]; y++)
				for (long x = 0; x < sz_grid[0]; x++)
				{
					if (z % 2 == 0 && z < sz_grid[2] - 4)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P0z[a2] + P1z[a2]) / 8; a2++;
					}
					if (z % 2 != 0 && z < sz_grid[2] - 3)
					{
						vec4D_grid_df[y][x][z][k] = 1 * (P0z[b2] + 6 * P1z[b2] + P2z[b2]) / 8; b2++;
					}
					if (z % 2 == 0 && z > 1 && z < sz_grid[2] - 2)
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P1z[c2] + P2z[c2]) / 8; c2++;
					}
					if (z % 2 != 0 && z > 2 && z < sz_grid[2] - 1)
					{
						vec4D_grid_df[y][x][z][k] = 1 * (P1z[d2] + 6 * P2z[d2] + P3z[d2]) / 8; d2++;
					}
					if (z % 2 == 0 && z > 3 && z < sz_grid[2])
					{
						vec4D_grid_df[y][x][z][k] = 4 * (P2z[e2] + P3z[e2]) / 8; e2++;
					}
				}

	if (!q_nonrigid_inigrid_generator(sz_img_sub, New_Spacing, vec4D_grid))
	{
		printf("ERROR: q_inigrid_generator() return false!\n");
		return false;
	}
	return true;
}

bool q_bspline_grid2field_3D(const vector< vector< vector< vector<double> > > > &vec4D_grid,
	const long Spacing, const Matrix &x_bsplinebasis,
	vector< vector< vector< vector<double> > > > &vec4D_grid_int)
{
	//check paras
	if (vec4D_grid.size() == 0 || vec4D_grid[0].size() == 0 || vec4D_grid[0][0].size() == 0 || vec4D_grid[0][0][0].size() != 3)
	{
		printf("ERROR: Invalid input grid size!\n");
		return false;
	}
	if (Spacing <= 1)
	{
		printf("ERROR: Invalid sz_gridwnd, it should >1!\n");
		return false;
	}
	if (x_bsplinebasis.ncols() != pow(4, 3))
	{
		printf("ERROR: Invalid input x_bsplinebasis size!\n");
		return false;
	}
	if (vec4D_grid_int.size() != 0)
	{
		printf("WARNNING: Output vec4D_grid_int is not empty, original data will be cleared!\n");
		vec4D_grid_int.clear();
	}


	long sz_grid[3];//w,h,z
	sz_grid[1] = vec4D_grid.size();
	sz_grid[0] = vec4D_grid[0].size();
	sz_grid[2] = vec4D_grid[0][0].size();

	long sz_grid_int[3];//w,h,z
	for (long i = 0; i<3; i++)
		sz_grid_int[i] = (sz_grid[i] - 3)*Spacing;
	
	vec4D_grid_int.assign(sz_grid_int[1], vector< vector< vector<double> > >(sz_grid_int[0], vector< vector<double> >(sz_grid_int[2], vector<double>(3, 0))));

#pragma omp parallel for
	for (long x = 0; x<sz_grid[0] - 3; x++)
		for (long y = 0; y<sz_grid[1] - 3; y++)
			for (long z = 0; z<sz_grid[2] - 3; z++)
				for (long xyz = 0; xyz<3; xyz++)
				{
					//vectorize the gridblock's nodes position that use for interpolation
					//(order is important!)
					Matrix x1D_gridblock(4 * 4 * 4, 1);
					long ind = 1;
					for (long dep = z; dep<z + 4; dep++)
						for (long col = x; col<x + 4; col++)
							for (long row = y; row<y + 4; row++)
							{
								x1D_gridblock(ind, 1) = vec4D_grid[row][col][dep][xyz];
								ind++;
							}

					//cubic B-spline interpolate the vectorized grid block
					Matrix x1D_gridblock_int = x_bsplinebasis*x1D_gridblock;

					//de-vectorize the interpolated grid block and save back to vec4D_grid_int
					ind = 1;
					for (long zz = 0; zz<Spacing; zz++)
						for (long xx = 0; xx<Spacing; xx++)
							for (long yy = 0; yy<Spacing; yy++)
							{
								vec4D_grid_int[y*Spacing + yy][x*Spacing + xx][z*Spacing + zz][xyz] = x1D_gridblock_int(ind, 1);
								ind++;
							}
				}

	return true;
}

bool q_nonrigid_interpolate_3D(const unsigned char *p_img8u_input, const long sz_img_input[4],
	const vector< vector< vector< vector<double> > > > &vec4D_grid_int,
	unsigned char *&p_img8u_output)
{
	//check paras
	if (p_img8u_input == 0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if (sz_img_input[0] <= 0 || sz_img_input[1] <= 0 || sz_img_input[2] <= 0 || sz_img_input[3] <= 0 || sz_img_input[3]>3)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if (vec4D_grid_int.size() != sz_img_input[1] || vec4D_grid_int[0].size() != sz_img_input[0] ||
		vec4D_grid_int[0][0].size() != sz_img_input[2] || vec4D_grid_int[0][0][0].size() != 3)
	{
		printf("ERROR: Invalid input grid size, it should be same as input image size!\n");
		return false;
	}
	if (p_img8u_output)
	{
		printf("WARNNING: Output image pointer is not empty, original data will be deleted!\n");
		if (p_img8u_output) 	{ delete[]p_img8u_output;		p_img8u_output = 0; }
	}

	//allocate memory
	p_img8u_output = new unsigned char[sz_img_input[0] * sz_img_input[1] * sz_img_input[2] * sz_img_input[3]]();
	if (!p_img8u_output)
	{
		printf("ERROR: Fail to allocate memory for warpped image!\n");
		return false;
	}

	unsigned char ****p_img_input_4d = 0, ****p_img_output_4d = 0;
	if (!new4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img8u_input) ||
		!new4dpointer(p_img_output_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3], p_img8u_output))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img8u_output) 		{ delete[]p_img8u_output;		p_img8u_output = 0; }
		if (p_img_input_4d) 		{ delete4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
		if (p_img_output_4d) 	{ delete4dpointer(p_img_output_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
		return false;
	}

	//linear interpolate the output image according to the given transformation field
	for (long x = 0; x<sz_img_input[0]; x++)
		for (long y = 0; y<sz_img_input[1]; y++)
			for (long z = 0; z<sz_img_input[2]; z++)
			{
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0] = vec4D_grid_int[y][x][z][0];
				cur_pos[1] = vec4D_grid_int[y][x][z][1];
				cur_pos[2] = vec4D_grid_int[y][x][z][2];

				//compensate the minor float type error (+-0) around 0
				if (fabs(cur_pos[0])<1e-10)	cur_pos[0] = 0.0;
				if (fabs(cur_pos[1])<1e-10)	cur_pos[1] = 0.0;
				if (fabs(cur_pos[2])<1e-10)	cur_pos[2] = 0.0;
				//if interpolate pixel is out of subject image region, set to -inf
				if (cur_pos[0]<0 || cur_pos[0]>sz_img_input[0] - 1 ||
					cur_pos[1]<0 || cur_pos[1]>sz_img_input[1] - 1 ||
					cur_pos[2]<0 || cur_pos[2]>sz_img_input[2] - 1)
				{
					p_img_output_4d[0][z][y][x] = 0.0;
					continue;
				}

				//find 8 neighor pixels boundary
				long x_s, x_b, y_s, y_b, z_s, z_b;
				x_s = floor(cur_pos[0]);		x_b = ceil(cur_pos[0]);
				y_s = floor(cur_pos[1]);		y_b = ceil(cur_pos[1]);
				z_s = floor(cur_pos[2]);		z_b = ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w, r_w, t_w, b_w;
				l_w = 1.0 - (cur_pos[0] - x_s);	r_w = 1.0 - l_w;
				t_w = 1.0 - (cur_pos[1] - y_s);	b_w = 1.0 - t_w;
				//compute weight for higer slice and lower slice
				double u_w, d_w;
				u_w = 1.0 - (cur_pos[2] - z_s);	d_w = 1.0 - u_w;

				//linear interpolate each channel
				for (long c = 0; c<sz_img_input[3]; c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice = t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s] + r_w*p_img_input_4d[c][z_s][y_s][x_b]) +
						b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s] + r_w*p_img_input_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice = t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s] + r_w*p_img_input_4d[c][z_b][y_s][x_b]) +
						b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s] + r_w*p_img_input_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					p_img_output_4d[c][z][y][x] = u_w*higher_slice + d_w*lower_slice + 0.5;
				}

			}

	//free memory
	if (p_img_input_4d) 		{ delete4dpointer(p_img_input_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }
	if (p_img_output_4d) 	{ delete4dpointer(p_img_output_4d, sz_img_input[0], sz_img_input[1], sz_img_input[2], sz_img_input[3]); }

	return true;
}
