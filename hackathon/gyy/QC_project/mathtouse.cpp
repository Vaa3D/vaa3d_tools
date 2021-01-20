#include "mathtouse.h"
#include "math.h"

MathToUse::MathToUse()
{

}
double MathToUse::dydx(vector<double> a, double b)
{
    double sum=0;
    for(int i =1;i<a.size();i++)
    {
        sum+(a[i]-a[i-1])/b;
    }
    return  sum/(a.size()-1);
}



vector<double> MathToUse::centerExpansionRatio(vector<x_y_shift> ABOS)
{
    vector<double> result_ratio;
    double ratioStandard = sqrt(ABOS[0].x*ABOS[0].x+ABOS[0].y*ABOS[0].y+0.0);
    for(int i = 0; i < ABOS.size(); i ++)
    {
        double ratio = sqrt(ABOS[i].x*ABOS[i].x+ABOS[i].y*ABOS[i].y+0.0);
        result_ratio.push_back(ratioStandard/ratio);
    }
    return result_ratio;
}
