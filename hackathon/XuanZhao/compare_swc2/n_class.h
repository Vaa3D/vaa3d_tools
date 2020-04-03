#ifndef N_CLASS_H
#define N_CLASS_H


#include <v3d_interface.h>
#include <vector>
#include <map>
#include <math.h>
#include <iostream>
using namespace std;

#define IN 1000000000

template<class T>
inline double norm_v(T &vector0)
{
    return sqrt(vector0.x*vector0.x+vector0.y*vector0.y+vector0.z*vector0.z);
}

template<class T>
inline double dot_vv(T &vector1,T &vector2)
{
    return (vector1.x*vector2.x+vector1.y*vector2.y+vector1.z*vector2.z);
}

template<class T1,class T2>
inline double p_to_line(T1 &point1,T1 &point2,T1 &point3)
{
    T2 a,c;
    a.x=point1.x-point2.x;
    a.y=point1.y-point2.y;
    a.z=point1.z-point2.z;

    c.x=point3.x-point2.x;
    c.y=point3.y-point2.y;
    c.z=point3.z-point2.z;

    double a_norm=norm_v(a);
    double c_norm=norm_v(c);

    double angle_ac=angle_three_point(point2,point1,point3);

    double sin_ac=sin(angle_ac);
    double cos_ac=cos(angle_ac);

    if(a_norm*cos_ac>c_norm||cos_ac<0)
    {
        double d1=distance_two_point(point1,point2);
        double d2=distance_two_point(point1,point3);
        return (d1>d2)?d1:d2;
    }

    return a_norm*sin_ac;
}

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

template<class T>
bool get3DImageBasedPoint(const char* filename, T &point, int resolutionX, int resolutionY, int resolutionZ, QString braindir, V3DPluginCallback2 &callback)
{
    unsigned char* pdata = 0;
    size_t x0,x1,y0,y1,z0,z1;
    x0 = point.x - resolutionX/2.0;
    x1 = point.x + resolutionX/2.0;
    y0 = point.y - resolutionY/2.0;
    y1 = point.y + resolutionY/2.0;
    z0 = point.z - resolutionZ/2.0;
    z1 = point.z + resolutionZ/2.0;
    cout<<"x0:"<<x0<<" x1:"<<x1<<" y0:"<<y0<<" y1"<<y1<<" z0"<<z0<<" z1:"<<z1<<endl;
    pdata = callback.getSubVolumeTeraFly(braindir.toStdString(),x0,x1,y0,y1,z0,z1);
    V3DLONG sz[4] = {resolutionX,resolutionY,resolutionZ,1};
    int datatype = 1;
    return simple_saveimage_wrapper(callback,filename,pdata,sz,datatype);
}

struct Angle{
    double x,y,z;
    Angle():x(0),y(0),z(0){}
    Angle(double x,double y,double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
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
    double distance,length,length_to_soma,sum_angle;
    Branch():head_angle(),end_angle(),level(0),distance(0),length(0),head_point(),end_point()
    {
        parent=0;
    }
    ~Branch(){}

    inline double get_distance()
    {
        return distance_two_point(head_point,end_point);
    }

    bool operator <(const Branch &other) const
    {
        if(level!=other.level)
            return (level>other.level);
        return (length<other.length);
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
        cout<<"branch<<<<<<<<<<<<<<<<<<<<<<<<<<"<<endl;
        for(int i=0;i<branchs.size();++i)
        {
            cout<<"level: "<<branchs[i].level<<" "
               <<"distance: "<<branchs[i].distance<<" "<<"length: "
              <<branchs[i].length<<" "<<"head_angle: "<<branchs[i].head_angle.x
             <<" "<<branchs[i].head_angle.y<<" "<<branchs[i].head_angle.z<<" "<<"end_angle: "<<branchs[i].end_angle.x
            <<" "<<branchs[i].end_angle.y<<" "<<branchs[i].end_angle.z<<endl;
        }
    }
    bool get_level_index(vector<int> &level_index,int level);
    bool get_points_of_branchs(vector<Branch> &b, vector<NeuronSWC> &points, NeuronTree &ntb);
    int get_max_level();

    bool get_bifurcation_image(QString dir, int resolutionX, int resolutionY, int resolutionZ, bool all, QString braindir, V3DPluginCallback2 &callback);
    bool get_un_bifurcation_image(QString dir, int resolutionX, int resolutionY, int resolutionZ, bool all, QString braindir, V3DPluginCallback2 &callback);
};

class Swc_Compare{
public:

};



#endif // N_CLASS_H
