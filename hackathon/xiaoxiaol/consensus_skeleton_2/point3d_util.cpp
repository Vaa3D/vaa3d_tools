#include "point3d_util.h"
#include <iostream>
#include "math.h"
#include <math.h>
#include <numeric>
#include <algorithm>


double PointDistance (Point3D A, Point3D B){
   return sqrt( (pow(A.x - B.x,2)+ pow(A.y - B.y,2)  + pow(A.z - B.z,2)) ) ;
}

Point3D average_point(vector<Point3D> points)
{
    vector<double> x;
    vector<double> y;
    vector<double> z;
    Point3D averageP;
    if (points.size() == 0){cout<<"empty point list!" <<endl;}
    for (int i = 0 ; i < points.size(); i++)
    {
        x.push_back(points[i].x);
        y.push_back(points[i].y);
        z.push_back(points[i].z);
    }
    averageP.x = accumulate( x.begin(), x.end(), 0.0 )/ x.size();
    averageP.y = accumulate( y.begin(), y.end(), 0.0 )/ y.size();
    averageP.z = accumulate( z.begin(), z.end(), 0.0 )/ z.size();
    return averageP;
}
