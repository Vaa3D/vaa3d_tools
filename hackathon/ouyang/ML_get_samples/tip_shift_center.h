#ifndef TIP_SHIFT_CENTER_H
#define TIP_SHIFT_CENTER_H

#include <v3d_interface.h>
//#include "../../../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h"
#include "../../../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_center_plugin.h"

class mean_shift_fun
{
public:
    mean_shift_fun();
    ~mean_shift_fun();

private:

    V3DLONG page_size;
    float * data1Dc_float;

public:

    V3DLONG sz_image[4];
    //unsigned char * mask1D;
    template <class T>
    void pushNewData(T * data1Dc_in, V3DLONG sz_img[4])
    {
        if(data1Dc_float!=0){
            memory_free_float1D(data1Dc_float);
        }
        sz_image[0]=sz_img[0];
        sz_image[1]=sz_img[1];
        sz_image[2]=sz_img[2];
        sz_image[3]=sz_img[3];
        page_size=sz_image[0]*sz_image[1]*sz_image[2];

        data1Dc_float = memory_allocate_float1D(sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(V3DLONG i=0; i<page_size*sz_img[3]; i++){
            data1Dc_float[i] = (float)(data1Dc_in[i]);
        }
      //  normalizeEachChannelTo255<float>(data1Dc_float, sz_img);

    }
};


void tip_shift_center(V3DPluginCallback2 & callback, const V3DPluginArgList & input,
                                          V3DPluginArgList & output);
void load_image_marker(V3DPluginCallback2 & callback,const V3DPluginArgList & input,
                        unsigned char * & image_data,LandmarkList &LList,int &intype,V3DLONG sz_img[4]);
void convert2UINT8_meanshift(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz);
vector<V3DLONG> landMarkList2poss(LandmarkList LandmarkList_input, V3DLONG _offset_Y, V3DLONG _offest_Z);
QList <LocationSimple> readPosFile_usingMarkerCode(const char * posFile);
#endif // TIP_SHIFT_CENTER_H
