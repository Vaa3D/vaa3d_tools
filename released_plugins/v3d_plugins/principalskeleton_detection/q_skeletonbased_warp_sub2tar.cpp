// q_skeletonbased_warp_sub2tar.cpp
// by Lei Qu
// 2009-11-11

#include <math.h>

#include "q_skeletonbased_warp_sub2tar.h"

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


//warp subject image to target image based on the given principal skeletons
//output sub2tar image has same as target image
//
//Processing flow:
//(1). resize subject image to the same size as target
//(2). resize the subject principal skeleton accoridingly
//(3). staighten every branches of subject and target image respectively (for mask region and mapping index, used to define control points)
//(4). define the control points for subject and target image
//(5). compute subject to target TPS warping displace field
//(6). warp the subject image to target image based on the given displace field
//
//Parameters:
//p_img_tar,sz_img_tar:			input target image pointer and size array pointer [width, height, z, channel]
//p_img_sub,sz_img_sub:			input subject image pointer and size array pointer [width, height, z, channel]
//vec_ql_branchmarker_tar/sub:	input 2D array that contain skeleton branches along with their control points index
//d_ctlpt2node_ratio_alongbranch:input ratio of number of anchor point to control point along branch
//l_ctrlpt_perslice:			input number of control point along each slice
//l_slice_width:				input width of straightened area
//p_img_sub2tar:				output subject to target warped image
//vec_cpt_tar/sub				output target and subject control points for TPS interpolation
//
bool q_skeletonbased_sub2tar(
		const unsigned char *p_img_tar,const V3DLONG *sz_img_tar,
		const unsigned char *p_img_sub,const V3DLONG *sz_img_sub,
		const vector< QList<ImageMarker> > &vec_ql_branchcpt_tar,const vector< QList<ImageMarker> > &vec_ql_branchcpt_sub,
		const long l_anchor2cpt_ratio_alongbranch,const long l_anchor_perslice,const long l_slice_width,
		unsigned char *&p_img_sub2tar,
		vector<Coord2D64F_SL> &vec_anchor_tar,vector<Coord2D64F_SL> &vec_anchor_sub)
{
	//check parameters
	if(!p_img_tar || !sz_img_tar || !p_img_sub || !sz_img_sub)
	{
		printf("ERROR: q_skeletonbased_sub2tar: At least one input pointer is NULL.\n");
		return false;
	}
	if(vec_ql_branchcpt_tar.size()==0 || vec_ql_branchcpt_sub.size()==0)
	{
		printf("ERROR: q_skeletonbased_sub2tar: At least one input branchmarker vector is empty.\n");
		return false;
	}
	if(l_anchor2cpt_ratio_alongbranch<=0 || l_anchor_perslice<1 || l_slice_width<=0)
	{
		printf("ERROR: q_skeletonbased_sub2tar: input d_ctlpt2node_ratio_alongbranch or l_ctrlpt_perslice not reasonable.\n");
		return false;
	}
	if(p_img_sub2tar)
	{
		printf("WARNNING: q_skeletonbased_sub2tar: output image pointer is not null, original memeroy it point to will lost!\n");
		p_img_sub2tar=0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//resize subject image to the size of target
	//for XY plane, we use nearest interpolation
	//for Z plane, we simply crop or replicate the last several slices
	printf(">>resize subject image to the size of target image ...\n");

	unsigned char *p_img_sub_resize=0;
	if(!q_resize_image(p_img_sub,sz_img_sub,sz_img_tar,p_img_sub_resize))
	{
		printf("ERROR: q_resize_image() return false.\n");
		return false;
	}
	printf("\t>>original sub image size: [%ld,%ld,%ld,%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t>>resized  sub image size: [%ld,%ld,%ld,%ld]\n",sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);

	//resize the marker postion of subject image accordingly
	printf(">>resize the subject branch nodes accordingly ...\n");

	vector< QList<ImageMarker> > vec_ql_branchmarker_sub_resize(vec_ql_branchcpt_sub);
	//compute the size ratio of subject to target
	double ratio_sub2tar_x=(double)sz_img_sub[0]/(double)sz_img_tar[0];
	double ratio_sub2tar_y=(double)sz_img_sub[1]/(double)sz_img_tar[1];
	for(unsigned long i=0;i<vec_ql_branchcpt_sub.size();i++)
		for(long j=0;j<vec_ql_branchcpt_sub[i].size();j++)
		{
			vec_ql_branchmarker_sub_resize[i][j].x/=ratio_sub2tar_x;
			vec_ql_branchmarker_sub_resize[i][j].y/=ratio_sub2tar_y;
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//compute the mask region and mapping index for each branch (prepare for control points definition)
	//actually we do this by staighten the head and tail respectively

	vector< vector< vector<Coord2D64F_SL> > > vecvec_index_str2ori_branches_tar,vecvec_index_str2ori_branches_sub;
	vector< vector< vector<Coord2D64F_SL> > > vecvec_index_ori2str_branches_tar,vecvec_index_ori2str_branches_sub;

	for(unsigned long i=0;i<vec_ql_branchcpt_tar.size();i++)
	{
		//if branch node num ==2, a new middel node is inserted to meet the requirement of cubic-spline
		QList<ImageMarker> ql_branchmarker_tar(vec_ql_branchcpt_tar[i]),ql_branchmarker_sub(vec_ql_branchmarker_sub_resize[i]);
		if(ql_branchmarker_tar.size()==2)
		{
			ImageMarker im_middle;
			im_middle.x=(vec_ql_branchcpt_tar[i][0].x+vec_ql_branchcpt_tar[i][1].x)/2.0;
			im_middle.y=(vec_ql_branchcpt_tar[i][0].y+vec_ql_branchcpt_tar[i][1].y)/2.0;
			im_middle.z=(vec_ql_branchcpt_tar[i][0].z+vec_ql_branchcpt_tar[i][1].z)/2.0;
			ql_branchmarker_tar.push_back(vec_ql_branchcpt_tar[i][1]);
			ql_branchmarker_tar.replace(1,im_middle);
		}
		if(ql_branchmarker_sub.size()==2)
		{
			ImageMarker im_middle;
			im_middle.x=(vec_ql_branchmarker_sub_resize[i][0].x+vec_ql_branchmarker_sub_resize[i][1].x)/2.0;
			im_middle.y=(vec_ql_branchmarker_sub_resize[i][0].y+vec_ql_branchmarker_sub_resize[i][1].y)/2.0;
			im_middle.z=(vec_ql_branchmarker_sub_resize[i][0].z+vec_ql_branchmarker_sub_resize[i][1].z)/2.0;
			ql_branchmarker_sub.push_back(vec_ql_branchmarker_sub_resize[i][1]);
			ql_branchmarker_sub.replace(1,im_middle);
		}

		//do smoothing and straightening
		unsigned char *p_img_strbranch=0;
		long *sz_img_strbranch=0;
		vector<Coord2D64F_SL> vec_centralline_branch;
		vector< vector<Coord2D64F_SL> > vec_index_str2ori_branch,vec_index_ori2str_branch;

		printf("\t>>straighten branch[%ld] of target image ...\n",i);
		if(!q_curve_smoothstraighten(
				p_img_tar,sz_img_tar,
				ql_branchmarker_tar,l_slice_width,
				p_img_strbranch,sz_img_strbranch,
				vec_centralline_branch,
				vec_index_str2ori_branch,vec_index_ori2str_branch))
		{
			printf("ERROR: q_curve_smoothstraighten() return false! \n");
			return false;
		}
		vecvec_index_str2ori_branches_tar.push_back(vec_index_str2ori_branch);
		vecvec_index_ori2str_branches_tar.push_back(vec_index_ori2str_branch);
		if(p_img_strbranch)	{delete []p_img_strbranch;		p_img_strbranch=0;}
		if(sz_img_strbranch){delete []sz_img_strbranch;		sz_img_strbranch=0;}

		printf("\t>>straighten branch[%ld] of subject image ...\n",i);
		if(!q_curve_smoothstraighten(
				p_img_sub_resize,sz_img_tar,
				ql_branchmarker_sub,l_slice_width,
				p_img_strbranch,sz_img_strbranch,
				vec_centralline_branch,
				vec_index_str2ori_branch,vec_index_ori2str_branch))
		{
			printf("ERROR: q_curve_smoothstraighten() return false! \n");
			return false;
		}
		vecvec_index_str2ori_branches_sub.push_back(vec_index_str2ori_branch);
		vecvec_index_ori2str_branches_sub.push_back(vec_index_ori2str_branch);
		if(p_img_strbranch)	{delete []p_img_strbranch;		p_img_strbranch=0;}
		if(sz_img_strbranch){delete []sz_img_strbranch;		sz_img_strbranch=0;}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//define the control points for subject larva image and target larva image
	printf(">>define the control points for target and subject image ...\n");

//	//define the number of control point in each branch
//	vector<long> vec_nctlpt_alongbranch(vec_ql_branchcpt_tar.size(),0),vec_nctlpt_perslice(vec_ql_branchcpt_tar.size(),l_cpt_perslice);
//	for(long i=0;i<vec_ql_branchcpt_tar.size();i++)
//	{
//		vec_nctlpt_alongbranch[i]=int(vec_ql_branchcpt_tar[i].size()*d_ctlpt2node_ratio_alongbranch);
//	}
//	//define control points
//	if(!q_controlpoint_definition_evendis(
//			vec_nctlpt_alongbranch,vec_nctlpt_perslice,
//			vecvec_index_str2ori_branches_tar,vecvec_index_ori2str_branches_tar,
//			vecvec_index_str2ori_branches_sub,vecvec_index_ori2str_branches_sub,
//			vec_cpt_tar,vec_cpt_sub))
//	{
//		printf("ERROR: q_controlpoint_definition() return false! \n");
//		return false;
//	}

	if(!q_anchorpoint_definition_unevendis(
			vec_ql_branchcpt_tar,vec_ql_branchmarker_sub_resize,
			2,l_anchor_perslice,
			vecvec_index_str2ori_branches_tar,vecvec_index_ori2str_branches_tar,
			vecvec_index_str2ori_branches_sub,vecvec_index_ori2str_branches_sub,
			vec_anchor_tar,vec_anchor_sub))
	{
		printf("ERROR: q_anchorpoint_definition_unevendis() return false! \n");
		return false;
	}


//	q_points_save2markerfile(vec_anchor_tar,"tar.marker");
//	q_points_save2markerfile(vec_anchor_sub,"sub.marker");
//	printf("\t>>%d valid control points found (target image).\n",vec_controlpoint_tar.size());
//	printf("\t>>%d valid control points found (subject image).\n",vec_controlpoint_sub.size());
//	q_saveimg_strmask_overlaid(p_img_tar,sz_img_tar,vecvec_index_ori2str_branches_tar,"mask_tar.tif");
//	q_saveimg_strmask_overlaid(p_img_sub_resize,sz_img_tar,vecvec_index_ori2str_branches_sub,"/Users/qul/work/Larva_standardization/paper/image/controlpoints/sub_mask.tif");

    //------------------------------------------------------------------------------------------------------------------------------------
	//compute subject to target TPS warping displace field
	printf(">>compute TPS displace field of subject to target ...\n");
	Vol3DSimple<DisplaceFieldF3D> *df_tar2sub=0;
	if(!q_compute_tps_df_tar2sub_2d(vec_anchor_sub,vec_anchor_tar,sz_img_tar[0],sz_img_tar[1],df_tar2sub))
	{
		printf("ERROR: q_compute_df_tps_2d() return false! \n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//warp the subject image to target image based on the given tar2sub displace field
	printf(">>warp subject image to target image ...\n");
	if(!q_warp_sub2tar_baseon_df_tar2sub(p_img_sub_resize,sz_img_tar,df_tar2sub,p_img_sub2tar))
	{
		printf("ERROR: q_warp_from_df() return false! \n");
		return false;
	}
//	saveImage("/Users/qul/work/v3d_2.0/q_skeletonbased_warp_sub2tar/data/VNC/sub2tar.tif",(unsigned char *)p_img_sub2tar,sz_img_tar,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	if(p_img_sub_resize)	{delete []p_img_sub_resize;		p_img_sub_resize=0;}
	if(df_tar2sub) 			{delete df_tar2sub;				df_tar2sub=0;}

	return true;
}


//resize the input image to the given size
//for XY plane, we use nearest interpolation
//for Z plane, we simply crop or replicate the last several slices
bool q_resize_image(
		const unsigned char *p_img_input,const V3DLONG sz_img_input[4],
		const V3DLONG sz_img_output[4],
		unsigned char *&p_img_output)
{
	//check parameters
	if(!p_img_input || !sz_img_input)
	{
		printf("ERROR: q_resize_image: input image pointer or size array pointer is NULL.\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 ||sz_img_input[2]<=0 || sz_img_input[0]<=0)
	{
		printf("ERROR: q_resize_image: input image size array is invalid.\n");
		return false;
	}
	if(!sz_img_output)
	{
		printf("ERROR: q_resize_image: input size array pointer of resized image is NULL.\n");
		return false;
	}
	if(sz_img_output[0]<=0 || sz_img_output[1]<=0 ||sz_img_output[2]<=0 || sz_img_output[0]<=0)
	{
		printf("ERROR: q_resize_image: input image size array of resized image is invalid.\n");
		return false;
	}
	if(sz_img_input[3]!=sz_img_output[3])
	{
		printf("ERROR: q_resize_image: input image channel number != output image channel number.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_resize_image: output image pointer is not null, original memeroy it point to will lost!\n");
		p_img_output=0;
	}

	//compute the size ratio of subject to target
	double ratio_sub2tar_x=(double)sz_img_input[0]/(double)sz_img_output[0];
	double ratio_sub2tar_y=(double)sz_img_input[1]/(double)sz_img_output[1];

	//allocate memory for resized subject image
	p_img_output=new unsigned char[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]];
	if(!p_img_output)
	{
		printf("ERROR: q_resize_image: Fail to allocate memory for the resized image. \n");
		return false;
	}

	//compute the page size for different dimention
	long pgsz3_output,pgsz2_output,pgsz1_output;
	long pgsz3_input,pgsz2_input,pgsz1_input;
	pgsz3_output=sz_img_output[0]*sz_img_output[1]*sz_img_output[2];
	pgsz2_output=sz_img_output[0]*sz_img_output[1];
	pgsz1_output=sz_img_output[0];
	pgsz3_input=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
	pgsz2_input=sz_img_input[0]*sz_img_input[1];
	pgsz1_input=sz_img_input[0];

	//NN interpolate
	long x_sub,y_sub,z_sub;
	for(long y=0;y<sz_img_output[1];y++)
		for(long x=0;x<sz_img_output[0];x++)
		{
			y_sub=y*ratio_sub2tar_y+0.5;
			x_sub=x*ratio_sub2tar_x+0.5;

			x_sub=x_sub<0?0:x_sub;	x_sub=x_sub>=sz_img_input[0]?sz_img_input[0]-1:x_sub;
			y_sub=y_sub<0?0:y_sub;	y_sub=y_sub>=sz_img_input[1]?sz_img_input[1]-1:y_sub;

			for(long z=0;z<sz_img_output[2];z++)
			{
				if(z>=sz_img_input[2])
					z_sub=sz_img_input[2]-1;
				else
					z_sub=z;

				//copy data
				for(long c=0;c<sz_img_input[3];c++)
					p_img_output[pgsz3_output*c+pgsz2_output*z+pgsz1_output*y+x]=p_img_input[pgsz3_input*c+pgsz2_input*z_sub+pgsz1_input*y_sub+x_sub];
			}
		}

	return true;
}


//cubic_spline interpolate the given curve, and then straighten the smooth curve with its neighbor to a rectangular
//the horizental central line of rectangular corresponding to the cubic_spline interpolated curve (same length)
bool q_curve_smoothstraighten(
		const unsigned char *p_inputimg,const V3DLONG *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg,
		vector<Coord2D64F_SL> &vec_centralcurve,
		vector< vector<Coord2D64F_SL> > &vec_index_str2ori,vector< vector<Coord2D64F_SL> > &vec_index_ori2str)
{
	//check parameters
	if(!p_inputimg)
	{
		printf("ERROR: q_smoothstraighten_curve: The input larval image pointer is empty.\n");
		return false;
	}
	if(!sz_inputimg)
	{
		printf("ERROR: q_smoothstraighten_curve: The input image SIZE pointer is empty.\n");
		return false;
	}
	if(ql_marker.size()==0)
	{
		printf("ERROR: q_smoothstraighten_curve: The input marker list is empty.\n");
		return false;
	}
	if(l_width<=0)
	{
		printf("ERROR: q_smoothstraighten_curve: The l_width should > 0.\n");
		return false;
	}
	if(p_strimg)
	{
		printf("ERROR: q_smoothstraighten_curve: The output image DATA pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}
	if(sz_strimg)
	{
		printf("ERROR: q_smoothstraighten_curve: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}
	if(!vec_centralcurve.empty())
	{
		vec_centralcurve.clear();
		printf("WARNNING: q_smoothstraighten_curve: input pos_centralcurve is not empty, original contents is cleared!\n");
	}
	if(!vec_index_str2ori.empty() || !vec_index_ori2str.empty())
	{
		vec_index_str2ori.clear();
		vec_index_ori2str.clear();
		printf("WARNNING: q_smoothstraighten_curve: input index is not empty, original contents is cleared!\n");
	}

	//estimate the cubic spline parameters for head and butt markers
	parameterCubicSpline **cpara=0;
	double *xpos=0, *ypos=0, *zpos=0;
	long NPoints=ql_marker.size();
	xpos=new double[NPoints];
	ypos=new double[NPoints];
	zpos=new double[NPoints];
	if(!xpos || !ypos || !zpos)
	{
		printf("ERROR: q_smoothstraighten_curve: Fail to allocate memory for cubic splin control points.\n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		return false;
	}
	for(int i=0;i<NPoints;i++)
	{
		xpos[i]=ql_marker.at(i).x;
		ypos[i]=ql_marker.at(i).y;
		zpos[i]=ql_marker.at(i).z;
	}
	cpara=est_cubic_spline_2d(xpos,ypos,NPoints,false);

	//cubic spline interpolate the head and butt markers(find all the interpolated locations on the backbone (1-pixel spacing))
	double *cp_x=0,*cp_y=0,*cp_z=0,*cp_alpha=0;
	V3DLONG cutPlaneNum=0;
	if(!interpolate_cubic_spline(cpara,2,cp_x,cp_y,cp_z,cp_alpha,cutPlaneNum))
	{
		printf("ERROR: q_smoothstraighten_curve: interpolate_cubic_spline() return false! \n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		if(cp_x) {delete []cp_x; cp_x=0;}
		if(cp_y) {delete []cp_y; cp_y=0;}
		if(cp_z) {delete []cp_z; cp_z=0;}
		if(cp_alpha) {delete []cp_alpha; cp_alpha=0;}
		if(cpara) //delete the cubic spline parameter data structure
		{
			for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
			delete []cpara; cpara=0;
		}
		return false;
	}
	printf("\t>>cutPlaneNum=%ld\n",cutPlaneNum);

	//save smoothed curve
	for(long i=0;i<cutPlaneNum;i++)//should skip the first point since it is wrong!
	{
		vec_centralcurve.push_back(Coord2D64F_SL(cp_x[i],cp_y[i]));
//		printf("[%d]:%f,%f\n",i,cp_x[i],cp_y[i]);	//the first point in cp_x and cp_y is wrong!
	}

	//straighten head and butt with given diameter via restacking
	if(!q_bcurve_straighten2rect(p_inputimg,sz_inputimg,cp_x,cp_y,cp_alpha,cutPlaneNum,l_width,p_strimg,sz_strimg,vec_index_str2ori,vec_index_ori2str))
	{
		printf("ERROR: q_smoothstraighten_curve: q_straighten_curv2rect() return false! \n");
		if(xpos) 		{delete []xpos; xpos=0;}
		if(ypos) 		{delete []ypos; ypos=0;}
		if(zpos) 		{delete []zpos; zpos=0;}
		if(cp_x) 		{delete []cp_x; cp_x=0;}
		if(cp_y) 		{delete []cp_y; cp_y=0;}
		if(cp_z) 		{delete []cp_z; cp_z=0;}
		if(cp_alpha) 	{delete []cp_alpha; cp_alpha=0;}
		if(cpara) //delete the cubic spline parameter data structure
		{
			for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
			delete []cpara; cpara=0;
		}
		if(p_strimg) 	{delete []p_strimg; p_strimg=0;	}
		if(sz_strimg) 	{delete []sz_strimg;sz_strimg=0;}
		return false;
	}

	//free memory
	if(xpos) 		{delete []xpos; xpos=0;}
	if(ypos) 		{delete []ypos; ypos=0;}
	if(zpos) 		{delete []zpos; zpos=0;}
	if(cp_x) 		{delete []cp_x; cp_x=0;}
	if(cp_y) 		{delete []cp_y; cp_y=0;}
	if(cp_z) 		{delete []cp_z; cp_z=0;}
	if(cp_alpha) 	{delete []cp_alpha; cp_alpha=0;}
	if(cpara) //delete the cubic spline parameter data structure
	{
		for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
		delete []cpara; cpara=0;
	}

	return true;
}

//straight the neighbor region along the given b-spline curve to rectangular and return the dual directional mapping index
bool q_bcurve_straighten2rect(
		const unsigned char *p_inputimg,const V3DLONG *sz_inputimg,
        const double *pos_curve_x,const double *pos_curve_y,const double *alpha_curve,const long length_curve,
		const long l_width,
		unsigned char *&p_strimg, long *&sz_strimg,
		vector< vector<Coord2D64F_SL> > &vec_index_str2ori,vector< vector<Coord2D64F_SL> > &vec_index_ori2str)
{
	//check parameters
	if(!p_inputimg || !sz_inputimg)
	{
		printf("ERROR: q_straighten_curv2rect: pointer p_inputimg or sz_inputimg is invalid. \n");
		return false;
	}
	long nx=sz_inputimg[0];
	long ny=sz_inputimg[1];
	long nz=sz_inputimg[2];
	long nc=sz_inputimg[3];
	if(nx<=1 || ny<=1 || nz<=0 || nc<=0)
	{
		printf("ERROR: q_straighten_curv2rect: The SIZE information of input image is not correct. \n");
		return false;
	}
	if(!pos_curve_x || !pos_curve_y || !alpha_curve || length_curve<=0 || l_width<=0)
	{
		printf("ERROR: q_straighten_curv2rect: The input b-spline curve's parameters are incorrect. \n");
		return false;
	}
	if(p_strimg)
	{
		printf("ERROR: q_straighten_curv2rect: The output image pointer is not empty (may contain data) or not initialized as NULL. \n");
		return false;
	}
	if(sz_strimg)
	{
		printf("ERROR: q_straighten_curv2rect: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL. \n");
		return false;
	}
	if(!vec_index_str2ori.empty())
	{
		vec_index_str2ori.clear();
		printf("WARNNING: q_straighten_curv2rect: input index_str2ori is not empty, original contents is cleared!\n");
	}
	vec_index_str2ori.assign(l_width,vector<Coord2D64F_SL>(length_curve,Coord2D64F_SL(-1.0,-1.0)));
	if(!vec_index_ori2str.empty())
	{
		vec_index_ori2str.clear();
		printf("WARNNING: q_straighten_curv2rect: input index_ori2str is not empty, original contents is cleared!\n");
	}
	vec_index_ori2str.assign(ny,vector<Coord2D64F_SL>(nx,Coord2D64F_SL(-1.0,-1.0)));

	//------------------------------------------------------------------------------------------------------------------------------------
	//allocate memory for input and output image, and size array
	sz_strimg=new long[4];
	if(!sz_strimg)
	{
		printf("ERROR: q_straighten_curv2rect: Fail to allocate memory for the SIZE array of the straightened image. \n");
		return false;
	}
	sz_strimg[0]=length_curve;
	sz_strimg[1]=l_width;
	sz_strimg[2]=nz;
	sz_strimg[3]=nc;

	p_strimg=new unsigned char[sz_strimg[0]*sz_strimg[1]*sz_strimg[2]*sz_strimg[3]]();
	if(!p_strimg)
	{
		printf("ERROR: q_straighten_curv2rect: Fail to allocate memory for the straightened image. \n");
		if(sz_strimg) {delete []sz_strimg; sz_strimg=0;}
		return false;
	}

	unsigned char ****p_strimg_4d=0, ****p_inputimg_4d=0;
	if (!new4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3], p_strimg) ||
		!new4dpointer(p_inputimg_4d, nx, ny, nz, nc, p_inputimg))
	{
		printf("ERROR: q_straighten_curv2rect: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_strimg)		{delete []p_strimg; p_strimg=0;}
		if(sz_strimg)		{delete []sz_strimg; sz_strimg=0;}
		if(p_strimg_4d)		{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}
		if(p_inputimg_4d)	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//straighten image along curve and build straightened--->original mapping index
	//every point in straightened image will have a correpongding coordinate in original image
	int ptspace=1; // default value
	double base0=0;
	long Krad=0;
	if(floor(double(l_width)/2)*2==l_width)
	{
		Krad=(l_width-1)/2;
		base0=0;
	}
	else
	{
		Krad=l_width/2;
		base0=ptspace/2;
	}

	//fill the straightened image: first along col(up -> down), then alone row(left -> right)
	//*************NOTE: the first point of pos_curve is wrong!***************
	for(long col=1;col<length_curve;col++)
	{
		double curalpha=alpha_curve[col];
		double ptminx=pos_curve_x[col]-cos(curalpha)*(base0+Krad*ptspace);
		double ptminy=pos_curve_y[col]-sin(curalpha)*(base0+Krad*ptspace);

		for(long row=0;row<l_width;row++)
		{
			double curpx=ptminx+cos(curalpha)*(row*ptspace);
			double curpy=ptminy+sin(curalpha)*(row*ptspace);
			if(curpx<0 || curpx>nx-1 || curpy<0 || curpy>ny-1)
			{
				vec_index_str2ori[row][col]=Coord2D64F_SL(-1.0,-1.0);//record the straightened--->original mapping index, set as invalid value(-1,-1)
				continue;
			}

			//linear interpolate using 4 neighbors
			long cpx0=long(floor(curpx)), cpx1=long(ceil(curpx));
			long cpy0=long(floor(curpy)), cpy1=long(ceil(curpy));
			double w0x0y=(cpx1-curpx)*(cpy1-curpy);
			double w0x1y=(cpx1-curpx)*(curpy-cpy0);
			double w1x0y=(curpx-cpx0)*(cpy1-curpy);
			double w1x1y=(curpx-cpx0)*(curpy-cpy0);
			for(long z=0;z<nz; z++)
			{
				for(long c=0;c<nc;c++)
				{
					p_strimg_4d[c][z][row][col]=(unsigned char)(w0x0y*double(p_inputimg_4d[c][z][cpy0][cpx0])+w0x1y*double(p_inputimg_4d[c][z][cpy1][cpx0])+
														     w1x0y*double(p_inputimg_4d[c][z][cpy0][cpx1])+w1x1y*double(p_inputimg_4d[c][z][cpy1][cpx1]));
				}
			}

			//record the straightened--->original mapping index
			vec_index_str2ori[row][col]=Coord2D64F_SL(floor(curpx+0.5),floor(curpy+0.5));
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//build original--->straightened mapping index
	//	1. find 4 adjacent pixels (they form a rectangle) in straightened image
	//	2. find the corresponding position of the 4 vertexs in the original image, they should form a quadrangle
	//	3. find all the pixels within the quadrangle
	//		3.1. find all pixels within the rectangle define by the left-up and right-down corners of the quadrangle
	//		3.2. suppose p1,p2,p3,p4 are 4 vertex points of the quadrangle ordered in clockwise, and the p0 is the pixel within the rectangle,
	//			  we then need to judge whether p0 is lies within the quadrangle or not
	//		3.3. compute the area of triangles: p0p1p2,p0p2p3,p0p1p3,p1p2p3;p0p3p4,p0p4p1,p1p3p4
	//			  if p0 lies within triangle p1p2p3, we will have: area_p1p2p3=area_p0p1p2+area_p0p2p3+area_p0p1p3
	//			  if p0 lies within triangle p1p3p4, we will have: area_p1p3p4=area_p0p1p3+area_p0p3p4+area_p0p4p1
	//			  if p0 lies within the quadrangle, it should satisfy at least one condition listed above
	//	4. compute the distance of the pixel to the 4 vertexs of the quadrangle, set the pixel's index same as the nearest vertext's index
	//
	for(long row=0;row<l_width-1;row++)
	{
		for(long col=0;col<length_curve-1;col++)
		{
			Coord2D64F_SL p0,p1,p2,p3,p4;	//current point and 4 vertexs of quadrangle, clockwise order
			Coord2D64F_SL lu,rd;			//left-up and right-down corner of the rectanguler which surround the quadrangle

			//find 4 vertexs of quadrangle
			p1=vec_index_str2ori[row][col];
			p2=vec_index_str2ori[row][col+1];
			p3=vec_index_str2ori[row+1][col+1];
			p4=vec_index_str2ori[row+1][col];
			if(p1.x==-1 || p2.x==-1 || p3.x==-1 || p4.x==-1)
			{
//				printf("\t(row=%ld,col=%ld): at least one vertex in quad is invalid, skipped!\n",row,col);
				continue;
			}

			//find the left-up and right-down corner of the rectanguler which surround the quadrangle
			lu.x=min(min(min(p1.x,p2.x),p3.x),p4.x);
			lu.y=min(min(min(p1.y,p2.y),p3.y),p4.y);
			rd.x=max(max(max(p1.x,p2.x),p3.x),p4.x);
			rd.y=max(max(max(p1.y,p2.y),p3.y),p4.y);

			//compute the length of p1p2,p2p3,p3p4,p4p1,p1p3 (use to judge whether a point lie in a quadrangle)
			double p1p2=sqrt(pow(p1.x-p2.x,2)+pow(p1.y-p2.y,2));
			double p2p3=sqrt(pow(p2.x-p3.x,2)+pow(p2.y-p3.y,2));
			double p3p4=sqrt(pow(p3.x-p4.x,2)+pow(p3.y-p4.y,2));
			double p4p1=sqrt(pow(p4.x-p1.x,2)+pow(p4.y-p1.y,2));
			double p1p3=sqrt(pow(p1.x-p3.x,2)+pow(p1.y-p3.y,2));

			//compute the area of triangle p1p2p3,p1p3p4 (use to judge whether a point lie in a quadrangle)
			double s,area_p1p2p3,area_p1p3p4;
			s=(p1p2+p2p3+p1p3)/2.0;		area_p1p2p3=sqrt(s*(s-p1p2)*(s-p2p3)*(s-p1p3));
			s=(p1p3+p3p4+p4p1)/2.0;		area_p1p3p4=sqrt(s*(s-p1p3)*(s-p3p4)*(s-p4p1));

			//find all pixels within the rectanguler define by the left-up and right-down corners of the quadrangle
			//judge whether the pixel is lies within the quadrangle
			//if yes, assign the pixel's index same as the nearest vertext's index
			for(long row_grid=floor(lu.y);row_grid<=ceil(rd.y);row_grid++)
			{
				for(long col_grid=floor(lu.x);col_grid<=ceil(rd.x);col_grid++)
				{
					p0.x=col_grid;
					p0.y=row_grid;
					//compute the length of p0p1,p0p2,p0p3,p0p4 (use to judge whether a point lies in a quadrangle)
					double p0p1=sqrt(pow(p0.x-p1.x,2)+pow(p0.y-p1.y,2));
					double p0p2=sqrt(pow(p0.x-p2.x,2)+pow(p0.y-p2.y,2));
					double p0p3=sqrt(pow(p0.x-p3.x,2)+pow(p0.y-p3.y,2));
					double p0p4=sqrt(pow(p0.x-p4.x,2)+pow(p0.y-p4.y,2));
					//compute the area of triangle p0p1p2,p0p2p3,p0p3p1, if p0 lies within triangle p1p2p3, we have area_p1p2p3=area_p0p1p2+area_p0p2p3+area_p0p3p1
					double area_p0p1p2,area_p0p2p3,area_p0p3p1;
					s=(p0p1+p1p2+p0p2)/2.0;		area_p0p1p2=sqrt(s*(s-p0p1)*(s-p1p2)*(s-p0p2));
					s=(p0p2+p2p3+p0p3)/2.0;		area_p0p2p3=sqrt(s*(s-p0p2)*(s-p2p3)*(s-p0p3));
					s=(p0p1+p1p3+p0p3)/2.0;		area_p0p3p1=sqrt(s*(s-p0p1)*(s-p0p3)*(s-p1p3));
					//compute the area of triangle p0p3p4,p0p4p1, 		 if p0 lies within triangle p1p3p4, we have area_p1p3p4=area_p0p3p1+area_p0p3p4+area_p0p4p1
					double area_p0p3p4,area_p0p4p1;
					s=(p0p3+p3p4+p0p4)/2.0;		area_p0p3p4=sqrt(s*(s-p0p3)*(s-p3p4)*(s-p0p4));
					s=(p0p1+p4p1+p0p4)/2.0;		area_p0p4p1=sqrt(s*(s-p0p4)*(s-p4p1)*(s-p0p1));

					//judge whether the pixel is lies within the quadrangle
					if(fabs(area_p1p2p3-(area_p0p1p2+area_p0p2p3+area_p0p3p1))<1e-10 || fabs(area_p1p3p4-(area_p0p3p1+area_p0p3p4+area_p0p4p1))<1e-10)
					{
						//the point is lies within the current quadrangle, need to update index=the nearest vertex's index
						if(p0p1<=p0p2 && p0p1<=p0p3 && p0p1<=p0p4)		//p1 is nearest to p0, set to p1's index
						{
							vec_index_ori2str[(long)p0.y][(long)p0.x].y=row;
							vec_index_ori2str[(long)p0.y][(long)p0.x].x=col;
						}
						else if(p0p2<=p0p1 && p0p2<=p0p3 && p0p2<=p0p4)	//p2 is nearest to p0, set to p2's index
						{
							vec_index_ori2str[(long)p0.y][(long)p0.x].y=row;
							vec_index_ori2str[(long)p0.y][(long)p0.x].x=col+1;
						}
						else if(p0p3<=p0p1 && p0p3<=p0p2 && p0p3<=p0p4)	//p3 is nearest to p0, set to p3's index
						{
							vec_index_ori2str[(long)p0.y][(long)p0.x].y=row+1;
							vec_index_ori2str[(long)p0.y][(long)p0.x].x=col+1;
						}
						else if(p0p4<=p0p1 && p0p4<=p0p2 && p0p4<=p0p3)	//p4 is nearest to p0, set to p4's index
						{
							vec_index_ori2str[(long)p0.y][(long)p0.x].y=row+1;
							vec_index_ori2str[(long)p0.y][(long)p0.x].x=col;
						}
					}
				}
			}
		}
	}

	//free memory
	if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
	if(p_strimg_4d) 	{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}

	return true;
}

//find control points in target and subject image (these control points will be used to do TPS warping)
//the output control points are a portion of candidate control points generated by q_candidatecontrolpoint_definition()
//the invalid candidate points judgement criteria is:
// (1).candidate control points which lie within the overlapped mask region of subject or target image
// (2).candidate control points which do not lie within the subject or target image region
bool q_anchorpoint_definition_evendis(
		const vector<long> vec_nctlpt_alongbranch,const vector<long> vec_nctlpt_perslice,
		const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_str2ori_branch_tar,const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_ori2str_branch_tar,
		const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_str2ori_branch_sub,const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_ori2str_branch_sub,
		vector<Coord2D64F_SL> &vec_controlpoint_tar,vector<Coord2D64F_SL> &vec_controlpoint_sub)
{
	//check parameters
	if(vec_nctlpt_alongbranch.size()!=vec_nctlpt_perslice.size() ||
	   vec_nctlpt_alongbranch.size()!=vecvec_index_str2ori_branch_tar.size() ||
	   vec_nctlpt_alongbranch.size()!=vecvec_index_ori2str_branch_tar.size() ||
	   vec_nctlpt_alongbranch.size()!=vecvec_index_str2ori_branch_sub.size() ||
	   vec_nctlpt_alongbranch.size()!=vecvec_index_ori2str_branch_sub.size() ||
	   vec_nctlpt_alongbranch.size()<=0)

	{
		printf("ERROR: q_controlpoint_definition: invalid input paras. \n");
		return false;
	}
	if(!vec_controlpoint_tar.empty())
	{
		printf("WARNNING: q_controlpoint_definition: input vec_controlpoint_tar is not empty, original contents is cleared!\n");
		vec_controlpoint_tar.clear();
	}
	if(!vec_controlpoint_sub.empty())
	{
		printf("WARNNING: q_controlpoint_definition: input vec_controlpoint_sub is not empty, original contents is cleared!\n");
		vec_controlpoint_sub.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//find all candidate control points in target and subject image
	vector<Coord2D64F_SL> vec_controlpoint_candidate_tar,vec_controlpoint_candidate_sub;

	//target
	if(!q_candidateanchorpoint_definition_evendis(
			vecvec_index_str2ori_branch_tar,
			vec_nctlpt_alongbranch,vec_nctlpt_perslice,
			vec_controlpoint_candidate_tar))
	{
		printf("ERROR: q_controlpoint_definition() return false! \n");
		return false;
	}
	//subject
	if(!q_candidateanchorpoint_definition_evendis(
			vecvec_index_str2ori_branch_sub,
			vec_nctlpt_alongbranch,vec_nctlpt_perslice,
			vec_controlpoint_candidate_sub))
	{
		printf("ERROR: q_controlpoint_definition() return false! \n");
		return false;
	}
//	q_points_save2markerfile(vec_controlpoint_candidate_tar,"/Users/qul/work/v3d_2.0/q_skeletonbased_warp_sub2tar/data/tar_candi.marker");
//	q_points_save2markerfile(vec_controlpoint_candidate_sub,"/Users/qul/work/v3d_2.0/q_skeletonbased_warp_sub2tar/data/sub_candi.marker");

	//------------------------------------------------------------------------------------------------------------------------------------
	//find the valid control points for the target and subject image
	// (1).candidate control points which lie within the overlapped region of subject or target image are invalid
	// (2).candidate control points which do not lie within the subject or target image region are invalid
	//
	//(1). markout the invalid candidate points which do not lie within the target or subject image region (invalid-->[-1,-1])
	for(unsigned long i=0;i<vec_controlpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_controlpoint_candidate_tar[i].x-(-1))<1e-10 || fabs(vec_controlpoint_candidate_tar[i].y-(-1))<1e-10 ||
		   fabs(vec_controlpoint_candidate_sub[i].x-(-1))<1e-10 || fabs(vec_controlpoint_candidate_sub[i].y-(-1))<1e-10)
		{
			vec_controlpoint_candidate_tar[i].x=-1;
			vec_controlpoint_candidate_tar[i].y=-1;
			vec_controlpoint_candidate_sub[i].x=-1;
			vec_controlpoint_candidate_sub[i].y=-1;
		}
	}

	//find the index of control points that lie in the skeleton
	vector<long> vec_index_ctlptonskeleton;
	long n_crlpt_onformerbranches=0;
	for(unsigned long i=0;i<vec_nctlpt_alongbranch.size();i++)
	{
		//only if the number of control point along per-slice is odd, there exis control points that lie on the skeleton
		if(vec_nctlpt_perslice[i]%2==1)
		{
			for(long k=0;k<vec_nctlpt_alongbranch[i];k++)
			{
				long ind=(k+1)*vec_nctlpt_perslice[i]-2;
				vec_index_ctlptonskeleton.push_back(n_crlpt_onformerbranches+ind);
			}
		}

		n_crlpt_onformerbranches+=vec_nctlpt_alongbranch[i]*vec_nctlpt_perslice[i];
	}

	//(2). markout the invalid candidate points which lie within the overlapped mask region of target or subject(invalid-->[-1,-1])
	//note: if the control points lie in the central line, we take it as valid no matter where it is
	for(unsigned long i=0;i<vec_controlpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_controlpoint_candidate_tar[i].x-(-1))>1e-10 && fabs(vec_controlpoint_candidate_tar[i].y-(-1))>1e-10 &&
		   fabs(vec_controlpoint_candidate_sub[i].x-(-1))>1e-10 && fabs(vec_controlpoint_candidate_sub[i].y-(-1))>1e-10)
		{
			//all control points lie on the skeleton are valid control points
			//not good, in this way there will be severval control points hunddled in the region near joint points, they will produce big distortion
			bool flag_lieonskeleotn=false;
//			for(long j=0;j<vec_index_ctlptonskeleton.size();j++)
//				if(i==vec_index_ctlptonskeleton[j])
//				{
//					flag_lieonskeleotn=true;
//					break;
//				}

			if(!flag_lieonskeleotn)
			{
				//if current control point lies in mask region of any other branch, it is invalid control point
				long pos_ori_x_tar=vec_controlpoint_candidate_tar[i].x;
				long pos_ori_y_tar=vec_controlpoint_candidate_tar[i].y;
				long pos_ori_x_sub=vec_controlpoint_candidate_sub[i].x;
				long pos_ori_y_sub=vec_controlpoint_candidate_sub[i].y;
				for(unsigned long b=0;b<vecvec_index_ori2str_branch_tar.size();b++)
				{
					//exclude the current branch
					unsigned long n_crlpt_onformerbranches=0;
					unsigned long k;
					for(k=0;k<vecvec_index_ori2str_branch_tar.size();k++)
					{
						n_crlpt_onformerbranches+=vec_nctlpt_alongbranch[k]*vec_nctlpt_perslice[k];
						if(i<n_crlpt_onformerbranches)
							break;
					}
					if(b==k) continue;

					//if current tail control point also lie within the mask region of head
					if(fabs(vecvec_index_ori2str_branch_tar[b][pos_ori_y_tar][pos_ori_x_tar].x-(-1))>1e-10 || fabs(vecvec_index_ori2str_branch_tar[b][pos_ori_y_tar][pos_ori_x_tar].y-(-1))>1e-10 ||
					   fabs(vecvec_index_ori2str_branch_sub[b][pos_ori_y_sub][pos_ori_x_sub].x-(-1))>1e-10 || fabs(vecvec_index_ori2str_branch_sub[b][pos_ori_y_sub][pos_ori_x_sub].y-(-1))>1e-10)
					{
						vec_controlpoint_candidate_tar[i].x=-1;
						vec_controlpoint_candidate_tar[i].y=-1;
						vec_controlpoint_candidate_sub[i].x=-1;
						vec_controlpoint_candidate_sub[i].y=-1;
					}
				}
			}
			//
		}
	}

	//fill the valid control point vector
	for(unsigned long i=0;i<vec_controlpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_controlpoint_candidate_tar[i].x-(-1))>1e-10 && fabs(vec_controlpoint_candidate_tar[i].y-(-1))>1e-10 &&
		   fabs(vec_controlpoint_candidate_sub[i].x-(-1))>1e-10 && fabs(vec_controlpoint_candidate_sub[i].y-(-1))>1e-10)
		{
			vec_controlpoint_tar.push_back(vec_controlpoint_candidate_tar[i]);
			vec_controlpoint_sub.push_back(vec_controlpoint_candidate_sub[i]);
		}
	}
//	q_points_save2markerfile(vec_controlpoint_tar,"/Users/qul/work/v3d_2.0/q_skeletonbased_warp_sub2tar/data/tar.marker");
//	q_points_save2markerfile(vec_controlpoint_sub,"/Users/qul/work/v3d_2.0/q_skeletonbased_warp_sub2tar/data/sub.marker");

	return true;
}
//find all the candidate control points (if a control point do not lies within image, the coordinate is [-1,-1])
//find given number of control points along the central line of each branch evenly
bool q_candidateanchorpoint_definition_evendis(
		const vector< vector< vector<Coord2D64F_SL> > > vecvec_index_str2ori_branch,
		const vector<long> vec_nctlpt_alongbranch,const vector<long> vec_nctlpt_perslice,
		vector<Coord2D64F_SL> &vec_controlpoint)
{
	//check parameters
	if(vecvec_index_str2ori_branch.size() != vec_nctlpt_alongbranch.size() ||
	   vecvec_index_str2ori_branch.size() != vec_nctlpt_perslice.size() ||
	   vecvec_index_str2ori_branch.size()==0)
	{
		printf("ERROR: q_find_candidatecontrolpoint: invalid input paras. \n");
		return false;
	}
	if(!vec_controlpoint.empty())
	{
		printf("WARNNING: q_find_candidatecontrolpoint: input pos_controlpoint is not empty, original contents is cleared!\n");
		vec_controlpoint.clear();
	}

	long n_branch=vecvec_index_str2ori_branch.size();

	for(long b=0;b<n_branch;b++)
	{
		long sz_strbranch[2];
		sz_strbranch[0]=vecvec_index_str2ori_branch[b][0].size();	//width
		sz_strbranch[1]=vecvec_index_str2ori_branch[b].size();		//height

		//compute sample step along x and y direction of straightened branch
		double d_samplestep_x=(double)sz_strbranch[0]/(double)(vec_nctlpt_alongbranch[b]-1);
		double d_samplestep_y=(double)sz_strbranch[1]/(double)(vec_nctlpt_perslice[b]-1);

		//find all candidate control points in current branch
		Coord2D64F_SL controlpoint;
		for(long i=0;i<vec_nctlpt_alongbranch[b];i++)
			for(long j=0;j<vec_nctlpt_perslice[b];j++)
			{
				long x,y;
				x=d_samplestep_x*i+0.5;
				y=d_samplestep_y*j+0.5;

				if(i==0) 				x=1; 	//skip the first point, this is a bug point derived from cubic-spline interpolation
				if(x>=sz_strbranch[0])	x=sz_strbranch[0]-1;
				if(y>=sz_strbranch[1])	y=sz_strbranch[1]-1;

				//if control point is out of image region, the coordinate is [-1,-1]
				controlpoint.x=vecvec_index_str2ori_branch[b][y][x].x;
				controlpoint.y=vecvec_index_str2ori_branch[b][y][x].y;
				vec_controlpoint.push_back(controlpoint);
			}

	}

	return true;
}

//find anchor points in target and subject image (these anchor points will be used to do TPS warping)
//the output anchor points are part of candidate anchor points generated by q_candidatecontrolpoint_definition_unevendis()
//the invalid candidate points judgement criterias are:
// (1).candidate anchor points which lie within the overlapped mask region of subject or target image but not on skeleton
// (2).candidate anchor points which do not lie within the subject or target image region
bool q_anchorpoint_definition_unevendis(
		const vector< QList<ImageMarker> > &vec_ql_branchcpt_tar,const vector< QList<ImageMarker> > &vec_ql_branchcpt_sub,
		const long l_anchor2cpt_ratio_alongbranch,const long l_anchor_perslice,
		const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_str2ori_branch_tar,const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_ori2str_branch_tar,
		const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_str2ori_branch_sub,const vector< vector< vector<Coord2D64F_SL> > > &vecvec_index_ori2str_branch_sub,
		vector<Coord2D64F_SL> &vec_anchorpoint_tar,vector<Coord2D64F_SL> &vec_anchorpoint_sub)
{
	//check parameters
	if(vec_ql_branchcpt_tar.size()!=vec_ql_branchcpt_sub.size() ||
	   vec_ql_branchcpt_tar.size()!=vecvec_index_str2ori_branch_tar.size() ||
	   vec_ql_branchcpt_tar.size()!=vecvec_index_ori2str_branch_tar.size() ||
	   vec_ql_branchcpt_tar.size()!=vecvec_index_str2ori_branch_sub.size() ||
	   vec_ql_branchcpt_tar.size()!=vecvec_index_ori2str_branch_sub.size() ||
	   vec_ql_branchcpt_tar.size()<=0)

	{
		printf("ERROR: q_anchorpoint_definition_unevendis: invalid input paras. \n");
		return false;
	}
	if(!vec_anchorpoint_tar.empty())
	{
		printf("WARNNING: q_anchorpoint_definition_unevendis: input vec_controlpoint_tar is not empty, original contents is cleared!\n");
		vec_anchorpoint_tar.clear();
	}
	if(!vec_anchorpoint_sub.empty())
	{
		printf("WARNNING: q_anchorpoint_definition_unevendis: input vec_controlpoint_sub is not empty, original contents is cleared!\n");
		vec_anchorpoint_sub.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	long l_nbranch=vec_ql_branchcpt_tar.size();

	//find all candidate anchor points in target and subject image
	vector<Coord2D64F_SL> vec_anchorpoint_candidate_tar,vec_anchorpoint_candidate_sub;

	//target
	if(!q_candidateanchorpoint_definition_unevendis(
			vec_ql_branchcpt_tar,
			vecvec_index_str2ori_branch_tar,vecvec_index_ori2str_branch_tar,
			l_anchor2cpt_ratio_alongbranch,l_anchor_perslice,
			vec_anchorpoint_candidate_tar))
	{
		printf("ERROR: q_candidateanchorpoint_definition_unevendis() for target return false! \n");
		return false;
	}
	//subject
	if(!q_candidateanchorpoint_definition_unevendis(
			vec_ql_branchcpt_sub,
			vecvec_index_str2ori_branch_sub,vecvec_index_ori2str_branch_sub,
			l_anchor2cpt_ratio_alongbranch,l_anchor_perslice,
			vec_anchorpoint_candidate_sub))
	{
		printf("ERROR: q_candidateanchorpoint_definition_unevendis() for subject return false! \n");
		return false;
	}
//	q_points_save2markerfile(vec_controlpoint_candidate_tar,"tar_candi.marker");
//	q_points_save2markerfile(vec_controlpoint_candidate_sub,"sub_candi.marker");

	//------------------------------------------------------------------------------------------------------------------------------------
	//find the valid anchor points for the target and subject image
	// (1).candidate anchor points which lie within the overlapped region of subject or target image are invalid
	// (2).candidate anchor points which do not lie within the subject or target image region are invalid
	//
	//(1). markout the invalid candidate points which do not lie within the target or subject image region (invalid-->[-1,-1])
	for(unsigned long i=0;i<vec_anchorpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_anchorpoint_candidate_tar[i].x-(-1))<1e-10 || fabs(vec_anchorpoint_candidate_tar[i].y-(-1))<1e-10 ||
		   fabs(vec_anchorpoint_candidate_sub[i].x-(-1))<1e-10 || fabs(vec_anchorpoint_candidate_sub[i].y-(-1))<1e-10)
		{
			vec_anchorpoint_candidate_tar[i].x=-1;
			vec_anchorpoint_candidate_tar[i].y=-1;
			vec_anchorpoint_candidate_sub[i].x=-1;
			vec_anchorpoint_candidate_sub[i].y=-1;
		}
	}

	//(2). markout the anchor points that lie on the skeleton and delete the redundant/overlapped ones
	// we will take them as valid eventhough they may lie in the overlapped mask regions
	//find the index of anchor points that lie on the skeleton
	vector<long> vec_ind_anhcoronskeleton;
	long l_ncandianchoronallformerbranch=0;
	if(l_anchor_perslice%2==1)//only if the number of anchor point along per-slice is odd, there will exis anchor points that lie on the skeleton
	{
		for(long b=0;b<l_nbranch;b++)
		{
			long l_nanchoroncurbranch=(vec_ql_branchcpt_tar[b].size()-1)*2+1;
			for(long k=0;k<l_nanchoroncurbranch;k++)
			{
				long ind=((k+1)*l_anchor_perslice-2)-1;
				vec_ind_anhcoronskeleton.push_back(l_ncandianchoronallformerbranch+ind);
			}

			long l_ncandianchoroncurbranch=l_nanchoroncurbranch*l_anchor_perslice;
			l_ncandianchoronallformerbranch+=l_ncandianchoroncurbranch;
		}
	}
	//delete redundant/overlapped anchor points that may exist near the common control points (the border of mask region of different domains)
	for(unsigned long i=0;i<vec_ind_anhcoronskeleton.size();i++)
		for(unsigned long j=i+1;j<vec_ind_anhcoronskeleton.size();j++)
		{
			if(vec_ind_anhcoronskeleton[i]==-1 || vec_ind_anhcoronskeleton[j]==-1) continue;

			//compute the distance of possible pair of anchor points that lie on the skeleton
			long index1=vec_ind_anhcoronskeleton[i];
			long index2=vec_ind_anhcoronskeleton[j];
			double pos1_x=vec_anchorpoint_candidate_tar[index1].x;
			double pos1_y=vec_anchorpoint_candidate_tar[index1].y;
			double pos2_x=vec_anchorpoint_candidate_tar[index2].x;
			double pos2_y=vec_anchorpoint_candidate_tar[index2].y;
			double dis=sqrt((pos1_x-pos2_x)*(pos1_x-pos2_x)+(pos1_y-pos2_y)*(pos1_y-pos2_y));

			//if the distance small enouth (<5 pixels), we only keep the first of them
			if(dis<5) vec_ind_anhcoronskeleton[j]=-1;
		}

	//(3). markout the invalid candidate anchor points which lie within the overlapped mask region of target or subject (invalid-->[-1,-1])
	for(unsigned long i=0;i<vec_anchorpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_anchorpoint_candidate_tar[i].x-(-1))>1e-10 && fabs(vec_anchorpoint_candidate_tar[i].y-(-1))>1e-10 &&
		   fabs(vec_anchorpoint_candidate_sub[i].x-(-1))>1e-10 && fabs(vec_anchorpoint_candidate_sub[i].y-(-1))>1e-10)
		{
			//if current anchor point lies on the skeleton, we take it as valid
			bool b_isonskeleton=false;
			for(unsigned long j=0;j<vec_ind_anhcoronskeleton.size();j++)
				if(vec_ind_anhcoronskeleton[j]!=-1 && i==vec_ind_anhcoronskeleton[j])
				{
					b_isonskeleton=true;
					break;
				}
			if(b_isonskeleton) continue;

			//if current anchor point lies in mask region of any other branch, it is invalid anchor point
			long pos_ori_x_tar=vec_anchorpoint_candidate_tar[i].x;
			long pos_ori_y_tar=vec_anchorpoint_candidate_tar[i].y;
			long pos_ori_x_sub=vec_anchorpoint_candidate_sub[i].x;
			long pos_ori_y_sub=vec_anchorpoint_candidate_sub[i].y;
			for(unsigned long b=0;b<vecvec_index_ori2str_branch_tar.size();b++)
			{
				//exclude the current branch
				unsigned long n_crlpt_onformerbranches=0;
				unsigned long k;
				for(k=0;k<vecvec_index_ori2str_branch_tar.size();k++)
				{
					n_crlpt_onformerbranches+=((vec_ql_branchcpt_tar[k].size()-1)*2+1)*l_anchor_perslice;
					if(i<n_crlpt_onformerbranches)
						break;
				}
				if(b==k) continue;

				//if current anchor point also lie within the mask region of another branch
				if(fabs(vecvec_index_ori2str_branch_tar[b][pos_ori_y_tar][pos_ori_x_tar].x-(-1))>1e-10 || fabs(vecvec_index_ori2str_branch_tar[b][pos_ori_y_tar][pos_ori_x_tar].y-(-1))>1e-10 ||
				   fabs(vecvec_index_ori2str_branch_sub[b][pos_ori_y_sub][pos_ori_x_sub].x-(-1))>1e-10 || fabs(vecvec_index_ori2str_branch_sub[b][pos_ori_y_sub][pos_ori_x_sub].y-(-1))>1e-10)
				{
					vec_anchorpoint_candidate_tar[i].x=-1;
					vec_anchorpoint_candidate_tar[i].y=-1;
					vec_anchorpoint_candidate_sub[i].x=-1;
					vec_anchorpoint_candidate_sub[i].y=-1;
				}
			}
			//
		}
	}

	//fill the valid anchor point array
	for(unsigned long i=0;i<vec_anchorpoint_candidate_tar.size();i++)
	{
		if(fabs(vec_anchorpoint_candidate_tar[i].x-(-1))>1e-10 && fabs(vec_anchorpoint_candidate_tar[i].y-(-1))>1e-10 &&
		   fabs(vec_anchorpoint_candidate_sub[i].x-(-1))>1e-10 && fabs(vec_anchorpoint_candidate_sub[i].y-(-1))>1e-10)
		{
			vec_anchorpoint_tar.push_back(vec_anchorpoint_candidate_tar[i]);
			vec_anchorpoint_sub.push_back(vec_anchorpoint_candidate_sub[i]);
		}
	}
//	q_points_save2markerfile(vec_anchorpoint_tar,"tar.marker");
//	q_points_save2markerfile(vec_anchorpoint_sub,"sub.marker");

	return true;
}
//find anchor points along the smoothed branches unevenly, according to the position of control points
// e.g. take the middle point of two control points if l_ctlpt2node_ratio_alongbranch=2
bool q_candidateanchorpoint_definition_unevendis(
		const vector< QList<ImageMarker> > &vec_ql_branchcpt,
		const vector< vector< vector<Coord2D64F_SL> > > vecvec_index_str2ori_branch,const vector< vector< vector<Coord2D64F_SL> > > vecvec_index_ori2str_branch,
		const long l_anchor2cpt_ratio_alongbranch,const long l_anchor_perslice,
		vector<Coord2D64F_SL> &vec_anchorpoint)
{
	//check parameters
	if(vec_ql_branchcpt.size() != vecvec_index_str2ori_branch.size() ||
	   vec_ql_branchcpt.size() != vecvec_index_ori2str_branch.size() ||
	   vec_ql_branchcpt.size()==0)
	{
		printf("ERROR: q_candidateanchorpoint_definition_unevendis: invalid input paras. \n");
		return false;
	}
	if(l_anchor2cpt_ratio_alongbranch<1)
	{
		printf("ERROR: q_candidateanchorpoint_definition_unevendis: l_anchor2cpt_ratio_alongbranch should >=1. \n");
		return false;
	}
	if(l_anchor_perslice<1)
	{
		printf("ERROR: q_candidateanchorpoint_definition_unevendis: l_anchor_perslice should >=1. \n");
		return false;
	}
	if(!vec_anchorpoint.empty())
	{
		printf("WARNNING: q_candidateanchorpoint_definition_unevendis: input vec_anchorpoint is not empty, original contents is cleared!\n");
		vec_anchorpoint.clear();
	}

	long n_branch=vec_ql_branchcpt.size();

	for(long b=0;b<n_branch;b++)
	{
		long sz_strbranch[2];
		sz_strbranch[0]=vecvec_index_str2ori_branch[b][0].size();	//width
		sz_strbranch[1]=vecvec_index_str2ori_branch[b].size();		//height

		//sampling step along y direction in straightened image (not evenly sampling in x direction)
		double d_samplestep_y=(double)sz_strbranch[1]/(double)(l_anchor_perslice-1);

		for(long c=0;c<vec_ql_branchcpt[b].size();c++)
		{
			if(c!=vec_ql_branchcpt[b].size()-1)
			{
				//compute the x sampling step in current branch segment
				double d_samplestep_x,d_pos_cpt_cur_str_x=-1,d_pos_cpt_next_str_x=-1;
				//compute the x coordinate of current control point in straightened coordinate
				if(c==0)
				{
					d_pos_cpt_cur_str_x=1;//start from 1 column since 0 column is buggy!
				}
				else
				{
					double d_pos_cpt_cur_ori_x=vec_ql_branchcpt[b][c].x;
					double d_pos_cpt_cur_ori_y=vec_ql_branchcpt[b][c].y;
					d_pos_cpt_cur_str_x=vecvec_index_ori2str_branch[b][d_pos_cpt_cur_ori_y][d_pos_cpt_cur_ori_x].x;
					if(d_pos_cpt_cur_str_x==-1)
					{
						printf("ERROR:d_pos_cpt_cur_str_x==-1; branch=%ld,cpt=%ld\n",b,c);
						return false;
					}
				}
				//compute the x coordinate of next control point in straightened coordinate
				if(c+1==vec_ql_branchcpt[b].size()-1)
				{
					d_pos_cpt_next_str_x=sz_strbranch[0]-1;
				}
				else
				{
					double d_pos_cpt_next_ori_x=vec_ql_branchcpt[b][c+1].x;
					double d_pos_cpt_next_ori_y=vec_ql_branchcpt[b][c+1].y;
					d_pos_cpt_next_str_x=vecvec_index_ori2str_branch[b][d_pos_cpt_next_ori_y][d_pos_cpt_next_ori_x].x;
					if(d_pos_cpt_next_str_x==-1)
					{
						printf("ERROR:d_pos_cpt_next_str_x==-1; branch=%ld,cpt=%ld\n",b,c);
						return false;
					}
				}
				//compute the x sampling step in current branch segment
				d_samplestep_x=fabs(d_pos_cpt_next_str_x-d_pos_cpt_cur_str_x)/l_anchor2cpt_ratio_alongbranch;

				//define anchor points in current branch segment area
				Coord2D64F_SL anchorpoint_ori,anchorpoint_str;
				for(long i=0;i<l_anchor2cpt_ratio_alongbranch;i++)
				{
					anchorpoint_str.x=d_pos_cpt_cur_str_x+d_samplestep_x*i;

					for(long j=0;j<l_anchor_perslice;j++)
					{
						anchorpoint_str.y=d_samplestep_y*j;

						if(anchorpoint_str.x>=sz_strbranch[0])	anchorpoint_str.x=sz_strbranch[0]-1;
						if(anchorpoint_str.y>=sz_strbranch[1])	anchorpoint_str.y=sz_strbranch[1]-1;

						anchorpoint_ori.x=vecvec_index_str2ori_branch[b][anchorpoint_str.y][anchorpoint_str.x].x;
						anchorpoint_ori.y=vecvec_index_str2ori_branch[b][anchorpoint_str.y][anchorpoint_str.x].y;
						if(anchorpoint_ori.x==-1 || anchorpoint_ori.y==-1)
							printf("Anchor point out of image region: branch=%ld,cpt=%ld,j=%ld\n",b,c,j);

						vec_anchorpoint.push_back(anchorpoint_ori);
					}
				}
			}
			else
			{
				Coord2D64F_SL anchorpoint_ori,anchorpoint_str;
				anchorpoint_str.x=sz_strbranch[0]-1;

				for(long j=0;j<l_anchor_perslice;j++)
				{
					anchorpoint_str.y=d_samplestep_y*j;

					if(anchorpoint_str.y>=sz_strbranch[1])	anchorpoint_str.y=sz_strbranch[1]-1;

					anchorpoint_ori.x=vecvec_index_str2ori_branch[b][anchorpoint_str.y][anchorpoint_str.x].x;
					anchorpoint_ori.y=vecvec_index_str2ori_branch[b][anchorpoint_str.y][anchorpoint_str.x].y;
					if(anchorpoint_ori.x==-1 || anchorpoint_ori.y==-1)
						printf("Anchor point out of image region: branch=%ld,cpt=%ld,j=%ld\n",b,c,j);

					vec_anchorpoint.push_back(anchorpoint_ori);
				}
			}
		}
	}

	return true;
}

//compute the tar2sub 2d TPS displace field based on the given subject and target control points
//note: not sub2tar in here, since we use inverse project to warp subject image to target image latter
bool q_compute_tps_df_tar2sub_2d(
		const vector <Coord2D64F_SL> &pos_controlpoint_sub,const vector <Coord2D64F_SL> &pos_controlpoint_tar,
		const long df_width,const long df_height,
		Vol3DSimple<DisplaceFieldF3D> *&df_tar2sub)
{
	//check parameters
	if(pos_controlpoint_tar.empty() || pos_controlpoint_sub.empty())
	{
		printf("ERROR: q_compute_tps_df_2d: The input subject or target marker vector is empty.\n");
		return false;
	}
	if(pos_controlpoint_tar.size()!=pos_controlpoint_sub.size())
	{
		printf("ERROR: q_compute_tps_df_2d: The number of markers in subject and target vector is different.\n");
		return false;
	}
	if(df_width<=0 || df_height<=0)
	{
		printf("ERROR: q_compute_tps_df_2d: The input width or height of displace field is invalid.\n");
		return false;
	}
	if(df_tar2sub)
	{
		printf("WARNING: q_compute_tps_df_2d: The input pointer of displace field is no NULL, previous memory it point to will be released.\n");
		delete df_tar2sub;
		df_tar2sub=0;
		return false;
	}

	//compute tar2sub tps warp parameters
	//tps_para_wa=[w;a], where w is a n_marker*1 vector and a is a 3*1 vector
	Matrix tps_para_wa_tar2sub;
	if(!q_compute_tps_paras_2d(pos_controlpoint_tar,pos_controlpoint_sub,tps_para_wa_tar2sub))
	{
		printf("ERROR: q_compute_df_tps_2d: q_compute_tps_paras_2d return false.\n");
		return false;
	}

	//compute the displace field for subject to target: subject+df_tps=target
	if(!q_compute_df_from_tpspara_2d(tps_para_wa_tar2sub,pos_controlpoint_tar,df_width,df_height,df_tar2sub))
	{
		printf("ERROR: q_compute_df_tps_2d: q_compute_tps_paras_2d return false.\n");
		return false;
	}

	return true;
}

//compute sub2tar tps warp parameters based on given subject and target control points
//tps_para_wa=[w;a]
bool q_compute_tps_paras_2d(
		const vector <Coord2D64F_SL> &pos_controlpoint_sub,const vector <Coord2D64F_SL> &pos_controlpoint_tar,
		Matrix &wa)
{
	//check parameters
	if(pos_controlpoint_sub.empty() || pos_controlpoint_tar.empty())
	{
		printf("ERROR: q_compute_tps_paras_2d: The input subject or target marker vector is empty.\n");
		return false;
	}
	if(pos_controlpoint_sub.size()!=pos_controlpoint_tar.size())
	{
		printf("ERROR: q_compute_tps_paras_2d: The number of markers in subject and target vector is different.\n");
		return false;
	}

	int n_marker=pos_controlpoint_sub.size();

	//compute the tps transform parameters
	double tmp,s;

	Matrix wR(n_marker,n_marker);
	for(long j=0;j<n_marker;j++)
	  for(long i=0;i<n_marker;i++)
	  {
		s=0.0;
		tmp=pos_controlpoint_sub.at(i).x-pos_controlpoint_sub.at(j).x;	s+=tmp*tmp;
		tmp=pos_controlpoint_sub.at(i).y-pos_controlpoint_sub.at(j).y; 	s+=tmp*tmp;
//	    wR(i+1,j+1)=2*s*log(s+1e-20);
	    wR(i+1,j+1)=s*log(s+1e-20);
	  }

	Matrix wP(n_marker,3);
	for(long i=0;i<n_marker;i++)
	{
	   wP(i+1,1)=1;
	   wP(i+1,2)=pos_controlpoint_sub.at(i).x;
	   wP(i+1,3)=pos_controlpoint_sub.at(i).y;
	}

	Matrix wL(n_marker+3,n_marker+3);
	wL.submatrix(1,n_marker,1,n_marker)=wR;
	wL.submatrix(1,n_marker,n_marker+1,n_marker+3)=wP;
	wL.submatrix(n_marker+1,n_marker+3,1,n_marker)=wP.t();
	wL.submatrix(n_marker+1,n_marker+3,n_marker+1,n_marker+3)=0;

	Matrix wY(n_marker+3,2);
	for(long i=0;i<n_marker;i++)
	{
	   wY(i+1,1)=pos_controlpoint_tar.at(i).x;
	   wY(i+1,2)=pos_controlpoint_tar.at(i).y;
	}
	wY.submatrix(n_marker+1,n_marker+3,1,2)=0;

	Try
	{
//	    wa=wL.i()*wY;

//		wa=wL.i();
		wa=q_pseudoinverse(wL);

		wa=wa*wY;
	}
	CatchAll
	{
		printf("ERROR: q_compute_tps_paras_2d: Fail to find the (pseudo)inverse of the wL matrix, maybe too sigular.\n");
		return false;
	}

	return true;
}

//compute the displace field based on the given tps parameters
//sub+df_sub2tar[sub]=tar (if the input wa is paras of sub2tar)
bool q_compute_df_from_tpspara_2d(
		const Matrix &wa_sub2tar,const vector <Coord2D64F_SL> &vec_marker_sub,
		const long df_width,const long df_height,
		Vol3DSimple<DisplaceFieldF3D> *&df_sub2tar)
{
	//check parameters
	if(vec_marker_sub.empty())
	{
		printf("ERROR: q_compute_df_from_tpspara_2d: The input subject or target marker vector is empty.\n");
		return false;
	}
	if(df_width<=0 || df_height<=0)
	{
		printf("ERROR: q_compute_df_from_tpspara_2d: The input width or height of displace field is invalid.\n");
		return false;
	}
	if(df_sub2tar)
	{
		printf("WARNING: q_compute_df_from_tpspara_2d: The input pointer of displace field is no NULL, previous memory it point to will be released.\n");
		delete df_sub2tar;
		df_sub2tar=0;
		return false;
	}

	//allocate memory
    double *Ua=new double[vec_marker_sub.size()+3];
	if(!Ua)
	{
		printf("ERROR: q_compute_df_from_tpspara_2d: Fail to allocate memory dist for tps warping.\n");
		return false;
	}

	df_sub2tar=new Vol3DSimple<DisplaceFieldF3D> (df_width,df_height,1);
	if(!df_sub2tar)
	{
		printf("ERROR: q_compute_df_from_tpspara_2d: Fail to allocate memory dist for displace field.\n");
		if(Ua) 	{delete []Ua;	Ua=0;}
		return false;
	}

	//fill displace field
	DisplaceFieldF3D ***df_sub2tar_3d=df_sub2tar->getData3dHandle();
	for(long y=0;y<df_height;y++)
		for(long x=0;x<df_width;x++)
		{
			Coord2D64F_SL pt_sub2tar;
			if(!q_compute_ptwarped_from_tpspara_2d(Coord2D64F_SL(x,y),vec_marker_sub,wa_sub2tar,pt_sub2tar))
			{
				printf("ERROR: q_compute_df_from_tpspara_2d: q_compute_ptwarped_from_tpspara_2d() return false.\n");
				if(Ua) 			{delete []Ua;		Ua=0;}
				if(df_sub2tar) 	{delete df_sub2tar;	df_sub2tar=0;}
				return false;
			}

			df_sub2tar_3d[0][y][x].sx=pt_sub2tar.x-x;
			df_sub2tar_3d[0][y][x].sy=pt_sub2tar.y-y;
			df_sub2tar_3d[0][y][x].sz=0;
		}

	//free memory
	if(Ua) 	{delete []Ua;	Ua=0;}

	return true;
}

//compute the sub2tar warped postion for give subject point based on given sub2tar tps warping parameters
bool q_compute_ptwarped_from_tpspara_2d(
		const Coord2D64F_SL &pt_sub,const vector <Coord2D64F_SL> &vec_ctlpt_sub,const Matrix &wa_sub2tar,
		Coord2D64F_SL &pt_sub2tar)
{
	//check parameters
	if(vec_ctlpt_sub.size()<3)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_2d: Are you sure the input control points are right?.\n");
		return false;
	}

	//allocate memory
    double *Ua=new double[vec_ctlpt_sub.size()+3];
	if(!Ua)
	{
		printf("ERROR: q_compute_ptwarped_from_tpspara_2d: Fail to allocate memory dist for tps warping.\n");
		return false;
	}

	int n_marker=vec_ctlpt_sub.size();
	double tmp,s;

	//calculate distance vector
	for(long n=0;n<n_marker;n++)
	{
		s=0;
		tmp=pt_sub.x-vec_ctlpt_sub.at(n).x;	s+=tmp*tmp;
		tmp=pt_sub.y-vec_ctlpt_sub.at(n).y;	s+=tmp*tmp;
		Ua[n]=s*log(s+1e-20);
	}

	Ua[n_marker]  =1;
	Ua[n_marker+1]=pt_sub.x;
	Ua[n_marker+2]=pt_sub.y;

	s=0;
	for(long p=0;p<n_marker+3;p++)	s+=Ua[p]*wa_sub2tar(p+1,1);
	pt_sub2tar.x=s;

	s=0;
	for(long p=0;p<n_marker+3;p++)	s+=Ua[p]*wa_sub2tar(p+1,2);
	pt_sub2tar.y=s;

	//free memory
	if(Ua) 	{delete []Ua;	Ua=0;}

	return true;
}

//warp subject image to target image based on 2d tar2sub displace field
bool q_warp_sub2tar_baseon_df_tar2sub(
		const unsigned char *p_img_sub,const V3DLONG *sz_img_sub,
		const Vol3DSimple<DisplaceFieldF3D> *df_tar2sub,
		unsigned char *&p_img_sub2tar)
{
	//check parameters
	if(!p_img_sub)
	{
		printf("ERROR: q_warp_sub2tar_baseon_df_tar2sub: The input subject image pointer is NULL.\n");
		return false;
	}
	if(!sz_img_sub || sz_img_sub[0]<=0 || sz_img_sub[1]<=0)
	{
		printf("ERROR: q_warp_sub2tar_baseon_df_tar2sub: The input size array of subject image is invalid.\n");
		return false;
	}
	if(!df_tar2sub)
	{
		printf("ERROR: q_warp_sub2tar_baseon_df_tar2sub: The input pointer of tar2sub displace field is invalid.\n");
		return false;
	}
	if(p_img_sub2tar)
	{
		printf("WARNING: q_warp_sub2tar_baseon_df_tar2sub: The input pointer of sub2tar image is no NULL, previous memory it point to will lost.\n");
		p_img_sub2tar=0;
		return false;
	}

	//allocate memory
	long *sz_img_sub2tar=new long[4];
	if(!sz_img_sub2tar)
	{
		printf("ERROR: q_warp_from_df: Fail to allocate memory dist for warpped image.\n");
		if(sz_img_sub2tar)	{delete[] sz_img_sub2tar; sz_img_sub2tar=0;}
		return false;
	}
	sz_img_sub2tar[0]=sz_img_sub[0];	//x dim
	sz_img_sub2tar[1]=sz_img_sub[1];	//y	dim
	sz_img_sub2tar[2]=sz_img_sub[2];	//z	dim
	sz_img_sub2tar[3]=sz_img_sub[3];	//c	dim

	p_img_sub2tar=new unsigned char[sz_img_sub2tar[0]*sz_img_sub2tar[1]*sz_img_sub2tar[2]*sz_img_sub2tar[3]];
	if(!p_img_sub2tar)
	{
		printf("ERROR: q_warp_from_df: Fail to allocate memory for sub2tar warpped image.\n");
		if(p_img_sub2tar)		{delete[] p_img_sub2tar; p_img_sub2tar=0;}
		if(sz_img_sub2tar)		{delete[] sz_img_sub2tar; sz_img_sub2tar=0;}
		return false;
	}

	unsigned char ****p_inputimg_4d=0,****p_img_sub2tar_4d=0;	//for avoid goto error!
	if(!new4dpointer(p_inputimg_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3],p_img_sub) ||
	   !new4dpointer(p_img_sub2tar_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3],p_img_sub2tar))
	{
		printf("ERROR: q_warp_from_df: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_inputimg_4d) 		{delete4dpointer(p_inputimg_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3]);}
		if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3]);}
		if(p_img_sub2tar)		{delete[] p_img_sub2tar; p_img_sub2tar=0;}
		if(sz_img_sub2tar)		{delete[] sz_img_sub2tar; sz_img_sub2tar=0;}
		return false;
	}

//	//warp image based on given subject to target displace field (NN interpolate)
//	DisplaceFieldF3D ***df_tar2sub_3d=df_tar2sub->getData3dHandle();
//	for(long z=0;z<sz_img_sub2tar[2];z++)
//		for(long y=0;y<sz_img_sub2tar[1];y++)
//			for(long x=0;x<sz_img_sub2tar[0];x++)
//				{
//					long x0,y0;
//					x0=x+floor(df_tar2sub_3d[0][y][x].sx+0.5);
//					y0=y+floor(df_tar2sub_3d[0][y][x].sy+0.5);
////					printf("[%ld,%ld]<-[%ld,%ld], df=[%.2f,%.2f]\n",x,y,x0,y0,df_3d[0][y][x].sx,df_3d[0][y][x].sy);
//					if(x0>0 && x0<sz_img_sub2tar[0] && y0>0 && y0<sz_img_sub2tar[1])
//						for(long c=0;c<sz_img_sub2tar[3];c++)
//							p_img_sub2tar_4d[c][z][y][x]=p_inputimg_4d[c][z][y0][x0];
//				}

	//warp image based on given subject to target displace field (linear interpolate)
	DisplaceFieldF3D ***df_tar2sub_3d=((Vol3DSimple<DisplaceFieldF3D> *)df_tar2sub)->getData3dHandle();
	for(long z=0;z<sz_img_sub2tar[2];z++)
		for(long y=0;y<sz_img_sub2tar[1];y++)
			for(long x=0;x<sz_img_sub2tar[0];x++)
			{
				//current position
				double cur_sub[2];
				cur_sub[0]=x+df_tar2sub_3d[0][y][x].sx;
				cur_sub[1]=y+df_tar2sub_3d[0][y][x].sy;

//				cur_sub[0]=cur_sub[0]<0?0:cur_sub[0];	cur_sub[0]=cur_sub[0]>=sz_warped[0]?sz_warped[0]-1:cur_sub[0];
//				cur_sub[1]=cur_sub[1]<0?0:cur_sub[1];	cur_sub[1]=cur_sub[1]>=sz_warped[1]?sz_warped[1]-1:cur_sub[1];
				//if use above code, the image edge will be streched
				if(cur_sub[0]<0 || cur_sub[0]>=sz_img_sub2tar[0] || cur_sub[1]<0 || cur_sub[1]>=sz_img_sub2tar[1])
					continue;

				//find 4 neighor pixels (use them to do interpolate)
				long lu[2],ru[2],ld[2],rd[2];//[0]:x; [1]:y;
				lu[0]=floor(cur_sub[0]); 	lu[1]=floor(cur_sub[1]);
				rd[0]=ceil(cur_sub[0]); 	rd[1]=ceil(cur_sub[1]);
				ru[0]=rd[0];				ru[1]=lu[1];
				ld[0]=lu[0];				ld[1]=rd[1];

				lu[0]=lu[0]<0?0:lu[0];		lu[0]=lu[0]>=sz_img_sub2tar[0]?sz_img_sub2tar[0]-1:lu[0];
				lu[1]=lu[1]<0?0:lu[1];		lu[1]=lu[1]>=sz_img_sub2tar[1]?sz_img_sub2tar[1]-1:lu[1];
				ru[0]=ru[0]<0?0:ru[0];		ru[0]=ru[0]>=sz_img_sub2tar[0]?sz_img_sub2tar[0]-1:ru[0];
				ru[1]=ru[1]<0?0:ru[1];		ru[1]=ru[1]>=sz_img_sub2tar[1]?sz_img_sub2tar[1]-1:ru[1];
				ld[0]=ld[0]<0?0:ld[0];		ld[0]=ld[0]>=sz_img_sub2tar[0]?sz_img_sub2tar[0]-1:ld[0];
				ld[1]=ld[1]<0?0:ld[1];		ld[1]=ld[1]>=sz_img_sub2tar[1]?sz_img_sub2tar[1]-1:ld[1];
				rd[0]=rd[0]<0?0:rd[0];		rd[0]=rd[0]>=sz_img_sub2tar[0]?sz_img_sub2tar[0]-1:rd[0];
				rd[1]=rd[1]<0?0:rd[1];		rd[1]=rd[1]>=sz_img_sub2tar[1]?sz_img_sub2tar[1]-1:rd[1];

				//compute weight for left and right, up and down
				double l_w,r_w,u_w,d_w;
				l_w=1.0-(cur_sub[0]-lu[0]);	r_w=1.0-l_w;
				u_w=1.0-(cur_sub[1]-lu[1]);	d_w=1.0-u_w;

				//linear iterpolate the current pixel value
				for(long c=0;c<sz_img_sub2tar[3];c++)
					p_img_sub2tar_4d[c][z][y][x]=u_w*(l_w*p_inputimg_4d[c][z][lu[1]][lu[0]]+r_w*p_inputimg_4d[c][z][ru[1]][ru[0]])+
											     d_w*(l_w*p_inputimg_4d[c][z][ld[1]][ld[0]]+r_w*p_inputimg_4d[c][z][rd[1]][rd[0]])+0.5;//+0.5 for round-off
			}

	//free memory
	if(p_inputimg_4d) 		{delete4dpointer(p_inputimg_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3]);}
	if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_sub2tar[0],sz_img_sub2tar[1],sz_img_sub2tar[2],sz_img_sub2tar[3]);}
	if(sz_img_sub2tar)		{delete[] sz_img_sub2tar; sz_img_sub2tar=0;}

	return true;
}


