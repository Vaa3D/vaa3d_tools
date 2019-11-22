#ifndef BRANCH_COUNT_SOMA_H
#define BRANCH_COUNT_SOMA_H
#include<v3d_interface.h>
#include<vector>
#include<math.h>
using namespace std;

//template<class T> bool compute_marker_pca_hp(vector<T> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);


template<class T>
inline double distance_two_point(T &point1,T &point2){
    return sqrt((point1.x-point2.x)*(point1.x-point2.x)+(point1.y-point2.y)*(point1.y-point2.y)+(point1.z-point2.z)*(point1.z-point2.z));
}

struct Branch{
    NeuronSWC head_point,end_point;
    double distance_to_soma=0;
    inline double get_distance()
    {
        return distance_two_point(head_point,end_point);

    }
};

struct location{

    double x;
    double y;
    double z;
    location(double x,double y,double z){
        this->x=x;
        this->y=y;
        this->z=z;
    }
    location(){
        x=y=z=0;
    }
};




struct SWCTree{

    vector<Branch> branchs;//存放从soma出来的所有分支
    vector<NeuronSWC> locations;
    void count_branch_location(NeuronTree t,QList<ImageMarker> &markers,vector<location> &points,double &max_radius);

};


bool four_point(vector<location> &points);

bool compute_marker_pca_hp(vector<location> markers,V3DLONG r,double &pc1,double &pc2,double &pc3);






#endif // BRANCH_COUNT_SOMA_H
