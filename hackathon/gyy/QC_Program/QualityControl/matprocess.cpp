#include "matprocess.h"

MatProcess::MatProcess()
{

}
Mat MatProcess::getRowPart(const Mat &m, int a, int b)
{
    Mat r=Mat(b-a,m.cols,m.type());
    return r;
}

Mat MatProcess::getColsPart(const Mat &m, int a, int b)
{
    Mat r=Mat(b-a,m.rows,m.type());
    return r;
}
