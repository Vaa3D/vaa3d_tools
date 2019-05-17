#ifndef SOME_FUNCTION_H
#define SOME_FUNCTION_H

#include <QString>
#include <basic_surf_objs.h>
#include <vector>
#include <string>
#include <cstring>
#include <v3d_interface.h>
#include "some_class.h"

using namespace std;



template<class T,class iterator>
inline T max0(iterator begin,iterator end){
    T temp=*begin;
    T max=*begin;
    while(begin!=end){
      max=max>temp?max:temp;
      begin++;
      temp=*begin;
    }
    return max;
}
template<class T,class iterator>
inline T min0(iterator begin,iterator end){
    T temp=*begin;
    T max=*begin;
    while(begin!=end){
      max=max<temp?max:temp;
      begin++;
      temp=*begin;
    }
    return max;
}

inline bool isin(V3DLONG parent,std::vector<V3DLONG> a)
{
    for(int i=0;i<a.size();++i)
    {
        if(parent==a[i])
            return true;
    }
    return false;
}
inline int findIndex(const char* s)
{
    if(s==NULL)
    {
        return -1;
    }
    for(int i=0;;++i)
    {
        if(s[i]=='\0')
            return -2;
        if(s[i]=='.')
            return i;
    }
}
inline QString path(const QString& s)
{
    std::string tmp=s.toStdString();
    const char* tmp0=tmp.c_str();

    int index=findIndex(tmp0);
    const int num=100;
    char tmp1[num];
    for(int i=0;i<index;++i)
    {
        tmp1[i]=tmp0[i];
    }
    tmp1[index]='\0';
    QString result(tmp1);

    return result;

    /*char* tmp1=new char[100];
    strcpy(tmp1,tmp0);
    char* index=strstr(tmp1,'.');
    char s0[index+1];
    for(int i=0;i<index;++i)
    {
        s0[i]=tmp0[i];
    }
    s0[index]='\0';
    QString result(s0);
    return result;*/
}

/*struct point{
    size_t x,y,z;
    enum type{up=0,down=1,left=2,right=3,front=4,back=5}t;
};*/

inline vector<point> get_up(NeuronTree nt,size_t min_x,size_t max_x,size_t min_y,size_t max_y,size_t max_z)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].x>min_x&&nt.listNeuron[i].x<=max_x&&nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=max_y&&nt.listNeuron[i].z>(max_z-1)&&nt.listNeuron[i].z<=max_z)
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::up;
            tmp.push_back(p);
        }
    }
    return tmp;
}

inline vector<point> get_down(NeuronTree nt,size_t min_x,size_t max_x,size_t min_y,size_t max_y,size_t min_z)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].x>min_x&&nt.listNeuron[i].x<=max_x&&nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=max_y&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=(min_z+1))
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::down;
            tmp.push_back(p);
        }
    }
    return tmp;
}

inline vector<point> get_left(NeuronTree nt,size_t min_x,size_t max_x,size_t min_z,size_t max_z,size_t min_y)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].x>min_x&&nt.listNeuron[i].x<=max_x&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=max_z&&nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=(min_y+1))
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::left;
            tmp.push_back(p);
        }
    }
    return tmp;
}

inline vector<point> get_right(NeuronTree nt,size_t min_x,size_t max_x,size_t min_z,size_t max_z,size_t max_y)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].x>min_x&&nt.listNeuron[i].x<=max_x&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=max_z&&nt.listNeuron[i].y>(max_y-1)&&nt.listNeuron[i].y<=max_y)
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::right;
            tmp.push_back(p);
        }
    }
    return tmp;
}

inline vector<point> get_front(NeuronTree nt,size_t min_y,size_t max_y,size_t min_z,size_t max_z,size_t max_x)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=max_y&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=max_z&&nt.listNeuron[i].x>(max_x-1)&&nt.listNeuron[i].x<=max_x)
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::front;
            tmp.push_back(p);
        }
    }
    return tmp;
}

inline vector<point> get_back(NeuronTree nt,size_t min_y,size_t max_y,size_t min_z,size_t max_z,size_t min_x)
{
    point p;
    vector<point> tmp;
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=max_y&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=max_z&&nt.listNeuron[i].x>min_x&&nt.listNeuron[i].x<=(min_x+1))
        {
            p.nswc.x=nt.listNeuron[i].x;
            p.nswc.y=nt.listNeuron[i].y;
            p.nswc.z=nt.listNeuron[i].z;
            p.t=point::back;
            tmp.push_back(p);
        }
    }
    return tmp;
}

void cut_image(V3DPluginCallback2 &callback,NeuronTree nt,point p,QString& qstr);


/*bool eswc_empty(NeuronTree nt,size_t min_x,size_t max_x,size_t min_y,size_t max_y,size_t min_z,size_t max_z)
{
    int size=nt.listNeuron.size();
    for(int i=0;i<size;++i)
    {
        if(nt.listNeuron[i].x>min_x&&nt.listNeuron[i]<=max_x&&nt.listNeuron[i].y>min_y&&nt.listNeuron[i].y<=max_y&&nt.listNeuron[i].z>min_z&&nt.listNeuron[i].z<=max_z)
        {
            return false;
        }
        return true;
    }
}*/

V3DLONG getBranch(NeuronSWC swct,vector<point> pts);


vector<point> trans(NeuronTree nt);

block getBlockOfOPoint(point p,int dx,int dy,int dz);

vector<point> getTPointOfBlock(vector<point> pts,block b);

blockTree getBlockTree(vector<point> nt,int dx,int dy,int dz);




#endif // SOME_FUNCTION_H
