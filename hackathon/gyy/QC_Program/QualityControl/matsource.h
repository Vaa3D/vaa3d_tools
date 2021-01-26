#ifndef MATSOURCE_H
#define MATSOURCE_H
#include "mainwindow.h"
#include "x_y_shift.h"
#include <QApplication>
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <QFileDialog>
#include <iostream>
#include <vector>
#include <algorithm>
#include <QAxObject>
#include <QMainWindow>
using namespace std;
using namespace cv;

class MatSource
{
public:
    MatSource(QString fileName ,int basicOverlap1,int basicOverLap2,int MIPsize);
public:
    vector<vector<vector<vector<String>>>> matSrc;
private:
    int MIPsize;
    int basicOverlap1;//仪器设定上下overlap
    int basicOverlap2;//仪器设定左右overlap
    int tileChannelNumbers;//一个tile内channel数量
    int SrcRowNumbers;//数据集行数
    int SrcColNumbers;//数据集列数
    int ChannelMatNumbers;//一个channel内图片数量
    void readMat(QString fileName);//读取指定文件种的数据集
public:
    static vector<String> readMats(QString fileName);
    static x_y_shift A_B_y_z_shift(const vector<String>& tileA,const vector<String>& tileB,int basicOverlap,QString orientation,int rows,int cols,int MIPsize);//计算不同tile同一channel之间z_shift
    static x_y_shift mat_x_y_shift(const Mat& matA,const Mat& matB,QString orientation,int basicOverlap); //计算相邻mat之间下x：overlap,y: shift,已知方向
    static double getMatDistortionK1UseOverLapAndShift(const Mat& matA,const Mat& matB,QString orientation,int overlap,int shift,int num=50);//使用overlap和shift计算切向畸变K1
    static double tileZEnlargeRatioUseOverLapAndShift(const vector<String>& tileA,const vector<String>& tileB,int basicOverlap);//使用overlap和shift计算tlie内Z方向上从0平面开始以0平面为基础线性放大率
    static vector<int> getMatDistortionOverLaps(const Mat& matA,const Mat& matB,QString orientation,int overlap,int shift,int num=50); //计算相邻mat之间从上到下的overlap
    static vector<x_y_shift> tile_x_y_shift(const vector<String>& tileA,const vector<String>& tileB,QString orientation,int basicOverlap); //计算相邻tile之间下x：overlap,y: shift,已知方向
    static vector<vector<double>> getDistortK1andZenlargeByOverlapandShift(const vector<String>& tileA,const vector<String>& tileB,QString orientation,vector<x_y_shift> tlieABoverlapsAndSshifts);
    static Mat getMIP(const vector<String>& tileA,int basicOverlap,QString orientation,int rows,int cols,int MIPsize);

};

#endif // MATSOURCE_H
