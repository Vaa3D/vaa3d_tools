//q_morphology.cpp
//perform morphological operation on the 8-bit 2D gray image
//include: dilation, erosion, closing and opening
//by Lei Qu
//2009-09-10

#include <math.h>
#include "q_morphology.h"

 
//generate kernel according to the specification
//rows,cols: 			height and weight of kernel
//anchor_x,anchor_y:	center position of kernel
//shape:				shape of kernel (0:rect, 1:disk, 2:to add)
//kernel:				output kernel[0,1]
bool q_create_kernel(const int rows,const int cols,const int anchor_x,const int anchor_y,const int shape,CKernelMP &kernel)
{
	//check parameters
	if(rows<=0 || cols<=0)
	{
		printf("ERROR: q_create_kernel: Input rows and cols of kernel should > 0.\n");
		return false;
	}
	if(anchor_x<0 || anchor_x>=cols || anchor_y<0 || anchor_y>=rows)
	{
		printf("ERROR: q_create_kernel: Input anchor position is invalid.\n");
		return false;
	}
	if(shape!=0 && shape!=1)
	{
		printf("ERROR: q_create_kernel: Input shape is invalid, currently only suppurt: 0:rect, 1:disk.\n");
		return false;
	}
	if(!kernel.kernel.empty())
	{
		printf("WARNNING: q_create_kernel: Input kernel is not NULL, original contents will be cleared!\n");
		kernel.kernel.clear();
	}

	//fill the kernel
	kernel.rows=rows;
	kernel.cols=cols;
	kernel.anchor_x=anchor_x;
	kernel.anchor_y=anchor_y;
	kernel.shape=shape;

	if(shape==0)		//rect
	{
		kernel.kernel.assign(rows,vector<int>(cols,1));
	}
	else if(shape==1)	//disk
	{
		//compute the maximal radius of disk in order to fit in the rect
		int radius_x=anchor_x<(cols-anchor_x)?anchor_x:(cols-anchor_x);
		int radius_y=anchor_y<(rows-anchor_y)?anchor_y:(rows-anchor_y);
		int radius=radius_x<radius_y?radius_x:radius_y;
		//fill the kernel
		kernel.kernel.assign(rows,vector<int>(cols,0));
		for(int x=0;x<cols;x++)
			for(int y=0;y<rows;y++)
			{
				float dis=sqrt(double((x-anchor_x)*(x-anchor_x)+(y-anchor_y)*(y-anchor_y)));
				if(dis<=radius)
					kernel.kernel[y][x]=1;
			}
	}

	return true;
}

//gray dilation: current pixel will be replaced with the maximal value under kernel
bool q_dilation(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const CKernelMP kernel,unsigned char *&p_img_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_dilation: Input image pointer is NULL.\n");
		return false;
	}
	if(l_img_width<=0 || l_img_height<=0)
	{
		printf("ERROR: q_dilation: Input image size is invalid.\n");
		return false;
	}
	if(kernel.kernel.empty())
	{
		printf("ERROR: q_dilation: Input kernel is empty.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_dilation: Output image pointer is not NULL, previous contents it point to will be losted!\n");
	}

	//allocate memory for output image
	p_img_output=new unsigned char[l_img_width*l_img_height];
	if(!p_img_output)
	{
		printf("ERROR: q_dilation: fail to allocate memory for output image.\n");
		return false;
	}

	//perform morphological operation
	for(long y=0;y<l_img_height;y++)
		for(long x=0;x<l_img_width;x++)
		{
			unsigned char max_value=0;
			long shift_x_min=-kernel.anchor_x;		long shift_x_max=kernel.cols-kernel.anchor_x;
			long shift_y_min=-kernel.anchor_y;		long shift_y_max=kernel.rows-kernel.anchor_y;
			for(long shift_y=shift_y_min;shift_y<shift_y_max;shift_y++)
				for(long shift_x=shift_x_min;shift_x<shift_x_max;shift_x++)
				{
					long xx=x+shift_x;
					long yy=y+shift_y;
					xx=xx<0?0:xx;	xx=xx>=l_img_width?l_img_width-1:xx;
					yy=yy<0?0:yy;	yy=yy>=l_img_height?l_img_height-1:yy;
					long index=l_img_width*yy+xx;

					//if dilation, replaced with the maximal value under kernel
					if(p_img_input[index]>max_value && kernel.kernel[kernel.anchor_y+shift_y][kernel.anchor_x+shift_x]==1)
						max_value=p_img_input[index];
				}
			
			long index=l_img_width*y+x;
			p_img_output[index]=max_value;
		}

	return true;
}

