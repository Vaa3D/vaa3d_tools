#ifndef JUDGEBRANCH_H
#define JUDGEBRANCH_H

#include "v3d_interface.h"

#include "basic_surf_objs.h"

#include <vector>

struct HierarchySegment;
struct MyMarker;

using namespace std;
struct keyPoint : public BasicSurfObj
{
    int type;
    bool isTip;
    float x, y, z;
    union{
        float r;
        float radius;
    };

    union{
        V3DLONG pn;
        V3DLONG parent;
    };
    int level;
    keyPoint() {n = type = pn = 0; x = y = z = r = 0; level = -1;}

    bool meanShift(unsigned char* pdata, V3DLONG* sz, int windowradius = 0);
};

struct keyTree
{
    QList<keyPoint> listKeyPoint;
    QHash<int,int> hashKeyPoint;
    keyTree() {}

    bool initial(NeuronTree nt);
//    void getBranchFeature(unsigned char* pdata, V3DLONG* sz);
};

vector<HierarchySegment*> splitHierarchySegment(HierarchySegment* segment);

void getHierarchySegmentFeature(HierarchySegment* segment,unsigned char* pdata, V3DLONG* sz);

void pruneNeuronTree(QString imagePath,V3DPluginCallback2& callback);

void pruneNeuronTree(QString imagePath, QString swcPath, V3DPluginCallback2& callback);

NeuronTree pruneNeuronTree(Image4DSimple *image, NeuronTree& nt);

void pruneNeuronTree(V3DPluginCallback2& callback);

vector<NeuronTree> splitNeuronTree(NeuronTree nt);

double getHierarchySegmentDirection(unsigned char*** data3d, V3DLONG* sz, HierarchySegment* segment);

vector<MyMarker*> deleteSegmentByType(vector<MyMarker*>& inswc, int type);


//void judgeBranch(QString imgPath, V3DPluginCallback2 &callback);

#endif // JUDGEBRANCH_H
