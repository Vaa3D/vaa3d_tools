#ifndef MYSURFACE_H
#define MYSURFACE_H

#include <map>
#include <math.h>
#include "basic_surf_objs.h"
//#include "imagectrl.h"
#include "pca.h"

#define IN 100000000000
#define PI 3.1415926535898

using namespace std;

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

template<class T1,class T2>
inline double p_to_line_2(T1 &point1,T1 &point2,T1 &point3)
{
    T2 a,c;
    a.x=point1.x-point2.x;
    a.y=point1.y-point2.y;
    a.z=point1.z-point2.z;

    c.x=point3.x-point2.x;
    c.y=point3.y-point2.y;
    c.z=point3.z-point2.z;

    double a_norm=norm_v(a);

    double angle_ac=angle_three_point(point2,point1,point3);

    double sin_ac=sin(angle_ac);

    return a_norm*sin_ac;
}

template<class T1,class T2>
inline double distance_two_point(T1 &point1,T2 &point2)
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

struct direction
{
    double x,y,z;
    direction():x(0),y(0),z(0){}
    direction(double _x,double _y,double _z)
    {
        x=_x;
        y=_y;
        z=_z;
    }
    inline direction& operator =(direction &d)
    {
        x=d.x;
        y=d.y;
        z=d.z;
        return *this;
    }
    inline direction operator -(direction &other)
    {
        direction t;
        t.x=this->x-other.x;
        t.y=this->y-other.y;
        t.z=this->z-other.z;
        return t;
    }
    inline double operator *(const direction &other) const
    {
        return (x*other.x+y*other.y+z*other.z);
    }

    inline direction& norm_dir()
    {
        double d = norm_v(*this);
        this->x = x/d;
        this->y = y/d;
        this->z = z/d;
        return *this;
    }
    inline direction& negative()
    {
        x=-x;
        y=-y;
        z=-z;
        return *this;
    }

    inline void round_xyz()
    {
        x=round(x);
        y=round(y);
        z=round(z);
    }

    inline direction& d_xyz(direction &other)
    {
        x=x*other.x;
        y=y*other.y;
        z=z*other.z;
        return *this;
    }
};

struct simplePoint
{
    V3DLONG x,y,z;
    direction dire;
    simplePoint():x(0),y(0),z(0),dire()
    {}
    simplePoint(V3DLONG x,V3DLONG y,V3DLONG z)
    {
        this->x=x;
        this->y=y;
        this->z=z;
    }
    direction getDirection(unsigned char ***data3d, V3DLONG *sz);
    bool getNbSimplePoint(vector<simplePoint> &nbsimplepoints,int mode);
};

struct assemblePoint
{
    static double r;
    vector<simplePoint> sps;
    vector<simplePoint> surface_sps;
    int size;
    double x,y,z;
    double dx0,dx1,dy0,dy1,dz0,dz1;
    double intensity;
    direction dire;

    assemblePoint():size(0),x(0),y(0),z(0),dx0(0),dx1(0),dy0(0),dy1(0),dz0(0),dz1(0),dire(),intensity(0)
    {
        sps.clear();
        surface_sps.clear();
    }
    assemblePoint& init(assemblePoint &other)
    {
        assemblePoint p;
        p.x=other.x;
        p.y=other.y;
        p.z=other.z;
        p.dx0=other.dx0;
        p.dx1=other.dx1;
        p.dy0=other.dy0;
        p.dy1=other.dy1;
        p.dz0=other.dz0;
        p.dz1=other.dz1;
        p.dire=other.dire;
        p.intensity=other.intensity;
        p.size=other.size;
        for(int i=0;i<other.sps.size();++i)
        {
            p.sps.push_back(other.sps[i]);
        }
        for(int i=0;i<other.surface_sps.size();++i)
        {
            p.surface_sps.push_back(other.surface_sps[i]);
        }
        return p;
    }

    assemblePoint& operator =(assemblePoint &other)
    {
        assemblePoint p;
        p=this->init(other);
        return p;
    }

    bool operator <(const assemblePoint &other) const
    {
        if(x!=other.x)
            return (x<other.x);
        if(this->y!=other.y)
            return (y<other.y);

        return (z<other.z);
    }

    ~assemblePoint()
    {
        this->sps.clear();
        this->surface_sps.clear();
    }

    double getIntensity(vector<vector<vector<unsigned char> > > &image);

    bool assemble(vector<vector<vector<unsigned char> > > &image, unsigned char ***data3d, vector<vector<vector<int> > > &mask, V3DLONG* sz);

    bool renewXYZ(vector<vector<vector<unsigned char> > > &image);

    bool getSurfacePoints();

    bool getNbPointsIndex(vector<assemblePoint> &apoints,vector<int> &nbpointsindex);

    double getDistance(assemblePoint & other);

    bool getDirection(unsigned char*** data3d, V3DLONG *sz);

    bool showDirection(QList<ImageMarker> &markers);

    bool meanShift(vector<vector<vector<unsigned char> > > &image, double r, long long *sz);

};

struct segment
{
    vector<NeuronSWC> points;
    NeuronSWC headpoint,tailpoint;
    direction headangle,tailangle;
    double length;

    NeuronSWC getHeadPoint();
    NeuronSWC getTailPoint();
    direction getHeadAngle();
    direction getTailAngle();
    double getLength();
    bool getSegMeanStdIntensity(vector<vector<vector<unsigned char> > > &image,double &mean,double &std);
};

class apTracer
{
public:
    apTracer() {}

    bool initialAssemblePoint(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, unsigned char ***data3d, V3DLONG* sz, double thres);

    bool writeAssemblePoints(const QString markerfile,vector<assemblePoint> &assemblepoints);

    bool aptrace(vector<assemblePoint> &assemblePoints,vector<vector<vector<unsigned char> > > &image,NeuronTree &nt,V3DLONG* sz);

    bool trace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, unsigned char ***data3d, NeuronTree &nt, V3DLONG* sz);

    bool findTips(vector<assemblePoint> &apoints, vector<int> &tipsindex, vector<vector<vector<unsigned char> > > &image, long long *sz);

    bool direc_trace(vector<assemblePoint> &assemblePoints, vector<vector<vector<unsigned char> > > &image, vector<segment> &v_segment, V3DLONG* sz);

    bool connectPointandSegment(vector<assemblePoint> &assemblePoints, vector<segment> &v_segment);

    bool connectSegment(vector<segment> &v_segment);
};




#endif // MYSURFACE_H
