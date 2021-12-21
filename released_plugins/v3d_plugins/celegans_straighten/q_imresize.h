// q_imresize.h
// by Lei Qu
// 2010-02-24

#ifndef __Q_IMRESIZE_H__
#define __Q_IMRESIZE_H__

#include <math.h>
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions


template<class T>
bool q_imresize_3D(const T *p_img_input,const long sz_img_input[4],
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
	if(p_img_output)
	{
		printf("WARNNING: Output image pointer is not NULL, original data will be cleared!\n");
		if(p_img_output) 	{delete []p_img_output;		p_img_output=0;}
	}

	//judge the datatype is int or float
	//if the datatype is not float or double, round the output
	bool b_isint=1;
	T tmp=1.1;
	if((tmp-1)>0.01) b_isint=0;

	//allocate memory
	p_img_output=new T[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]]();
	if(!p_img_output)
	{
		printf("ERROR: Fail to allocate memory for resized image!\n");
		return false;
	}

	T ****p_img_input_4d=0,****p_img_output_4d=0;
	if(!new4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3],p_img_input) ||
	   !new4dpointer(p_img_output_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3],p_img_output))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
		if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}
		if(p_img_output)		{delete[] p_img_output; p_img_output=0;}
		return false;
	}

	//resize ratio = output/input
	//x_o=(x_i+1)*resize_ratio-1;
	double arr_resize_ratio[3];
	arr_resize_ratio[0]=(double)(sz_img_output[0])/(double)(sz_img_input[0]);
	arr_resize_ratio[1]=(double)(sz_img_output[1])/(double)(sz_img_input[1]);
	arr_resize_ratio[2]=(double)(sz_img_output[2])/(double)(sz_img_input[2]);

	//resize image by linear interpolating
	for(long x=0;x<sz_img_output[0];x++)
		for(long y=0;y<sz_img_output[1];y++)
			for(long z=0;z<sz_img_output[2];z++)
			{
				//current position
				double cur_pos[3];//x,y,z
				cur_pos[0]=(x+1)/arr_resize_ratio[0]-1;
				cur_pos[1]=(y+1)/arr_resize_ratio[1]-1;
				cur_pos[2]=(z+1)/arr_resize_ratio[2]-1;

				//find 8 neighor pixels boundary
				long x_s,x_b,y_s,y_b,z_s,z_b;
				x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
				y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
				z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);
				x_b=x_b>=sz_img_input[0]?sz_img_input[0]-1:x_b;
				y_b=y_b>=sz_img_input[1]?sz_img_input[1]-1:y_b;
				z_b=z_b>=sz_img_input[2]?sz_img_input[2]-1:z_b;

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w,r_w,t_w,b_w;
				l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
				t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
				//compute weight for higer slice and lower slice
				double u_w,d_w;
				u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

				//linear interpolate each channel
				for(long c=0;c<sz_img_output[3];c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
								 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					if(b_isint)
						p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice+0.5;//+0.5 for round-off
					else
						p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
				}
			}

	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}

	return true;
}


#endif
