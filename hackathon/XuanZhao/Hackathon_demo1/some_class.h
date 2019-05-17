#ifndef SOME_CLASS_H
#define SOME_CLASS_H


#include <vector>
#include <QtGui>
#include <v3d_interface.h>
using namespace std;

struct block{
    int n;
    NeuronSWC o;
    vector<NeuronSWC> tips;
    //vector<point> pointTree;
    V3DLONG min_x,max_x,min_y,max_y,min_z,max_z;
    int parent;
};

class blockTree{
public:
    QList<block> blocklist;
    QString name;
};































#endif // SOME_CLASS_H
