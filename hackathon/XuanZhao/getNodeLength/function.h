#ifndef FUNCTION_H
#define FUNCTION_H

#include "basic_surf_objs.h"
#include <QList>
#include <QHash>
#include <vector>
#include <iostream>

using namespace std;

template<typename T>
double dis(T node1, T node2){
    double a = (node1.x - node2.x)*(node1.x - node2.x) + (node1.y - node2.y)*(node1.y - node2.y) + (node1.z - node2.z)*(node1.z - node2.z);
    return sqrt(a);
}

void getNodeLength(NeuronTree &nt, int maxR, double dendritR, double otherR, double thre);
double getNodeLength2(NeuronTree& nt,int maxR,double dendritR, double otherR,double thre);

struct Branch{
    NeuronSWC head_point,end_point;
    Branch* parent;
    int level;
    Branch():level(0),head_point(),end_point()
    {
        parent=0;
    }
    ~Branch(){}

//    bool operator <(const Branch &other) const
//    {
//        if(level!=other.level)
//            return (level>other.level);
//        return (length<other.length);
//    }

    bool get_points_of_branch(vector<int> &points,NeuronTree &nt);
    bool get_r_points_of_branch(vector<int> &r_points,NeuronTree &nt);
};


struct SwcTree{
    vector<Branch> branchs;
    NeuronTree nt;
    SwcTree():nt()
    {
        branchs.clear();
    }
    bool initialize(NeuronTree t);
    bool get_level_index(vector<int> &level_index,int level);
    int get_max_level();

};


#endif // FUNCTION_H
