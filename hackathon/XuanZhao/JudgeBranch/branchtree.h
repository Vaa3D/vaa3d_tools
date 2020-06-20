#ifndef BRANCHTREE_H
#define BRANCHTREE_H

#include <vector>
#include <iostream>
#include <math.h>

#include "basic_surf_objs.h"


using namespace std;

template<typename T>
double dis(T node1, T node2){
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    return sqrt(a);
}

struct Angle
{
    float x,y,z;
    Angle() {
        x = y = z = 0;
    }

    ~Angle(){}

    Angle(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void setXYZ(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }

    void setXYZ(const Angle& other){
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
    }

    void normAngle(){
        float s = sqrt(x*x + y*y + z*z);
        if(s>0){
            this->x /= s;
            this->y /= s;
            this->z /= s;
        }
    }

    double dot(const Angle& other){
        return (this->x*other.x + this->y*other.y + this->z*other.z);
    }
};

struct Branch{
    V3DLONG headPointIndex,endPointIndex;
    Branch* parent;
    int level;
    float length, distance;
    float intensityMean, intensityStd, intensityRatioToGlobal, intensityRationToLocal;
    float gradientMean;
    float angleChangeMean;
    float sigma12;
    float sigma13;

    Branch():level(-1),headPointIndex(-1),endPointIndex(-1)
    {
        parent=0;
    }
    ~Branch(){}

    bool get_pointsIndex_of_branch(vector<V3DLONG> &points,NeuronTree &nt);
    bool get_r_pointsIndex_of_branch(vector<V3DLONG> &r_points, NeuronTree &nt);

    bool caculateFeature(unsigned char* data1d, V3DLONG* sz, NeuronTree &nt);
};


struct BranchTree{
    vector<Branch> branchs;
    NeuronTree nt;
    BranchTree():nt()
    {
        branchs.clear();
    }
    ~BranchTree(){
        branchs.clear();
    }

    bool initialize(NeuronTree t);
    bool get_level_index(vector<int> &level_index,int level);
    int get_max_level();

};

#endif // BRANCHTREE_H
