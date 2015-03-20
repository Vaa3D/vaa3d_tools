//q_morphology3D.h
//perform morphological operation on the 8-bit 3D gray image
//by Lei Qu
//2015-03-12
//modified by Gao Shan
//2015/3/19
 
#ifndef __Q_MORPHOLOGY_H__
#define __Q_MORPHOLOGY_H__

#include <stdio.h>
#include <vector>
using namespace std;


class CKernelMP
{
public:
	int rows,cols,deps;
	int anchor_x,anchor_y,anchor_z;
	int shape;
	vector< vector< vector<int> > > kernel;
	CKernelMP() {rows=cols=deps=anchor_x=anchor_y=anchor_z=shape=0; kernel.clear();}
};


//generate kernel according to the specification
//rows,cols: 			height and weight of kernel
//anchor_x,anchor_y:	center position of kernel
//shape:				shape of kernel (0:rect, 1:disk, 2:to add)
//kernel:				output kernel[0,1]
bool q_create_kernel3D(const int rows,const int cols,const int deps,
					   const int anchor_x,const int anchor_y,const int anchor_z,
					   const int shape,
					   CKernelMP &kernel);

//gray dilation: current pixel will be replaced with the maximal value under kernel
bool q_dilation3D(const double *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
				  const CKernelMP kernel,double *&p_img_output);

bool q_erosion3D(const double *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
                                  const CKernelMP kernel,double *&p_img_output);

#endif
