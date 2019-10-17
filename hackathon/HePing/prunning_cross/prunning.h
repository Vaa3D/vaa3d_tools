#ifndef PRUNNING_H
#define PRUNNING_H
#include <v3d_interface.h>
#include<vector>
#include<math.h>
using namespace std;
#define IN 100000000000
#define min_step 3
#define max_step 5
#define PI 3.1415926
template<class T>
inline double norm_v(T &vector0)
{
    return sqrt(vector0.x*vector0.x+vector0.y*vector0.y+vector0.z*vector0.z);
}

template<class T>
inline double distance_two_point(T &point1,T &point2)
{
    return sqrt(((double)point1.x-(double)point2.x)*((double)point1.x-(double)point2.x)+((double)point1.y-(double)point2.y)*((double)point1.y-(double)point2.y)+((double)point1.z-(double)point2.z)*((double)point1.z-point2.z));
}

struct Point_xyz
{
    int x,y,z;
    Point_xyz(int x,int y,int z){
        this->x=x;
        this->y=y;
        this->z=z;
    }

};

struct Angle{
    double x,y,z;
    Angle():x(0),y(0),z(0){}
    Angle(double x,double y,double z)
    {
        this->x=x;
        this->y=y;
        this->z=z;
    }

    Angle norm_angle()
    {
        double s = norm_v(*this);
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    double operator *(const Angle &other) const
    {
        return (x*other.x+y*other.y+z*other.z);
    }
};

struct Branch{
    NeuronSWC head_point,end_point;
    Branch* parent;
    Angle head_angle,end_angle;
    int level;
    double distance,length,distance_to_soma,sum_angle;
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
        return distance_two_point(head_point,end_point);  //目前没什么用，不应该是起点和终点之间的距离这样对于弯曲度较大的段会造成distance较小
        //return length;
    }

    bool operator <(const Branch &other) const
    {
        if(level!=other.level)
            return (level>other.level);
        return (length<other.length);
    }

    bool get_points_of_branch(vector<NeuronSWC> &points,NeuronTree &nt);
    bool get_r_points_of_branch(vector<NeuronSWC> &r_points,NeuronTree &nt);
    bool get_mean_std(QString input_path,V3DPluginCallback2 &callback,NeuronTree &nt,double &mean,double &std);
    bool get_meanstd(QString braindir, V3DPluginCallback2 &callback, NeuronTree &nt, double &branchmean, double &branchstd, int resolution);

};

struct SWCTree{
    vector<Branch> branchs;
    NeuronTree nt;
    bool initialize(NeuronTree t);
    bool cut_cross(QString input_path,V3DPluginCallback2 &callback);
};



#endif // PRUNNING_H
