#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
using namespace std;

struct Point
{
    float x = 0;
    float y = 0;
    float z = 0;
};

struct swcnode
{
    int id = -2;
    int type = 2;
    Point p;
    int radius = 1;
    int parent = -2;
};

struct Path
{
    vector<Point> l;
    bool dump = false;
    bool merged = false;
};

#endif