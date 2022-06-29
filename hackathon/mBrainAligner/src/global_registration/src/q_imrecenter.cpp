// q_imrecenter.cpp
// by Lei Qu
// 2010-04-29

#ifndef __Q_IMRECENTER_CPP__
#define __Q_IMRECENTER_CPP__

#include <stdio.h>
#include <math.h>

//recenter 3D image stack
template<class T>
bool q_imrecenter_3D(const T *p_img_input,const long sz_img_input[4],const T p_fillvalue[3],
		const long sz_img_output[4],T *&p_img_output)
{
	//check paras
	if(p_img_input==0)
	{
		printf("ERROR: Invalid input image pointer!\n");
		return false;
	}
	if(sz_img_input[0]<=0 || sz_img_input[1]<=0 || sz_img_input[2]<=0 || sz_img_input[3]<=0)
	{
		printf("ERROR: Invalid input image size!\n");
		return false;
	}
	if(sz_img_output[0]<=0 || sz_img_output[1]<=0 || sz_img_output[2]<=0 || sz_img_output[3]!=sz_img_input[3])
	{
		printf("ERROR: Invalid output image size!\n");
		return false;
	}
	if(p_fillvalue==0)
	{
		printf("ERROR: Invalid pointer: p_fillvalue!\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: Output image pointer is not NULL, original data will be cleared!\n");
		if(p_img_output) 	{delete []p_img_output;		p_img_output=0;}
	}

	long pgsz_y_src=sz_img_input[0];
	long pgsz_xy_src=sz_img_input[0]*sz_img_input[1];
	long pgsz_xyz_src=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
	long pgsz_y_des=sz_img_output[0];
	long pgsz_xy_des=sz_img_output[0]*sz_img_output[1];
	long pgsz_xyz_des=sz_img_output[0]*sz_img_output[1]*sz_img_output[2];

	//allocate memory
	p_img_output=new T[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]]();
	if(!p_img_output)
	{
		printf("ERROR: Fail to allocate memory for recenter image!\n");
		return false;
	}

	//if input image size same as output image size, do direct copy and then return
	if(sz_img_input[0]==sz_img_output[0] && sz_img_input[1]==sz_img_output[1] && sz_img_input[2]==sz_img_output[2])
	{
		long l_npixels=sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3];
		for(long i=0;i<l_npixels;i++)	p_img_output[i]=p_img_input[i];
		return true;
	}

	//fill output image with initial value
	if(fabs(double(p_fillvalue[0]+p_fillvalue[1]+p_fillvalue[2]))>1e-10)
		for(long c=0;c<sz_img_output[3];c++)
		{
			long ind_start=c*pgsz_xyz_des;
			long ind_end=ind_start+pgsz_xyz_des-1;
			for(long i=ind_start;i<=ind_end;i++)
				p_img_output[i]=p_fillvalue[c];
		}

	//compute the valid image region offsets in src and des images
	long l_offset_src[3][2];//3 rows: x,y,z; 2 cols: left and right
	long l_offset_des[3][2];//3 rows: x,y,z; 2 cols: left and right
	for(int i=0;i<3;i++)
	{
		float offset=(sz_img_output[i]-sz_img_input[i])/2.0;
		if(offset>=0)	//expand src image in this dim
		{
			l_offset_des[i][0]=offset+0.5;		l_offset_des[i][1]=l_offset_des[i][0]+sz_img_input[i]-1;
			l_offset_src[i][0]=0;				l_offset_src[i][1]=sz_img_input[i]-1;
		}
		else			//crop src image in this dim
		{
			l_offset_des[i][0]=0;				l_offset_des[i][1]=sz_img_output[i]-1;
			l_offset_src[i][0]=-offset-0.5;		l_offset_src[i][1]=l_offset_src[i][0]+sz_img_output[i]-1;
		}
	}
//	printf("src:x[%ld,%ld],y[%ld,%ld],z[%ld,%ld]\n",
//			l_offset_src[0][0],l_offset_src[0][1],
//			l_offset_src[1][0],l_offset_src[1][1],
//			l_offset_src[2][0],l_offset_src[2][1]);
//	printf("des:x[%ld,%ld],y[%ld,%ld],z[%ld,%ld]\n",
//			l_offset_des[0][0],l_offset_des[0][1],
//			l_offset_des[1][0],l_offset_des[1][1],
//			l_offset_des[2][0],l_offset_des[2][1]);
	//copy image data (only valid)
	for(long x_des=l_offset_des[0][0],x_src=l_offset_src[0][0];x_des<=l_offset_des[0][1];x_des++,x_src++)
		for(long y_des=l_offset_des[1][0],y_src=l_offset_src[1][0];y_des<=l_offset_des[1][1];y_des++,y_src++)
			for(long z_des=l_offset_des[2][0],z_src=l_offset_src[2][0];z_des<=l_offset_des[2][1];z_des++,z_src++)
				for(long c=0;c<sz_img_output[3];c++)
				{
					long ind_des=pgsz_xyz_des*c+z_des*pgsz_xy_des+y_des*pgsz_y_des+x_des;
					long ind_src=pgsz_xyz_src*c+z_src*pgsz_xy_src+y_src*pgsz_y_src+x_src;

					p_img_output[ind_des]=p_img_input[ind_src];
				}

	return true;
}


#endif
