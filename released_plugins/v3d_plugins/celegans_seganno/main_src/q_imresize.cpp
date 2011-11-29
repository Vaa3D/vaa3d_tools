// q_imresize.cpp
// by Lei Qu
// 2010-02-24

#ifndef __Q_IMRESIZE_CPP__
#define __Q_IMRESIZE_CPP__

#include <math.h>
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions


//resize 3D image stack
template<class T>
bool q_imresize_3D(const T *p_img_input,const long sz_img_input[4],const int mode,
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
	if(mode!=0 && mode!=1 && mode!=2)
	{
		printf("ERROR: Unknow mode!\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: Output image pointer is not NULL, original data will be cleared!\n");
		if(p_img_output) 	{delete []p_img_output;		p_img_output=0;}
	}

	bool b_isint=1;
	T tmp=1.1;
	if((tmp-1)>0.01) b_isint=0;

	p_img_output=new T[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]]();
	if(!p_img_output)
	{
		printf("ERROR: Fail to allocate memory for resized image!\n");
		return false;
	}

	if(sz_img_input[0]==sz_img_output[0] && sz_img_input[1]==sz_img_output[1] && sz_img_input[2]==sz_img_output[2])
	{
		long l_npixels=sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3];
		for(long i=0;i<l_npixels;i++)	p_img_output[i]=p_img_input[i];
		return true;
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

	double arr_resize_ratio[3];
	arr_resize_ratio[0]=(double)(sz_img_output[0])/(double)(sz_img_input[0]);
	arr_resize_ratio[1]=(double)(sz_img_output[1])/(double)(sz_img_input[1]);
	arr_resize_ratio[2]=(double)(sz_img_output[2])/(double)(sz_img_input[2]);

	if(mode==2)
	{
		for(long x=0;x<sz_img_output[0];x++)
			for(long y=0;y<sz_img_output[1];y++)
				for(long z=0;z<sz_img_output[2];z++)
				{
					double cur_pos[3];
					cur_pos[0]=(x+1)/arr_resize_ratio[0]-1;
					cur_pos[1]=(y+1)/arr_resize_ratio[1]-1;
					cur_pos[2]=(z+1)/arr_resize_ratio[2]-1;

					long nn_pos[3];
					nn_pos[0]=cur_pos[0]+0.5;
					nn_pos[1]=cur_pos[1]+0.5;
					nn_pos[2]=cur_pos[2]+0.5;
					nn_pos[0]=nn_pos[0]<0?0:nn_pos[0];
					nn_pos[1]=nn_pos[1]<0?0:nn_pos[1];
					nn_pos[2]=nn_pos[2]<0?0:nn_pos[2];
					nn_pos[0]=nn_pos[0]>=sz_img_input[0]?sz_img_input[0]-1:nn_pos[0];
					nn_pos[1]=nn_pos[1]>=sz_img_input[1]?sz_img_input[1]-1:nn_pos[1];
					nn_pos[2]=nn_pos[2]>=sz_img_input[2]?sz_img_input[2]-1:nn_pos[2];

					for(long c=0;c<sz_img_output[3];c++)
						p_img_output_4d[c][z][y][x]=p_img_input_4d[c][nn_pos[2]][nn_pos[1]][nn_pos[0]];
				}

	}
	else if(mode==1 && (sz_img_output[0]<sz_img_input[0] || sz_img_output[1]<sz_img_input[1] || sz_img_output[2]<sz_img_input[2]))
	{
		for(long x=0;x<sz_img_output[0];x++)
			for(long y=0;y<sz_img_output[1];y++)
				for(long z=0;z<sz_img_output[2];z++)
				{
					double x_l_s,x_r_s,y_l_s,y_r_s,z_l_s,z_r_s;
					x_l_s=x-0.5;	x_r_s=x+0.5;
					y_l_s=y-0.5;	y_r_s=y+0.5;
					z_l_s=z-0.5;	z_r_s=z+0.5;
					long x_l_b,x_r_b,y_l_b,y_r_b,z_l_b,z_r_b;
					x_l_b=(x_l_s+1)/arr_resize_ratio[0]-1+0.5;	x_r_b=(x_r_s+1)/arr_resize_ratio[0]-1+0.5;
					y_l_b=(y_l_s+1)/arr_resize_ratio[1]-1+0.5;	y_r_b=(y_r_s+1)/arr_resize_ratio[1]-1+0.5;
					z_l_b=(z_l_s+1)/arr_resize_ratio[2]-1+0.5;	z_r_b=(z_r_s+1)/arr_resize_ratio[2]-1+0.5;
					x_l_b=x_l_b<0 ? 0:x_l_b;	x_r_b=x_r_b>=sz_img_input[0] ? sz_img_input[0]-1:x_r_b;
					y_l_b=y_l_b<0 ? 0:y_l_b;	y_r_b=y_r_b>=sz_img_input[1] ? sz_img_input[1]-1:y_r_b;
					z_l_b=z_l_b<0 ? 0:z_l_b;	z_r_b=z_r_b>=sz_img_input[2] ? sz_img_input[2]-1:z_r_b;
					for(long c=0;c<sz_img_output[3];c++)
					{
						double avg=0;
						long npixles=0;
						for(long xx=x_l_b;xx<=x_r_b;xx++)
							for(long yy=y_l_b;yy<=y_r_b;yy++)
								for(long zz=z_l_b;zz<=z_r_b;zz++)
								{
									avg+=p_img_input_4d[c][zz][yy][xx];
									npixles++;
								}
						if(b_isint)
							p_img_output_4d[c][z][y][x]=avg/npixles+0.5;
						else
							p_img_output_4d[c][z][y][x]=avg/npixles;
					}
				}
	}
	else
	{
		for(long x=0;x<sz_img_output[0];x++)
			for(long y=0;y<sz_img_output[1];y++)
				for(long z=0;z<sz_img_output[2];z++)
				{
					double cur_pos[3];//x,y,z
					cur_pos[0]=(x+1)/arr_resize_ratio[0]-1;
					cur_pos[1]=(y+1)/arr_resize_ratio[1]-1;
					cur_pos[2]=(z+1)/arr_resize_ratio[2]-1;

					long x_s,x_b,y_s,y_b,z_s,z_b;
					x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
					y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
					z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);
					x_s=x_s<0?0:x_s;
					y_s=y_s<0?0:y_s;
					z_s=z_s<0?0:z_s;
					x_b=x_b>=sz_img_input[0]?sz_img_input[0]-1:x_b;
					y_b=y_b>=sz_img_input[1]?sz_img_input[1]-1:y_b;
					z_b=z_b>=sz_img_input[2]?sz_img_input[2]-1:z_b;

					double l_w,r_w,t_w,b_w;
					l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
					t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
					double u_w,d_w;
					u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

					for(long c=0;c<sz_img_output[3];c++)
					{
						double higher_slice;
						higher_slice=t_w*(l_w*p_img_input_4d[c][z_s][y_s][x_s]+r_w*p_img_input_4d[c][z_s][y_s][x_b])+
									 b_w*(l_w*p_img_input_4d[c][z_s][y_b][x_s]+r_w*p_img_input_4d[c][z_s][y_b][x_b]);
						double lower_slice;
						lower_slice =t_w*(l_w*p_img_input_4d[c][z_b][y_s][x_s]+r_w*p_img_input_4d[c][z_b][y_s][x_b])+
									 b_w*(l_w*p_img_input_4d[c][z_b][y_b][x_s]+r_w*p_img_input_4d[c][z_b][y_b][x_b]);
						if(b_isint)
							p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice+0.5;//+0.5 for round-off
						else
							p_img_output_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
					}
				}
	}

	if(p_img_input_4d) 		{delete4dpointer(p_img_input_4d,sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);}
	if(p_img_output_4d) 	{delete4dpointer(p_img_output_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}

	return true;
}


#endif
