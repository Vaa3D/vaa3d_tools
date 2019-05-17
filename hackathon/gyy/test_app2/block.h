#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <QtGui>
#include <v3d_interface.h>

using namespace std;

struct point : public NeuronSWC
{
    enum type{ori=0, up=1, down=2, reft=3, right=4, front=5,back=6}t;
    point(point &p){
        NeuronSWC();
        this->t=p.t;
    }
};

struct block
{
    int n;
    point o;
    vector<point> tips;
    V3DLONG min_x, max_x, min_y, max_y, min_z, max_z;
    int parent;
};

class blockTree
{
public:
    QList<block> blocklist;
    QString name;
};

void seg_neuron(block &b);

#endif // BLOCK_H
