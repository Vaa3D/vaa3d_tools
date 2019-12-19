#ifndef NEURON_TOOLS_H
#define NEURON_TOOLS_H
#include"my_surf_objs.h"
#include "basic_surf_objs.h"
#include "v3d_interface.h"
vector<MyMarker*>  NeuronTree2vectorofMyMarker(NeuronTree nt);
NeuronTree vectorofMyMarker2NeuronTree ( vector<MyMarker*> swc );
LocationSimple MyMarker2LocationSimple(MyMarker* marker);

void merge_two_neuron(vector<MyMarker*> & swc ,NeuronTree nt);
NeuronTree merge_two_neuron(NeuronTree swc1 , NeuronTree nt);
QList <ImageMarker> LandmarkListQList_ImageMarker(LandmarkList marker_l);

LandmarkList QList_ImageMarker2LandmarkList(QList <ImageMarker> markers);
LandmarkList Readdata_from_TMI_result(QString img_name);
LandmarkList Match_leaf_tip(LandmarkList leaf_swc,LandmarkList real_tip,double threshold=5,bool ignore_z=false);
LocationSimple Nearest_Match(LocationSimple point_a,LandmarkList curlist);
unsigned char *NeuronTree2img(NeuronTree nt1,V3DLONG sz[]);
unsigned char * NeuronTree2img_special(NeuronTree nt1, V3DLONG sz[]);
NeuronSWC interger(NeuronSWC point_a);
float online_confidece_one_branch(V3DLONG tip_point_num,NeuronTree nt,unsigned char * bimg_datald,V3DLONG sz[]);
float online_confidece(LocationSimple pointa,LocationSimple pointb,unsigned char * bimg_datald,V3DLONG sz[]);
//NeuronTree denseTree(NeuronTree sparse_tree);
vector<NeuronSWC> pointsOfTwoPoint(NeuronSWC point_a,NeuronSWC point_b);
bool vectorMyMarkerX2vectorMyMarker(vector<MyMarkerX*> inputMarkers, vector<MyMarker*> & outmarkers);
#endif // NEURON_TOOLS_H
