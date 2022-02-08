//imgwarp_tps_quicksmallmemory.cpp
//
// by Lei Qu
//2012-07-08

#include "q_imgwarp_tps_quicksmallmemory.h"


//warp image block based on given DF
//use 3d or 4d pointer instead of 1d, since generating 3d or 4d pointer from 1d is time consuming
//template <class T>
bool q_imgblockwarp(unsigned char ****&p_img_sub_4d, const V3DLONG *sz_img_sub, DisplaceFieldF3D ***&pppDFBlock,
		const V3DLONG szBlock_x,const V3DLONG szBlock_y,const V3DLONG szBlock_z,const int i_interpmethod_img,
		const V3DLONG substart_x,const V3DLONG substart_y,const V3DLONG substart_z,
		unsigned char ****&p_img_warp_4d)
{
	V3DLONG start_x,start_y,start_z;
	start_x=substart_x*szBlock_x;
	start_y=substart_y*szBlock_y;
	start_z=substart_z*szBlock_z;
	for(V3DLONG z=0;z<szBlock_z;z++)
		for(V3DLONG y=0;y<szBlock_y;y++)
			for(V3DLONG x=0;x<szBlock_x;x++)
			{
				V3DLONG pos_warp[3];
				pos_warp[0]=start_x+x;
				pos_warp[1]=start_y+y;
				pos_warp[2]=start_z+z;
				if(pos_warp[0]>=sz_img_sub[0] || pos_warp[1]>=sz_img_sub[1] || pos_warp[2]>=sz_img_sub[2])
					continue;

				double pos_sub[3];
				pos_sub[0]=pos_warp[0]+pppDFBlock[z][y][x].sx;
				pos_sub[1]=pos_warp[1]+pppDFBlock[z][y][x].sy;
				pos_sub[2]=pos_warp[2]+pppDFBlock[z][y][x].sz;
				if(pos_sub[0]<0 || pos_sub[0]>sz_img_sub[0]-1 ||
				   pos_sub[1]<0 || pos_sub[1]>sz_img_sub[1]-1 ||
				   pos_sub[2]<0 || pos_sub[2]>sz_img_sub[2]-1)
				{
					for(V3DLONG c=0;c<sz_img_sub[3];c++)
						p_img_warp_4d[c][pos_warp[2]][pos_warp[1]][pos_warp[0]]=0;
					continue;
				}

				//nearest neighbor interpolate
				if(i_interpmethod_img==1)
				{
					V3DLONG pos_sub_nn[3];
					for(int i=0;i<3;i++)
					{
						pos_sub_nn[i]=pos_sub[i]+0.5;
						pos_sub_nn[i]=pos_sub_nn[i]<sz_img_sub[i]?pos_sub_nn[i]:sz_img_sub[i]-1;
					}
					for(V3DLONG c=0;c<sz_img_sub[3];c++)
						p_img_warp_4d[c][pos_warp[2]][pos_warp[1]][pos_warp[0]]=p_img_sub_4d[c][pos_sub_nn[2]][pos_sub_nn[1]][pos_sub_nn[0]];
				}
				//linear interpolate
				else if(i_interpmethod_img==0)
				{
					//find 8 neighor pixels boundary
					V3DLONG x_s,x_b,y_s,y_b,z_s,z_b;
					x_s=floor(pos_sub[0]);		x_b=ceil(pos_sub[0]);
					y_s=floor(pos_sub[1]);		y_b=ceil(pos_sub[1]);
					z_s=floor(pos_sub[2]);		z_b=ceil(pos_sub[2]);

					//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
					double l_w,r_w,t_w,b_w;
					l_w=1.0-(pos_sub[0]-x_s);	r_w=1.0-l_w;
					t_w=1.0-(pos_sub[1]-y_s);	b_w=1.0-t_w;
					//compute weight for higer slice and lower slice
					double u_w,d_w;
					u_w=1.0-(pos_sub[2]-z_s);	d_w=1.0-u_w;

					//linear interpolate each channel
					for(V3DLONG c=0;c<sz_img_sub[3];c++)
					{
						//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double higher_slice;
						higher_slice=t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s]+r_w*p_img_sub_4d[c][z_s][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s]+r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
						//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double lower_slice;
						lower_slice =t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s]+r_w*p_img_sub_4d[c][z_b][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s]+r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
						//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
						double intval = (u_w*higher_slice + d_w*lower_slice + 0.5);
						p_img_warp_4d[c][pos_warp[2]][pos_warp[1]][pos_warp[0]]=intval;
					}
				}

			}

	return true;
}


