//q_principalskeleton_detection.cpp
//deform the input initial principal skeleton based on given image
//by Lei Qu
//2009-11-17

#include <stdio.h>
#include <vector>
using namespace std;

#include <math.h>
#include "stackutil.h"
#include "q_morphology.h"
#include "q_principalskeleton_detection.h"
 

//given 2D gray image and initial skeleton (position, domain+weight), output deformed principal skeleton
//Processing flow:
//(1). parse the topology of skeleton from input domain file (extract neighbor index and weight, skeleton end points ...)
//(4). extract foreground region from input image
//(5). perform morphology operation(closing+opening) on the foreground image (fill small hole and remove small object)
//(6). generate base image(on which we deform the principal skeleton). [method 1:base=foregound, method 2:base=mask+bone]
//(7). compute voronoi region of each control point, the external term and internal term, update the skeleton iteratively
//(8). stop iteration when the total shift of skeleton less than the given threshold (0.01)
//
//Parameters:
//p_img_input:			input 2D gray image pointer
//sz_img_input:			input 2D gray image size array pointer, [width, height]
//vec_cptpos_input:		input control points position array
//vecvec_domaincptind_length: input domain definition for length constraint (index of control points in every domain)
//vec_domainweight_length: input domain weight definition for length constraint
//paras_input:			input parameters for principal skeleton detection
//vec_cptpos_output:	output deformed control points position array
//
bool q_principalskeleton_detection(
		const unsigned char *p_img_input,const V3DLONG sz_img_input[2],
		const vector<point3D64F> &vec_cptpos_input,
		const vector< vector<V3DLONG> > &vecvec_domaincptind_length,const vector<double> &vec_domainweight_length,
		const vector< vector<V3DLONG> > &vecvec_domaincptind_smooth,const vector<double> &vec_domainweight_smooth,
		const PSDParas &paras_input,
		vector<point3D64F> &vec_cptpos_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_larva_principal_skeleton: Input image pointer is NULL.\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0)
	{
		printf("ERROR: q_larva_principal_skeleton: Input image size is invalid.\n");
		return false;
	}
	if(vec_cptpos_input.empty())
	{
		printf("ERROR: q_larva_principal_skeleton: Input skeleton position array is emtpy.\n");
		return false;
	}
	if(vecvec_domaincptind_length.empty() || vecvec_domaincptind_length.size()!=vec_domainweight_length.size())
	{
		printf("ERROR: q_larva_principal_skeleton: Input domain definition for lenght constraint is invalid or do not match with weight definition.\n");
		return false;
	}
	if(vecvec_domaincptind_smooth.empty() || vecvec_domaincptind_smooth.size()!=vec_domainweight_smooth.size())
	{
		printf("ERROR: q_larva_principal_skeleton: Input domain definition for smooth constraint is invalid or do not match with weight definition.\n");
		return false;
	}
	if(paras_input.d_stopiter_threshold<=0 || paras_input.d_foreground_treshold<0 || paras_input.l_diskradius_openning<0 || paras_input.l_diskradius_closing<0)
	{
		printf("ERROR: q_larva_principal_skeleton: Input parameters is invalid.\n");
		return false;
	}
	if(!vec_cptpos_output.empty())
	{
		printf("WARNNING: q_larva_principal_skeleton: Output skeleton position array is not empty, original contents will be cleared!\n");
		vec_cptpos_output.clear();
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//parse the topology of initial prinicipal skeleton (including neighbor index, weight and skeleton ends)
	printf("\t>>parse the topology of initial prinicipal skeleton:\n");
	vector< vector<V3DLONG> > vecvec_neighborcptind_length,vecvec_neighborcptind_smooth;
	vector< vector<double> > vecvec_neighborweight_length,vecvec_neighborweight_smooth;
	//initialize neighbor array
	vecvec_neighborcptind_length.assign(vec_cptpos_input.size(),vector<V3DLONG>(0,0));
	vecvec_neighborweight_length.assign(vec_cptpos_input.size(),vector<double>(0,0));
	vecvec_neighborcptind_smooth.assign(vec_cptpos_input.size(),vector<V3DLONG>(0,0));
	vecvec_neighborweight_smooth.assign(vec_cptpos_input.size(),vector<double>(0,0));
	//fill neighbor control point array
	for(unsigned V3DLONG k=0;k<vec_cptpos_input.size();k++)
	{
		//lenght constraint domain
		for(unsigned V3DLONG i=0;i<vecvec_domaincptind_length.size();i++)
			for(unsigned V3DLONG j=0;j<vecvec_domaincptind_length[i].size();j++)
			{
				V3DLONG index=vecvec_domaincptind_length[i][j];
				if(index==(V3DLONG)k)
				{
					if(j>0)										//left neighbor exist
					{
						vecvec_neighborcptind_length[k].push_back(vecvec_domaincptind_length[i][j-1]);
						vecvec_neighborweight_length[k].push_back(vec_domainweight_length[i]);
					}
					if(j<vecvec_domaincptind_length[i].size()-1)//right neighbor exist
					{
						vecvec_neighborcptind_length[k].push_back(vecvec_domaincptind_length[i][j+1]);
						vecvec_neighborweight_length[k].push_back(vec_domainweight_length[i]);
					}
				}
			}
		//smooth constraint domain
		for(unsigned V3DLONG i=0;i<vecvec_domaincptind_smooth.size();i++)
			for(unsigned V3DLONG j=0;j<vecvec_domaincptind_smooth[i].size();j++)
			{
				V3DLONG index=vecvec_domaincptind_smooth[i][j];
				if(index==(V3DLONG)k)
				{
					if(j>0)										//left neighbor exist
					{
						vecvec_neighborcptind_smooth[k].push_back(vecvec_domaincptind_smooth[i][j-1]);
						vecvec_neighborweight_smooth[k].push_back(vec_domainweight_smooth[i]);
					}
					if(j<vecvec_domaincptind_smooth[i].size()-1)//right neighbor exist
					{
						vecvec_neighborcptind_smooth[k].push_back(vecvec_domaincptind_smooth[i][j+1]);
						vecvec_neighborweight_smooth[k].push_back(vec_domainweight_smooth[i]);
					}
				}
			}
	}
	//find skeleton ends (which need special treatment) and their two nearest connected neighbors
	//if a control point only has one neighbor according to the lenght constraint domain definition, we take it as skeleton end
	vector<V3DLONG> vec_skeletonend_ind;
	vector< vector<V3DLONG> > vecvec_skeletonend_2neighbor_ind;
	for(unsigned V3DLONG i=0;i<vecvec_neighborcptind_length.size();i++)
	{
		if(vecvec_neighborcptind_length[i].size()==1)
		{
			vec_skeletonend_ind.push_back(i);

			vector<V3DLONG> vec_skeletonend_2neighbor_ind;
			V3DLONG neighbor1_ind=vecvec_neighborcptind_length[i][0];
			vec_skeletonend_2neighbor_ind.push_back(neighbor1_ind);
			if(vecvec_neighborcptind_length[neighbor1_ind].size()==2)
			{
				if(vecvec_neighborcptind_length[neighbor1_ind][0]!=(V3DLONG)i)
					vec_skeletonend_2neighbor_ind.push_back(vecvec_neighborcptind_length[neighbor1_ind][0]);
				else if(vecvec_neighborcptind_length[neighbor1_ind][1]!=(V3DLONG)i)
					vec_skeletonend_2neighbor_ind.push_back(vecvec_neighborcptind_length[neighbor1_ind][1]);
			}
			vecvec_skeletonend_2neighbor_ind.push_back(vec_skeletonend_2neighbor_ind);
		}
	}

	printf("\t\tcpt[n]'s neighbor: [index,weight,type]\n");
	for(unsigned V3DLONG i=0;i<vecvec_neighborcptind_length.size();i++)
	{
		printf("\t\tcpt[%ld]'s neighbor: ",i);
		for(unsigned V3DLONG j=0;j<vecvec_neighborcptind_length[i].size();j++)
			printf("[%ld,%.2f,L];",vecvec_neighborcptind_length[i][j],vecvec_neighborweight_length[i][j]);
		for(unsigned V3DLONG j=0;j<vecvec_neighborcptind_smooth[i].size();j++)
			printf("[%ld,%.2f,S];",vecvec_neighborcptind_smooth[i][j],vecvec_neighborweight_smooth[i][j]);
		printf("\n");
	}
	printf("\t\tend points and their 2 connected neighbors:\n");
	for(unsigned V3DLONG i=0;i<vec_skeletonend_ind.size();i++)
	{
		printf("\t\tend pt index: [%ld], neighbor index: ",vec_skeletonend_ind[i]);
		for(unsigned V3DLONG j=0;j<vecvec_skeletonend_2neighbor_ind[i].size();j++)
			printf("[%ld] ",vecvec_skeletonend_2neighbor_ind[i][j]);
		printf("\n");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//extract foreground region (on which we find the voronoi region for every control points)
	printf("\t>>extract foreground region ... \n");
	V3DLONG n_imgpixel=sz_img_input[0]*sz_img_input[1];

	//compute the mean and std of image
	double d_img_mean=0.0,d_img_std=0.0;
	for(V3DLONG i=0;i<n_imgpixel;i++)
		d_img_mean+=p_img_input[i];
	d_img_mean/=n_imgpixel;
	for(V3DLONG i=0;i<n_imgpixel;i++)
	{
		double temp=p_img_input[i]-d_img_mean;
		d_img_std+=temp*temp;
	}
	d_img_std=sqrt(d_img_std/n_imgpixel);
	printf("\t\t>>img_mean=%.2f, img_std=%.2f\n",d_img_mean,d_img_std);

	//extract foreground region
	double f_thresh_mask;
	f_thresh_mask=d_img_mean*paras_input.d_foreground_treshold;

    unsigned char *p_img_foreground=0;
    p_img_foreground=new unsigned char[sz_img_input[0]*sz_img_input[1]]();
	if(!p_img_foreground)
	{
		printf("ERROR:Fail to allocate memory for foreground image. \n");
    	if(p_img_foreground) 		{delete []p_img_foreground;		p_img_foreground=0;}
		return false;
	}

	for(V3DLONG y=0;y<sz_img_input[1];y++)
		for(V3DLONG x=0;x<sz_img_input[0];x++)
		{
			V3DLONG index=sz_img_input[0]*y+x;
			if(p_img_input[index]>f_thresh_mask)
			{
				p_img_foreground[index]=100;
			}
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//perform morphology operation(closing+opening) on the foreground image (in orer to fill small hole and remove small object)
	printf("\t>>perform morphology operation on the foreground image ... \n");
	CKernelMP kernel_mask;
	q_create_kernel(paras_input.l_diskradius_openning*2+1,paras_input.l_diskradius_openning*2+1,paras_input.l_diskradius_openning,paras_input.l_diskradius_openning,1,kernel_mask);
	unsigned char *p_img_foreground_co=0,*p_img_temp=0;
	printf("\t\t>>opening ... \n");
	if(!q_opening(p_img_foreground,sz_img_input[0],sz_img_input[1],kernel_mask,p_img_temp))
	{
		printf("ERROR:q_closing() return false. \n");
		if(p_img_foreground)	{delete []p_img_foreground;		p_img_foreground=0;}
		if(p_img_temp) 			{delete []p_img_temp;			p_img_temp=0;}
		return false;
	}
	printf("\t\t>>closing ... \n");
	if(!q_closing(p_img_temp,sz_img_input[0],sz_img_input[1],kernel_mask,p_img_foreground_co))
	{
		printf("ERROR:q_opening() return false. \n");
		if(p_img_foreground)	{delete []p_img_foreground;		p_img_foreground=0;}
		if(p_img_temp) 			{delete []p_img_temp;			p_img_temp=0;}
		return false;
	}
	if(p_img_temp) 				{delete []p_img_temp;			p_img_temp=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	//remove noise tissue by keeping the connected region in the central area (by qul@20120730)
	if(paras_input.b_removeboundaryartifact)
	{
		unsigned char *p_img_connected=0,*p_img_connected_tmp=0;
		p_img_connected=new unsigned char[sz_img_input[0]*sz_img_input[1]]();
		//find seed
		V3DLONG pos_seed[2];
		{
		pos_seed[0]=sz_img_input[0]/2.0+0.5;//x
		pos_seed[1]=sz_img_input[1]/2.0+0.5;//y
		for(V3DLONG i=0;i<sz_img_input[0];i++)
		{
			V3DLONG index=sz_img_input[0]*pos_seed[1]+pos_seed[0];
			if(p_img_foreground_co[index]>10)
				break;
			else
				pos_seed[0]++;
		}
		}
		//find connected region according to given seed
		p_img_connected[sz_img_input[0]*pos_seed[1]+pos_seed[0]]=100;
		CKernelMP kernel_8connect;
		q_create_kernel(3,3,1,1,0,kernel_8connect);
		while(1)
		{
			q_dilation(p_img_connected,sz_img_input[0],sz_img_input[1],kernel_mask,p_img_connected_tmp);

			V3DLONG error=0;
			for(V3DLONG y=0;y<sz_img_input[1];y++)
				for(V3DLONG x=0;x<sz_img_input[0];x++)
				{
					V3DLONG index=sz_img_input[0]*y+x;
					if(p_img_foreground_co[index]>10 && p_img_connected_tmp[index]>10 && p_img_connected[index]<=10)
					{
						p_img_connected[index]=100;
						error++;
					}
				}
			if(p_img_connected_tmp) 				{delete []p_img_connected_tmp;			p_img_connected_tmp=0;}

			if(error==0) break;
		}

		if(p_img_foreground_co) {delete []p_img_foreground_co;	p_img_foreground_co=0;}
		p_img_foreground_co=p_img_connected;
		p_img_connected=0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//record the foreground pixel position (for voronoi decompositon) and generate base image
	//generate base image (on which we deform the principal skeleton)
	printf("\t>>record the foreground pixel position and generate base image ... \n");
	vector<V3DLONG> vec_foregroundpixel_x,vec_foregroundpixel_y;
    unsigned char *p_img_base=0;
    p_img_base=new unsigned char[sz_img_input[0]*sz_img_input[1]]();
	if(!p_img_base)
	{
		printf("ERROR:Fail to allocate memory for base image. \n");
		if(p_img_foreground)	{delete []p_img_foreground;		p_img_foreground=0;}
		if(p_img_foreground_co) {delete []p_img_foreground_co;	p_img_foreground_co=0;}
		return false;
	}

	for(V3DLONG y=0;y<sz_img_input[1];y++)
		for(V3DLONG x=0;x<sz_img_input[0];x++)
		{
			V3DLONG index=sz_img_input[0]*y+x;
			if(p_img_foreground_co[index]>10)
			{
				//record the mask pixel position
				vec_foregroundpixel_x.push_back(x);
				vec_foregroundpixel_y.push_back(y);

				//generate base image
				if(paras_input.i_baseimage_methhod==0)
					p_img_base[index]=p_img_input[index];
				else
					p_img_base[index]=100;
			}
		}

//	V3DLONG sz_img_test[4];
//	sz_img_test[0]=sz_img_input[0];	//x dim
//	sz_img_test[1]=sz_img_input[1];	//y	dim
//	sz_img_test[2]=1;	//z	dim
//	sz_img_test[3]=1;	//c	dim
////	saveImage("baseimg.tif",(unsigned char *)p_img_base,sz_img_test,1);
//	saveImage("baseimg.tif",(unsigned char *)p_img_foreground_co,sz_img_test,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	//initialize the skeleton (shift to the centroid of foreground and zoom out)
	printf("\t>>initialize the prinicpal skeleton (shift and scale) ... \n");
	vector<point3D64F> vec_cptpos_ini(vec_cptpos_input);

	{
		double skeletoncentroid_x=0,skeletoncentroid_y=0;
		double foregroundcentroid_x=0,foregroundcentroid_y=0;

		//compute the centroid of skeleton
		V3DLONG skeleton_left,skeleton_right,skeleton_top,skeleton_bottom;
		skeleton_left=sz_img_input[0];	skeleton_right=0;
		skeleton_top=sz_img_input[1];	skeleton_bottom=0;
		for(unsigned V3DLONG i=0;i<vec_cptpos_input.size();i++)
		{
			if(vec_cptpos_input[i].x<skeleton_left)			skeleton_left	=vec_cptpos_input[i].x;
			else if(vec_cptpos_input[i].x>skeleton_right)	skeleton_right	=vec_cptpos_input[i].x;
			if(vec_cptpos_input[i].y<skeleton_top)			skeleton_top	=vec_cptpos_input[i].y;
			else if(vec_cptpos_input[i].y>skeleton_bottom)	skeleton_bottom	=vec_cptpos_input[i].y;
		}
		skeletoncentroid_x=(skeleton_left+skeleton_right)/2.0;
		skeletoncentroid_y=(skeleton_top+skeleton_bottom)/2.0;

		//compute the centroid of foreground region
		for(unsigned V3DLONG i=0;i<vec_foregroundpixel_x.size();i++)
		{
			foregroundcentroid_x+=vec_foregroundpixel_x[i];
			foregroundcentroid_y+=vec_foregroundpixel_y[i];
		}
		foregroundcentroid_x/=vec_foregroundpixel_x.size();
		foregroundcentroid_y/=vec_foregroundpixel_y.size();

		//align the centroid of skeleton and foreground
		V3DLONG offset_skeleton_x,offset_skeleton_y;
		offset_skeleton_x=foregroundcentroid_x-skeletoncentroid_x;
		offset_skeleton_y=foregroundcentroid_y-skeletoncentroid_y;
		for(unsigned V3DLONG i=0;i<vec_cptpos_ini.size();i++)
		{
			vec_cptpos_ini[i].x+=offset_skeleton_x;
			vec_cptpos_ini[i].y+=offset_skeleton_y;
		}

		//zoom out the skeleton (if the skeleton is too big, some marker will not find valid voronoi region)
		float f_zoomfactor_skeleton=1;//larva:0.1, vnc:1
//		f_zoomfactor_skeleton=paras_input.d_inizoomfactor_skeleton;
		for(unsigned V3DLONG i=0;i<vec_cptpos_ini.size();i++)
		{
			vec_cptpos_ini[i].x=(vec_cptpos_ini[i].x-foregroundcentroid_x)*f_zoomfactor_skeleton+foregroundcentroid_x;
			vec_cptpos_ini[i].y=(vec_cptpos_ini[i].y-foregroundcentroid_y)*f_zoomfactor_skeleton+foregroundcentroid_y;
		}
	}


	//************************************************************************************************************************************
	// enter skeleton deformation iteration
	//************************************************************************************************************************************
	printf("\t>>enter iteration loop ... \n");
	vector<point3D64F> vec_cptpos_last(vec_cptpos_ini),vec_cptpos_new(vec_cptpos_ini);

	V3DLONG n_maxiter=paras_input.l_maxitertimes;
	for(V3DLONG iter=0;iter<n_maxiter;iter++)
	{
		//------------------------------------------------------------------------------------------------------------------------------------
		//compute the voronoi region for each control point in the skeleton
		vector< vector<V3DLONG> > vecvec_voronoi_x(vec_cptpos_ini.size(),vector<V3DLONG>(0,0));
		vector< vector<V3DLONG> > vecvec_voronoi_y(vec_cptpos_ini.size(),vector<V3DLONG>(0,0));

		//compute the distance matrix of every foreground pixel to every control point
		vector< vector<V3DLONG> > vecvec_dismatrix_pixel2ctlpt(vec_foregroundpixel_x.size(),vector<V3DLONG>(vec_cptpos_ini.size(),0));
		for(unsigned V3DLONG p=0;p<vec_foregroundpixel_x.size();p++)
			for(unsigned V3DLONG m=0;m<vec_cptpos_ini.size();m++)
			{
				V3DLONG dif_x=vec_foregroundpixel_x[p]-vec_cptpos_last[m].x;
				V3DLONG dif_y=vec_foregroundpixel_y[p]-vec_cptpos_last[m].y;
				vecvec_dismatrix_pixel2ctlpt[p][m]=dif_x*dif_x+dif_y*dif_y;
			}

		//find the voronoi region (coordinate of foreground pixels) for each control point
		for(unsigned V3DLONG p=0;p<vec_foregroundpixel_x.size();p++)
		{
			//find the index of nearest control point for each foreground pixel
			V3DLONG l_nearestctlpt_ind=0;
			for(unsigned V3DLONG m=0;m<vec_cptpos_ini.size();m++)
				if(vecvec_dismatrix_pixel2ctlpt[p][m]<vecvec_dismatrix_pixel2ctlpt[p][l_nearestctlpt_ind])
					l_nearestctlpt_ind=m;

			//fill voronoi vector
			vecvec_voronoi_x[l_nearestctlpt_ind].push_back(vec_foregroundpixel_x[p]);
			vecvec_voronoi_y[l_nearestctlpt_ind].push_back(vec_foregroundpixel_y[p]);
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		//update the postion of control points
		for(unsigned V3DLONG ind_cpt=0;ind_cpt<vec_cptpos_ini.size();ind_cpt++)
		{
			double E_image[2]={0.0,0.0},E_length[2]={0.0,0.0};

			//compute the image term (mass center of current control point's voronoi region)
			V3DLONG n_voronoipixel=vecvec_voronoi_x[ind_cpt].size();
			double sum_intensity=0;
			if(n_voronoipixel!=0)
			{
				for(V3DLONG p=0;p<n_voronoipixel;p++)
				{
					V3DLONG index=sz_img_input[0]*vecvec_voronoi_y[ind_cpt][p]+vecvec_voronoi_x[ind_cpt][p];
					unsigned char intensity=p_img_base[index];
					E_image[0]+=intensity*vecvec_voronoi_x[ind_cpt][p];
					E_image[1]+=intensity*vecvec_voronoi_y[ind_cpt][p];
					sum_intensity+=intensity;
				}
				E_image[0]/=sum_intensity;
				E_image[1]/=sum_intensity;
			}
			//if the voronoi region of this marker is NULL, we keep the image term same as last iteration
			//this means that in this case, the marker will only be dragged by length term(i.e. by neighbor marker points)
			else
			{
				E_image[0]=vec_cptpos_last[ind_cpt].x;
				E_image[1]=vec_cptpos_last[ind_cpt].y;
			}

			//compute the length term
			double d_weightsum=0.0;
			for(unsigned V3DLONG i=0;i<vecvec_neighborcptind_length[ind_cpt].size();i++)
			{
				V3DLONG l_neighbor_ind=vecvec_neighborcptind_length[ind_cpt][i];
				double d_neighbor_weight=vecvec_neighborweight_length[ind_cpt][i];
				E_length[0]+=vec_cptpos_last[l_neighbor_ind].x*d_neighbor_weight;
				E_length[1]+=vec_cptpos_last[l_neighbor_ind].y*d_neighbor_weight;
				d_weightsum+=d_neighbor_weight;
			}
			for(unsigned V3DLONG i=0;i<vecvec_neighborcptind_smooth[ind_cpt].size();i++)
			{
				V3DLONG l_neighbor_ind=vecvec_neighborcptind_smooth[ind_cpt][i];
				double d_neighbor_weight=vecvec_neighborweight_smooth[ind_cpt][i];
				E_length[0]+=vec_cptpos_last[l_neighbor_ind].x*d_neighbor_weight;
				E_length[1]+=vec_cptpos_last[l_neighbor_ind].y*d_neighbor_weight;
				d_weightsum+=d_neighbor_weight;
			}
			E_length[0]/=d_weightsum;
			E_length[1]/=d_weightsum;
			//if the current control point is the skeleton end point, we compute its lenght term use other formular
			for(unsigned V3DLONG i=0;i<vec_skeletonend_ind.size();i++)
			{
				if((V3DLONG)ind_cpt==vec_skeletonend_ind[i] && vecvec_skeletonend_2neighbor_ind[i].size()==2)
				{
					V3DLONG neighbor1_ind=vecvec_skeletonend_2neighbor_ind[i][0];
					V3DLONG neighbor2_ind=vecvec_skeletonend_2neighbor_ind[i][1];
					E_length[0]=vec_cptpos_last[neighbor1_ind].x*2-vec_cptpos_last[neighbor2_ind].x;
					E_length[1]=vec_cptpos_last[neighbor1_ind].y*2-vec_cptpos_last[neighbor2_ind].y;
				}
				if((V3DLONG)ind_cpt==vec_skeletonend_ind[i] && vecvec_skeletonend_2neighbor_ind[i].size()==1)
				{
					E_length[0]=vec_cptpos_last[ind_cpt].x;
					E_length[1]=vec_cptpos_last[ind_cpt].y;
				}
			}

			//check and update marker positon
			double d_cptpos_new[2];
			d_cptpos_new[0]=E_image[0]*0.5+E_length[0]*0.5;
			d_cptpos_new[1]=E_image[1]*0.5+E_length[1]*0.5;
			if(d_cptpos_new[0]<0 || d_cptpos_new[0]>=sz_img_input[0] ||
			   d_cptpos_new[1]<0 || d_cptpos_new[1]>=sz_img_input[1])
			{
				printf("ERROR: new positon of skeleton is invalid, maybe due to bad skeleton initialiation().\n");
				if(p_img_foreground)		{delete []p_img_foreground;		p_img_foreground=0;}
				if(p_img_foreground_co) 	{delete []p_img_foreground_co;	p_img_foreground_co=0;}
				if(p_img_base) 				{delete []p_img_base;			p_img_base=0;}
				return false;
			}
			else
			{
				vec_cptpos_new[ind_cpt].x=d_cptpos_new[0];
				vec_cptpos_new[ind_cpt].y=d_cptpos_new[1];
			}
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		//judge whether the skeleton is stable enought to stop iteration
		double d_stopiter_thresh=0.01;
		double d_totalshift=0.0;
		for(unsigned V3DLONG i=0;i<vec_cptpos_ini.size();i++)
		{
			double dif_x=vec_cptpos_new[i].x-vec_cptpos_last[i].x;
			double dif_y=vec_cptpos_new[i].y-vec_cptpos_last[i].y;
			d_totalshift+=sqrt(dif_x*dif_x+dif_y*dif_y);
		}
		printf("\t\titer:[%ld],\ttotal_shift=%f\n",iter,d_totalshift);

		if(d_totalshift<d_stopiter_thresh)
		{
			printf("\t>>quit iteration loop.\n");
			break;
		}
		else
			vec_cptpos_last=vec_cptpos_new;

//		//output deformed control points to marker file (for debug)
//		QList<ImageMarker> ql_cptpos_output;
//		for(V3DLONG i=0;i<vec_cptpos_ini.size();i++)
//		{
//			ql_cptpos_output.push_back(ImageMarker());
//			ql_cptpos_output[i].x=vec_cptpos_last[i].x*2;
//			ql_cptpos_output[i].y=vec_cptpos_last[i].y*2;
//		}
//		QString qs_filename_output=QString("skeleton_output_%1.marker").arg(iter);
//		wirteMarker_file(qs_filename_output,ql_cptpos_output);
	}

	//update for output
	vec_cptpos_output=vec_cptpos_last;

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	if(p_img_foreground)		{delete []p_img_foreground;		p_img_foreground=0;}
	if(p_img_foreground_co) 	{delete []p_img_foreground_co;	p_img_foreground_co=0;}
	if(p_img_base) 				{delete []p_img_base;			p_img_base=0;}

	return true;
}


////save test image (for debug)
//V3DLONG sz_img_test[4];
//sz_img_test[0]=sz_img_input[0];	//x dim
//sz_img_test[1]=sz_img_input[1];	//y	dim
//sz_img_test[2]=1;	//z	dim
//sz_img_test[3]=1;	//c	dim
//saveImage("foreground.tif",(unsigned char *)p_img_foreground_co,sz_img_test,1);

