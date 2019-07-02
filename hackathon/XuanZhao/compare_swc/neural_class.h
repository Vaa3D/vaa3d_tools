#ifndef NEURAL_CLASS_H
#define NEURAL_CLASS_H

#include <v3d_interface.h>
#include <vector>
#include <math.h>
#include <iostream>
using namespace std;

template<class T>
inline double distance_two_point(T &point1,T &point2)
{
    return sqrt(((double)point1.x-(double)point2.x)*((double)point1.x-(double)point2.x)+((double)point1.y-(double)point2.y)*((double)point1.y-(double)point2.y)+((double)point1.z-(double)point2.z)*((double)point1.z-point2.z));
}

template<class T>
inline double angle_three_point(T &point1,T &point2,T &point3)
{
    double x1=point2.x-point1.x;
    double y1=point2.y-point1.y;
    double z1=point2.z-point1.z;

    double x2=point3.x-point1.x;
    double y2=point3.y-point1.y;
    double z2=point3.z-point1.z;

    return acos((x1*x2+y1*y2+z1*z2)/(distance_two_point(point1,point2)*distance_two_point(point1,point3)));
}

template<class T>
inline double x_angle_two_point(T &point1,T &point2)
{
    double xy_d=sqrt((point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y));
    double x_d=point2.x-point1.x;
    return acos(x_d/xy_d);
}

template<class T>
inline double z_angle_two_point(T &point1,T &point2)
{
    return acos((point2.z-point1.z)/distance_two_point(point1,point2));
}

struct Angle{
    double x_angle,z_angle;
    Angle():x_angle(0),z_angle(0){}
};

struct Branch{
    NeuronSWC head_point,end_point;
    Branch* parent;
    Angle head_angle,end_angle;
    int level;
    double distance,length;
    Branch():head_angle(),end_angle(),level(0),distance(0),length(0),head_point(),end_point()
    {
        //head_point=0;
        //end_point=0;
        parent=0;
    }
    ~Branch()
    {
        //if(head_point) delete head_point;
        //if(end_point) delete end_point;
        //if(parent) delete parent;
    }
    inline double get_distance()
    {
        return distance_two_point(head_point,end_point);
    }
    bool get_points_of_branch(vector<NeuronSWC> &points,NeuronTree &nt);
    bool get_r_points_of_branch(vector<NeuronSWC> &r_points,NeuronTree &nt);
};

struct SwcTree{
    vector<Branch> branchs;
    NeuronTree nt;
    SwcTree():nt()
    {
        branchs.clear();
    }
    bool initialize(NeuronTree t);
    void inline display()
    {
        for(int i=0;i<branchs.size();++i)
        {
            cout<<"level: "<<branchs[i].level<<" "
               <<"distance: "<<branchs[i].distance<<" "<<"length: "
              <<branchs[i].length<<" "<<"head_angle: "<<branchs[i].head_angle.x_angle
             <<" "<<branchs[i].head_angle.z_angle<<" "<<"end_angle: "<<branchs[i].end_angle.x_angle
            <<" "<<branchs[i].end_angle.z_angle<<endl;
        }
    }
    bool branchs_to_nt();
    bool cut_cross();
    bool find_big_turn();
};
























#endif // NEURAL_CLASS_H
