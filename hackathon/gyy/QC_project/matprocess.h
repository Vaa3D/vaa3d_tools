#ifndef MATPROCESS_H
#define MATPROCESS_H
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"

using namespace std;
using namespace cv;
class MatProcess
{
public:
    MatProcess();
public:
    static Mat getRowPart(const Mat& m,int a,int b);
    static Mat getColsPart(const Mat& m,int a,int b);
};

#endif // MATPROCESS_H
