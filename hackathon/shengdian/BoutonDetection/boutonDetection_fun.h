#ifndef BOUTONDETECTION_FUN_H
#define BOUTONDETECTION_FUN_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <vector>
using namespace std;
void getBoutonInTerafly(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int allnode=0);
void getSWCIntensityInTerafly(V3DPluginCallback2 &callback,string imgPath, QString inswc_file);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size);
//QList <CellAPO> splitSWC(string inswc_file, string outpath, int cropx=256, int cropy=256, int cropz=256);
void splitSWC(V3DPluginCallback2 &callback, string imgPath,string inswc_file, string outpath, int cropx=256, int cropy=256, int cropz=256);
void maskImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,QString outpath,V3DLONG in_sz[4], NeuronTree& nt,int maskRadius=12);
void erosionImg(unsigned char * & inimg1d,V3DLONG in_sz[4],int kernelSize=3);
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int cropx=1024,int cropy=1024,int cropz=512);
QList <CellAPO> getBouton(NeuronTree nt,int threshold,int allnode=0);
void getBoutonInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea=0);
void getNodeRadius(unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt);
void printHelp();
NeuronSWC nodeIntensity(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size);
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size=256);
#endif // BOUTONDETECTION_FUN_H
