//q_morphology3D.cpp
//perform morphological operation on the 8-bit 3D gray image
//by Lei Qu
//2015-03-12
//modified by Gao Shan
//2015/3/19

#include <math.h>
#include "q_morphology3D.h"

 
//generate kernel according to the specification
//rows,cols: 			height and weight of kernel
//anchor_x,anchor_y:	center position of kernel
//shape:				shape of kernel (0:rect, 1:disk, 2:to add)
//kernel:				output kernel[0,1]
bool q_create_kernel3D(const int rows,const int cols,const int deps,
					   const int anchor_x,const int anchor_y,const int anchor_z,
					   const int shape,
					   CKernelMP &kernel)
{
	//check parameters
	if(rows<=0 || cols<=0 || deps<=0)
	{
		printf("ERROR: q_create_kernel3D: Input rows, cols and dep of kernel should > 0.\n");
		return false;
	}
	if(anchor_x<0 || anchor_x>=cols ||
	   anchor_y<0 || anchor_y>=rows ||
	   anchor_z<0 || anchor_z>=deps)
	{
		printf("ERROR: q_create_kernel3D: Input anchor position is invalid.\n");
		return false;
	}
	if(shape!=0 && shape!=1)
	{
		printf("ERROR: q_create_kernel3D: Input shape is invalid, currently only support: 0:rect, 1:disk.\n");
		return false;
	}
	if(!kernel.kernel.empty())
	{
		printf("WARNNING: q_create_kernel3D: Input kernel is not NULL, original contents will be cleared!\n");
		kernel.kernel.clear();
	}

	//fill the kernel
	kernel.rows=rows;
	kernel.cols=cols;
	kernel.deps=deps;
	kernel.anchor_x=anchor_x;
	kernel.anchor_y=anchor_y;
	kernel.anchor_z=anchor_z;
	kernel.shape=shape;

	if(shape==0)		//rect
	{
		kernel.kernel.assign(rows,vector< vector<int> >(cols,vector<int>(deps,1)));
	}
	else if(shape==1)	//disk
	{
		//compute the maximal radius of disk in order to fit in the rect
		int radius_x=anchor_x<(cols-anchor_x)?anchor_x:(cols-anchor_x);
		int radius_y=anchor_y<(rows-anchor_y)?anchor_y:(rows-anchor_y);
		int radius_z=anchor_z<(deps-anchor_z)?anchor_z:(deps-anchor_z);
		int radius;
		radius=radius_x<radius_y?radius_x:radius_y;
		radius=radius  <radius_z?radius  :radius_z;
		//fill the kernel
		kernel.kernel.assign(rows,vector< vector<int> >(cols,vector<int>(deps,0)));
		for(int x=0;x<cols;x++)
			for(int y=0;y<rows;y++)
				for(int z=0;z<deps;z++)
				{
					float dis=(float)sqrt(double((x-anchor_x)*(x-anchor_x)+(y-anchor_y)*(y-anchor_y)+(z-anchor_z)*(z-anchor_z)));
					if(dis<=radius)
						kernel.kernel[y][x][z]=1;
				}
	}

	return true;
}

