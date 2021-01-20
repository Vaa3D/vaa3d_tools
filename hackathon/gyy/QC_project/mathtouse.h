#ifndef MATHTOUSE_H
#define MATHTOUSE_H
#include <iostream>
#include <vector>
#include <algorithm>
#include "x_y_shift.h"

using namespace std;


class MathToUse
{
public:
    MathToUse();
public:
    static double dydx(vector<double> a,double b);
    static vector<double> centerExpansionRatio(vector<x_y_shift> ABOS);
};

#endif // MATHTOUSE_H
