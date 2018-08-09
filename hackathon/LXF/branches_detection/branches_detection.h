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
bool branches_detection(V3DPluginCallback2 &callback, QWidget *parent,BRANCH_LS_PARA &P);
bool do_each_block(V3DPluginCallback2 &callback,NeuronTree &curr,BRANCH_LS_PARA &P);
bool do_each_block_v2(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,NeuronSWC curr_point,NeuronTree &nt);
bool do_curr_point(V3DPluginCallback2 &callback,NeuronSWC &S,BRANCH_LS_PARA &P,NeuronTree &nt,unsigned char* &data);
bool do_detection(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,LandmarkList &curlist);
bool do_detection_v2(V3DPluginCallback2 &callback,BRANCH_LS_PARA &PA,NeuronSWC curr_point,LandmarkList &curlist,NeuronTree &nt);
void raymodel(V3DPluginCallback2 &callback, QWidget *parent);
int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz,V3DLONG * & region_sz, int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz);
void swap2bytes(void *targetp);
void swap4bytes(void *targetp);
char checkMachineEndian();
#endif // BRANCHES_DETECTION_H