//save the points position to marker file
bool q_points_save2markerfile(
		const vector<Coord2D64F_SL> &pos_pointset,
		const char *filename_marker)
{
	//check parameters
	if(pos_pointset.empty())
	{
		printf("ERROR: q_savepoints_to_markerfile: The input point set vector is empty!\n");
		return false;
	}
	if(!filename_marker)
	{
		printf("ERROR: q_savepoints_to_markerfile: The output marker filename pointer is invalid.\n");
		return false;
	}

	FILE *fp;
	fp=fopen(filename_marker,"w");
	if(fp==NULL)
	{
		printf("ERROR: q_savepoints_to_markerfile: Failed to open file to write! \n");
		return false;
	}
	fprintf(fp,"#x,y,z,radius,shape,name,comment\n");
	for(unsigned long i=0;i<pos_pointset.size();i++)
	{
		fprintf(fp,"%5.3f,%5.3f,1,0,1,,\n",pos_pointset[i].x,pos_pointset[i].y);
	}
	fclose(fp);

	return true;
}

//save the input larval image with head and butt straighten area mask overlaid
bool q_saveimg_strmask_overlaid(
		const unsigned char *p_inputimg,const V3DLONG sz_inputimg[4],
		const vector< vector< vector<Coord2D64F_SL> > > vecvecvec_index_ori2str_branches,
		const char *filename_outputimg)
{
	//check parameters
	if(!p_inputimg)
	{
		printf("ERROR: q_saveimg_strmask_overlaid: The input image DATA pointer is empty.\n");
		return false;
	}
	if(!sz_inputimg)
	{
		printf("ERROR: q_saveimg_strmask_overlaid: The input image SIZE pointer is empty.\n");
		return false;
	}
	if(vecvecvec_index_ori2str_branches.empty())
	{
		printf("ERROR: q_saveimg_strmask_overlaid: The input ori2str mapping index is empty!\n");
		return false;
	}
	if(!filename_outputimg)
	{
		printf("ERROR: q_saveimg_strmask_overlaid: The output image filename pointer is invalid.\n");
		return false;
	}

	//allocate memory
	unsigned char *p_outputimg=0;
	unsigned char ****p_outputimg_4d=0,****p_inputimg_4d=0;
	p_outputimg=new unsigned char[sz_inputimg[0]*sz_inputimg[1]*sz_inputimg[2]*sz_inputimg[3]];
	if(!p_outputimg)
	{
		printf("ERROR: q_saveimg_strmask_overlaid: fall to allocate memory for output image!\n");
		if(p_outputimg)		{delete []p_outputimg;	p_outputimg=0;}
		return false;
	}
	if(!new4dpointer(p_outputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3],p_outputimg) ||
	   !new4dpointer(p_inputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3],p_inputimg))
	{
		printf("ERROR: q_saveimg_strmask_overlaid: fall to allocate memory for 4d image pointer!\n");
		if(p_outputimg_4d)	{delete4dpointer(p_outputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}
		if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}
		return false;
	}

	//draw the mask
	long l_mask_gain=50;
	double d_background_damp=0.5;
	for(long x=0;x<sz_inputimg[0];x++)
		for(long y=0;y<sz_inputimg[1];y++)
			for(long z=0;z<sz_inputimg[2];z++)
				for(long c=0;c<sz_inputimg[3];c++)
				{
					p_outputimg_4d[c][z][y][x]=p_inputimg_4d[c][z][y][x]*d_background_damp;
					for(unsigned long i=0;i<vecvecvec_index_ori2str_branches.size();i++)
					{
						if(vecvecvec_index_ori2str_branches[i][y][x].x!=-1)
						{
							if(p_outputimg_4d[c][z][y][x]>255-l_mask_gain)
								p_outputimg_4d[c][z][y][x]=255;
							else
								p_outputimg_4d[c][z][y][x]+=l_mask_gain;
						}
					}
				}

	//save central line overlaid image to file
	if(!saveImage(filename_outputimg,(unsigned char *)p_outputimg,sz_inputimg,1))
	{
		printf("ERROR: q_saveimg_strmask_overlaid: write image to [%s] fall!\n",filename_outputimg);
		if(p_outputimg)		{delete []p_outputimg;	p_outputimg=0;}
		if(p_outputimg_4d)	{delete4dpointer(p_outputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}
		if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}
		return false;
	}

	//free memory
	if(p_outputimg)		{delete []p_outputimg;	p_outputimg=0;}
	if(p_outputimg_4d)	{delete4dpointer(p_outputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}
	if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d,sz_inputimg[0],sz_inputimg[1],sz_inputimg[2],sz_inputimg[3]);}

	return true;
}

