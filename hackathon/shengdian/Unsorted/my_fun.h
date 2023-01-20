#ifndef MY_FUN_H
#define MY_FUN_H
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <vector>
#include "volimg_proc.h"
#include <QHash>
#include <fstream>
#include <sstream>
#include <v3d_interface.h>
using namespace std;
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code);
void swc_profile_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,long block_size);
void getSWCIntensityInTerafly(V3DPluginCallback2 &callback, string imgPath, QString inswc_file);
void getTipComponent(QString inswc_file, QString outpath, int cropx, int cropy, int cropz);
//void getSomaFeature(V3DPluginCallback2 &callback, string imgPath, QString inapo_file);
void getSomaBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo_file, QString outpath, int cropx, int cropy, int cropz);
void getMarkerRadius(unsigned char *&inimg1d, long in_sz[], NeuronSWC& s);
void getTipBlock(V3DPluginCallback2 &callback, string imgPath, QString inswc_file, QString outpath, int cropx, int cropy, int cropz);
//void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath, QString inapo, QString outpath, int cropx, int cropy, int cropz);
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size=4);
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath, QList<CellAPO> apolist, string outpath, int cropx, int cropy, int cropz, int sample=1);
void maskImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,QString outpath,V3DLONG in_sz[4], NeuronTree& nt,int maskRadius=12,int erosion_kernel_size=0);
void erosionImg(unsigned char * & inimg1d,V3DLONG in_sz[4],int kernelSize=3);
#endif // MY_FUN_H
