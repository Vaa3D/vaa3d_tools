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
#include <QHash>
#include "volimg_proc.h"
#include <fstream>
#include "FL_coordDefinition.h"
#include "FL_defType.h"
#include "neuronbranchtree.h"
#include "../../SGuo/imPreProcess/helpFunc.h"
#include "../../SGuo/imPreProcess/bilateral_filter.h"

using namespace std;

#define BoutonType 1
#define BoutonSWCNodeType 5
#define MinRefineAngle 60.0
#define PI 3.1415926
#define NeuronTreeRefine 0
#define NodeRefine 1
#define RefineAllinOne 2
#define RadiusEstimation 3

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

/*bouton detection*/
void boutonDetection_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly);
void swc_profile_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,long block_size=128,int bkg_bias=15,double upfactor=2.0);
void swc_profile_image_fun(V3DPluginCallback2 &callback,string inimg_file, NeuronTree& nt,int bkg_bias=15,double upfactor=2.0);
void boutonFilter_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);
void boutonSWC_postprocessing(NeuronTree& nt,QList <NeuronSWC> init_bouton_sites,QString outpath,string inswc_file,
                              float min_bouton_dist=1.0,float pruning_internode_thre=1.0,float pruning_tip_thre=1.0);
/*filter*/
QList <CellAPO> getBouton_1D_filter(NeuronTree nt,double radius_delta=1.3,double intensity_delta=0.05,double AXON_BACKBONE_RADIUS=4);//old version, will remove later
QList <NeuronSWC> boutonFilter_fun(NeuronTree nt,double radius_delta=1.3,double intensity_delta=1,double AXON_BACKBONE_RADIUS=3);
void map_bouton_2_neuronTree(NeuronTree& nt,QList <NeuronSWC> bouton_sites);
void swc_profile_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,bool in_terafly);
std::vector<double> get_sorted_fea_of_seg(V_NeuronSWC inseg,bool radiusfea=false);
std::vector<double> mean_and_std_seg_fea(std::vector<double> input);
std::vector<int> smoothedZScore(std::vector<float> input,float threshold=2.0,float influence=0.5,int lag=10);
std::vector<int> peaks_in_seg(std::vector<double> input,int isRadius_fea=0,float delta=0.5);

/*refinement: 1. mean-shift; 2. node_refine;3.line_refine*/
void refinement_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output,int method_code=0,bool in_terafly=true);
void refinement_terafly_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int method_code=0,int refine_radius=8,long half_block_size=128,int nodeRefine_radius=2);
void refinement_Image_fun(V3DPluginCallback2 &callback,string imgPath, NeuronTree& nt,int method_code=0,int refine_radius=8,int nodeRefine_radius=2);
NeuronSWC calc_mean_shift_center(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre=40, int windowradius=15);
NeuronSWC calc_mean_shift_center_v2(unsigned char * & inimg1d,NeuronSWC snode,V3DLONG sz_image[], double bkg_thre=40,int windowradius=15);
NeuronSWC nodeRefine(unsigned char * & inimg1d,NeuronSWC s,V3DLONG * sz,int neighbor_size=5);
double getAngleofNodeVector(NeuronSWC n0,NeuronSWC n1,NeuronSWC n2);
NeuronSWC lineRefine(unsigned char * & inimg1d,V3DLONG * sz,NeuronSWC snode,NeuronSWC spnode, int sqhere_radius=5,int searching_line_radius=2);