//first: compute the df_sub2tar based on the input df_tar2sub (note: we did not interpolate the df_sub2tar due to lazy)
//then : warp central line of subject image to target use df_sub2tar (note: not all subpos can find df in df_sub2tar, we only keep the valid one)
//last:  save the warped central line to swc file
//the warped central line should be very similar to the central line of target image
//the warped central line do not have the same points as input, since we did not interpolate the df_sub2tar
//we simply output the points happen to be in the displace field of tar2sub
bool q_save_warpedskeleton2swcfile_1(
		const vector<Coord2D64F_SL> &vec_centralline_head_tar,const vector<Coord2D64F_SL> &vec_centralline_tail_tar,
		const vector<Coord2D64F_SL> &vec_centralline_head_sub,const vector<Coord2D64F_SL> &vec_centralline_tail_sub,
		Vol3DSimple<DisplaceFieldF3D> *df_tar2sub,
		const char *filename_outputswc)
{
	//check parameters
	if(vec_centralline_head_tar.empty() || vec_centralline_tail_tar.empty() ||
	   vec_centralline_head_sub.empty() || vec_centralline_tail_sub.empty())
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_1: at least one input vector is empty.\n");
		return false;
	}
	if(!df_tar2sub)
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_1: input target to subject displace field pointer is NULL.\n");
		return false;
	}
	if(!filename_outputswc)
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_1: output swc file name pointer is NULL.\n");
		return false;
	}

	//local variables definition
	vector<Coord2D64F_SL> vec_centralline_head_sub_warped,vec_centralline_tail_sub_warped;

	long sz_img_tar[2];
	sz_img_tar[0]=df_tar2sub->sz0();
	sz_img_tar[1]=df_tar2sub->sz1();

	Vol3DSimple<DisplaceFieldF3D> *df_sub2tar=0;
	df_sub2tar=new Vol3DSimple<DisplaceFieldF3D>(sz_img_tar[0],sz_img_tar[1],1);
	if(!df_sub2tar)
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_1: fall to allocate memory for displace field!\n");
		return false;
	}
	DisplaceFieldF3D ***df_sub2tar_3d=df_sub2tar->getData3dHandle();
	DisplaceFieldF3D ***df_tar2sub_3d=df_tar2sub->getData3dHandle();

	//initial fill the sub2tar displace field
	//it's better to interpolate the df_sub2tar, but i am too lazy to do that
	for(long y_tar=0;y_tar<sz_img_tar[1];y_tar++)
		for(long x_tar=0;x_tar<sz_img_tar[0];x_tar++)
		{
			long x_sub,y_sub;
			x_sub=x_tar+floor(df_tar2sub_3d[0][y_tar][x_tar].sx+0.5);
			y_sub=y_tar+floor(df_tar2sub_3d[0][y_tar][x_tar].sy+0.5);
			if(x_sub>=0 && x_sub<sz_img_tar[0] && y_sub>=0 && y_sub<sz_img_tar[1])
			{
				df_sub2tar_3d[0][y_sub][x_sub].sx=-df_tar2sub_3d[0][y_tar][x_tar].sx;
				df_sub2tar_3d[0][y_sub][x_sub].sy=-df_tar2sub_3d[0][y_tar][x_tar].sy;
			}
		}

	//find the corresponding points of central line in warped image use df_sub2tar
	for(unsigned long i=1;i<vec_centralline_tail_sub.size();i++)//should skip the first point since it is wrong!
	{
		long x_sub=vec_centralline_tail_sub[i].x+0.5;
		long y_sub=vec_centralline_tail_sub[i].y+0.5;
		if(x_sub>=0 && x_sub<sz_img_tar[0] && y_sub>=0 && y_sub<sz_img_tar[1])
		{
			long x_warp=x_sub+df_sub2tar_3d[0][y_sub][x_sub].sx;
			long y_warp=y_sub+df_sub2tar_3d[0][y_sub][x_sub].sy;
			if(fabs(df_sub2tar_3d[0][y_sub][x_sub].sx)>1e-10 || fabs(df_sub2tar_3d[0][y_sub][x_sub].sy)>1e-10)
				vec_centralline_tail_sub_warped.push_back(Coord2D64F_SL(x_warp,y_warp));
		}
	}
	for(unsigned long i=1;i<vec_centralline_head_sub.size();i++)//should skip the first point since it is wrong!
	{
		long x_sub=vec_centralline_head_sub[i].x+0.5;
		long y_sub=vec_centralline_head_sub[i].y+0.5;
		if(x_sub>=0 && x_sub<sz_img_tar[0] && y_sub>=0 && y_sub<sz_img_tar[1])
		{
			long x_warp=x_sub+df_sub2tar_3d[0][y_sub][x_sub].sx;
			long y_warp=y_sub+df_sub2tar_3d[0][y_sub][x_sub].sy;
			if(fabs(df_sub2tar_3d[0][y_sub][x_sub].sx)>1e-10 || fabs(df_sub2tar_3d[0][y_sub][x_sub].sy)>1e-10)
				vec_centralline_head_sub_warped.push_back(Coord2D64F_SL(x_warp,y_warp));
		}
	}

	//save the warped central line to swc file
	NeuronTree nt_skeleton;
	NeuronSWC ns_marker;
	//build larva skeleton NeuronTree
	for(unsigned long i=0;i<vec_centralline_tail_sub_warped.size();i++)
	{
		ns_marker.n=i+1;							//index
		ns_marker.r=2;								//radius
		ns_marker.x=vec_centralline_tail_sub_warped[i].x;			//x
		ns_marker.y=vec_centralline_tail_sub_warped[i].y;			//y
		ns_marker.pn=i;								//fill parent node index
		if(i==0) ns_marker.pn=-1;
		nt_skeleton.listNeuron.push_back(ns_marker);
	}
	for(unsigned long i=0;i<vec_centralline_head_sub_warped.size();i++)
	{
		ns_marker.n=vec_centralline_tail_sub_warped.size()+i+1;	//index
		ns_marker.r=2;								//radius
		ns_marker.x=vec_centralline_head_sub_warped[i].x;			//x
		ns_marker.y=vec_centralline_head_sub_warped[i].y;			//y
		ns_marker.pn=vec_centralline_tail_sub_warped.size()+i;		//fill parent node index
		if(i==0) ns_marker.pn=-1;
		nt_skeleton.listNeuron.push_back(ns_marker);
	}

	writeSWC_file(filename_outputswc,nt_skeleton);

	//free memory
	if(df_sub2tar) 		{delete df_sub2tar;				df_sub2tar=0;}

	return true;
}