//gray dilation: current pixel will be replaced with the maximal value under kernel
bool q_dilation3D(const double *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
				  const CKernelMP kernel,double *&p_img_output)
{
	//check parameters
	if(!p_img_input)
	{
		printf("ERROR: q_dilation3D: Input image pointer is NULL.\n");
		return false;
	}
	if(l_img_width<=0 || l_img_height<=0 || l_img_depth<=0)
	{
		printf("ERROR: q_dilation3D: Input image size is invalid.\n");
		return false;
	}
	if(kernel.kernel.empty())
	{
		printf("ERROR: q_dilation3D: Input kernel is empty.\n");
		return false;
	}
	if(p_img_output)
	{
		printf("WARNNING: q_dilation3D: Output image pointer is not NULL, previous contents it point to will be released!\n");
	}

	//allocate memory for output image
	p_img_output=new double[l_img_width*l_img_height*l_img_depth]();
	if(!p_img_output)
	{
		printf("ERROR: q_dilation3D: fail to allocate memory for output image.\n");
		return false;
	}

	//perform morphological operation
	long l_npix1slice=l_img_height*l_img_width;
        long shift_x_min=-kernel.anchor_x;		long shift_x_max=kernel.cols-kernel.anchor_x;
        long shift_y_min=-kernel.anchor_y;		long shift_y_max=kernel.rows-kernel.anchor_y;
        long shift_z_min=-kernel.anchor_z;		long shift_z_max=kernel.deps-kernel.anchor_z;
	for(long y=0;y<l_img_height;y++)
	{
		for(long x=0;x<l_img_width;x++)
		{
			for(long z=0;z<l_img_depth;z++)
			{
				double max_value=0;
				for(long shift_y=shift_y_min;shift_y<shift_y_max;shift_y++)
					for(long shift_x=shift_x_min;shift_x<shift_x_max;shift_x++)
						for(long shift_z=shift_z_min;shift_z<shift_z_max;shift_z++)
					{
						long xx=x+shift_x;
						long yy=y+shift_y;
						long zz=z+shift_z;
						xx=xx<0?0:xx;	xx=xx>=l_img_width?l_img_width-1:xx;
						yy=yy<0?0:yy;	yy=yy>=l_img_height?l_img_height-1:yy;
						zz=zz<0?0:zz;	zz=zz>=l_img_depth?l_img_depth-1:zz;
						long index=l_npix1slice*zz+l_img_width*yy+xx;

						//if dilation, replaced with the maximal value under kernel
						if(p_img_input[index]>max_value && kernel.kernel[kernel.anchor_y+shift_y][kernel.anchor_x+shift_x][kernel.anchor_z+shift_z]==1)
							max_value=p_img_input[index];
					}
				
				long index=l_npix1slice*z+l_img_width*y+x;
				p_img_output[index]=max_value;
			}
		}
		printf("%d,",y);
	}

	return true;
}

bool q_erosion3D(const double *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
                                  const CKernelMP kernel,double *&p_img_output)
{
        //check parameters
        if(!p_img_input)
        {
                printf("ERROR: q_erosion3D: Input image pointer is NULL.\n");
                return false;
        }
        if(l_img_width<=0 || l_img_height<=0 || l_img_depth<=0)
        {
                printf("ERROR: q_erosion3D: Input image size is invalid.\n");
                return false;
        }
        if(kernel.kernel.empty())
        {
                printf("ERROR: q_erosion3D: Input kernel is empty.\n");
                return false;
        }
        if(p_img_output)
        {
                printf("WARNNING: q_erosion3D: Output image pointer is not NULL, previous contents it point to will be released!\n");
        }

        //allocate memory for output image
        p_img_output=new double[l_img_width*l_img_height*l_img_depth]();
        if(!p_img_output)
        {
                printf("ERROR: q_erosion3D: fail to allocate memory for output image.\n");
                return false;
        }

        //perform morphological operation
        long l_npix1slice=l_img_height*l_img_width;
        long shift_x_min=-kernel.anchor_x;		long shift_x_max=kernel.cols-kernel.anchor_x;
        long shift_y_min=-kernel.anchor_y;		long shift_y_max=kernel.rows-kernel.anchor_y;
        long shift_z_min=-kernel.anchor_z;		long shift_z_max=kernel.deps-kernel.anchor_z;
        for(long y=0;y<l_img_height;y++)
        {
                for(long x=0;x<l_img_width;x++)
                {
                        for(long z=0;z<l_img_depth;z++)
                        {
                                double min_value=255;
                                for(long shift_y=shift_y_min;shift_y<shift_y_max;shift_y++)
                                        for(long shift_x=shift_x_min;shift_x<shift_x_max;shift_x++)
                                                for(long shift_z=shift_z_min;shift_z<shift_z_max;shift_z++)
                                        {
                                                long xx=x+shift_x;
                                                long yy=y+shift_y;
                                                long zz=z+shift_z;
                                                xx=xx<0?0:xx;	xx=xx>=l_img_width?l_img_width-1:xx;
                                                yy=yy<0?0:yy;	yy=yy>=l_img_height?l_img_height-1:yy;
                                                zz=zz<0?0:zz;	zz=zz>=l_img_depth?l_img_depth-1:zz;
                                                long index=l_npix1slice*zz+l_img_width*yy+xx;

                                                //if erosion, replaced with the maximal value under kernel
                                                if(p_img_input[index]<min_value && kernel.kernel[kernel.anchor_y+shift_y][kernel.anchor_x+shift_x][kernel.anchor_z+shift_z]==1)
                                                        min_value=p_img_input[index];
                                        }

                                long index=l_npix1slice*z+l_img_width*y+x;
                                p_img_output[index]=min_value;
                        }
                }
                printf("%d,",y);
        }

        return true;
}