//TPS_linear_blockbyblock image warping
//	i_interp_method_df:  0-trilinear, 1-bspline
//	i_interp_method_img: 0-trilinear, 1-nearest neighbor
//template <class T>
bool imgwarp_smallmemory(unsigned char *p_img_sub, const V3DLONG *sz_img_sub,
		const QList<ImageMarker> &ql_marker_tar,const QList<ImageMarker> &ql_marker_sub,
		V3DLONG szBlock_x, V3DLONG szBlock_y, V3DLONG szBlock_z, int i_interpmethod_df,int i_interpmethod_img,
		unsigned char *&p_img_warp, Matrix &x4x4_affine, Matrix &xnx4_c, Matrix &xnxn_K)
{
	//check parameters
	if(p_img_sub==0 || sz_img_sub==0)
	{
		printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
		return false;
	}
	if(ql_marker_tar.size()==0 || ql_marker_sub.size()==0 || ql_marker_tar.size()!=ql_marker_sub.size())
	{
		printf("ERROR: target or subject control points is invalid!\n");
		return false;
	}
	if(szBlock_x<=0 || szBlock_y<=0 || szBlock_z<=0)
	{
		printf("ERROR: block size is invalid!\n");
		return false;
	}
	if(szBlock_x>=sz_img_sub[0] || szBlock_y>=sz_img_sub[1] || szBlock_z>=sz_img_sub[2])
	{
		printf("ERROR: block size should smaller than the image size!\n");
		return false;
	}
	if(i_interpmethod_df!=0 && i_interpmethod_df!=1)
	{
		printf("ERROR: DF_interp_method should be 0(linear) or 1(bspline)!\n");
		return false;
	}
	if(i_interpmethod_img!=0 && i_interpmethod_img!=1)
	{
		printf("ERROR: img_interp_method should be 0(linear) or 1(nn)!\n");
		return false;
	}
	if(i_interpmethod_df==1 && (szBlock_x!=szBlock_y || szBlock_x!=szBlock_z))
	{
		printf("ERROR: df_interp_method=bspline need szBlock_x=szBlock_y=szBlock_z!\n");
		return false;
	}
	if(p_img_warp)
	{
		printf("WARNNING: output image pointer is not null, original memeroy it point to will be released!\n");
		if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>>>compute the subsampled displace field \n");
    vector<Coord3D_JBA> matchTargetPos, matchSubjectPos;
	for (V3DLONG i=0;i<ql_marker_tar.size();i++)
	{
		Coord3D_JBA tmpc;
		tmpc.x=ql_marker_tar.at(i).x;	tmpc.y=ql_marker_tar.at(i).y;	tmpc.z=ql_marker_tar.at(i).z;
		matchTargetPos.push_back(tmpc);
		tmpc.x=ql_marker_sub.at(i).x;	tmpc.y=ql_marker_sub.at(i).y;	tmpc.z=ql_marker_sub.at(i).z;
		matchSubjectPos.push_back(tmpc);
	}
	Vol3DSimple<DisplaceFieldF3D> *pSubDF=0;
	if(i_interpmethod_df==0)
		pSubDF=compute_df_tps_subsampled_volume(matchTargetPos,matchSubjectPos,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],szBlock_x,szBlock_y,szBlock_z);
	else
		pSubDF = compute_df_stps_subsampled_volume_4bspline(matchTargetPos, matchSubjectPos, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], szBlock_x, szBlock_y, szBlock_z, x4x4_affine, xnx4_c, xnxn_K);
	if(!pSubDF)
	{
		printf("Fail to produce the subsampled DF.\n");
		return false;
	}
	DisplaceFieldF3D ***pppSubDF=pSubDF->getData3dHandle();
	//printf("subsampled DF size: [%ld,%ld,%ld]\n",pSubDF->sz0(),pSubDF->sz1(),pSubDF->sz2());

	//------------------------------------------------------------------------
	//allocate memory
	//printf(">>>>interpolate the subsampled displace field and warp block by block\n");
	p_img_warp = new unsigned char[sz_img_sub[0] * sz_img_sub[1] * sz_img_sub[2] * sz_img_sub[3]]();
	if(!p_img_warp)
	{
		printf("ERROR: Fail to allocate memory for p_img_warp.\n");
		if(pSubDF)				{delete pSubDF;					pSubDF=0;}
		return false;
	}
	unsigned char ****p_img_warp_4d = 0, ****p_img_sub_4d = 0;
	if(!new4dpointer(p_img_warp_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3],p_img_warp) ||
	   !new4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3],p_img_sub))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img_warp_4d) 		{delete4dpointer(p_img_warp_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
		if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
		if(p_img_warp) 			{delete []p_img_warp;			p_img_warp=0;}
		if(pSubDF)				{delete pSubDF;					pSubDF=0;}
		return false;
	}
	Vol3DSimple<DisplaceFieldF3D> *pDFBlock=new Vol3DSimple<DisplaceFieldF3D> (szBlock_x,szBlock_y,szBlock_z);
	if(!pDFBlock)
	{
		printf("ERROR: Fail to allocate memory for pDFBlock.\n");
		if(p_img_warp_4d) 		{delete4dpointer(p_img_warp_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
		if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
		if(p_img_warp) 			{delete []p_img_warp;			p_img_warp=0;}
		if(pSubDF)				{delete pSubDF;					pSubDF=0;}
		return false;
	}
	DisplaceFieldF3D ***pppDFBlock=pDFBlock->getData3dHandle();

	//------------------------------------------------------------------------
	//interpolate the SubDfBlock to DFBlock and do warp block by block
	if(i_interpmethod_df==0)		printf("\t>>subsampled displace field interpolate method: trilinear\n");
	else if(i_interpmethod_df==1)	printf("\t>>subsampled displace field interpolate method: B-spline\n");
	if(i_interpmethod_img==0)		printf("\t>>image value               interpolate method: trilinear\n");
	else if(i_interpmethod_img==1)	printf("\t>>image value               interpolate method: nearest neighbor\n");

	if(i_interpmethod_df==0)	//linear interpolate the SubDfBlock to DFBlock and do warp block by block
	{
		for(V3DLONG substart_z=0;substart_z<pSubDF->sz2()-1;substart_z++)
			for(V3DLONG substart_y=0;substart_y<pSubDF->sz1()-1;substart_y++)
				for(V3DLONG substart_x=0;substart_x<pSubDF->sz0()-1;substart_x++)
				{
					//linear interpolate the SubDfBlock to DFBlock
					q_dfblcokinterp_linear(pppSubDF,szBlock_x,szBlock_y,szBlock_z,substart_x,substart_y,substart_z,pppDFBlock);
					//warp image block using DFBlock
					q_imgblockwarp(p_img_sub_4d,sz_img_sub,pppDFBlock,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,substart_x,substart_y,substart_z,p_img_warp_4d);
				}
	}
	else						//bspline interpolate the SubDfBlock to DFBlock and do warp block by block
	{
		//initialize the bspline basis function
		V3DLONG sz_gridwnd=szBlock_x;
		Matrix x_bsplinebasis(pow(double(sz_gridwnd),3.0),pow(4.0,3.0));
		if(!q_nonrigid_ini_bsplinebasis_3D(sz_gridwnd,x_bsplinebasis))
		{
			printf("ERROR: q_ini_bsplinebasis_3D() return false!\n");
			if(p_img_warp_4d) 		{delete4dpointer(p_img_warp_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
			if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
			if(p_img_warp) 			{delete []p_img_warp;			p_img_warp=0;}
			if(pSubDF)				{delete pSubDF;					pSubDF=0;}
			return false;
		}
		printf("\t>>x_bsplinebasis:[%d,%d]\n",x_bsplinebasis.nrows(),x_bsplinebasis.ncols());

		for(V3DLONG substart_z=0;substart_z<pSubDF->sz2()-1-2;substart_z++)
			for(V3DLONG substart_y=0;substart_y<pSubDF->sz1()-1-2;substart_y++)
				for(V3DLONG substart_x=0;substart_x<pSubDF->sz0()-1-2;substart_x++)
				{
					//bspline interpolate the SubDfBlock to DFBlock
					q_dfblcokinterp_bspline(pppSubDF,x_bsplinebasis,sz_gridwnd,substart_x,substart_y,substart_z,pppDFBlock);
					//warp image block using DFBlock
					q_imgblockwarp(p_img_sub_4d,sz_img_sub,pppDFBlock,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,substart_x,substart_y,substart_z,p_img_warp_4d);
				}
	}

	//free memory
	if(p_img_warp_4d) 		{delete4dpointer(p_img_warp_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
	if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
	if(pDFBlock)			{delete pDFBlock;			pDFBlock=0;}
	if(pSubDF)				{delete pSubDF;				pSubDF=0;}

	return true;
}


bool q_dfblcokinterp_linear(DisplaceFieldF3D ***&pppSubDF,
	const V3DLONG szBlock_x, const V3DLONG szBlock_y, const V3DLONG szBlock_z,
	const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock)
{
	Vol3DSimple <MYFLOAT_JBA> 		*pDFBlock1C = 0;
	MYFLOAT_JBA			    	***pppDFBlock1C = 0;
	Vol3DSimple<MYFLOAT_JBA> 		*pSubDFBlock1c = new Vol3DSimple<MYFLOAT_JBA>(2, 2, 2);
	MYFLOAT_JBA 		     	***pppSubDFBlock1c = pSubDFBlock1c->getData3dHandle();

	V3DLONG i, j, k, is, js, ks;
	//x
	for (k = 0; k<2; k++)
		for (j = 0; j<2; j++)
			for (i = 0; i<2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sx;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k<szBlock_z; k++) for (j = 0; j<szBlock_y; j++) for (i = 0; i<szBlock_x; i++) pppDFBlock[k][j][i].sx = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }
	//y
	for (k = 0; k<2; k++)
		for (j = 0; j<2; j++)
			for (i = 0; i<2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sy;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k<szBlock_z; k++) for (j = 0; j<szBlock_y; j++) for (i = 0; i<szBlock_x; i++) pppDFBlock[k][j][i].sy = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }
	//z
	for (k = 0; k<2; k++)
		for (j = 0; j<2; j++)
			for (i = 0; i<2; i++)
			{
				ks = k + substart_z;
				js = j + substart_y;
				is = i + substart_x;
				pppSubDFBlock1c[k][j][i] = pppSubDF[ks][js][is].sz;
			}
	pDFBlock1C = linearinterp_regularmesh_3d(szBlock_x, szBlock_y, szBlock_z, pSubDFBlock1c);
	pppDFBlock1C = pDFBlock1C->getData3dHandle();
	for (k = 0; k<szBlock_z; k++) for (j = 0; j<szBlock_y; j++) for (i = 0; i<szBlock_x; i++) pppDFBlock[k][j][i].sz = pppDFBlock1C[k][j][i];
	if (pDFBlock1C) { delete pDFBlock1C; pDFBlock1C = 0; }

	if (pSubDFBlock1c)		{ delete pSubDFBlock1c;		pSubDFBlock1c = 0; }

	return true;
}


bool q_dfblcokinterp_bspline(DisplaceFieldF3D ***&pppSubDF, const Matrix &x_bsplinebasis,
	const V3DLONG sz_gridwnd, const V3DLONG substart_x, const V3DLONG substart_y, const V3DLONG substart_z,
	DisplaceFieldF3D ***&pppDFBlock)
{
	//vectorize the gridblock's nodes position that use for interpolation
	Matrix x1D_gridblock(4 * 4 * 4, 3);
	long ind = 1;
	for (long dep = substart_z; dep<substart_z + 4; dep++)
		for (long col = substart_x; col<substart_x + 4; col++)
			for (long row = substart_y; row<substart_y + 4; row++)
			{
				x1D_gridblock(ind, 1) = pppSubDF[dep][row][col].sx;
				x1D_gridblock(ind, 2) = pppSubDF[dep][row][col].sy;
				x1D_gridblock(ind, 3) = pppSubDF[dep][row][col].sz;
				ind++;
			}
	//cubic B-spline interpolate the vectorized grid block
	Matrix x1D_gridblock_int = x_bsplinebasis*x1D_gridblock;
	//de-vectorize the interpolated grid block and save back to vec4D_grid_int
	ind = 1;
	for (long zz = 0; zz<sz_gridwnd; zz++)
		for (long xx = 0; xx<sz_gridwnd; xx++)
			for (long yy = 0; yy<sz_gridwnd; yy++)
			{
				pppDFBlock[zz][yy][xx].sx = x1D_gridblock_int(ind, 1);
				pppDFBlock[zz][yy][xx].sy = x1D_gridblock_int(ind, 2);
				pppDFBlock[zz][yy][xx].sz = x1D_gridblock_int(ind, 3);
				ind++;
			}

	return true;
}

Vol3DSimple<DisplaceFieldF3D> * compute_df_tps_subsampled_volume(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
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

	MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
	MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();
	//printf("\n---------------------------------\n");
	for (n = 0; n<nCpt; n++)
	{
		cpt_target_ref[n][0] = matchTargetPos.at(n).x;
		cpt_target_ref[n][1] = matchTargetPos.at(n).y;
		cpt_target_ref[n][2] = matchTargetPos.at(n).z;

		cpt_subject_ref[n][0] = matchSubjectPos.at(n).x;
		cpt_subject_ref[n][1] = matchSubjectPos.at(n).y;
		cpt_subject_ref[n][2] = matchSubjectPos.at(n).z;

		//printf("n=%d \tx=[%5.3f -> %5.3f] y=[%5.3f -> %5.3f] z=[%5.3f -> %5.3f] \n",
		//       n, cpt_target_ref[n][0], cpt_subject_ref[n][0], cpt_target_ref[n][1], cpt_subject_ref[n][1], cpt_target_ref[n][2], cpt_subject_ref[n][2]);
	}
	//printf("\n#################################\n");

	Matrix wR(nCpt, nCpt);

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
	}

	V3DLONG p;

	V3DLONG gsz0 = (V3DLONG)(ceil((double(sz0) / gfactor_x))) + 1, gsz1 = (V3DLONG)(ceil((double(sz1) / gfactor_y))) + 1, gsz2 = (V3DLONG)(ceil((double(sz2) / gfactor_z))) + 1;
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

	V3DLONG ndimpt = 3;
	double * dist = new double[nCpt + ndimpt + 1];
	if (!dist)
	{
		fprintf(stderr, "Fail to allocate memory dist for tps warping [%d].\n", __LINE__);

		if (cpt_target) { delete cpt_target; cpt_target = 0; }
		if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }
		if (df_local) { delete df_local; df_local = 0; }
		return 0;
	}

	printf("-------------------- Now compute the distances of pixels to the mapping points. -------\n\n");

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
				for (n = 0; n<nCpt; n++)
				{
					s = 0;
					tmp = (i*gfactor_x) - cpt_target_ref[n][0]; s += tmp*tmp;
					tmp = (j*gfactor_y) - cpt_target_ref[n][1]; s += tmp*tmp;
					tmp = (k*gfactor_z) - cpt_target_ref[n][2]; s += tmp*tmp;
					dist[n] = 2 * s*log(s + 1e-20);
				}

				dist[nCpt] = 1;
				dist[nCpt + 1] = i*gfactor_x;
				dist[nCpt + 2] = j*gfactor_y;
				dist[nCpt + 3] = k*gfactor_z;

				s = 0;  for (p = 0; p<nCpt + ndimpt + 1; p++) { s += dist[p] * wW(p + 1, 1); }
				df_local_3d[k][j][i].sx = s - i*gfactor_x;

				s = 0;  for (p = 0; p<nCpt + ndimpt + 1; p++) { s += dist[p] * wW(p + 1, 2); }
				df_local_3d[k][j][i].sy = s - j*gfactor_y;

				s = 0;  for (p = 0; p<nCpt + ndimpt + 1; p++) { s += dist[p] * wW(p + 1, 3); }
				df_local_3d[k][j][i].sz = s - k*gfactor_z;
			}//i
		}//j
		printf("z=%ld ", k); fflush(stdout);
	}//k
	printf("\n");

	if (dist) { delete[]dist; dist = 0; }
	if (cpt_target) { delete cpt_target; cpt_target = 0; }
	if (cpt_subject) { delete cpt_subject; cpt_subject = 0; }

	return df_local;
}