//gray erosion: current pixel will be replaced with the minimal value under kernel
bool q_erosion(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const CKernelMP kernel,unsigned char *&p_img_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_erosion: Input image pointer is NULL.\n");
		return false;
	}
	if(l_img_width<=0 || l_img_height<=0)
	{
		printf("ERROR: q_erosion: Input image size is invalid.\n");
		return false;
	}
	if(kernel.kernel.empty())
	{
		printf("ERROR: q_erosion: Input kernel is empty.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_erosion: Output image pointer is not NULL, previous contents it point to will be losted!\n");
	}

	//allocate memory for output image
	p_img_output=new unsigned char[l_img_width*l_img_height];
	if(!p_img_output)
	{
		printf("ERROR: q_erosion: fail to allocate memory for output image.\n");
		return false;
	}

	//perform morphological operation
	for(long y=0;y<l_img_height;y++)
		for(long x=0;x<l_img_width;x++)
		{
			unsigned char min_value=255;
			long shift_x_min=-kernel.anchor_x;		long shift_x_max=kernel.cols-kernel.anchor_x;
			long shift_y_min=-kernel.anchor_y;		long shift_y_max=kernel.rows-kernel.anchor_y;
			for(long shift_y=shift_y_min;shift_y<shift_y_max;shift_y++)
				for(long shift_x=shift_x_min;shift_x<shift_x_max;shift_x++)
				{
					long xx=x+shift_x;
					long yy=y+shift_y;
					xx=xx<0?0:xx;	xx=xx>=l_img_width?l_img_width-1:xx;
					yy=yy<0?0:yy;	yy=yy>=l_img_height?l_img_height-1:yy;
					long index=l_img_width*yy+xx;

					//if erosion, replaced with the minimal value under kernel
					if(p_img_input[index]<min_value  && kernel.kernel[kernel.anchor_y+shift_y][kernel.anchor_x+shift_x]==1)
						min_value=p_img_input[index];
				}

			long index=l_img_width*y+x;
			p_img_output[index]=min_value;
		}

		return true;
}

//gray closing=dilation+erosion, it can be used to fill small hole
bool q_closing(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const CKernelMP kernel,unsigned char *&p_img_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_closing: Input image pointer is NULL.\n");
		return false;
	}
	if(l_img_width<=0 || l_img_height<=0)
	{
		printf("ERROR: q_closing: Input image size is invalid.\n");
		return false;
	}
	if(kernel.kernel.empty())
	{
		printf("ERROR: q_closing: Input kernel is empty.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_closing: Output image pointer is not NULL, previous contents it point to will be losted!\n");
	}

	unsigned char *p_img_temp=0;
	//dilation
	if(!q_dilation(p_img_input,l_img_width,l_img_height,kernel,p_img_temp))
	{
		printf("ERROR: dilation() return false.\n");
		if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}
		return false;
	}
	//erosion
	if(!q_erosion(p_img_temp,l_img_width,l_img_height,kernel,p_img_output))
	{
		printf("ERROR: q_erosion() return false.\n");
		if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}
		return false;
	}

	if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}

	return true;
}

//gray opening=erosion+dilation, it can be used to remove small object
bool q_opening(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const CKernelMP kernel,unsigned char *&p_img_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_opening: Input image pointer is NULL.\n");
		return false;
	}
	if(l_img_width<=0 || l_img_height<=0)
	{
		printf("ERROR: q_opening: Input image size is invalid.\n");
		return false;
	}
	if(kernel.kernel.empty())
	{
		printf("ERROR: q_opening: Input kernel is empty.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_opening: Output image pointer is not NULL, previous contents it point to will be losted!\n");
	}

	unsigned char *p_img_temp=0;
	//erosion
	if(!q_erosion(p_img_input,l_img_width,l_img_height,kernel,p_img_temp))
	{
		printf("ERROR: q_erosion() return false.\n");
		if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}
		return false;
	}
	//dilation
	if(!q_dilation(p_img_temp,l_img_width,l_img_height,kernel,p_img_output))
	{
		printf("ERROR: dilation() return false.\n");
		if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}
		return false;
	}

	if(p_img_temp) {delete[] p_img_temp; p_img_temp=0;}

	return true;
}
