#ifndef NEURONQC_FUNC_H
#define NEURONQC_FUNC_H

#include "v3d_interface.h"
#include <vector>
#include <set>
#include <queue>

#include <fstream>
#include <sstream>
#include <iostream>

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

#define MAX_DOUBLE 1.79769e+308
#define GRID_LENGTH 50

using namespace std;

static string featureName[] = {
    "loop",
    "threeBifurcation",
    "isSort",
    "somaType",
    "gap",
    "allTypes",
    "shortBranch",
    "nodeLength"
};
static int featureSize = 8;
static QList<ImageMarker> errorMarkerList;

static unsigned char colortable[][3]={
{255,   0,    0},
{  0, 255,    0},
{  0,   0,  255},
{255, 255,    0},
{  0, 255,  255},
{255,   0,  255},
{255, 128,    0},
{  0, 255,  128},
{128,   0,  255},
{128, 255,    0},
{  0, 128,  255},
{255,   0,  128},
{128,   0,    0},
{  0, 128,    0},
{  0,   0,  128},
{128, 128,    0},
{  0, 128,  128},
{128,   0,  128},
{255, 128,  128},
{128, 255,  128},
{128, 128,  255},
};

struct neuronQCFeature{
    vector<bool> bFeature;
    vector<string> featureInfo;
};

bool getNeuronFeatureForBatch(QString swcDir, float sLengthThres, float nodeLengthThres, bool loopThreeBifurcation, ofstream &csvFile);

bool writeNeuronFeature(vector<neuronQCFeature> nqcfv, vector<string> neuronId, ofstream& csvFile);

bool writeErrorMarkerList(QString errorMarkersFile);
bool writeErrorApoList(QString errorMarkersFile);

bool getNeuronFeature(const NeuronTree& nt, neuronQCFeature& nqcf, float sLengthThres, float nodeLengthThres, bool loopThreeBifurcation);

//judge if sort or not
bool judgeSort(const NeuronTree& nt, QString& sortInfo);

//split neuronTree
vector<NeuronTree> splitNeuronTree(const NeuronTree& nt);

//judge soma type
bool judgeSomaType(const NeuronTree& nt, QString& somaTypeInfo);

NeuronSWC getSoma(const NeuronTree& nt);

//get three bifurcation count
int getThreeBifurcationCount(const vector<NeuronSWC>& outputErroneousPoints);

int getThreeBifurcationCount(const vector<NeuronSWC> &outputErroneousPoints, NeuronSWC soma);

//get loop count
int getLoopCount(const vector<NeuronSWC>& outputErroneousPoints);

//judge all type
bool judgeTypes(const NeuronTree& nt, string &allTypesInfo);

//get count of short branches
int getShortBranchesCount(const NeuronTree& nt, float lengthThres);

//get node path min max
bool getMinMaxNodePath(const NeuronTree& nt, float& minPath, float& maxPath, float lengthThres);

void stringToXYZ(string xyz, float &x, float &y, float &z);

vector<NeuronSWC> loopDetection2(V_NeuronSWC_list inputSegList);

vector<NeuronSWC> loopDetection(V_NeuronSWC_list inputSegList);
void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk);
vector<V_NeuronSWC_list> showConnectedSegs(const V_NeuronSWC_list& inputSegList);
void rc_findConnectedSegs(V_NeuronSWC_list& inputSegList, set<size_t>& singleTreeSegs, size_t inputSegID, multimap<string, size_t>& segEnd2segIDmap);
set<size_t> segEndRegionCheck(V_NeuronSWC_list& inputSegList, size_t inputSegID);

V_NeuronSWC_list removeSameSegment(NeuronTree& nt);


#endif // NEURONQC_FUNC_H
