#ifndef SHRINK_3DMORPH
#define SHRINK_3DMORPH
//shrink_3dmorph
//perform morphological operation on the 8-bit 3D gray image
//by Lei Qu
//2015-03-12
//modified by Gao Shan
//2015/4/28

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
bool shrink_create_kernel3D(const int rows,const int cols,const int deps,
                       const int anchor_x,const int anchor_y,const int anchor_z,
                       const int shape,
                       CKernelMP *kernel);

bool shrink_dilation3D(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
                  const CKernelMP kernel,unsigned char *p_img_output);

bool shrink_erosion3D(const unsigned char *p_img_input,const long l_img_width,const long l_img_height,const long l_img_depth,
                 const CKernelMP kernel,unsigned char *p_img_output);

#endif // SHRINK_3DMORPH

