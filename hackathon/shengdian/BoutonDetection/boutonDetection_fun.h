#ifndef BOUTONDETECTION_FUN_H
#define BOUTONDETECTION_FUN_H

#include <QtGui>
#include <v3d_interface.h>
#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <iostream>
#include <vector>
using namespace std;
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
QList <CellAPO> removeBoutons(QList <CellAPO> inapo,V3DLONG removed_dist_thre=2);
void getBoutonInTerafly(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int allnode=0,int ne_area=2);
void getSWCIntensityInTerafly(V3DPluginCallback2 &callback,string imgPath, QString inswc_file);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int block_size=32);
void maskImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,QString outpath,V3DLONG in_sz[4], NeuronTree& nt,int maskRadius=12);
void erosionImg(unsigned char * & inimg1d,V3DLONG in_sz[4],int kernelSize=3);
void getTeraflyBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int cropx=1024,int cropy=1024,int cropz=512);
QList <CellAPO> getBouton(NeuronTree nt,int in_thre,int allnode=0);
NeuronTree getBouton_toSWC(NeuronTree nt,int in_thre,int allnode=0,float dis_thre=2.0);
void getBoutonInImg(V3DPluginCallback2 &callback, unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt,int useNeighborArea);
void getNodeRadius(unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronTree& nt);
void printHelp();
NeuronSWC nodeRefine(unsigned char * & inimg1d, V3DLONG nodex, V3DLONG nodey , V3DLONG nodez,V3DLONG * sz,int neighbor_size=2);
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size=128);
#endif // BOUTONDETECTION_FUN_H
