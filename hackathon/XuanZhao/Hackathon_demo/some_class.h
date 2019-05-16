#ifndef SOME_CLASS_H
#define SOME_CLASS_H
#include <vector>
#include <QtGui>
#include <v3d_interface.h>
using namespace std;

struct point{
    enum type{ori=0,up=1,down=2,left=3,right=4,front=5,back=6}t;
    NeuronSWC nswc;
    vector<int> children;
    V3DLONG branch;
    point()=default;
    point operator=(point& p)
    {
        this->nswc=p.nswc;
        this->t=p.t;
        this->branch=p.branch;
        this->children.assign(p.children.begin(),p.children.end());
        return *this;
    }
};


struct block{
    int n;
    point o;
    vector<point> tips;
    //vector<point> pointTree;
    V3DLONG min_x,max_x,min_y,max_y,min_z,max_z;
    int parent;

};

class blockTree{
public:
    QList<block> blocklist;
    QString name;
};

struct swcNode
{
    NeuronSWC swc;
    struct swcNode* parent;
    struct swcNode* lchild;
    struct swcNode* rchild;
};






















#endif // SOME_CLASS_H