Vol3DSimple<DisplaceFieldF3D> * compute_df_stps_subsampled_volume_4bspline(const vector <Coord3D_JBA> & matchTargetPos, const vector <Coord3D_JBA> & matchSubjectPos, V3DLONG sz0, V3DLONG sz1, V3DLONG sz2,
	V3DLONG gfactor_x, V3DLONG gfactor_y, V3DLONG gfactor_z, Matrix &x4x4_d, Matrix &xnx4_c, Matrix &xnxn_K)
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
	//Matrix x4x4_d, xnx4_c, xnxn_K;
	if (xnx4_c.nrows() != nCpt || xnx4_c.ncols() != 4)
		xnx4_c.ReSize(nCpt, 4);
	if (x4x4_d.nrows() != 4 || xnx4_c.ncols() != 4)
		x4x4_d.ReSize(4, 4);
	if (xnxn_K.nrows() != nCpt || xnxn_K.ncols() != nCpt)
		xnxn_K.ReSize(nCpt, nCpt);





	MYFLOAT_JBA ** cpt_target_ref = cpt_target->getData2dHandle();
	MYFLOAT_JBA ** cpt_subject_ref = cpt_subject->getData2dHandle();

	//printf("\n---------------------------------\n");
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
	Matrix A = q2.t()*xnxn_K*q2 + IdentityMatrix(nCpt - 4)*0.2;
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

bool q_nonrigid_ini_bsplinebasis_3D(const long n, Matrix &BxBxB)
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

