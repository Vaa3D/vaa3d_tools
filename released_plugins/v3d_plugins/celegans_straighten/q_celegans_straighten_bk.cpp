// q_celegans_straighten.h
// by Lei Qu
// 2010-08-11

#include <vector>
using namespace std;

#include "../../v3d_main/basic_c_fun/stackutil.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"
#include "../../v3d_main/worm_straighten_c/spline_cubic.h"

#include "q_imresize.h"
#include "q_principalskeleton_detection.h"

#include "q_celegans_straighten.h"

bool q_celegans_straighten(V3DPluginCallback &callback,const CSParas &paras,
		const unsigned char *p_img_input,const long sz_img_input[4],
		const double arr_marker[2][3],
		unsigned char *&p_img_output,long *&sz_img_output)
{
	//check paras
	if(p_img_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]<1)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(paras.l_refchannel>sz_img_input[3] || paras.l_refchannel<0)
	{
		printf("ERROR: l_refchannel is invalid!\n");
		return false;
	}
	if(p_img_output!=0 || sz_img_output!=0)
	{
		printf("WARNNING: Output image or size pointer is not null!\n");
		p_img_output=0;
		sz_img_output=0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//generate domain info
	vector< vector<long> > vecvec_domain_length_ind,vecvec_domain_smooth_ind;	//the index of control point of each domain is refer to the corresponding marker file
	vector<double> vec_domain_length_weight,vec_domain_smooth_weight;
	{
	vector<long> tmp;
	for(long i=0;i<paras.l_ctlpts_num;i++)
		tmp.push_back(i);
	vecvec_domain_length_ind.push_back(tmp);
	vecvec_domain_smooth_ind.push_back(tmp);
	vec_domain_length_weight.push_back(1);
	vec_domain_smooth_weight.push_back(1);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(1). Extract the interesting channel: [%d]. \n",paras.l_refchannel);
	unsigned char *p_img_1c=0;
	long sz_img_1c[4]={sz_img_input[0],sz_img_input[1],sz_img_input[2],1};

	{
	p_img_1c=new unsigned char[sz_img_1c[0]*sz_img_1c[1]*sz_img_input[2]]();
	if(!p_img_1c)
	{
		printf("ERROR: fail to allocate memory for reference image!\n");
		return false;
	}

	long pgsz_x=sz_img_1c[0];
	long pgsz_xy=sz_img_1c[0]*sz_img_1c[1];
	long pgsz_xyz=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];
	for(long z=0;z<sz_img_1c[2];z++)
		for(long y=0;y<sz_img_1c[1];y++)
			for(long x=0;x<sz_img_1c[0];x++)
			{
				long index=pgsz_xyz*paras.l_refchannel+pgsz_xy*z+pgsz_x*y+x;
				long index_1c=pgsz_xy*z+pgsz_x*y+x;
				p_img_1c[index_1c]=p_img_input[index];
			}
	}

//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/1c.raw",(unsigned char *)p_img_1c,sz_img_1c,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(2). Resize image. \n");
	unsigned char *p_img_s=0;
	long sz_img_s[4]={1,1,1,1};

	{
	for(long i=0;i<3;i++)
		sz_img_s[i]=sz_img_1c[i]/paras.d_downsampleratio+0.5;

	if(!q_imresize8u_3D(p_img_1c,sz_img_1c,sz_img_s,p_img_s))
	{
		printf("ERROR: q_imresize8u_3D() return false!\n");
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
	printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);
	}

//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/s.raw",(unsigned char *)p_img_s,sz_img_s,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(3). Straighten in XY plane. \n");
	unsigned char *p_img_sub2tar_xy=0;
	long *sz_img_sub2tar_xy=0;

	//------------------------------------------------------------------
	printf("\t>>generate mip_xy image. \n");
	unsigned char *p_img_mip_xy=0;
	long sz_img_mip_xy[4]={sz_img_s[0],sz_img_s[1],1,sz_img_s[3]};

	{
	p_img_mip_xy=new unsigned char[sz_img_s[0]*sz_img_s[1]*sz_img_s[3]]();
	if(!p_img_mip_xy)
	{
		printf("ERROR:Fail to allocate memory for the MIP image. \n");
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}

	long pgsz_x=sz_img_s[0];
	long pgsz_xy=sz_img_s[0]*sz_img_s[1];
	long pgsz_xyz=sz_img_s[0]*sz_img_s[1]*sz_img_s[2];
	unsigned char u_MIP_rgb[3];
	for(long y=0;y<sz_img_s[1];y++)
		for(long x=0;x<sz_img_s[0];x++)
		{
			u_MIP_rgb[0]=u_MIP_rgb[1]=u_MIP_rgb[2]=0;
			for(long z=0;z<sz_img_s[2];z++)
				for(long c=0;c<sz_img_s[3];c++)
				{
					long index=pgsz_xyz*c+pgsz_xy*z+pgsz_x*y+x;
					if(p_img_s[index]>u_MIP_rgb[c])
						u_MIP_rgb[c]=p_img_s[index];
				}

			for(long c=0;c<sz_img_s[3];c++)
			{
				long index_MIP=pgsz_xy*c+pgsz_x*y+x;
				p_img_mip_xy[index_MIP]=u_MIP_rgb[c];
			}
		}
	}
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/mip_xy.raw",(unsigned char *)p_img_mip_xy,sz_img_mip_xy,1);

	//------------------------------------------------------------------
	printf("\t>>detect principal skeleton. \n");
	QList<ImageMarker> ql_cptpos_output_xy;

	{
	vector<point3D64F> vec_cptpos_input,vec_cptpos_output;

	//initialize the skeleton
	double a=(arr_marker[1][1]-arr_marker[0][1])/(arr_marker[1][0]-arr_marker[0][0]);
	double b=arr_marker[0][1]-a*arr_marker[0][0];
	double d_step=(arr_marker[1][0]-arr_marker[0][0])/(paras.l_ctlpts_num-1);
	for(long i=0;i<paras.l_ctlpts_num;i++)
	{
		point3D64F tmp;
		tmp.x=arr_marker[0][0]+i*d_step;
		tmp.y=a*tmp.x+b;
		tmp.x/=paras.d_downsampleratio;
		tmp.y/=paras.d_downsampleratio;
		vec_cptpos_input.push_back(tmp);
	}

	//principal skeleton detection parameters
	PSDParas paras_input;
	paras_input.l_maxitertimes=500;
	paras_input.d_stopiter_threshold=0.01;
	paras_input.d_foreground_treshold=paras.d_fgthresh_xy;
	paras_input.l_diskradius_openning=paras.l_radius_openclose;
	paras_input.l_diskradius_closing=paras.l_radius_openclose;
	paras_input.b_showinV3D_pts=paras.b_showinV3D_pts;
	if(!q_principalskeleton_detection(callback,
			p_img_mip_xy,sz_img_mip_xy,
			vec_cptpos_input,
			vecvec_domain_length_ind,vec_domain_length_weight,
			vecvec_domain_smooth_ind,vec_domain_smooth_weight,
			paras_input,
			vec_cptpos_output))
	{
		printf("ERROR:q_principalskeleton_detection() return false!\n");
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}

	for(long i=0;i<vec_cptpos_output.size();i++)
	{
		ImageMarker tmp;
		tmp.x=vec_cptpos_output[i].x;
		tmp.y=vec_cptpos_output[i].y;
		ql_cptpos_output_xy.push_back(tmp);
	}
	}

	//------------------------------------------------------------------
	printf("\t>>do straighten in xy plane. \n");
	{
	long l_cuttingplane_width=(paras.l_radius_cuttingplane/paras.d_downsampleratio)*2+1;;
	if(!q_celegans_restacking_xy(
			p_img_s,sz_img_s,
			ql_cptpos_output_xy,l_cuttingplane_width,
			p_img_sub2tar_xy,sz_img_sub2tar_xy))
   {
		printf("ERROR:q_celegans_restacking_xy() return false!\n");
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	printf("sz_img_sub2tar:%d,%d,%d,%d\n",sz_img_sub2tar_xy[0],sz_img_sub2tar_xy[1],sz_img_sub2tar_xy[2],sz_img_sub2tar_xy[3]);
	}

	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/sub2tar_xy.raw",(unsigned char *)p_img_sub2tar_xy,sz_img_sub2tar_xy,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(4). Straighten in XZ plane. \n");
	unsigned char *p_img_sub2tar_xz=0;
	long *sz_img_sub2tar_xz=0;

	//------------------------------------------------------------------
	printf("\t>>generate mip_xz image. \n");
	unsigned char *p_img_mip_xz=0;
	long sz_img_mip_xz[4]={sz_img_sub2tar_xy[0],sz_img_sub2tar_xy[2],1,sz_img_sub2tar_xy[3]};

	{
	p_img_mip_xz=new unsigned char[sz_img_sub2tar_xy[0]*sz_img_sub2tar_xy[2]*sz_img_sub2tar_xy[3]]();
	if(!p_img_mip_xz)
	{
		printf("ERROR:Fail to allocate memory for the MIP image. \n");
		if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
		if(sz_img_sub2tar_xy) 			{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}

	long pgsz_x=sz_img_sub2tar_xy[0];
	long pgsz_xy=sz_img_sub2tar_xy[0]*sz_img_sub2tar_xy[1];
	long pgsz_xz=sz_img_sub2tar_xy[0]*sz_img_sub2tar_xy[2];
	long pgsz_xyz=sz_img_sub2tar_xy[0]*sz_img_sub2tar_xy[1]*sz_img_sub2tar_xy[2];
	unsigned char u_MIP_rgb[3];
	for(long z=0;z<sz_img_sub2tar_xy[2];z++)
		for(long x=0;x<sz_img_sub2tar_xy[0];x++)
		{
			u_MIP_rgb[0]=u_MIP_rgb[1]=u_MIP_rgb[2]=0;
			for(long y=0;y<sz_img_sub2tar_xy[1];y++)
				for(long c=0;c<sz_img_sub2tar_xy[3];c++)
				{
					long index=pgsz_xyz*c+pgsz_xy*z+pgsz_x*y+x;
					if(p_img_sub2tar_xy[index]>u_MIP_rgb[c])
						u_MIP_rgb[c]=p_img_sub2tar_xy[index];
				}

			for(long c=0;c<sz_img_sub2tar_xy[3];c++)
			{
				long index_MIP=pgsz_xz*c+pgsz_x*z+x;
				p_img_mip_xz[index_MIP]=u_MIP_rgb[c];
			}
		}
	if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
	}
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/mip_xz.raw",(unsigned char *)p_img_mip_xz,sz_img_mip_xz,1);

	//------------------------------------------------------------------
	printf("\t>>detect principal skeleton. \n");
	QList<ImageMarker> ql_cptpos_output_xz;

	{
	vector<point3D64F> vec_cptpos_input,vec_cptpos_output;

	//initialize skeleton
	point3D64F marker_head,marker_tail;
	marker_head.x=0;					marker_head.y=0;	marker_head.z=arr_marker[0][2]/paras.d_downsampleratio;
	marker_tail.x=sz_img_mip_xz[0]-1;	marker_tail.y=0;	marker_tail.z=arr_marker[1][2]/paras.d_downsampleratio;
	double a=(marker_tail.z-marker_head.z)/(marker_tail.x-marker_head.x);
	double b=marker_head.z-a*marker_head.x;
	double d_step=(marker_tail.x-marker_head.x)/(paras.l_ctlpts_num-1);

	for(long i=0;i<paras.l_ctlpts_num;i++)
	{
		point3D64F tmp;
		tmp.x=i*d_step;
		tmp.y=a*tmp.x+b;;
		vec_cptpos_input.push_back(tmp);
	}

	//principal skeleton detection
	PSDParas paras_input;
	paras_input.l_maxitertimes=500;
	paras_input.d_stopiter_threshold=0.01;
	paras_input.d_foreground_treshold=paras.d_fgthresh_xz;
	paras_input.l_diskradius_openning=paras.l_radius_openclose;
	paras_input.l_diskradius_closing=paras.l_radius_openclose;
	if(!q_principalskeleton_detection(callback,
			p_img_mip_xz,sz_img_mip_xz,
			vec_cptpos_input,
			vecvec_domain_length_ind,vec_domain_length_weight,
			vecvec_domain_smooth_ind,vec_domain_smooth_weight,
			paras_input,
			vec_cptpos_output))
	{
		printf("ERROR:q_principalskeleton_detection() return false!\n");
		if(p_img_mip_xz) 				{delete []p_img_mip_xz;			p_img_mip_xz=0;}
		if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
		if(sz_img_sub2tar_xy) 			{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	if(p_img_mip_xz) 				{delete []p_img_mip_xz;			p_img_mip_xz=0;}
	if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
	if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}

	for(long i=0;i<vec_cptpos_output.size();i++)
	{
		ImageMarker tmp;
		tmp.x=vec_cptpos_output[i].x;
		tmp.z=vec_cptpos_output[i].y;
		ql_cptpos_output_xz.push_back(tmp);
	}
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("(5). Prepare the full-resolution output. \n");
	for(long i=0;i<ql_cptpos_output_xy.size();i++)
	{
		ql_cptpos_output_xy[i].x*=paras.d_downsampleratio;
		ql_cptpos_output_xy[i].y*=paras.d_downsampleratio;
	}
	for(long i=0;i<ql_cptpos_output_xz.size();i++)
	{
		ql_cptpos_output_xz[i].x*=paras.d_downsampleratio;
		ql_cptpos_output_xz[i].z*=paras.d_downsampleratio;
	}

	printf("\t>>do straighten in xy plane. \n");
	{
	if(p_img_sub2tar_xy) 	{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
	if(sz_img_sub2tar_xy) 	{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}

	long l_cuttingplane_width=paras.l_radius_cuttingplane*2+1;
	if(!q_celegans_restacking_xy(
			p_img_input,sz_img_input,
			ql_cptpos_output_xy,l_cuttingplane_width,
			p_img_sub2tar_xy,sz_img_sub2tar_xy))
	{
		printf("ERROR:q_curve_smoothstraighten() return false!\n");
		if(p_img_mip_xz) 				{delete []p_img_mip_xz;			p_img_mip_xz=0;}
		if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
		if(sz_img_sub2tar_xy) 			{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	printf("sz_img_sub2tar:%d,%d,%d,%d\n",sz_img_sub2tar_xy[0],sz_img_sub2tar_xy[1],sz_img_sub2tar_xy[2],sz_img_sub2tar_xy[3]);
	}
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/sub2tar_xy.raw",(unsigned char *)p_img_sub2tar_xy,sz_img_sub2tar_xy,1);

	printf("\t>>do straighten in xz plane. \n");
	if(p_img_sub2tar_xz) 	{delete []p_img_sub2tar_xz;		p_img_sub2tar_xz=0;}
	if(sz_img_sub2tar_xz) 	{delete []sz_img_sub2tar_xz;	sz_img_sub2tar_xz=0;}

	{
	long l_cuttingplane_width=paras.l_radius_cuttingplane*2+1;
	if(!q_celegans_restacking_xz(
			p_img_sub2tar_xy,sz_img_sub2tar_xy,
			ql_cptpos_output_xz,l_cuttingplane_width,
			p_img_sub2tar_xz,sz_img_sub2tar_xz))
	{
		printf("ERROR:q_curve_smoothstraighten() return false!\n");
		if(p_img_mip_xz) 				{delete []p_img_mip_xz;			p_img_mip_xz=0;}
		if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
		if(sz_img_sub2tar_xy) 			{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}
		if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
		if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
		if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}
		return false;
	}
	printf("sz_img_sub2tar:%d,%d,%d,%d\n",sz_img_sub2tar_xz[0],sz_img_sub2tar_xz[1],sz_img_sub2tar_xz[2],sz_img_sub2tar_xz[3]);
	}
//	saveImage("/Users/qul/work/v3d_2.0/sub_projects/celegant_straighten/sub2tar_xz.raw",(unsigned char *)p_img_sub2tar_xz,sz_img_sub2tar_xz,1);


	p_img_output=p_img_sub2tar_xz;
	sz_img_output=sz_img_sub2tar_xz;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Free memory\n");
//	if(p_img_sub2tar_xz) 			{delete []p_img_sub2tar_xz;		p_img_sub2tar_xz=0;}
//	if(sz_img_sub2tar_xz) 			{delete []sz_img_sub2tar_xz;	sz_img_sub2tar_xz=0;}
	if(p_img_mip_xz) 				{delete []p_img_mip_xz;			p_img_mip_xz=0;}
	if(p_img_sub2tar_xy) 			{delete []p_img_sub2tar_xy;		p_img_sub2tar_xy=0;}
	if(sz_img_sub2tar_xy) 			{delete []sz_img_sub2tar_xy;	sz_img_sub2tar_xy=0;}
	if(p_img_mip_xy) 				{delete []p_img_mip_xy;			p_img_mip_xy=0;}
	if(p_img_s) 					{delete []p_img_s;				p_img_s=0;}
	if(p_img_1c) 					{delete []p_img_1c;				p_img_1c=0;}

	return true;
}


bool q_celegans_restacking_xy(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg)
{
	//check parameters
	if(!p_inputimg)
	{
		printf("ERROR: The input larval image pointer is empty.\n");
		return false;
	}
	if(!sz_inputimg)
	{
		printf("ERROR: The input image SIZE pointer is empty.\n");
		return false;
	}
	if(ql_marker.size()==0)
	{
		printf("ERROR: The input marker list is empty.\n");
		return false;
	}
	if(l_width<=0)
	{
		printf("ERROR: The l_width should > 0.\n");
		return false;
	}
	if(p_strimg)
	{
		printf("ERROR: The output image DATA pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}
	if(sz_strimg)
	{
		printf("ERROR: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//estimate the cubic spline parameters
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

	//cubic spline interpolate the input markers
	double *cp_x=0,*cp_y=0,*cp_z=0,*cp_alpha=0;
	long cutPlaneNum=0;
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

	//------------------------------------------------------------------------------------------------------------------------------------
	//restacking along the smoothed curve
	if(!q_restacking_alongbcurve_xy(p_inputimg,sz_inputimg,cp_x,cp_y,cp_alpha,cutPlaneNum,l_width,p_strimg,sz_strimg))
	{
		printf("ERROR: q_restacking_alongbcurve_xy() return false! \n");
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

bool q_celegans_restacking_xz(
		const unsigned char *p_inputimg,const long *sz_inputimg,
		const QList<ImageMarker> &ql_marker,const long l_width,
		unsigned char *&p_strimg,long *&sz_strimg)
{
	//check parameters
	if(!p_inputimg)
	{
		printf("ERROR: The input larval image pointer is empty.\n");
		return false;
	}
	if(!sz_inputimg)
	{
		printf("ERROR: The input image SIZE pointer is empty.\n");
		return false;
	}
	if(ql_marker.size()==0)
	{
		printf("ERROR: The input marker list is empty.\n");
		return false;
	}
	if(l_width<=0)
	{
		printf("ERROR: The l_width should > 0.\n");
		return false;
	}
	if(p_strimg)
	{
		printf("ERROR: The output image DATA pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}
	if(sz_strimg)
	{
		printf("ERROR: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL.\n");
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//estimate the cubic spline parameters
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
	cpara=est_cubic_spline_2d(xpos,zpos,NPoints,false);

	//cubic spline interpolate the input markers
	double *cp_x=0,*cp_y=0,*cp_z=0,*cp_alpha=0;
	long cutPlaneNum=0;
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

	//------------------------------------------------------------------------------------------------------------------------------------
	//restacking along the smoothed curve
	if(!q_restacking_alongbcurve_xz(p_inputimg,sz_inputimg,cp_x,cp_y,cp_alpha,cutPlaneNum,l_width,p_strimg,sz_strimg))
	{
		printf("ERROR: q_restacking_alongbcurve_xy() return false! \n");
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


bool q_restacking_alongbcurve_xy(
		const unsigned char *p_inputimg,const long *sz_inputimg,
        const double *pos_curve_x,const double *pos_curve_y,const double *alpha_curve,const long length_curve,
		const long l_width,
		unsigned char *&p_strimg, long *&sz_strimg)
{
	//check parameters
	if(!p_inputimg || !sz_inputimg)
	{
		printf("ERROR: pointer p_inputimg or sz_inputimg is invalid. \n");
		return false;
	}
	long nx=sz_inputimg[0];
	long ny=sz_inputimg[1];
	long nz=sz_inputimg[2];
	long nc=sz_inputimg[3];
	if(nx<=1 || ny<=1 || nz<=0 || nc<=0)
	{
		printf("ERROR: The SIZE information of input image is not correct. \n");
		return false;
	}
	if(!pos_curve_x || !pos_curve_y || !alpha_curve || length_curve<=0 || l_width<=0)
	{
		printf("ERROR: The input b-spline curve's parameters are incorrect. \n");
		return false;
	}
	if(p_strimg)
	{
		printf("WARNNING: The output image pointer is not empty (may contain data) or not initialized as NULL. \n");
		p_strimg=0;
	}
	if(sz_strimg)
	{
		printf("WARNNING: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL. \n");
		sz_strimg=0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//allocate memory for input and output image, and size array
	sz_strimg=new long[4];
	if(!sz_strimg)
	{
		printf("ERROR: Fail to allocate memory for the SIZE array of the straightened image. \n");
		return false;
	}
	sz_strimg[0]=length_curve;
	sz_strimg[1]=l_width;
	sz_strimg[2]=nz;
	sz_strimg[3]=nc;

	p_strimg=new unsigned char[sz_strimg[0]*sz_strimg[1]*sz_strimg[2]*sz_strimg[3]]();
	if(!p_strimg)
	{
		printf("ERROR: Fail to allocate memory for the straightened image. \n");
		if(sz_strimg) {delete []sz_strimg; sz_strimg=0;}
		return false;
	}

	unsigned char ****p_strimg_4d=0, ****p_inputimg_4d=0;
	if (!new4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3], p_strimg) ||
		!new4dpointer(p_inputimg_4d, nx, ny, nz, nc, p_inputimg))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_strimg)		{delete []p_strimg; p_strimg=0;}
		if(sz_strimg)		{delete []sz_strimg; sz_strimg=0;}
		if(p_strimg_4d)		{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}
		if(p_inputimg_4d)	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//straighten image along curve
	int ptspace=1; // default value
	double base0=0;
	long Krad=0;
	if(floor(l_width/2)*2==l_width)
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
				continue;

			//linear interpolate using 4 neighbors
			long cpx0=long(floor(curpx)), cpx1=long(ceil(curpx));
			long cpy0=long(floor(curpy)), cpy1=long(ceil(curpy));
			double w0x0y=(cpx1-curpx)*(cpy1-curpy);
			double w0x1y=(cpx1-curpx)*(curpy-cpy0);
			double w1x0y=(curpx-cpx0)*(cpy1-curpy);
			double w1x1y=(curpx-cpx0)*(curpy-cpy0);
			for(long z=0;z<nz; z++)
				for(long c=0;c<nc;c++)
					p_strimg_4d[c][z][row][col]=(unsigned char)(w0x0y*double(p_inputimg_4d[c][z][cpy0][cpx0])+w0x1y*double(p_inputimg_4d[c][z][cpy1][cpx0])+
														     w1x0y*double(p_inputimg_4d[c][z][cpy0][cpx1])+w1x1y*double(p_inputimg_4d[c][z][cpy1][cpx1]));
		}
	}

	//free memory
	if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
	if(p_strimg_4d) 	{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}

	return true;
}

bool q_restacking_alongbcurve_xz(
		const unsigned char *p_inputimg,const long *sz_inputimg,
        const double *pos_curve_x,const double *pos_curve_z,const double *alpha_curve,const long length_curve,
		const long l_width,
		unsigned char *&p_strimg, long *&sz_strimg)
{
	//check parameters
	if(!p_inputimg || !sz_inputimg)
	{
		printf("ERROR: pointer p_inputimg or sz_inputimg is invalid. \n");
		return false;
	}
	long nx=sz_inputimg[0];
	long ny=sz_inputimg[1];
	long nz=sz_inputimg[2];
	long nc=sz_inputimg[3];
	if(nx<=1 || ny<=1 || nz<=0 || nc<=0)
	{
		printf("ERROR: The SIZE information of input image is not correct. \n");
		return false;
	}
	if(!pos_curve_x || !pos_curve_z || !alpha_curve || length_curve<=0 || l_width<=0)
	{
		printf("ERROR: The input b-spline curve's parameters are incorrect. \n");
		return false;
	}
	if(p_strimg)
	{
		printf("WARNNING: The output image pointer is not empty (may contain data) or not initialized as NULL. \n");
		p_strimg=0;
	}
	if(sz_strimg)
	{
		printf("WARNNING: The output image SIZE pointer is not empty (may contain data) or not initialized as NULL. \n");
		sz_strimg=0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//allocate memory for input and output image, and size array
	sz_strimg=new long[4];
	if(!sz_strimg)
	{
		printf("ERROR: Fail to allocate memory for the SIZE array of the straightened image. \n");
		return false;
	}
	sz_strimg[0]=length_curve;
	sz_strimg[1]=ny;
	sz_strimg[2]=l_width;
	sz_strimg[3]=nc;

	p_strimg=new unsigned char[sz_strimg[0]*sz_strimg[1]*sz_strimg[2]*sz_strimg[3]]();
	if(!p_strimg)
	{
		printf("ERROR: Fail to allocate memory for the straightened image. \n");
		if(sz_strimg) {delete []sz_strimg; sz_strimg=0;}
		return false;
	}

	unsigned char ****p_strimg_4d=0, ****p_inputimg_4d=0;
	if (!new4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3], p_strimg) ||
		!new4dpointer(p_inputimg_4d, nx, ny, nz, nc, p_inputimg))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_strimg)		{delete []p_strimg; p_strimg=0;}
		if(sz_strimg)		{delete []sz_strimg; sz_strimg=0;}
		if(p_strimg_4d)		{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}
		if(p_inputimg_4d)	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//straighten image along curve
	int ptspace=1; // default value
	double base0=0;
	long Krad=0;
	if(floor(l_width/2)*2==l_width)
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
		double ptminz=pos_curve_z[col]-sin(curalpha)*(base0+Krad*ptspace);

		for(long row=0;row<l_width;row++)
		{
			double curpx=ptminx+cos(curalpha)*(row*ptspace);
			double curpz=ptminz+sin(curalpha)*(row*ptspace);
			if(curpx<0 || curpx>nx-1 || curpz<0 || curpz>nz-1)
				continue;

			//linear interpolate using 4 neighbors
			long cpx0=long(floor(curpx)), cpx1=long(ceil(curpx));
			long cpz0=long(floor(curpz)), cpz1=long(ceil(curpz));
			double w0x0z=(cpx1-curpx)*(cpz1-curpz);
			double w0x1z=(cpx1-curpx)*(curpz-cpz0);
			double w1x0z=(curpx-cpx0)*(cpz1-curpz);
			double w1x1z=(curpx-cpx0)*(curpz-cpz0);
			for(long y=0;y<ny;y++)
				for(long c=0;c<nc;c++)
					p_strimg_4d[c][row][y][col]=(unsigned char)(w0x0z*double(p_inputimg_4d[c][cpz0][y][cpx0])+w0x1z*double(p_inputimg_4d[c][cpz1][y][cpx0])+
														     w1x0z*double(p_inputimg_4d[c][cpz0][y][cpx1])+w1x1z*double(p_inputimg_4d[c][cpz1][y][cpx1]));
		}
	}

	//free memory
	if(p_inputimg_4d) 	{delete4dpointer(p_inputimg_4d, nx, ny, nz, nc);}
	if(p_strimg_4d) 	{delete4dpointer(p_strimg_4d, sz_strimg[0], sz_strimg[1], sz_strimg[2], sz_strimg[3]);}

	return true;
}
