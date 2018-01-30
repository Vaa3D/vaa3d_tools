#ifndef MRMR_H
#define MRMR_H

#include <QVector>

using namespace  std;
class DataTable
{
public:
    float *data;
    float **data2d;
    long nsample;
    long nvar;
    int *classLabel;
    int *sampleNo;
    char **variableName;
    int b_zscore;
    int b_discetize;

    DataTable ();
    ~DataTable ();
    int buildData2d();
    void destroyData2d();
    void printData(long nsample_start, long nsample_end, long nvar_start,long nvar_end);
    void printData();
    void zscore(long indExcludeColumn, int b_discretize);
    void discretize(double threshold, int b_discretize);
};

long * runmrmr(float * data, long x_sample, long y_variable, int nfea, int sel_method, int binNumber);

#endif // MRMR_H
