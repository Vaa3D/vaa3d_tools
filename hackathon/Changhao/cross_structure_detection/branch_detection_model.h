#ifndef BRANCH_DETECTION_MODEL_H
#define BRANCH_DETECTION_MODEL_H
#include <v3d_interface.h>
#include <vector>
#include"../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
class Branch_Detection_Model
{
private:

public:
    int thres_2d ;
    int window_size;
    int length_point;
    int ray_numbers_2d;
    int x_coordinate, y_coordinate,z_coordinate;
    int based_distance;  // for rorate method;
    int soma_radius;
    int num_layer;       // select the number of layer to generate the MIPs;
    V3DLONG size_2D_image,size_xy_image,size_yz_image,size_xz_image;
    V3DLONG size_3D_image;
    Image4DSimple * p4DImage;
    V3DLONG sz[4];
    unsigned char* data1d;



    Branch_Detection_Model();
    void Get_Image_Date(V3DPluginCallback2 &callback);
    void GUI_input_Thin_Method();
    void Gui_input_Rorate_Method();
    void Gui_input_Rorate_display_Method();
    void Gui_input_Rorate_3D_Method();
    void Gui_input_CrossPoint_3D_Method();
    ~Branch_Detection_Model();



};

#endif // BRANCH_DETECTION_MODEL_H
