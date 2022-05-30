#ifndef NODE_H
#define NODE_H
#include <vector>

class Node
{
public:

    float x;
    float y;
    float z;
    float r;
    int type;

    // opitonal directions (for those guidepoint nodes that will initialize the traces)
    float vx;
    float vy;
    float vz;
    // optional correlation at this location direction and scale (to sort the nodes)
    float corr;

    std::vector<int> nbr; // list of neighbouring node list indexes

    // types as in neuromorpho.org description
    static int NOTHING;
    static int SOMA;
    static int AXON;
    static int BASAL_DENDRITE;
    static int APICAL_DENDRITE;
    static int FORK;
    static int END;
    static int UNDEFINED;

    Node(float xn, float yn, float zn, float rn, int typ);
    Node(float xn, float yn, float zn, float vx1, float vy1, float vz1, float corr1, float rn, int typ);

    ~Node();

};

#endif // NODE_H
