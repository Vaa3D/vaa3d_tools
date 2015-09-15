#ifndef POINT3D_UTIL_H
#define POINT3D_UTIL_H

#include <vector>
using namespace std;

struct Point3D{
    float x;
    float y;
    float z;
};


double PointDistance (Point3D A, Point3D B );
Point3D average_point (vector<Point3D> points );

#endif // POINT3D_UTIL_H
