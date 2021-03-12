//q_morphology.h
//perform morphological operation on the 8-bit 2D gray image
//include: dilation, erosion, closing and opening
//by Lei Qu
//2009-09-10
 
#ifndef __Q_MORPHOLOGY_H__
#define __Q_MORPHOLOGY_H__

#include <stdio.h>
#include <vector>
using namespace std;


class CKernelMP2D
{
public:
	int rows,cols;
	int anchor_x,anchor_y;
	int shape;
	vector< vector<int> > kernel;
	CKernelMP2D() {rows=cols=anchor_x=anchor_y=shape=0; kernel.clear();}
};
class CKernelMP3D
{
public:
	int rows, cols, deps;
	int anchor_x, anchor_y, anchor_z;
	int shape;
	vector< vector< vector<int> > > kernel;
	CKernelMP3D() { rows = cols = deps = anchor_x = anchor_y =anchor_z = shape = 0; kernel.clear(); }
};

//generate kernel according to the specification
//rows,cols: 			height and weight of kernel
//anchor_x,anchor_y:	center position of kernel
//shape:				shape of kernel (0:rect, 1:disk, 2:to add)
//kernel:				output kernel[0,1]
bool q_create_kernel_2D(const int rows,const int cols,
	const int anchor_x,const int anchor_y,const int shape,CKernelMP2D &kernel);
bool q_create_kernel_3D(const int rows, const int cols, const int deps, 
	const int anchor_x, const int anchor_y, const int anchor_z, const int shape, CKernelMP3D &kernel);

//gray dilation: current pixel will be replaced with the maximal value under kernel
bool q_dilation_2D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height,
	const CKernelMP2D kernel,unsigned char *&p_img_output);
bool q_dilation_3D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height, const long l_img_depth,
	const CKernelMP3D kernel, unsigned char *&p_img_output);
//gray erosion: current pixel will be replaced with the minimal value under kernel
bool q_erosion_2D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height,
	const CKernelMP2D kernel,unsigned char *&p_img_output);
bool q_erosion_3D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height, const long l_img_depth,
	const CKernelMP3D kernel, unsigned char *&p_img_output);
//gray closing=dilation+erosion, it can be used to fill small hole
bool q_closing_2D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height,
	const CKernelMP2D kernel,unsigned char *&p_img_output);
bool q_closing_3D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height, const long l_img_depth,
	const CKernelMP3D kernel, unsigned char *&p_img_output);
//gray opening=erosion+dilation, it can be used to remove small object
bool q_opening_2D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height,
	const CKernelMP2D kernel,unsigned char *&p_img_output);
bool q_opening_3D(const unsigned char *p_img_input, const long l_img_width, const long l_img_height, const long l_img_depth,
	const CKernelMP3D kernel, unsigned char *&p_img_output);

#endif
