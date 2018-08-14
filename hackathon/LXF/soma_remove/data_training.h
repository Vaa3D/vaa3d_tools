#ifndef DATA_TRAINING_H
#define DATA_TRAINING_H
//#include "qstringlist.h"
#include "v3d_message.h"
#include "v3d_interface.h"
#include "../../heyishan/blastneuron_bjut/overlap/overlap_gold.h"
#include "soma_remove_main.h"
struct Each_line
{
    double x1;
    double x2;
    double x3;
    double x4;
    double x5;
};
struct Chart
{
    Each_line first_line;
    Each_line second_line;
    Each_line third_line;
    Each_line forth_line;
    Each_line fifth_line;
};
struct Feature
{
    vector<bool> y_n;
    vector<double> overlap_level;
    vector<double> ratio_v;
    vector<double> count_v;
    vector<double> D;
    vector<double> grey_mean;
    vector<double> grey_std;
};


vector<Coordinate> readtxt_LXF(const QString& filename);
bool data_training(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
#endif // DATA_TRAINING_H
