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

Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume_4bspline(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
																 V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z)
{
	int nCpt = matchTargetPos.size();
	if (nCpt!=matchSubjectPos.size() || nCpt<=0)
	{
		fprintf(stderr, "The input vectors are invalid in compute_tps_df_field().\n");
		return 0;
	}

	Image2DSimple<MYFLOAT_JBA> * cpt_target = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	Image2DSimple<MYFLOAT_JBA> * cpt_subject = new Image2DSimple<MYFLOAT_JBA> (3, nCpt);
	if (!cpt_target || !cpt_target->valid() || !cpt_subject || !cpt_subject->valid())
	{
		fprintf(stderr, "Fail to allocate memory.");
		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		return 0;
	}

    V3DLONG n;

    MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
    MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();
	printf("\n---------------------------------\n");
    for (n=0;n<nCpt;n++)
    {
		cpt_target_ref[n][0] = matchTargetPos.at(n).x;
		cpt_target_ref[n][1] = matchTargetPos.at(n).y;
		cpt_target_ref[n][2] = matchTargetPos.at(n).z;

		cpt_subject_ref[n][0] = matchSubjectPos.at(n).x;
		cpt_subject_ref[n][1] = matchSubjectPos.at(n).y;
		cpt_subject_ref[n][2] = matchSubjectPos.at(n).z;

		printf("n=%d \tx=[%5.3f -> %5.3f] y=[%5.3f -> %5.3f] z=[%5.3f -> %5.3f] \n",
		       n, cpt_target_ref[n][0], cpt_subject_ref[n][0], cpt_target_ref[n][1], cpt_subject_ref[n][1], cpt_target_ref[n][2], cpt_subject_ref[n][2]);
    }
	printf("\n#################################\n");

	Matrix wR(nCpt, nCpt);

	double tmp, s;

	V3DLONG i, j, k;
	for (j=0; j<nCpt; j++)
	{
		for (i=0; i<nCpt;i++)
		{
			s = 0.0;
			tmp = cpt_target_ref[i][0] - cpt_target_ref[j][0]; s += tmp*tmp;
			tmp = cpt_target_ref[i][1] - cpt_target_ref[j][1]; s += tmp*tmp;
			tmp = cpt_target_ref[i][2] - cpt_target_ref[j][2]; s += tmp*tmp;
			wR(i+1,j+1) = 2*s*log(s+1e-20);
		}
	}

	Matrix wP(nCpt, 4);
	for (j=0; j<nCpt; j++)
	{
		wP(j+1,1) = 1;
		wP(j+1,2) = cpt_target_ref[j][0];
		wP(j+1,3) = cpt_target_ref[j][1];
		wP(j+1,4) = cpt_target_ref[j][2];
	}

	Matrix wL(nCpt+4, nCpt+4);
	wL.submatrix(1, nCpt, 1, nCpt) = wR;
	wL.submatrix(1, nCpt, nCpt+1, nCpt+4) = wP;
	wL.submatrix(nCpt+1, nCpt+4, 1, nCpt) = wP.t();
	wL.submatrix(nCpt+1, nCpt+4, nCpt+1, nCpt+4) = 0;

	Matrix wY(nCpt+4, 3);
	for (j=0; j<nCpt; j++)
	{
		wY(j+1,1) = cpt_subject_ref[j][0];
		wY(j+1,2) = cpt_subject_ref[j][1];
		wY(j+1,3) = cpt_subject_ref[j][2];
	}
	wY.submatrix(nCpt+1, nCpt+4, 1, 3) = 0;

	Matrix wW;

	Try
	{
		wW = wL.i() * wY;
	}
	CatchAll
	{
		fprintf(stderr, "Fail to find the inverse of the wL matrix.\n");

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		return 0;
	}

	V3DLONG p;

//	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0)/gfactor_x)))+1, gsz1 = (V3DLONG)(ceil((double(sz1)/gfactor_y)))+1, gsz2 = (V3DLONG)(ceil((double(sz2)/gfactor_z)))+1;
	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0)/gfactor_x)))+1+2, gsz1 = (V3DLONG)(ceil((double(sz1)/gfactor_y)))+1+2, gsz2 = (V3DLONG)(ceil((double(sz2)/gfactor_z)))+1+2;//+2 for bspline
	Vol3DSimple<DisplaceFieldF3D> * df_local = new Vol3DSimple<DisplaceFieldF3D> (gsz0, gsz1, gsz2);
	DisplaceFieldF3D *** df_local_3d = df_local->getData3dHandle();

	if (!df_local || !df_local->valid())
	{
		fprintf(stderr, "Fail to allocate memory for the subsampled DF volume memory [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    V3DLONG ndimpt = 3;
    double * dist = new double [nCpt+ndimpt+1];
	if (!dist)
	{
		fprintf(stderr, "Fail to allocate memory dist for tps warping [%d].\n", __LINE__);

		if (cpt_target) {delete cpt_target; cpt_target=0;}
		if (cpt_subject) {delete cpt_subject; cpt_subject=0;}
		if (df_local) {delete df_local; df_local=0;}
		return 0;
	}

    printf("-------------------- Now compute the distances of pixels to the mapping points. -------\n\n");

	DisplaceFieldF3D * df_local_1d = df_local->getData1dHandle();
	for (k=0;k<df_local->getTotalElementNumber();k++)
	{
		df_local_1d[k].sz = df_local_1d[k].sy = df_local_1d[k].sx = 0;
	}
	for (k=0;k<gsz2;k++)
	{
		for (j=0;j<gsz1;j++)
		{
			for (i=0;i<gsz0;i++)
			{
				for (n=0;n<nCpt; n++)
				{
					s = 0;
//					tmp = (i*gfactor_x)-cpt_target_ref[n][0]; s += tmp*tmp;
//					tmp = (j*gfactor_y)-cpt_target_ref[n][1]; s += tmp*tmp;
//					tmp = (k*gfactor_z)-cpt_target_ref[n][2]; s += tmp*tmp;
					tmp = ((i-1)*gfactor_x)-cpt_target_ref[n][0]; s += tmp*tmp;
					tmp = ((j-1)*gfactor_y)-cpt_target_ref[n][1]; s += tmp*tmp;
					tmp = ((k-1)*gfactor_z)-cpt_target_ref[n][2]; s += tmp*tmp;
					dist[n] = 2*s*log(s+1e-20);
				}

				dist[nCpt] = 1;
				dist[nCpt+1] = (i-1)*gfactor_x;
				dist[nCpt+2] = (j-1)*gfactor_y;
				dist[nCpt+3] = (k-1)*gfactor_z;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 1);}
				df_local_3d[k][j][i].sx = s - (i-1)*gfactor_x;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 2);}
				df_local_3d[k][j][i].sy = s - (j-1)*gfactor_y;

				s = 0;  for (p=0; p<nCpt+ndimpt+1; p++) {s += dist[p]*wW(p+1, 3);}
				df_local_3d[k][j][i].sz = s - (k-1)*gfactor_z;
			}//i
		}//j
		printf("z=%ld ",k); fflush(stdout);
	}//k
	printf("\n");

	if (dist) {delete []dist; dist=0;}
	if (cpt_target) {delete cpt_target; cpt_target=0;}
	if (cpt_subject) {delete cpt_subject; cpt_subject=0;}

	return df_local;
}
