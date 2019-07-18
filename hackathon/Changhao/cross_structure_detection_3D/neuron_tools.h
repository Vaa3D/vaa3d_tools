#ifndef NEURON_TOOLS_H
#define NEURON_TOOLS_H
#include "./../../../v3d_external/released_plugins_more/v3d_plugins/pruning_swc/my_surf_objs.h"
#include "basic_surf_objs.h"
#include "v3d_interface.h"


vector<MyMarker*>  NeuronTree2vectorofMyMarker(NeuronTree nt);
NeuronTree vectorofMyMarker2NeuronTree ( vector<MyMarker*> swc );
LocationSimple MyMarker2LocationSimple(MyMarker* marker);
LocationSimple MyMarker2LocationSimple(MyMarker marker);
MyMarker* LocationSimple2MyMarkerP(LocationSimple s);
MyMarker LocationSimple2MyMarker(LocationSimple s);

void merge_two_neuron(vector<MyMarker*> & swc ,NeuronTree nt);
void merge_two_neuron(vector<MyMarker*> & swc_main ,vector<MyMarker*> & swc);
void merge_two_neuron(NeuronTree nt1 ,NeuronTree nt2, NeuronTree &nt_merged);
NeuronTree merge_two_neuron(NeuronTree swc1 , NeuronTree nt);
QList <ImageMarker> LandmarkListQList_ImageMarker(LandmarkList marker_l);
vector<MyMarker*> readSWC_file1(string swc_file);
NeuronTree readSWC_file1(const QString& filename);
bool writeSWC_file1(const QString& filename, const NeuronTree& nt);
bool saveSWC_file1(string swc_file, vector<MyMarker*> & outmarkers);
LandmarkList QList_ImageMarker2LandmarkList(QList <ImageMarker> markers);
LandmarkList Readdata_from_TMI_result(QString img_name);
LandmarkList Match_leaf_tip(LandmarkList leaf_swc,LandmarkList real_tip,double threshold=5,bool ignore_z=false);


LandmarkList FromTreeGetBranch(NeuronTree nt);
LandmarkList getCalcuMarker(LandmarkList markerFromMarkerFile);
void getCalcuMarker(LandmarkList markerFromMarkerFile, LandmarkList &calcuMarker);
void getCalcuMarker(LandmarkList markerFromMarkerFile,  vector<MyMarker> &calcuMarker);
void displayMarker(V3DPluginCallback2 &callback, v3dhandle windows, LandmarkList calcuMarker);
LandmarkList mergeLandmark(LandmarkList a, LandmarkList b);
vector<LocationSimple> LandmarkList2vectorLocationSimple(LandmarkList a);
LandmarkList vectorLocationSimple2LandmarkList(vector<LocationSimple> a);
LandmarkList calibrate_tipPoints(V3DPluginCallback2 &callback, LandmarkList original_tipPoint, int raidus3D, int bkg_thres);
bool saveSoma(QString path,LocationSimple SOMA);
void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin);
void ComputemaskImage(NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin, LandmarkList missedBranchPoint);
void ComputemaskImage(V3DPluginCallback2 &callback, NeuronTree neurons,unsigned char* pImMask,V3DLONG sx,V3DLONG sy,V3DLONG sz, double margin, LandmarkList missedBranchPoint);
QHash<V3DLONG, V3DLONG> NeuronNextPn(const NeuronTree &neurons);

void Erosion(unsigned char* image, unsigned char* res, int nHeight, int nWidth);
void Dilation(unsigned char* image, unsigned char* res, int nHeight, int nWidth);
void Closeoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth);
void Openoperations(unsigned char* image, unsigned char* res, int nHeight, int nWidth);
void gaussian_filter(unsigned char * data1d,
                                        V3DLONG *in_sz,
                                        unsigned int Wx,
                                        unsigned int Wy,
                                        unsigned int Wz,
                                        unsigned int c,
                                        double sigma,
                                        float * &outimg);
double dist(MyMarker a, LocationSimple b);
double dist(LocationSimple a, MyMarker b);
double dist(LocationSimple a, LocationSimple b);
double dist(NeuronSWC a, LocationSimple b);

void nt_processing_for_mask(NeuronTree nt, LandmarkList pointSet, NeuronTree &nt_new, double margin);
bool saveSWC_file_app2(string swc_file, vector<MyMarker*> & outmarkers, list<string> & infostring);
QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code);

#endif // NEURON_TOOLS_H
