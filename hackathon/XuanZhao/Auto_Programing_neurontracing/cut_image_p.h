#ifndef CUT_IMAGE_P_H
#define CUT_IMAGE_P_H

#include <vector>
#include <QtGui>
#include <v3d_interface.h>

using namespace std;

struct block{
    int n;
    NeuronSWC o;
    vector<NeuronSWC> tips;
    V3DLONG min_x,max_x,min_y,max_y,min_z,max_z;
    int parent;
};

class blockTree{
public:
    QList<block> blocklist;
    QString name;
};


class parameter1{
public:
    int dx;
    int dy;
    int dz;
    QStringList eswcfiles;
    QString brain;
    QString savepath;
    bool singleTree;
};
























#endif // CUT_IMAGE_P_H
