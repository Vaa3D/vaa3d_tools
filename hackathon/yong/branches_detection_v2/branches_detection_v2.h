#ifndef BRANCHES_DETECTION_H
#define BRANCHES_DETECTION_H
#include <QString>
#include "v3d_interface.h"

struct BRANCH_LS_PARA
{
    NeuronTree nt;
    V3DLONG xb;
    V3DLONG yb;
    V3DLONG zb;
    unsigned char *data1d = 0;
    V3DLONG in_sz[3];
    //LandmarkList listLandmarks;
    QString inimg_file,rawfilename,markerfilename,swcfilename,inimg_file_2nd;
};

bool branches_detection_v2(V3DPluginCallback2 &callback, QWidget *parent,BRANCH_LS_PARA &P);
bool get_sub_terafly(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA);
int branch_detection(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,LandmarkList &curlist);
//bool flag=rayinten_2D(k,i,ray_numbers_2d ,ray_length_2d,ray_x, ray_y, image_binary,sz[0],sz[1]);
int raymodel(V3DPluginCallback2 &callback, QWidget *parent);
int curve_detection(V3DPluginCallback2 &callback, QWidget *parent);
int gassion_filter(V3DPluginCallback2 &callback, QWidget *parent);
int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz,V3DLONG * & region_sz, int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz);
void swap2bytes(void *targetp);
void swap4bytes(void *targetp);
char checkMachineEndian();
#endif // BRANCHES_DETECTION_V2_H