/*Image processing*/
void enhanceImage(unsigned char * & data1d,unsigned char * & dst,V3DLONG *mysz,bool biilateral_filter=false);
bool upsampleImage(unsigned char * & inimg1d,unsigned char * & outimg1d,V3DLONG *szin, V3DLONG *szout, double *dfactor);
template <class T> bool upsample3dvol(T *outdata, T *indata, V3DLONG *szout, V3DLONG *szin, double *dfactor);
template <class T> bool interpolate_coord_linear(T * interpolatedVal, Coord3D *c, V3DLONG numCoord,
                       T *** templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
                       V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
template <class T> bool interpolate_coord_linear(T * interpolatedVal, Coord3D *c, V3DLONG numCoord,
                                                 T * templateVol3d, V3DLONG tsz0, V3DLONG tsz1, V3DLONG tsz2,
                                                 V3DLONG tlow0, V3DLONG tup0, V3DLONG tlow1, V3DLONG tup1, V3DLONG tlow2, V3DLONG tup2);
void erosionImg(unsigned char *&inimg1d, long in_sz[], int kernelSize);
void maskImg(V3DPluginCallback2 &callback, unsigned char *&inimg1d, QString outpath, long in_sz[], NeuronTree &nt, int maskRadius,int erosion_kernel_size);
void getBoutonMIP(V3DPluginCallback2 &callback, unsigned char *& inimg1d,V3DLONG in_sz[],QString outpath);
void getBoutonBlock(V3DPluginCallback2 &callback, string imgPath,QList <CellAPO> apolist,string outpath,int half_block_size=8,uint mip_flag=0);
void getBoutonBlockSWC(NeuronTree nt,string outpath,int half_block_size=8);
void getBoutonBlock_inImg(V3DPluginCallback2 &callback,string inimg_file,QList <CellAPO> apolist,string outpath,int block_size=16);

/*swc processing*/
NeuronTree linearInterpolation(NeuronTree nt,int Min_Interpolation_Pixels=1);
NeuronTree boutonSWC_internode_pruning(NeuronTree nt,float pruning_dist=1.0,bool ccf_domain=false);
NeuronTree internode_pruning(NeuronTree nt,float pruning_dist=1.5);
void nearBouton_pruning(NeuronTree& nt,float pruning_dist=5.0,bool ccf_domain=false);
NeuronTree tipNode_pruning(NeuronTree nt, float pruning_dist=1.0,bool ccf_domain=false);
void bouton_feature_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);
void boutonType_label(NeuronTree& nt,bool ccf_domain=true,int max_terminaux_bouton_branch_len=5);
void rendering_different_bouton(NeuronTree& nt, int type_bias=BoutonSWCNodeType);
void boutonDesity_computing(NeuronTree& nt,bool ccf_domain=true);
void bouton_dist_to_soma(NeuronTree& nt,bool ccf_domain=true);
void boutonswc_pruning_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);
void ccf_profile_dofunc(V3DPluginCallback2 & callback, const V3DPluginArgList & input,V3DPluginArgList & output);// for processing registered swc
void scale_registered_swc(NeuronTree& nt,float xshift_pixels=20.0,float scale_xyz=25.0);
void merge_registered_swc_onto_raw(NeuronTree& nt_raw,NeuronTree nt_registered);
QList <CellAPO> nt_2_multi_centers(NeuronTree nt,float xs=256.0,float ys=256.0,float zs=256.0);

/*radius estimation*/
double radiusEstimation(unsigned char * & inimg1d,V3DLONG in_zz[4], NeuronSWC s,double dfactor,double bkg_thresh);
double getNodeRadius(unsigned char * & inimg1d,V3DLONG in_sz[4], NeuronSWC s,double bkg_thresh=40);
double getNodeRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronSWC s, double bkg_thresh,int z_half_win_size);
double getNodeRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronSWC s, double bkg_thresh);
void getNTRadius_XY(unsigned char *&inimg1d, long in_sz[], NeuronTree& nt, double bkg_thresh=40);

/*other func:*/
bool teraImage_swc_crop(V3DPluginCallback2 &callback, string inimg, string inswc, string inapo,QString save_path, int cropx, int cropy, int cropz);
QHash<V3DLONG,int> getIntensityStd(NeuronTree nt,int thre_size=64);
void printHelp();
#endif // BOUTONDETECTION_FUN_H
