#ifndef BOUTONDETECTION_FUN_H
#define BOUTONDETECTION_FUN_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <numeric>
#include "neuron_format_converter.h"
#include "v_neuronswc.h"
#include <functional>
using namespace std;
#define MaxIntensity 256;
#define MinIntensity -1;
struct Bouton_Color_Basic
{
    int color_ele_r;
    int color_ele_g;
    int color_ele_b;
    Bouton_Color_Basic() {color_ele_r=color_ele_g=0;color_ele_b=255;}
};
struct BoutonMarker
{
    float x,y,z;
    float rx,ry,rz;//radius of each direction
    int intensity;
    int type; //1, internal bouton; 2,tip bouton
    bool on;
    int confidenceScore;// using for the future: 1-100
    V3DLONG volume;
    Bouton_Color_Basic color;
    string comment;
    BoutonMarker(){
        x=y=z=rx=ry=rz=0.0;
        type=1;
        intensity=0;
        on=true;
        confidenceScore=100;
        comment="";
        color.color_ele_r=color.color_ele_g=0;color.color_ele_b=255;
    }
};
struct Bouton_Color_List
{
    QList <Bouton_Color_Basic> listcolor;
    bool on;
    Bouton_Color_List()
    {
        listcolor.clear();
        on=true;
        init();
    }
    void init()
    {
        int colorR[10]={120,200,0,200,0,220,188,180,250,120};
        int colorG[10]={200,20,20,0,200,200,200,94,200,100};
        int colorB[10]={200,0,200,200,200,0,20,37,120,120};
        for(int i=0;i<10;i++)
        {
            Bouton_Color_Basic tmp;
            tmp.color_ele_r=colorR[i];
            tmp.color_ele_g=colorG[i];
            tmp.color_ele_b=colorB[i];
            listcolor.append(tmp);
        }
    }
};
NeuronTree removeDupNodes(NeuronTree nt,V3DLONG removed_dist_thre=1);
QList <CellAPO> rmNearMarkers(QList <CellAPO> inapo,V3DLONG removed_dist_thre=4);
void getBoutonInTerafly(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int allnode=0,int ne_area=2);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int half_block_size=8,uint mip_flag=0);
void getBoutonBlockSWC(NeuronTree nt,string outpath,int half_block_size=8);
void getBoutonMIP(V3DPluginCallback2 &callback, unsigned char *& inimg1d,V3DLONG in_sz[],QString outpath);
void erosionImg(unsigned char *&inimg1d, long in_sz[], int kernelSize);
void maskImg(V3DPluginCallback2 &callback, unsigned char *&inimg1d, QString outpath, long in_sz[], NeuronTree &nt, int maskRadius,int erosion_kernel_size);
QList <CellAPO> getBouton(NeuronTree nt,int in_thre,int allnode=0);
QList <CellAPO> getBouton_1D_filter(NeuronTree nt);
void getBoutonBlock_inImg(V3DPluginCallback2 &callback,string inimg_file,QList <CellAPO> apolist,string outpath,int block_size=16);
NeuronTree getBouton_toSWC(NeuronTree nt,int in_thre,int allnode=0,float dis_thre=2.0);
void getSWCIntensityInImg(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int useNeighborArea=2,int is2dRadius=1);
void getSWCIntensityInTeraflyImg(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int useNeighborArea);
NeuronTree linearInterpolation(NeuronTree nt,int Min_Interpolation_Pixels=1);

double getNodeRadius(unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronSWC s,double bkg_thresh=40);
double getNodeRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronSWC s, double bkg_thresh=40);
void getNTRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronTree& nt, double bkg_thresh=40);

void printHelp();
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size=2);
std::vector<double> get_sorted_fea_of_seg(V_NeuronSWC inseg,bool radiusfea=false);
std::vector<double> mean_and_std_seg_fea(std::vector<double> input);
std::vector<int> smoothedZScore(std::vector<float> input,float threshold=2.0,float influence=0.5,int lag=10);
std::vector<int> peaks_in_seg(std::vector<double> input,int MAX_IN_VALUE=100,float delta=0.5);
std::vector<int> peaks_filter(std::vector<double> input,std::vector<int> init_peaks,std::vector<int> init_valleys, int MAX_IN_VALUE=100);
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size=64);
#endif // BOUTONDETECTION_FUN_H
