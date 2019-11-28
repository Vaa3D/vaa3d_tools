#include <v3d_interface.h>
//#include "vn_imgpreprocess.h"
#include "../../../v3d_main/v3d/compute_win_pca_wp.h"

#include<time.h>
#include<Windows.h>
//template <class T> bool grayValue_at_xyz(T *indata1d, int x, int y, int z, int c, int sx, int sy, int sz, int cc);
//template <class T> bool grayValue_at_xyz(T ***indata3d, int x, int y, int z);
//template <class T> bool compute_rgn_stat_new(LocationSimple & pt, T *indata1d, int channo, int sx, int sy, int sz, int sc, int datatype);




//template <class T> bool grayValue_at_xyz(T *indata1d, int x, int y, int z, int c, int sx, int sy, int sz, int cc){
//	return double(indata1d[z*sx*sy + y*sx + x]);
//}

template <class T> bool wpshowDirection(T vec1[], T vec2[], T vec3[], double sigma1, double sigma2, double sigma3,LocationSimple & pt)
{
	QList<ImageMarker> markers;
	double mx, my, mz;
	int scale = 5;
	for (int i = -int(sigma1 / sigma3)*scale; i<int(sigma1 / sigma3)*scale; i++)
	{
		mx = pt.x + i*vec1[0];
		my = pt.y + i*vec1[1];
		mz = pt.z + i*vec1[2];
		ImageMarker m(mx, my, mz);
		m.color.r = 0;
		m.color.g = 0;
		m.color.b = 255;
		m.radius = 1;
		markers.push_back(m);
	}

	for (int i = -int(sigma2 / sigma3)*scale; i<int(sigma2 / sigma3)*scale; i++)
	{
		mx = pt.x + i*vec2[0];
		my = pt.y + i*vec2[1];
		mz = pt.z + i*vec2[2];
		ImageMarker m(mx, my, mz);
		m.color.r = 0;
		m.color.g = 255;
		m.color.b = 0;
		m.radius = 1;
		markers.push_back(m);
	}

	for (int i = -int(sigma3 / sigma3)*scale; i<int(sigma3 / sigma3)*scale; i++)
	{
		mx = pt.x + i*vec3[0];
		my = pt.y + i*vec3[1];
		mz = pt.z + i*vec3[2];
		ImageMarker m(mx, my, mz);
		m.color.r = 255;
		m.color.g = 0;
		m.color.b = 0;
		m.radius = 1;
		markers.push_back(m);
	}
	writeMarker_file("C:/Users/wpkenan/Desktop/data/result.marker", markers);

	return true;
}

template <class T> bool grayValue_at_xyz(T ***indata3d, int x, int y, int z){
	return double(indata3d[z][y][x]);
}

