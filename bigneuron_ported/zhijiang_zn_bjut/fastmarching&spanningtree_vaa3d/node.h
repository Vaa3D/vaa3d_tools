#ifndef NODE_H
#define NODE_H
#include <v3d_interface.h>

class Node
{
public:
    double x;
    double y;
    double z;
    double r;
public:
    Node(double x,double y,double z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->r = 0;
    }
};


#endif // NODE_H
