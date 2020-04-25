#ifndef SEG_DENDRITE_H
#define SEG_DENDRITE_H
#include<v3d_interface.h>
#include<vector>

template<class T>
inline double distance_two_point(T &point1,T &point2){
    return sqrt((point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y)+(point1.z-point2.z)*(point1.z-point2.z));
}

struct Point{
    V3DLONG n;
    V3DLONG parent;
    double x,y,z;
    double dis_to_soma=0;
    Point(){
        x=y=z=0;
        parent=0;
    }

    Point(V3DLONG n,V3DLONG parent,double x,double y,double z,double dis){
        this->n=n;
        this->x=x;
        this->y=y;
        this->z=z;
        this->parent=parent;
        dis_to_soma=dis;
    }
};

struct segment{
    Point head,end;
    double distance=0;
    segment* parent=0;
    segment():head(),end(),distance(0){
        parent=0;
    }

    inline double get_distance(){
        return distance_two_point(head,end);
    }
};



bool seg_dendrite(QString outpath,NeuronTree &nt);
#endif // SEG_DENDRITE_H
