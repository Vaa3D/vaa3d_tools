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
    vector<double> y_n;
    vector<double> overlap_level;
    vector<double> ratio_v;
    vector<double> count_v;
    vector<double> D;
    vector<double> grey_mean;
    vector<double> grey_std;
};
//class Matrix
//{
//public:
//    double data[999][999];
//    int size;
//    static int times;
//public:
//    void qiuNi(vector<vector<double> > &V1);
//    void setSize();
//    void show();
//    void chuShi(vector<vector<double> > &V1);
//};
bool export_TXT(Each_line &E,Chart &chart,QString fileSaveName);
double get_D(vector<double> &v1,vector<double> &v2);
Each_line E_calculate(Feature &feature);
bool Cov_calculate(Chart &chart,Feature &feature);
vector<Coordinate> readtxt_LXF(const QString& filename);
bool data_training(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
#endif // DATA_TRAINING_H