//first: compute the sub2tar tps warp paras based on the input sub and tar control points
//then:  warp central line of subject image to target use sub2tar tps pars
//last:  save warped central line to swc file
bool q_save_warpedskeleton2swcfile_2(
		const vector <Coord2D64F_SL> &pos_controlpoint_sub,const vector <Coord2D64F_SL> &pos_controlpoint_tar,
		const vector<Coord2D64F_SL> &vec_centralline_head_sub,const vector<Coord2D64F_SL> &vec_centralline_tail_sub,
		const char *filename_outputswc)
{
	//check parameters
	if(pos_controlpoint_sub.empty() || pos_controlpoint_tar.empty())
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_2: at least one input vector is empty.\n");
		return false;
	}
	if(vec_centralline_head_sub.empty() || vec_centralline_tail_sub.empty())
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_2: at least one input vector is empty.\n");
		return false;
	}
	if(!filename_outputswc)
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_2: output swc file name pointer is NULL.\n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//compute the sub2tar tps paras
	Matrix tps_para_wa_sub2tar;
	if(!q_compute_tps_paras_2d(pos_controlpoint_sub,pos_controlpoint_tar,tps_para_wa_sub2tar))
	{
		printf("ERROR: q_save_warpedskeleton2swcfile_2: q_compute_tps_paras_2d return false.\n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//warp central line of subject 2 target using sub2tar tps para
	vector <Coord2D64F_SL> vec_centralline_head_sub2tar(vec_centralline_head_sub),vec_centralline_tail_sub2tar(vec_centralline_tail_sub);
	Coord2D64F_SL pt_sub2tar;
	//head
	for(unsigned long i=0;i<vec_centralline_head_sub.size();i++)
	{
		if(!q_compute_ptwarped_from_tpspara_2d(vec_centralline_head_sub[i],pos_controlpoint_sub,tps_para_wa_sub2tar,pt_sub2tar))
		{
			printf("ERROR: q_save_warpedskeleton2swcfile_2: q_compute_ptwarped_from_tpspara_2d return false.\n");
			return false;
		}
		vec_centralline_head_sub2tar[i].x=pt_sub2tar.x;
		vec_centralline_head_sub2tar[i].y=pt_sub2tar.y;
	}
	//tail
	for(unsigned long i=0;i<vec_centralline_tail_sub.size();i++)
	{
		if(!q_compute_ptwarped_from_tpspara_2d(vec_centralline_tail_sub[i],pos_controlpoint_sub,tps_para_wa_sub2tar,pt_sub2tar))
		{
			printf("ERROR: q_save_warpedskeleton2swcfile_2: q_compute_ptwarped_from_tpspara_2d return false.\n");
			return false;
		}
		vec_centralline_tail_sub2tar[i].x=pt_sub2tar.x;
		vec_centralline_tail_sub2tar[i].y=pt_sub2tar.y;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//save the warped central line to swc file
	NeuronTree nt_skeleton;
	NeuronSWC ns_marker;
	//build larva skeleton NeuronTree
	for(unsigned long i=1;i<vec_centralline_tail_sub2tar.size();i++)
	{
		ns_marker.n=i+1;							//index
		ns_marker.r=2;								//radius
		ns_marker.x=vec_centralline_tail_sub2tar[i].x;			//x
		ns_marker.y=vec_centralline_tail_sub2tar[i].y;			//y
		ns_marker.pn=i;								//fill parent node index
		if(i==0) ns_marker.pn=-1;
		nt_skeleton.listNeuron.push_back(ns_marker);
	}
	for(unsigned long i=1;i<vec_centralline_head_sub2tar.size();i++)
	{
		ns_marker.n=vec_centralline_tail_sub2tar.size()+i+1;	//index
		ns_marker.r=2;								//radius
		ns_marker.x=vec_centralline_head_sub2tar[i].x;			//x
		ns_marker.y=vec_centralline_head_sub2tar[i].y;			//y
		ns_marker.pn=vec_centralline_tail_sub2tar.size()+i;		//fill parent node index
		if(i==0) ns_marker.pn=-1;
		nt_skeleton.listNeuron.push_back(ns_marker);
	}

	writeSWC_file(filename_outputswc,nt_skeleton);

	return true;
}


////************************************************************************************************************************************
////save test image (for debug)
//long sz_img_test[4];
//sz_img_test[0]=nx;	//x dim
//sz_img_test[1]=ny;	//y	dim
//sz_img_test[2]=nz;	//z	dim
//sz_img_test[3]=nc;	//c	dim
//saveImage("test.tif",(unsigned char *)p_img_test,sz_img_test,1);
////************************************************************************************************************************************
