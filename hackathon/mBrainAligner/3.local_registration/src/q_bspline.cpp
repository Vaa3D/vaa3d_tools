// q_bspline.cpp
// by Lei Qu
// 2012-07-23

#include "q_bspline.h"


#include <QtGui>
#include "../../basic_c_fun/basic_memory.cpp"


bool q_nonrigid_ini_bsplinebasis_3D(const long n,Matrix &BxBxB)
{
	//check paras
	if(n<=0)
	{
		printf("ERROR: n should > 0!\n");
		return false;
	}

	//cubic B-spline basis matrix
	Matrix B(4,4);
	B.row(1) << -1 << 3 <<-3 << 1;
	B.row(2) <<  3 <<-6 << 3 << 0;
	B.row(3) << -3 << 0 << 3 << 0;
	B.row(4) <<  1 << 4 << 1 << 0;
	B/=6.0;

	//construct T(i,:)=[t^3 t^2 t^1 1]
	Matrix T(n,4);
	double t_step=1.0/n;
	for(long i=0;i<n;i++)
	{
		double t=t_step*i;
		for(long j=0;j<=3;j++)
			T(i+1,j+1)=pow(t,3-j);
	}

	//construct B-spline basis/blending functions B=T*B
	Matrix TB=T*B;//n x 4

	//construct B-spline basis/blending functions for 2D interpolation B=BxB
	Matrix BxB=KP(TB,TB);//n^2 x 4^2
	//construct B-spline basis/blending functions for 3D interpolation B=BxBxB
	BxBxB=KP(BxB,TB);//n^3 x 4^3

	return true;
}

Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume_4bspline(const vector <Coord3D_JBA> & matchSubjectPos, const vector <Coord3D_JBA> &  matchTargetPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z)
{
	int nCpt = matchTargetPos.size();
	if (nCpt != matchSubjectPos.size() || nCpt <= 0)
	{
		fprintf(stderr, "The input vectors are invalid in compute_tps_df_field().\n");
		return 0;
	}

	Image2DSimple<MYFLOAT_JBA> * cpt_target = new Image2DSimple<MYFLOAT_JBA>(3, nCpt);
	Image2DSimple<MYFLOAT_JBA> * cpt_subject = new Image2DSimple<MYFLOAT_JBA>(3, nCpt);
	if (!cpt_target || !cpt_target->valid() || !cpt_subject || !cpt_subject->valid())
	{
		fprintf(stderr, "Fail to allocate memory.");
		if (cpt_target) { delete cpt_target; cpt_target = 0; }
		if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }
		return 0;
	}

	V3DLONG n;
	Matrix x4x4_d, xnx4_c, xnxn_K;
	if (xnx4_c.nrows() != nCpt || xnx4_c.ncols() != 4)
		xnx4_c.ReSize(nCpt, 4);
	if (x4x4_d.nrows() != 4 || xnx4_c.ncols() != 4)
		x4x4_d.ReSize(4, 4);
	if (xnxn_K.nrows() != nCpt || xnxn_K.ncols() != nCpt)
		xnxn_K.ReSize(nCpt, nCpt);

	MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
	MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();

	printf("\n---------------------------------\n");
	for (n = 0; n<nCpt; n++)
	{
		cpt_target_ref[n][0] = matchTargetPos.at(n).x;
		cpt_target_ref[n][1] = matchTargetPos.at(n).y;
		cpt_target_ref[n][2] = matchTargetPos.at(n).z;

		cpt_subject_ref[n][0] = matchSubjectPos.at(n).x;
		cpt_subject_ref[n][1] = matchSubjectPos.at(n).y;
		cpt_subject_ref[n][2] = matchSubjectPos.at(n).z;

		/*printf("n=%d \tx=[%5.3f -> %5.3f] y=[%5.3f -> %5.3f] z=[%5.3f -> %5.3f] \n",
		n, cpt_target_ref[n][0], cpt_subject_ref[n][0], cpt_target_ref[n][1], cpt_subject_ref[n][1], cpt_target_ref[n][2], cpt_subject_ref[n][2]);*/
	}
	printf("\n#################################\n");
	//compute K=-r=-|xi-xj|
	double d_x, d_y, d_z;
	for (unsigned V3DLONG i = 0; i<nCpt; i++)
		for (unsigned V3DLONG j = 0; j<nCpt; j++)
		{
			d_x = cpt_subject_ref[i][0] - cpt_subject_ref[j][0];
			d_y = cpt_subject_ref[i][1] - cpt_subject_ref[j][1];
			d_z = cpt_subject_ref[i][2] - cpt_subject_ref[j][2];
			xnxn_K(i + 1, j + 1) = -sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
		}



	Matrix X(nCpt, 4), Y(nCpt, 4);
	Matrix Q(nCpt, nCpt); Q = 0.0;
	for (V3DLONG i = 0; i<nCpt; i++)
	{
		Q(i + 1, 1) = X(i + 1, 1) = 1;
		Q(i + 1, 2) = X(i + 1, 2) = cpt_subject_ref[i][0];
		Q(i + 1, 3) = X(i + 1, 3) = cpt_subject_ref[i][1];
		Q(i + 1, 4) = X(i + 1, 4) = cpt_subject_ref[i][2];

		Y(i + 1, 1) = 1;
		Y(i + 1, 2) = cpt_target_ref[i][0];
		Y(i + 1, 3) = cpt_target_ref[i][1];
		Y(i + 1, 4) = cpt_target_ref[i][2];
	}
	UpperTriangularMatrix R;
	QRZ(Q, R);

	extend_orthonormal(Q, 4);//otherwise q2=0
	Matrix q1 = Q.columns(1, 4);
	Matrix q2 = Q.columns(5, nCpt);
	Matrix r = R.submatrix(1, 4, 1, 4);
	//compute non-affine term c which decomposed from TPS
	Matrix A = q2.t()*xnxn_K*q2 + IdentityMatrix(nCpt - 4)*0.5;
	xnx4_c = q2*(A.i()*q2.t()*Y);
	//compute affine term d (normal)
	x4x4_d = r.i()*q1.t()*(Y - xnxn_K*xnx4_c);


	/*Matrix wR(nCpt, nCpt);

	double tmp, s;

	V3DLONG i, j, k;
	for (j = 0; j<nCpt; j++)
	{
	for (i = 0; i<nCpt; i++)
	{
	s = 0.0;
	tmp = cpt_target_ref[i][0] - cpt_target_ref[j][0]; s += tmp*tmp;
	tmp = cpt_target_ref[i][1] - cpt_target_ref[j][1]; s += tmp*tmp;
	tmp = cpt_target_ref[i][2] - cpt_target_ref[j][2]; s += tmp*tmp;
	wR(i + 1, j + 1) = 2 * s*log(s + 1e-20);
	}
	}

	Matrix wP(nCpt, 4);
	for (j = 0; j<nCpt; j++)
	{
	wP(j + 1, 1) = 1;
	wP(j + 1, 2) = cpt_target_ref[j][0];
	wP(j + 1, 3) = cpt_target_ref[j][1];
	wP(j + 1, 4) = cpt_target_ref[j][2];
	}

	Matrix wL(nCpt + 4, nCpt + 4);
	wL.submatrix(1, nCpt, 1, nCpt) = wR;
	wL.submatrix(1, nCpt, nCpt + 1, nCpt + 4) = wP;
	wL.submatrix(nCpt + 1, nCpt + 4, 1, nCpt) = wP.t();
	wL.submatrix(nCpt + 1, nCpt + 4, nCpt + 1, nCpt + 4) = 0;

	Matrix wY(nCpt + 4, 3);
	for (j = 0; j<nCpt; j++)
	{
	wY(j + 1, 1) = cpt_subject_ref[j][0];
	wY(j + 1, 2) = cpt_subject_ref[j][1];
	wY(j + 1, 3) = cpt_subject_ref[j][2];
	}
	wY.submatrix(nCpt + 1, nCpt + 4, 1, 3) = 0;

	Matrix wW;

	Try
	{
	wW = wL.i() * wY;
	}
	CatchAll
	{
	fprintf(stderr, "Fail to find the inverse of the wL matrix.\n");

	if (cpt_target) { delete cpt_target; cpt_target = 0; }
	if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }
	return 0;
	}*/

	V3DLONG p;

	//	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0)/gfactor_x)))+1, gsz1 = (V3DLONG)(ceil((double(sz1)/gfactor_y)))+1, gsz2 = (V3DLONG)(ceil((double(sz2)/gfactor_z)))+1;
	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0) / gfactor_x))) + 1 + 2, gsz1 = (V3DLONG)(ceil((double(sz1) / gfactor_y))) + 1 + 2, gsz2 = (V3DLONG)(ceil((double(sz2) / gfactor_z))) + 1 + 2;//+2 for bspline
	Vol3DSimple<DisplaceFieldF3D> * df_local = new Vol3DSimple<DisplaceFieldF3D>(gsz0, gsz1, gsz2);
	DisplaceFieldF3D *** df_local_3d = df_local->getData3dHandle();

	if (!df_local || !df_local->valid())
	{
		fprintf(stderr, "Fail to allocate memory for the subsampled DF volume memory [%d].\n", __LINE__);

		if (cpt_target) { delete cpt_target; cpt_target = 0; }
		if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }
		if (df_local) { delete df_local; df_local = 0; }
		return 0;
	}

	/*V3DLONG ndimpt = 3;
	double * dist = new double[nCpt + ndimpt + 1];
	if (!dist)
	{
	fprintf(stderr, "Fail to allocate memory dist for tps warping [%d].\n", __LINE__);

	if (cpt_target) { delete cpt_target; cpt_target = 0; }
	if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }
	if (df_local) { delete df_local; df_local = 0; }
	return 0;
	}*/

	printf("-------------------- Now compute the distances of pixels to the mapping points. -------\n\n");

	V3DLONG i, j, k;
	DisplaceFieldF3D * df_local_1d = df_local->getData1dHandle();
	for (k = 0; k<df_local->getTotalElementNumber(); k++)
	{
		df_local_1d[k].sz = df_local_1d[k].sy = df_local_1d[k].sx = 0;
	}
	for (k = 0; k<gsz2; k++)
	{
		for (j = 0; j<gsz1; j++)
		{
			for (i = 0; i<gsz0; i++)
			{

				Matrix x_ori(1, 4);
				x_ori(1, 1) = 1.0;
				x_ori(1, 2) = (i - 1)*gfactor_x;
				x_ori(1, 3) = (j - 1)*gfactor_y;
				x_ori(1, 4) = (k - 1)*gfactor_z;

				Matrix x_stps(1, 4);

				Matrix xmxn_K;
				xmxn_K.resize(1, nCpt);
				double d_x, d_y, d_z;
				for (unsigned V3DLONG n = 0; n<nCpt; n++)
				{
					d_x = (i - 1)*gfactor_x - cpt_subject_ref[n][0];
					d_y = (j - 1)*gfactor_y - cpt_subject_ref[n][1];
					d_z = (k - 1)*gfactor_z - cpt_subject_ref[n][2];
					xmxn_K(1, n + 1) = -sqrt(d_x*d_x + d_y*d_y + d_z*d_z);
				}
				x_stps = x_ori*x4x4_d + xmxn_K*xnx4_c;

				df_local_3d[k][j][i].sx = x_stps(1, 2) - (i - 1)*gfactor_x;
				df_local_3d[k][j][i].sy = x_stps(1, 3) - (j - 1)*gfactor_y;
				df_local_3d[k][j][i].sz = x_stps(1, 4) - (k - 1)*gfactor_z;

			}//i
		}//j
		printf("z=%ld ", k); fflush(stdout);
	}//k
	printf("\n");

	//if (dist) { delete[]dist; dist = 0; }
	if (cpt_target) { delete cpt_target; cpt_target = 0; }
	if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }

	return df_local;
}