template <class T> bool compute_rgn_stat_new(LocationSimple & pt, T *indata1d, int channo, 
	int sx, int sy, int sz, int sc, 
	int datatype,
	double vec1[], double vec2[], double vec3[],
	double &sigma1, double &sigma2, double &sigma3)
{
	//int datatype = p4dImage->getDatatype();
	//unsigned char * indata1d = p4dImage->getRawDataAtChannel(0);

	
	//cout << "wp_debug: " << __FUNCTION__ << " " << __LINE__ << endl;
	//cout << pt.x << " " << pt.y << " " << pt.z << " " << pt.radius << endl;
	T ***indata3d;
	indata3d = (T ***)malloc(sz*sizeof(T**));
	for (int i = 0; i < sz; i++){
		indata3d[i] = (T **)malloc(sy*sizeof(T*));
		for (int j = 0; j < sy; j++){
			indata3d[i][j] = (T *)malloc(sx*sizeof(T));
		}
	}

	//cout << sx << " " << sy << " " << sz << endl;
	//cout << "wp_debug: " << __FUNCTION__ << " " << __LINE__ << endl;
	for (int z = 0; z < sz; z++){
		for (int y = 0; y < sy; y++){
			for (int x = 0; x < sx; x++){
				indata3d[z][y][x] = indata1d[z*sy*sx + y*sx + x];
				//cout << i << " " << j << " " << k << endl;
			}
		}
	}

	//cout << "wp_debug: " << __FUNCTION__ << " " << __LINE__ << endl;
	V3DLONG xx = V3DLONG(pt.x + 0.5);
	V3DLONG yy = V3DLONG(pt.y + 0.5);
	V3DLONG zz = V3DLONG(pt.z + 0.5);
	//V3DLONG cc = channo; if (cc<0) cc = 0; if (cc >= sc) cc = sc - 1;
	V3DLONG rr = pt.radius; if (rr<0) rr = 0;
	PxLocationMarkerShape ss = pt.shape;

	//now do the computation
	int res_peak = 0;
	double res_mean = 0, res_std = 0;
	double res_size = 0, res_mass = 0;
	V3DLONG i, j, k;
	XYZ res_mcenter; //mass center
	double res_xI = 0, res_yI = 0, res_zI = 0; // for mass center computation

	V3DLONG xs, xe, ys, ye, zs, ze;
	xs = xx - rr; if (xs<0) xs = 0;
	xe = xx + rr; if (xe >= sx) xe = sx - 1;
	ys = yy - rr; if (ys<0) ys = 0;
	ye = yy + rr; if (ye >= sy) ye = sy - 1;
	zs = zz - rr; if (zs<0) zs = 0;
	ze = zz + rr; if (ze >= sz) ze = sz - 1;
	double r2 = double(rr + 1)*(rr + 1);

	switch (ss)
	{
	case pxSphere:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCube:
		for (k = zs; k <= ze; k++)
			for (j = ys; j <= ye; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleX:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xx; i <= xx; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleY:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = yy; j <= yy; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleZ:
		for (k = zz; k <= zz; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareX:
		for (k = zs; k <= ze; k++)
			for (j = ys; j <= ye; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareY:
		for (k = zs; k <= ze; k++)
			for (j = yy; j <= yy; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareZ:
		for (k = zz; k <= zz; k++)
			for (j = ys; j <= ye; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineX:
		for (k = zz; k <= zz; k++)
			for (j = yy; j <= yy; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineY:
		for (k = zz; k <= zz; k++)
			for (j = ys; j <= ye; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineZ:
		for (k = zs; k <= ze; k++)
			for (j = yy; j <= yy; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxDot:
		for (k = zz; k <= zz; k++)
			for (j = yy; j <= yy; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxUnset:
	case pxTriangle:
	default:
		printf("Not supported shape. Not compute.\n");
		return false;
		break;
	}

	//now compute the eigen value info

	int b_win_shape = 1; //0 for cube and 1 for sphere
	bool b_disp_CoM_etc = false; //if display center of mass and covariance info



	/*double vec1[3], vec2[3], vec3[3];*/


	/*cout << sx << " " << sy << " " << sz << endl;
	cout << xx << " " << yy << " " << zz << endl;
	cout << rr << endl;
	*/
	//cout << "sigma: " << " " << pt.ev_pc1 << " " << pt.ev_pc2 << " " << pt.ev_pc3 << endl;

	compute_win3d_pca_eigVec(indata3d, 
		sx, sy, sz,
		xx, yy, zz,
		rr, rr, rr,
		pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, 
		vec1, vec2, vec3, 
		b_win_shape, b_disp_CoM_etc);


	bool b_compute_all_radius = false;
	if (b_compute_all_radius)
	{
		for (i = 0; i <= rr; i++)
		{
			bool b_continue = true;


			compute_cube_win3d_pca_eigVec(indata3d, 
				sx, sy, sz,
				xx, yy, zz,
				i, i, i,
				pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, 
				vec1, vec2, vec3, 
				b_win_shape, b_disp_CoM_etc);

			if (!b_continue)
				break;

			double Lscore = exp(-((pt.ev_pc1 - pt.ev_pc2)*(pt.ev_pc1 - pt.ev_pc2) + (pt.ev_pc2 - pt.ev_pc3)*(pt.ev_pc2 - pt.ev_pc3) + (pt.ev_pc1 - pt.ev_pc3)*(pt.ev_pc1 - pt.ev_pc3)) /
				(pt.ev_pc1*pt.ev_pc1 + pt.ev_pc2*pt.ev_pc2 + pt.ev_pc3*pt.ev_pc3));
			double s_linear = (pt.ev_pc1 - pt.ev_pc2) / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			double s_planar = 2.0*(pt.ev_pc2 - pt.ev_pc3) / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			double s_sphere = 3.0*pt.ev_pc3 / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			printf("r=%d \t lamba1=%5.3f lamba2=%5.3f lamba3=%5.3f L_score=%5.3f linear_c=%5.3f planar_c=%5.3f spherical_c=%5.3f\n", i, pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, Lscore, s_linear, s_planar, s_sphere);
		}
	}

	
	cout << pt.ev_pc1 << "," << pt.ev_pc2 << "," << pt.ev_pc3 << endl;
	sigma1 = pt.ev_pc1;
	sigma2 = pt.ev_pc2;
	sigma3 = pt.ev_pc3;


	for (int i = 0; i < 3; i++){
		cout << vec1[i] << ",";
	}
	cout << endl;
	for (int i = 0; i < 3; i++){
		cout << vec2[i] << ",";
	}
	cout << endl;
	for (int i = 0; i < 3; i++){
		cout << vec3[i] << ",";
	}
	cout << endl;

	for (int i = 0; i < sz; i++){
		for (int j = 0; j < sy; j++){
			delete indata3d[i][j];
		}
		delete indata3d[i];
	}
	delete indata3d;

	//now update the value of the respective


	/*QString tmp;
	pt.pixmax = res_peak;
	pt.ave = res_mean;
	pt.sdev = res_std;
	pt.size = res_size;
	pt.mass = res_mass;
	pt.mcenter = res_mcenter;
*/
	//no need to update pt's ev_pc field as they have been updated

	return true;
}


template <class T> bool compute_rgn_stat_new(LocationSimple & pt, T ***indata3d, int channo, 
	int sx, int sy, int sz, int sc, 
	int datatype,
	double vec1[], double vec2[],double vec3[],
	double &sigma1, double &sigma2, double &sigma3)
{

	//DWORD startTime, endTime1, endTime2;
	//startTime = GetTickCount();

	//cout << "wp_debug: " << __FUNCTION__ << " " << __LINE__ << endl;
	V3DLONG xx = V3DLONG(pt.x + 0.5);
	V3DLONG yy = V3DLONG(pt.y + 0.5);
	V3DLONG zz = V3DLONG(pt.z + 0.5);
	//V3DLONG cc = channo; if (cc<0) cc = 0; if (cc >= sc) cc = sc - 1;
	V3DLONG rr = pt.radius; if (rr<0) rr = 0;
	PxLocationMarkerShape ss = pt.shape;

	//now do the computation
	int res_peak = 0;
	double res_mean = 0, res_std = 0;
	double res_size = 0, res_mass = 0;
	V3DLONG i, j, k;
	XYZ res_mcenter; //mass center
	double res_xI = 0, res_yI = 0, res_zI = 0; // for mass center computation

	V3DLONG xs, xe, ys, ye, zs, ze;
	xs = xx - rr; if (xs<0) xs = 0;
	xe = xx + rr; if (xe >= sx) xe = sx - 1;
	ys = yy - rr; if (ys<0) ys = 0;
	ye = yy + rr; if (ye >= sy) ye = sy - 1;
	zs = zz - rr; if (zs<0) zs = 0;
	ze = zz + rr; if (ze >= sz) ze = sz - 1;
	double r2 = double(rr + 1)*(rr + 1);

	switch (ss)
	{
	case pxSphere:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCube:
		for (k = zs; k <= ze; k++)
			for (j = ys; j <= ye; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleX:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xx; i <= xx; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleY:
		for (k = zs; k <= ze; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = yy; j <= yy; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxCircleZ:
		for (k = zz; k <= zz; k++)
		{
			double cur_dk = (k - zz)*(k - zz);
			for (j = ys; j <= ye; j++)
			{
				double cur_dj = (j - yy)*(j - yy);
				double cur_d = cur_dk + cur_dj;
				if (cur_d>r2) continue;
				for (i = xs; i <= xe; i++)
				{
					double cur_di = (i - xx)*(i - xx);
					cur_d = cur_dk + cur_dj + cur_di;
					if (cur_d>r2) continue;

					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
			}
		}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareX:
		for (k = zs; k <= ze; k++)
			for (j = ys; j <= ye; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareY:
		for (k = zs; k <= ze; k++)
			for (j = yy; j <= yy; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxSquareZ:
		for (k = zz; k <= zz; k++)
			for (j = ys; j <= ye; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineX:
		for (k = zz; k <= zz; k++)
			for (j = yy; j <= yy; j++)
				for (i = xs; i <= xe; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineY:
		for (k = zz; k <= zz; k++)
			for (j = ys; j <= ye; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxLineZ:
		for (k = zs; k <= ze; k++)
			for (j = yy; j <= yy; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxDot:
		for (k = zz; k <= zz; k++)
			for (j = yy; j <= yy; j++)
				for (i = xx; i <= xx; i++)
				{
					double cur_v = grayValue_at_xyz(indata3d, i, j, k);
					if (res_peak<cur_v) res_peak = cur_v;
					res_size++;
					res_mass += cur_v;
					res_std += cur_v*cur_v; //use the incremental formula
					res_xI += i*cur_v;
					res_yI += j*cur_v;
					res_zI += k*cur_v;
				}
		res_mean = res_mass / res_size;
		res_std = sqrt(res_std / res_size - res_mean*res_mean);
		res_mcenter.x = res_xI / res_mass;
		res_mcenter.y = res_yI / res_mass;
		res_mcenter.z = res_zI / res_mass;
		break;

	case pxUnset:
	case pxTriangle:
	default:
		printf("Not supported shape. Not compute.\n");
		return false;
		break;
	}

	//now compute the eigen value info

	int b_win_shape = 1; //0 for cube and 1 for sphere
	bool b_disp_CoM_etc = false; //if display center of mass and covariance info

	//double vec1[3], vec2[3], vec3[3];


	/*cout << sx << " " << sy << " " << sz << endl;
	cout << xx << " " << yy << " " << zz << endl;
	cout << rr << endl;
	*/
	//cout << "sigma: " << " " << pt.ev_pc1 << " " << pt.ev_pc2 << " " << pt.ev_pc3 << endl;

	//endTime1 = GetTickCount();
	//cout << "wp_debug: " << __FUNCTION__ << " " << __LINE__ << endl;
	
	compute_win3d_pca_eigVec(indata3d,
		sx, sy, sz,
		xx, yy, zz,
		rr, rr, rr,
		pt.ev_pc1, pt.ev_pc2, pt.ev_pc3,
		vec1, vec2, vec3,
		b_win_shape, b_disp_CoM_etc);
	

	//_sleep(1000);
	//endTime2 = GetTickCount();

	//cout << "time: " << (endTime1 - startTime)  << " " << (endTime2 - endTime2) << endl;
	bool b_compute_all_radius = false;
	if (b_compute_all_radius)
	{
		for (i = 0; i <= rr; i++)
		{
			bool b_continue = true;


			compute_cube_win3d_pca_eigVec(indata3d,
				sx, sy, sz,
				xx, yy, zz,
				i, i, i,
				pt.ev_pc1, pt.ev_pc2, pt.ev_pc3,
				vec1, vec2, vec3,
				b_win_shape, b_disp_CoM_etc);

			if (!b_continue)
				break;

			double Lscore = exp(-((pt.ev_pc1 - pt.ev_pc2)*(pt.ev_pc1 - pt.ev_pc2) + (pt.ev_pc2 - pt.ev_pc3)*(pt.ev_pc2 - pt.ev_pc3) + (pt.ev_pc1 - pt.ev_pc3)*(pt.ev_pc1 - pt.ev_pc3)) /
				(pt.ev_pc1*pt.ev_pc1 + pt.ev_pc2*pt.ev_pc2 + pt.ev_pc3*pt.ev_pc3));
			double s_linear = (pt.ev_pc1 - pt.ev_pc2) / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			double s_planar = 2.0*(pt.ev_pc2 - pt.ev_pc3) / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			double s_sphere = 3.0*pt.ev_pc3 / (pt.ev_pc1 + pt.ev_pc2 + pt.ev_pc3);
			printf("r=%d \t lamba1=%5.3f lamba2=%5.3f lamba3=%5.3f L_score=%5.3f linear_c=%5.3f planar_c=%5.3f spherical_c=%5.3f\n", i, pt.ev_pc1, pt.ev_pc2, pt.ev_pc3, Lscore, s_linear, s_planar, s_sphere);
		}
	}


	sigma1 = pt.ev_pc1;
	sigma2 = pt.ev_pc2;
	sigma3 = pt.ev_pc3;

	/*cout << pt.ev_pc1 << "," << pt.ev_pc2 << "," << pt.ev_pc3 << endl;
	for (int i = 0; i < 3; i++){
	cout << vec1[i] << ",";
	}
	cout << endl;
	for (int i = 0; i < 3; i++){
	cout << vec2[i] << ",";
	}
	cout << endl;
	for (int i = 0; i < 3; i++){
	cout << vec3[i] << ",";
	}
	cout << endl;*/



	//now update the value of the respective


	/*QString tmp;
	pt.pixmax = res_peak;
	pt.ave = res_mean;
	pt.sdev = res_std;
	pt.size = res_size;
	pt.mass = res_mass;
	pt.mcenter = res_mcenter;*/

	//no need to update pt's ev_pc field as they have been updated

	return true;
}

