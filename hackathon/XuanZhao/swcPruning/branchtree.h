#ifndef BRANCHTREE_H
#define BRANCHTREE_H


#include "v3d_interface.h"

#include <fstream>
#include <sstream>
#include <iostream>

#define zx_dist(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define PI 3.14157292653589793238

using namespace std;


struct Branch{
    V3DLONG headPointIndex,endPointIndex;
    Branch* parent;
    int level,rLevel;
    float length, distance;
    float lengthToSoma, weight, sWeight;

    float localAngle1,localAngle2,localAngle3;
    float globalAngle1,globalAngle2,globalAngle3;

    int isNormalY;
    int isNeighbor;

    int inflectionNum;

    Branch():level(-1),rLevel(-1),headPointIndex(-1),endPointIndex(-1),length(0),distance(0),lengthToSoma(0),weight(0),sWeight(0)
    {
        isNormalY = true;
        isNeighbor = false;
        inflectionNum = 0;
        parent=0;
    }
    ~Branch(){}

    bool get_pointsIndex_of_branch(vector<V3DLONG> &points,NeuronTree &nt);
    bool get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt);
    void calculateMidPointsLocalAngle(NeuronTree &nt, ofstream &csvFile, double toBifurcationD);

    void findInflectionPoint(NeuronTree &nt, double d, double cosAngleThres, ofstream &csvFile);

    bool operator <(const Branch& other) const;
};

struct BranchTree
{
    vector<Branch> branches;
    NeuronTree nt;
    float maxWeight;
    BranchTree():nt()
    {
        branches.clear();
        maxWeight = 0;
    }
    ~BranchTree(){
        branches.clear();
    }

    bool initialize(NeuronTree t);
    bool get_level_index(vector<int> &level_index,int level);
    int get_max_level();

    void calculateBranchMidPointsLocalAngle(ofstream &csvFile, double toBifurcationD);

    void findBranchInflectionPoint(ofstream &csvFile, double d, double cosAngleThres);
    void groupBifurcationPoint(ofstream &csvFile, double d);
    void groupBifurcationPoint2(ofstream &csvFile, double d);

    void groupBifurcationPoint3(ofstream &csvFile, double d);

    XYZ getBranchVector(vector<V3DLONG> pointsIndex, double d);

    XYZ getBranchLocalVector(vector<V3DLONG> pointsIndex, double d);

    XYZ getBranchGlobalVector(vector<V3DLONG> pointsIndex);

    void saveMarkerFlag(QString markerPath);

    void refineBifurcationPoint();

    void judgeIsNormalY(int i, vector<vector<int> > branchChildren, double d);

    void pruningCross(double d);

    void calculateChildrenBranchAngle(ofstream &csvFile, double d);

    void calculateChildrenBranchGlobalAngle(ofstream &csvFile, double d);


};

#endif // BRANCHTREE_H
