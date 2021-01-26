#include "mainwindow.h"
#include <QApplication>
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include "tifMapScribe.h"
#include <QFileDialog>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <QAxObject>

using namespace cv;
using namespace std;

static vector<Mat> tiles_vec;
static vector<tifMapScribe> tms;

static vector<vector<Mat>> tiles_vec_vec;
static vector<vector<tifMapScribe>> tms_vec_vec;

void background_correction_one_image();
int tile_region_background_ave_rows();
int correction_tiles_via_background();
int correction_tile_via_ave_region_peak();

void stitch_one_plane();
int stitch_all_planes_24tiles();
int stitch_2img_down_cover_up();

bool polynomial_curve_fit(vector<unsigned short> fitValue_vec, int n, Mat & A);
int equalization_image();
int image_distorted_correction();
int shift_global_optimization();
int cut_one_image();

bool compareNat(const std::string& a, const std::string& b);
int XZ_YZ_projection_20px_MIP();
int XZ_projection_20px_MIP();
int YZ_projection_20px_MIP();
int XZ_YZ_projection_20px_MIP_24Tiles();
int XZ_projection_20px_two_MIP();
int YZ_projection_20px_two_MIP();


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //background_correction_one_image();
    //tile_region_background_ave_rows();
    //correction_tiles_via_background();
    //correction_tile_via_ave_region_peak();


    //stitch_one_plane();
    stitch_all_planes_24tiles();
    //stitch_2img_down_cover_up();

    //shift_global_optimization();
    //equalization_image();
    //image_distorted_correction();
    //cut_one_image();

    //XZ_YZ_projection_20px_MIP_24Tiles();
    //XZ_YZ_projection_20px_MIP();
    //XZ_projection_20px_MIP();
    //YZ_projection_20px_MIP();
    //XZ_projection_20px_two_MIP();
    //YZ_projection_20px_two_MIP();


    cout<<"Successful!"<<endl;
    return a.exec();
}


QAxObject * appendSheet(QAxObject * &worksheets, const QString sheetName)
{
    int cnt = worksheets -> property("Count").toInt();
    QAxObject * pLastSheet = worksheets -> querySubObject("Item(int)", cnt);
    worksheets -> querySubObject("Add(QVariant)", pLastSheet->asVariant()); // create a new sheet
    QAxObject *pNewSheet = worksheets->querySubObject("Item(int)", cnt);
    pLastSheet -> dynamicCall("Move(QVariant)",pNewSheet->asVariant());
    pNewSheet -> setProperty("Name", sheetName);

    return pNewSheet;
}

unsigned short getdeep(int x, int y, vector<tifMapScribe>::size_type n)
{
       unsigned short deep = 0;

       if((x>tms[n].rightUpX || x<tms[n].leftDownX) || (y>tms[n].rightUpY || y<tms[n].leftDownY))
       {
           deep = 0;
       }

       else
       {
            deep = tiles_vec[n].at<unsigned short>( y-tms[n].leftDownY, x-tms[n].leftDownX);
       }

       return deep;
}

unsigned short getdeepImageI(int x, int y, vector<tifMapScribe>::size_type n, int image_i)
{
    unsigned short deep = 0;

    if((x>tms_vec_vec[image_i][n].rightUpX || x<tms_vec_vec[image_i][n].leftDownX) || (y>tms_vec_vec[image_i][n].rightUpY || y<tms_vec_vec[image_i][n].leftDownY))
    {
        deep = 0;
    }

    else
    {
         deep = tiles_vec_vec[image_i][n].at<unsigned short>( y-tms_vec_vec[image_i][n].leftDownY, x-tms_vec_vec[image_i][n].leftDownX);
    }

    return deep;
}

int shift_global_optimization()
{
    double s1_2   = 0;
    double s1_5  = -4;
    double s2_3   = 1;
    double s2_6   = -1;
    double s3_4   = -2;
    double s3_7   = -5;
    double s4_8   = -5;

    double s5_6   = 2;
    double s5_9   = -6;
    double s6_7   = -2;
    double s6_10  = -5;
    double s7_8   = -2;
    double s7_11  = -6;
    double s8_12   = -5;

    double s9_10  = 2;
    double s9_13  = -2;
    double s10_11 = 0;
    double s10_14 = -2;
    double s11_12 = -1;
    double s11_15 = -4;
    double s12_16 = -4;

    double s13_14 = 3;
    double s13_17 = -2;
    double s14_15 = -1;
    double s14_18 = -2;
    double s15_16 = -1;
    double s15_19 = -4;
    double s16_20 = -5;

    double s17_18 = 3;
    double s17_21 = -2;
    double s18_19 = -1;
    double s18_22 = -2;
    double s19_20 = -1;
    double s19_23 = -4;
    double s20_24 = -5;

    double s21_22 = 2;
    double s22_23 = -1;
    double s23_24 = -1;

    int tileNum = 24;
    Mat A = Mat::zeros(tileNum, tileNum, CV_64F);
    Mat B = Mat::zeros(tileNum, 1, CV_64F);
    Mat x = Mat::zeros(tileNum, 1, CV_64F);

    // 1
    A.at<double>(0, 0) = -1;
    A.at<double>(0, 1) = 3;
    A.at<double>(0, 2) = -1;
    A.at<double>(0, 5) = -1;
    B.at<double>(0, 0) = s2_3+s2_6-s1_2;

    // 2
    A.at<double>(1, 1) = -1;
    A.at<double>(1, 2) = 3;
    A.at<double>(1, 3) = -1;
    A.at<double>(1, 6) = -1;
    B.at<double>(1, 0) = s3_4+s3_7-s2_3;

    // 3
    A.at<double>(2, 2) = -1;
    A.at<double>(2, 3) = 2;
    A.at<double>(2, 7) = -1;
    B.at<double>(2, 0) = s4_8-s3_4;

    // 4
    A.at<double>(3, 0) = -1;
    A.at<double>(3, 4) = 3;
    A.at<double>(3, 5) = -1;
    A.at<double>(3, 8) = -1;
    B.at<double>(3, 0) = s5_6+s5_9-s1_5;

    // 5
    A.at<double>(4, 1) = -1;
    A.at<double>(4, 4) = -1;
    A.at<double>(4, 5) = 4;
    A.at<double>(4, 6) = -1;
    A.at<double>(4, 9) = -1;
    B.at<double>(4, 0) = s6_7+s6_10-s5_6-s2_6;

    // 6
    A.at<double>(5, 2) = -1;
    A.at<double>(5, 5) = -1;
    A.at<double>(5, 6) = 4;
    A.at<double>(5, 7) = -1;
    A.at<double>(5, 10) = -1;
    B.at<double>(5, 0) = s7_8+s7_11-s3_7-s6_7;

    // 7
    A.at<double>(6, 3) = -1;
    A.at<double>(6, 6) = -1;
    A.at<double>(6, 7) = 3;
    A.at<double>(6, 11) = -1;
    B.at<double>(6, 0) = s8_12-s7_8-s4_8;

    // 8
    A.at<double>(7, 4) = -1;
    A.at<double>(7, 8) = 3;
    A.at<double>(7, 9) = -1;
    A.at<double>(7, 12) = -1;
    B.at<double>(7, 0) = s9_10+s9_13-s5_9;

    // 9
    A.at<double>(8, 5) = -1;
    A.at<double>(8, 8) = -1;
    A.at<double>(8, 9) = 4;
    A.at<double>(8, 10) = -1;
    A.at<double>(8, 13) = -1;
    B.at<double>(8, 0) = s10_11+s10_14-s6_10-s9_10;

    // 10
    A.at<double>(9, 6) = -1;
    A.at<double>(9, 9) = -1;
    A.at<double>(9, 10) = 4;
    A.at<double>(9, 11) = -1;
    A.at<double>(9, 14) = -1;
    B.at<double>(9, 0) = s11_12+s11_15-s7_11-s10_11;

    // 11
    A.at<double>(10, 7) = -1;
    A.at<double>(10, 10) = -1;
    A.at<double>(10, 11) = 3;
    A.at<double>(10, 15) = -1;
    B.at<double>(10, 0) = s12_16-s8_12-s11_12;

    // 12
    A.at<double>(11, 8) = -1;
    A.at<double>(11, 12) = 3;
    A.at<double>(11, 13) = -1;
    A.at<double>(11, 16) = -1;
    B.at<double>(11, 0) = s13_14+s13_17-s9_13;

    // 13
    A.at<double>(12, 9) = -1;
    A.at<double>(12, 12) = -1;
    A.at<double>(12, 13) = 4;
    A.at<double>(12, 14) = -1;
    A.at<double>(12, 17) = -1;
    B.at<double>(12, 0) = s14_15+s14_18-s10_14-s13_14;

    // 14
    A.at<double>(13, 10) = -1;
    A.at<double>(13, 13) = -1;
    A.at<double>(13, 14) = 4;
    A.at<double>(13, 15) = -1;
    A.at<double>(13, 18) = -1;
    B.at<double>(13, 0) = s15_16+s15_19-s11_15-s14_15;

    // 15
    A.at<double>(14, 11) = -1;
    A.at<double>(14, 14) = -1;
    A.at<double>(14, 15) = 3;
    A.at<double>(14, 19) = -1;
    B.at<double>(14, 0) = s16_20-s12_16-s15_16;

    // 16
    A.at<double>(15, 12) = -1;
    A.at<double>(15, 16) = 3;
    A.at<double>(15, 17) = -1;
    A.at<double>(15, 20) = -1;
    B.at<double>(15, 0) = s17_18+s17_21-s13_17;

    // 17
    A.at<double>(16, 13) = -1;
    A.at<double>(16, 16) = -1;
    A.at<double>(16, 17) = 4;
    A.at<double>(16, 18) = -1;
    A.at<double>(16, 21) = -1;
    B.at<double>(16, 0) = s18_19+s18_22-s14_18-s17_18;

    // 18
    A.at<double>(17, 14) = -1;
    A.at<double>(17, 17) = -1;
    A.at<double>(17, 18) = 4;
    A.at<double>(17, 19) = -1;
    A.at<double>(17, 22) = -1;
    B.at<double>(17, 0) = s19_20+s19_23-s15_19-s18_19;

    // 19
    A.at<double>(18, 15) = -1;
    A.at<double>(18, 18) = -1;
    A.at<double>(18, 19) = 3;
    A.at<double>(18, 23) = -1;
    B.at<double>(18, 0) = s20_24-s16_20-s19_20;

    // 20
    A.at<double>(19, 16) = -1;
    A.at<double>(19, 20) = 2;
    A.at<double>(19, 21) = -1;
    B.at<double>(19, 0) = s21_22-s17_21;

    // 21
    A.at<double>(20, 17) = -1;
    A.at<double>(20, 20) = -1;
    A.at<double>(20, 21) = 3;
    A.at<double>(20, 22) = -1;
    B.at<double>(20, 0) = s22_23-s18_22-s21_22;

    // 22
    A.at<double>(21, 18) = -1;
    A.at<double>(21, 21) = -1;
    A.at<double>(21, 22) = 3;
    A.at<double>(21, 23) = -1;
    B.at<double>(21, 0) = s23_24-s19_23-s22_23;

    // 23
    A.at<double>(22, 19) = -1;
    A.at<double>(22, 22) = -1;
    A.at<double>(22, 23) = 2;
    B.at<double>(22, 0) = -s20_24-s23_24;

    // 24
    A.at<double>(23, 9) = 1;
    B.at<double>(23, 0) = 0;

    solve(A, B, x, DECOMP_LU);

    cout<<"tile_1  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(0)<<endl;
    cout<<"tile_2  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(1)<<endl;
    cout<<"tile_3  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(2)<<endl;
    cout<<"tile_4  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(3)<<endl;
    cout<<"tile_5  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(4)<<endl;
    cout<<"tile_6  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(5)<<endl;
    cout<<"tile_7  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(6)<<endl;
    cout<<"tile_8  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(7)<<endl;
    cout<<"tile_9  = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(8)<<endl;
    cout<<"tile_10 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(9)<<endl;

    cout<<"tile_11 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(10)<<endl;
    cout<<"tile_12 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(11)<<endl;
    cout<<"tile_13 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(12)<<endl;
    cout<<"tile_14 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(13)<<endl;
    cout<<"tile_15 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(14)<<endl;
    cout<<"tile_16 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(15)<<endl;
    cout<<"tile_17 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(16)<<endl;
    cout<<"tile_18 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(17)<<endl;
    cout<<"tile_19 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(18)<<endl;
    cout<<"tile_20 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(19)<<endl;

    cout<<"tile_21 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(20)<<endl;
    cout<<"tile_22 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(21)<<endl;
    cout<<"tile_23 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(22)<<endl;
    cout<<"tile_24 = "<<std::fixed<<std::setprecision(2)<<-x.at<double>(23)<<endl;


    return 1;
}


int ratial_distortion_correction()
{
    //    calculate a folder correction

    cout<<"Start!"<<endl;
    QString folderFile = QFileDialog::getExistingDirectory(nullptr, "Select tile folder","D:\\");
    cout<<"Input folder: "<<folderFile.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select an empty folder to save","D:\\");
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;

    QDir folderDir(folderFile);
    QFileInfoList imageList = folderDir.entryInfoList();
    cout<<__LINE__<<": imageList.size = "<<imageList.size()-2<<endl;

    for(int i_image = 0; i_image < imageList.size()-2; i_image ++)
    {
        double a1[9] = {1, 0, 960, 0, 1, 960, 0, 0, 1};
        Mat aa(3, 3, CV_64FC1, a1);//内参
        //Mat bb = (Mat_<double>(5, 1)<<0.0000000048, 0, 0, -3.13686838e-20, -5.68434189e-24);//畸变系数
        Mat bb = (Mat_<double>(5, 1)<<0.0000000048, 0, 0, 0, 0);

        Mat src = imread(imageList[i_image+2].filePath().toStdString(), 2);
        Mat dst;

        Mat warp;
        Point center = Point(src.cols/2, src.rows/2);
        undistort(src, warp, aa, bb, aa);
        //Mat affineTrans = getRotationMatrix2D(center, -0.106271, 1/0.998173);
        //warpAffine(warp, dst, affineTrans, src.size(), INTER_NEAREST);

        QString saveName = saveFolder + "\\" + imageList[i_image+2].baseName() + "_after.tif";
        imwrite(saveName.toStdString(), dst);
    }

    return 1;
}


int equalization_image()
{
    QString imagePath = QFileDialog::getOpenFileName(nullptr, "Select Image1", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select an empty folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo imageInfo(imagePath);

    Mat image_cut = imread(imageInfo.filePath().toStdString(), 2);

    int rows = image_cut.rows, cols = image_cut.cols;
    Mat img_res = Mat::zeros(rows, cols, CV_16UC1);
    int num[65536] = {0};
    double p[65536] = {0};
    double c[65536] = {0};

    for(int i = 0; i < rows; i ++)
    {
        for(int j = 0; j < cols; j ++)
        {
           num[image_cut.at<unsigned short>(i, j)] ++;
        }
    }

    for(int i = 0; i < 65536; i ++)
    {
        p[i] = num[i]/(rows*cols+0.0);
    }

    for(int i = 0; i < 65536; i ++)
    {
        for(int j = 0; j <= i; j ++)
        {
           c[i] += p[j];
        }
    }

    for(int i = 0; i < rows; i ++)
    {
        for(int j = 0; j < cols; j ++)
        {
           img_res.at<unsigned short>(i, j) = c[image_cut.at<unsigned short>(i, j)]*65535;
        }
    }
    cout<<__LINE__<<endl;

    QString saveName = saveFolder + "\\" + imageInfo.baseName() + "_equ.tiff";
    imwrite(saveName.toStdString(), img_res);

    return 1;
}

bool compareNat(const std::string& a, const std::string& b)
{
    if (a.empty())
        return true;
    if (b.empty())
        return false;
    if (std::isdigit(a[0]) && !std::isdigit(b[0]))
        return true;
    if (!std::isdigit(a[0]) && std::isdigit(b[0]))
        return false;
    if (!std::isdigit(a[0]) && !std::isdigit(b[0]))
    {
        if (std::toupper(a[0]) == std::toupper(b[0]))
            return compareNat(a.substr(1), b.substr(1));
        return (std::toupper(a[0]) < std::toupper(b[0]));
    }

    // Both strings begin with digit --> parse both numbers
    std::istringstream issa(a);
    std::istringstream issb(b);
    int ia, ib;
    issa >> ia;
    issb >> ib;
    if (ia != ib)
        return ia < ib;

    // Numbers are the same --> remove numbers and recurse
    std::string anew, bnew;
    std::getline(issa, anew);
    std::getline(issb, bnew);
    return (compareNat(anew, bnew));
}

int XZ_YZ_projection_20px_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);
    QStringList filters;
    filters << "*.tif" << "*.tiff";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    Mat image_para = imread(imageList[0].filePath().toStdString(), 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res_XZ = Mat::zeros(Z_length, X_length, CV_16UC1);
    Mat image_res_YZ = Mat::zeros(Z_length, Y_length, CV_16UC1);
    int overlapCenterY = Y_length/2;
    int overlapCenterX = X_length/2;

    cout<<__LINE__<<endl;
    for(int i = 0; i < imageList.size(); i ++)
    {
        Mat image_i = imread(imageList[i].filePath().toStdString(), 2);
        for(int j = 0; j < X_length; j ++)
        {
            unsigned short maxPro_XZ = 0;
            for(int k = overlapCenterY-10; k < overlapCenterY+10; k ++)
            {
                maxPro_XZ = (maxPro_XZ>image_i.at<unsigned short>(k, j) ? maxPro_XZ : image_i.at<unsigned short>(k, j));
            }
            image_res_XZ.at<unsigned short>(i, j) = maxPro_XZ;
        }

        for(int j = 0; j < Y_length; j ++)
        {
            unsigned short maxPro_YZ = 0;
            for(int k = overlapCenterX-10; k < overlapCenterX+10; k ++)
            {
                maxPro_YZ = (maxPro_YZ>image_i.at<unsigned short>(j,k) ? maxPro_YZ : image_i.at<unsigned short>(j,k));
            }
            image_res_YZ.at<unsigned short>(i, j) = maxPro_YZ;
        }
    }
    cout<<__LINE__<<endl;
    QString saveNameXZ = saveFolder + "\\" + tileInfo.baseName() + "_XZ_pro.tif";
    imwrite(saveNameXZ.toStdString(), image_res_XZ);
    QString saveNameYZ = saveFolder + "\\" + tileInfo.baseName() + "_YZ_pro.tif";
    imwrite(saveNameYZ.toStdString(), image_res_YZ);

    return 1;
}

int XZ_projection_20px_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);
    QStringList filters;
    filters << "*.tif" << "*.tiff";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    vector<string> fileNameList;
    for(int i = 0; i < imageList.size(); i ++)
    {
        fileNameList.push_back(imageList[i].filePath().toStdString());
    }
    cout<<"fileNameList.size = "<<fileNameList.size()<<endl;


    //sort function
    std::sort(fileNameList.begin(), fileNameList.end(), compareNat);

    Mat image_para = imread(fileNameList[0], 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res = Mat::zeros(Z_length, X_length, CV_16UC1);
    int overlapCenter = Y_length/2;

    for(int i = 0; i < fileNameList.size(); i ++)
    {
        QFileInfo fileInfo_tmp(QString ::fromStdString(fileNameList[i]));
        //cout<<"Image name "<<i+1<<": "<<fileInfo_tmp.fileName().toStdString().c_str()<<endl;

        Mat image_i = imread(fileNameList[i], 2);
        for(int j = 0; j < X_length; j ++)
        {
            unsigned short maxPro = 0;
            for(int k = overlapCenter-10; k < overlapCenter+10; k ++)
            {
                maxPro = (maxPro>image_i.at<unsigned short>(k, j) ? maxPro : image_i.at<unsigned short>(k, j));
            }
            image_res.at<unsigned short>(i, j) = maxPro;
        }
    }
    cout<<__LINE__<<endl;
    QString saveName = saveFolder + "\\" + tileInfo.baseName() + "_XZ_pro.tif";
    imwrite(saveName.toStdString(), image_res);

    return 1;
}

int YZ_projection_20px_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);

    // filter
    QStringList filters;
    filters << "*.tif" << "*.tiff";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    vector<string> fileNameList;
    for(int i = 0; i < imageList.size(); i ++)
    {
        fileNameList.push_back(imageList[i].filePath().toStdString());
    }
    cout<<"fileNameList.size = "<<fileNameList.size()<<endl;


    //sort function
    std::sort(fileNameList.begin(), fileNameList.end(), compareNat);

    Mat image_para = imread(fileNameList[0], 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res = Mat::zeros(Z_length, Y_length, CV_16UC1);
    int overlapCenter = X_length/2;

    for(int i = 0; i < fileNameList.size(); i ++)
    {
        QFileInfo fileInfo_tmp(QString ::fromStdString(fileNameList[i]));
//        cout<<"Image name "<<i+1<<": "<<fileInfo_tmp.fileName().toStdString().c_str()<<endl;

        Mat image_i = imread(fileNameList[i], 2);
        for(int j = 0; j < Y_length; j ++)
        {
            unsigned short maxPro = 0;
            for(int k = overlapCenter-10; k < overlapCenter+10; k ++)
            {
                maxPro = (maxPro>image_i.at<unsigned short>(j, k) ? maxPro : image_i.at<unsigned short>(j, k));
            }
            image_res.at<unsigned short>(i, j) = maxPro;
        }
    }
    cout<<__LINE__<<endl;
    QString saveName = saveFolder + "\\" + tileInfo.baseName() + "_YZ_pro.tif";
    imwrite(saveName.toStdString(), image_res);

    return 1;
}

int XZ_YZ_projection_20px_MIP_24Tiles()
{
    QString allTileName = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"allTileName: "<<allTileName.toStdString().c_str()<<endl;
    QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
    cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;
    vector<int> Z_shift_vec;

    QDir allTileDir(allTileName);
    allTileDir.setFilter(QDir::Dirs);
    QFileInfoList tilesListCols = allTileDir.entryInfoList();

    vector<QFileInfoList> tilesListRows;
    for(int i = 0; i < tilesListCols.size()-2; i ++)
    {
        QDir rowsTileDir(tilesListCols[i+2].filePath());
        rowsTileDir.setFilter(QDir::Dirs);
        tilesListRows.push_back(rowsTileDir.entryInfoList());
    }

    for(int i = 0; i < tilesListRows.size(); i ++)
    {
        for(int j = 0; j <tilesListRows[i].size()-2; j ++)
        {
            cout<<"tileList["<<i<<"] Name: "<<tilesListRows[i][j+2].fileName().toStdString().c_str()<<endl;
            QString tile = tilesListRows[i][j+2].filePath();
            QFileInfo tileInfo(tile);
            QDir tileDir(tile);
            QStringList filters;
            filters << "*.tif" << "*.tiff";
            QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

            Mat image_para = imread(imageList[0].filePath().toStdString(), 2);
            int Z_length = imageList.size();
            int X_length = image_para.cols, Y_length = image_para.rows;
            Mat image_res_XZ = Mat::zeros(Z_length, X_length, CV_16UC1);
            Mat image_res_YZ = Mat::zeros(Z_length, Y_length, CV_16UC1);
            int overlapCenterY = Y_length/2;
            int overlapCenterX = X_length/2;

            cout<<__LINE__<<endl;
            for(int i = 0; i < imageList.size(); i ++)
            {
                Mat image_i = imread(imageList[i].filePath().toStdString(), 2);
                for(int j = 0; j < X_length; j ++)
                {
                    unsigned short maxPro_XZ = 0;
                    for(int k = overlapCenterY-10; k < overlapCenterY+10; k ++)
                    {
                        maxPro_XZ = (maxPro_XZ>image_i.at<unsigned short>(k, j) ? maxPro_XZ : image_i.at<unsigned short>(k, j));
                    }
                    image_res_XZ.at<unsigned short>(i, j) = maxPro_XZ;
                }

                for(int j = 0; j < Y_length; j ++)
                {
                    unsigned short maxPro_YZ = 0;
                    for(int k = overlapCenterX-10; k < overlapCenterX+10; k ++)
                    {
                        maxPro_YZ = (maxPro_YZ>image_i.at<unsigned short>(j,k) ? maxPro_YZ : image_i.at<unsigned short>(j,k));
                    }
                    image_res_YZ.at<unsigned short>(i, j) = maxPro_YZ;
                }
            }
            cout<<__LINE__<<endl;
            QString saveNameXZ = savePath + "\\" + tileInfo.baseName() + "_XZ_pro.tif";
            imwrite(saveNameXZ.toStdString(), image_res_XZ);
            QString saveNameYZ = savePath + "\\" + tileInfo.baseName() + "_YZ_pro.tif";
            imwrite(saveNameYZ.toStdString(), image_res_YZ);

        }
    }

    return 1;
}

int XZ_projection_20px_two_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);
    QStringList filters;
    filters << "*.tif" << "*.tiff";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    vector<string> fileNameList;
    for(int i = 0; i < imageList.size(); i ++)
    {
        fileNameList.push_back(imageList[i].filePath().toStdString());
    }
    cout<<"fileNameList.size = "<<fileNameList.size()<<endl;


    //sort function
    std::sort(fileNameList.begin(), fileNameList.end(), compareNat);


    Mat image_para = imread(fileNameList[0], 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res1 = Mat::zeros(Z_length, X_length, CV_16UC1);
    Mat image_res2 = Mat::zeros(Z_length, X_length, CV_16UC1);
    int overlapCenter1 = Y_length/4;
    int overlapCenter2 = Y_length*3/4;

    for(int i = 0; i < fileNameList.size(); i ++)
    {
        QFileInfo fileInfo_tmp(QString ::fromStdString(fileNameList[i]));
//        cout<<"Image name "<<i+1<<": "<<fileInfo_tmp.fileName().toStdString().c_str()<<endl;
        Mat image_i = imread(fileNameList[i], 2);
        for(int j = 0; j < X_length; j ++)
        {
            unsigned short maxPro1 = 0;
            for(int k = overlapCenter1-10; k < overlapCenter1+10; k ++)
            {
                maxPro1 = (maxPro1>image_i.at<unsigned short>(k, j) ? maxPro1 : image_i.at<unsigned short>(k, j));
            }
            image_res1.at<unsigned short>(i, j) = maxPro1;

            unsigned short maxPro2 = 0;
            for(int k = overlapCenter2-10; k < overlapCenter2+10; k ++)
            {
                maxPro2 = (maxPro2>image_i.at<unsigned short>(k, j) ? maxPro2 : image_i.at<unsigned short>(k, j));
            }
            image_res2.at<unsigned short>(i, j) = maxPro2;
        }
    }
    cout<<__LINE__<<endl;
    QString saveName1 = saveFolder + "\\" + tileInfo.baseName() + "_XZ_1_4Y_pro.tif";
    imwrite(saveName1.toStdString(), image_res1);

    QString saveName2 = saveFolder + "\\" + tileInfo.baseName() + "_XZ_3_4Y_pro.tif";
    imwrite(saveName2.toStdString(), image_res2);

    return 1;
}

int YZ_projection_20px_two_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);
    QStringList filters;
    filters << "*.tif" << "*.tiff";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    vector<string> fileNameList;
    for(int i = 0; i < imageList.size(); i ++)
    {
        fileNameList.push_back(imageList[i].filePath().toStdString());
    }
    cout<<"fileNameList.size = "<<fileNameList.size()<<endl;


    //sort function
    std::sort(fileNameList.begin(), fileNameList.end(), compareNat);


    Mat image_para = imread(fileNameList[0], 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res1 = Mat::zeros(Z_length, Y_length, CV_16UC1);
    int overlapCenter1 = X_length*3/8;
    Mat image_res2 = Mat::zeros(Z_length, Y_length, CV_16UC1);
    int overlapCenter2 = X_length*5/8;

    for(int i = 0; i < fileNameList.size(); i ++)
    {
        QFileInfo fileInfo_tmp(QString ::fromStdString(fileNameList[i]));
//        cout<<"Image name "<<i+1<<": "<<fileInfo_tmp.fileName().toStdString().c_str()<<endl;

        Mat image_i = imread(fileNameList[i], 2);
        for(int j = 0; j < Y_length; j ++)
        {
            unsigned short maxPro1 = 0;
            for(int k = overlapCenter1-10; k < overlapCenter1+10; k ++)
            {
                maxPro1 = (maxPro1>image_i.at<unsigned short>(j, k) ? maxPro1 : image_i.at<unsigned short>(j, k));
            }
            image_res1.at<unsigned short>(i, j) = maxPro1;

            unsigned short maxPro2 = 0;
            for(int k = overlapCenter2-10; k < overlapCenter2+10; k ++)
            {
                maxPro2 = (maxPro2>image_i.at<unsigned short>(j, k) ? maxPro2 : image_i.at<unsigned short>(j, k));
            }
            image_res2.at<unsigned short>(i, j) = maxPro2;
        }
    }
    cout<<__LINE__<<endl;
    QString saveName1 = saveFolder + "\\" + tileInfo.baseName() + "_YZ_3_8X_pro.tif";
    imwrite(saveName1.toStdString(), image_res1);
    QString saveName2 = saveFolder + "\\" + tileInfo.baseName() + "_YZ_5_8X_pro.tif";
    imwrite(saveName2.toStdString(), image_res2);

    return 1;
}

bool polynomial_curve_fit(vector<unsigned short> fitValue_vec, int n, Mat & A)
{
    int N = fitValue_vec.size();
    vector<Point> points;
    for(int i = 0; i < N; i ++)
    {
        points.push_back(Point((double)(i+1), (double)fitValue_vec[i]));
    }

    Mat X = Mat::zeros(n+1, n+1, CV_64F);
    for(int i = 0; i < n+1; i ++)
    {
        for(int j = 0; j < n+1; j ++)
        {
            for(int k = 0; k < N; k ++)
            {
                X.at<double>(i, j) = X.at<double>(i, j) + pow(points[k].x, i+j);
            }
        }
    }

    Mat Y = Mat::zeros(n+1, 1, CV_64F);
    for(int i = 0; i < n+1; i ++)
    {
         for(int k = 0; k < N; k ++)
         {
             Y.at<double>(i, 0) = Y.at<double>(i, 0) + pow(points[k].x, i)*points[k].y;
         }
    }

    A = Mat::zeros(n+1, 1, CV_64F);
    solve(X, Y, A, DECOMP_LU);

    return true;
}


int image_distorted_correction()
{
    QString imagePath1 = QFileDialog::getOpenFileName(nullptr, "Select Image1", "D:\\");
    cout<<"ImagePath1: "<<imagePath1.toStdString().c_str()<<endl;
    QString imagePath2 = QFileDialog::getOpenFileName(nullptr, "Select Image2", imagePath1);
    cout<<"ImagePath2: "<<imagePath2.toStdString().c_str()<<endl;
    QString saveImageDir = QFileDialog::getExistingDirectory(nullptr, "Save Images", imagePath1);
    cout<<"saveImageDir: "<<saveImageDir.toStdString().c_str()<<endl;

    QFileInfo imageInfo1(imagePath1), imageInfo2(imagePath2);
    Mat src1 = imread(imageInfo1.filePath().toStdString(), -1);
    Mat src2 = imread(imageInfo2.filePath().toStdString(), -1);

    int a1[9] = {1, 0, src1.cols/2, 0, 1, src1.rows/2, 0, 0, 1};
    Mat aa(3, 3, CV_64FC1, a1);
    Mat bb = (Mat_<double>(5, 1)<<0.0000000048, 0, 0, -3.13686838e-20, -5.68434189e-24);

    Mat dst1;
    Mat warp1;
    Point center = Point(src1.cols/2, src1.rows/2);
    undistort(src1, warp1, aa, bb, aa);
    Mat affineTrans1 = getRotationMatrix2D(center, -0.106271, 1/0.998173);
    warpAffine(warp1, dst1, affineTrans1, src1.size(), INTER_NEAREST);
    QString saveName1 = saveImageDir + "\\" + imageInfo1.baseName() + "_after.tif";
    imwrite(saveName1.toStdString(), dst1);

    Mat dst2;
    Mat warp2;
    undistort(src2, warp2, aa, bb, aa);
    Mat affineTrans2 = getRotationMatrix2D(center, -0.106271, 1/0.998173);
    warpAffine(warp2, dst2, affineTrans2, src2.size(), INTER_NEAREST);
    QString saveName2 = saveImageDir + "\\" + imageInfo2.baseName() + "_after.tif";
    imwrite(saveName2.toStdString(), dst2);

    return 1;
}

int cut_one_image()
{
    cout<<"Start!"<<endl;
    QString imagePath = QFileDialog::getOpenFileName(nullptr, "Select Image", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveImageDir = QFileDialog::getExistingDirectory(nullptr, "Save Images", imagePath);
    cout<<"saveImageDir: "<<saveImageDir.toStdString().c_str()<<endl;

    QFileInfo imageInfo(imagePath);
    Mat src = imread(imageInfo.filePath().toStdString(), -1);
    Mat result = Mat::zeros(8795, 7397, CV_16UC1);
    for(int i = 0; i < 8795; i ++)
    {
        for(int j = 0; j < 7397; j ++)
        {
            result.at<unsigned short>(i, j) = src.at<unsigned short>(i, j);
        }
    }
    QString saveName = saveImageDir + "\\" + "TeraStitched_cut.tif";
    imwrite(saveName.toStdString(), result);

    return 1;
}

void background_correction_one_image()
{

    QString imagePathOri = QFileDialog::getOpenFileName(nullptr, "Select Image1", "D:\\", QObject::tr("Images ( *.tif *.tiff);;All(*)"));
    cout<<"ImagePath: "<<imagePathOri.toStdString().c_str()<<endl;
    QFileInfo imageOriInfo(imagePathOri);
    QString saveImageName = imageOriInfo.path()+"\\"+imageOriInfo.baseName()+"_background_correction.tif";
    cout<<"saveImageName: "<<saveImageName.toStdString().c_str()<<endl;
    Mat imageOriMat = imread(imagePathOri.toStdString(), -1);

    int lengthX = imageOriMat.cols, lengthY = imageOriMat.rows;
    int region_length = 10;
    Mat imageResultMat = Mat::zeros(lengthY, lengthX, CV_16UC1);
    vector<unsigned short> back_ave_vec, region_threshold_rows, region_threshold_cols;
    cout<<"lengthX = "<<lengthX<<"; lengthY = "<<lengthY<<endl;

    for(int j = 0; j < lengthX; j += region_length)
    {
        unsigned int region_sum_BS=0, region_ave_BS=0, region_diff_sum=0, thres=0;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthY; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(n, j+m);
                region_sum_BS += temp;
            }
        }
        region_ave_BS = region_sum_BS/(region_length*lengthY);

        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthY; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(n, j+m);
                region_diff_sum += (temp-region_ave_BS)*(temp-region_ave_BS);
            }
        }
        thres = region_ave_BS + 0.5*sqrt(region_diff_sum);
        region_threshold_cols.push_back(thres);
    }

    for(int j = 0; j < lengthY; j += region_length)
    {
        unsigned int region_sum_BS=0, region_ave_BS=0, region_diff_sum=0, thres=0;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthX; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(j+m, n);
                region_sum_BS += temp;
            }
        }
        region_ave_BS = region_sum_BS/(region_length*lengthX);

        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthX; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(j+m, n);
                region_diff_sum += (temp-region_ave_BS)*(temp-region_ave_BS);
            }
        }
        thres = region_ave_BS + 0.5*sqrt(region_diff_sum);
        region_threshold_rows.push_back(thres);
    }

    cout<<__LINE__<<endl;
    short int max_region_ave = 0;
    vector<unsigned short> back_ave_rows, back_ave_cols;
    for(int j = 0; j < lengthX; j += region_length)
    {
        int count = 0;
        unsigned int region_sum_BS=0;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthY; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(n, j+m);
                if(temp<region_threshold_cols[j/region_length])
                {
                    count ++;
                    region_sum_BS += temp;
                }
            }
        }
        unsigned short back_ave = 0;
        back_ave = region_sum_BS/count;
        max_region_ave = (max_region_ave<back_ave)?back_ave:max_region_ave;
        back_ave_cols.push_back(back_ave);
    }

    for(int j = 0; j < lengthY; j += region_length)
    {
        int count = 0;
        unsigned int region_sum_BS=0;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < lengthX; n ++)
            {
                unsigned short temp = imageOriMat.at<unsigned short>(j+m, n);
                if(temp<region_threshold_rows[j/region_length])
                {
                    count ++;
                    region_sum_BS += temp;
                }
            }
        }
        unsigned short back_ave = 0;
        back_ave = region_sum_BS/count;
        max_region_ave = (max_region_ave<back_ave)?back_ave:max_region_ave;
        back_ave_rows.push_back(back_ave);
    }
    cout<<__LINE__<<endl;

    Mat poly_A_cols, poly_A_rows;
    polynomial_curve_fit(back_ave_cols, 1, poly_A_cols);
    polynomial_curve_fit(back_ave_rows, 1, poly_A_rows);

    for(int i = 0; i < lengthX; i += region_length)
    {
        double ratio_cols = 0;
        double y_fit = poly_A_cols.at<double>(0, 0) + poly_A_cols.at<double>(1, 0)*(i+1);
        ratio_cols = (double)max_region_ave/y_fit;

        for(int j = 0; j < lengthY; j ++)
        {
            imageResultMat.at<unsigned short>(j, i) = (unsigned short)(((double)(imageOriMat.at<unsigned short>(j, i)))*ratio_cols);
        }
    }

    for(int i = 0; i < lengthY; i += region_length)
    {
        double ratio_rows = 0;
        double x_fit = poly_A_rows.at<double>(0, 0) + poly_A_rows.at<double>(1, 0)*(i+1);
        ratio_rows = (double)max_region_ave/x_fit;

        for(int j = 0; j < lengthX; j ++)
        {
            imageResultMat.at<unsigned short>(i, j) = (unsigned short)(((double)(imageOriMat.at<unsigned short>(i, j)))*ratio_rows);
        }
    }

    imwrite(saveImageName.toStdString(), imageResultMat);
}


int tile_region_background_ave_rows()
{
    int length = 1880, region_length = 1, maxBack1 = 1400, minBack1 = 600, maxBack2, minBack2;
    Mat image = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images_cut/ZW2-UC3R1(17)_z0517_Cam1-T3_ORG_after_cut.tif", -1);
    QString saveExcelName = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\excel_file\\background_ave_1col_17.xlsx";
    //Mat img_2 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images_cut/ZW2-UC3R1(13)_z0519_Cam1-T3_ORG_after_cut.tif", -1);

    vector<unsigned short> back_ave_vec1, back_ave_vec2, back_ave_vec3;
    vector<unsigned short> region_most_vec, region_threshold;
    vector<unordered_map<unsigned short, int>> most_int_map_cev;

    cout<<__LINE__<<endl;
    for(int j = 0; j < length; j += region_length)
    {
        unsigned int region_sum_BS=0, region_ave_BS=0, region_diff_sum=0, thres=0;
        unordered_map<unsigned short, int> most_int_map;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < length; n ++)
            {
                unsigned short temp = image.at<unsigned short>(n, j+m);
                most_int_map[temp]++;

                region_sum_BS += temp;
            }
        }
        region_ave_BS = region_sum_BS/(region_length*length);

        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < length; n ++)
            {
                unsigned short temp = image.at<unsigned short>(n, j+m);
                region_diff_sum += (temp-region_ave_BS)*(temp-region_ave_BS);
            }
        }
        thres = region_ave_BS + 0.5*sqrt(region_diff_sum);
        region_threshold.push_back(thres);


        pair<unsigned short, int> most_int = make_pair(most_int_map.begin()->first, most_int_map.begin()->second);
        unordered_map<unsigned short, int>::iterator it = most_int_map.begin();
        while (it != most_int_map.end())
        {
            if(most_int.second<it->second)
            {
                most_int.first = it->first;
                most_int.second = it->second;
            }
            ++it;
        }

        unsigned short region_back = most_int.first;
        region_most_vec.push_back(region_back);
    }

    cout<<__LINE__<<endl;
    maxBack2 = region_most_vec[0]; minBack2 = region_most_vec[0];
    for(int i = 1; i < region_most_vec.size(); i ++)
    {
        if(region_most_vec[i]>maxBack2)
            maxBack2 = region_most_vec[i];
        if(region_most_vec[i]<minBack2)
            minBack2 = region_most_vec[i];
    }

    cout<<__LINE__<<endl;
    for(int j = 0; j < length/region_length; j ++)
    {
        unsigned int region_sum1=0, region_ave1=0;
        unsigned int region_sum2=0, region_ave2=0;
        unsigned int region_sum3=0, region_ave3=0;
        int count1=0, count2=0, count3=0;
        for(int m = 0; m < region_length; m ++)
        {
            for(int n = 0; n < length; n ++)
            {
                unsigned short temp = image.at<unsigned short>(n, j*region_length+m);
                if(temp>minBack1 && temp<maxBack1)
                {
                    region_sum1 += temp;
                    count1 ++;
                }
                if(temp>minBack2 && temp<maxBack2)
                {
                    count2 ++;
                    region_sum2 += temp;
                }
                if(temp<region_threshold[j])
                {
                    count3 ++;
                    region_sum3 += temp;
                }
            }
        }
        cout<<__LINE__<<": count1 = "<<count1<<endl;
        cout<<__LINE__<<": count2 = "<<count2<<endl;
        cout<<__LINE__<<": count3 = "<<count3<<endl;

        region_ave1 = region_sum1/count1;
        back_ave_vec1.push_back(region_ave1);
        region_ave2 = region_sum2/count2;
        back_ave_vec2.push_back(region_ave2);
        region_ave3 = region_sum3/count3;
        back_ave_vec3.push_back(region_ave3);
    }

    cout<<__LINE__<<": "<<saveExcelName.toStdString()<<endl;
    QAxObject *excel = new QAxObject();
    excel -> setControl("Excel.Application"); // connect EXCEL control
    excel -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            saveExcelName, 51, QString(""), QString(""), false, false);

    //51xlsx, 56xls
    QAxObject *worksheets = workbook -> querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "Background");

    QList <QList<QVariant>> datas_sheet1;
    for(int i = 0; i < length/region_length+1; i ++)
    {
        QList<QVariant> rows;
        if(i == 0)
        {
            rows.append("Num");
            rows.append("600-1400");
            rows.append("min-max");
            rows.append("threshold");
        }

        else
        {
            rows.append(i);
            rows.append(back_ave_vec1[i-1]);
            rows.append(back_ave_vec2[i-1]);
            rows.append(back_ave_vec3[i-1]);
        }
        datas_sheet1.append(rows);
    }
    cout<<__LINE__<<endl;

    QList<QVariant> vars1;
    for(auto v:datas_sheet1)
        vars1.append(QVariant(v));
    QVariant var1 = QVariant(vars1);

    QString range = "A1:D1889";
    cout<<__LINE__<<": excel range: "<<range.toStdString()<<endl;

    QAxObject *excel_property1 = worksheet1 -> querySubObject("Range(const QString&)", range);
    excel_property1 -> setProperty("Value", var1);
    excel_property1 -> setProperty("HorizontalAlignment", -4108);

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;

    return 1;
}

int correction_tiles_via_background()
{
    int length = 1920, region_length = 1, maxBack = 1400, minBack = 600;
    int maxBackVary_rows1, maxBackVary_rows2, minBackVary_rows1, minBackVary_rows2, maxBackVary_cols1, maxBackVary_cols2, minBackVary_cols1, minBackVary_cols2;
    unsigned short max_back_ave_rows_1 = 0, max_back_ave_cols_1 = 0;
    unsigned short max_back_ave_rows_2 = 0, max_back_ave_cols_2 = 0;
    unsigned short max_back_ave_rows_3 = 0, max_back_ave_cols_3 = 0;
    Mat img_1 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images/14.tif", -1);
    Mat img_2 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images/17.tif", -1);

    vector<unsigned short> back_ave_vec_rows11, back_ave_vec_rows12, back_ave_vec_rows13;
    vector<unsigned short> region_most_vec_rows1, region_threshold_rows1;

    vector<unsigned short> back_ave_vec_rows21, back_ave_vec_rows22, back_ave_vec_rows23;
    vector<unsigned short> region_most_vec_rows2, region_threshold_rows2;

    vector<unsigned short> back_ave_vec_cols11, back_ave_vec_cols12, back_ave_vec_cols13;
    vector<unsigned short> region_most_vec_cols1, region_threshold_cols1;

    vector<unsigned short> back_ave_vec_cols21, back_ave_vec_cols22, back_ave_vec_cols23;
    vector<unsigned short> region_most_vec_cols2, region_threshold_cols2;

    cout<<__LINE__<<endl;
    for(int m = 0; m < length; m ++)
    {
        unsigned int region_sum_BS_rows1=0, region_ave_BS_rows1=0, region_diff_sum_rows1=0, thres_rows1=0;
        unsigned int region_sum_BS_rows2=0, region_ave_BS_rows2=0, region_diff_sum_rows2=0, thres_rows2=0;
        unordered_map<unsigned short, int> most_int_map_rows1, most_int_map_rows2;

        unsigned int region_sum_BS_cols1=0, region_ave_BS_cols1=0, region_diff_sum_cols1=0, thres_cols1=0;
        unsigned int region_sum_BS_cols2=0, region_ave_BS_cols2=0, region_diff_sum_cols2=0, thres_cols2=0;
        unordered_map<unsigned short, int> most_int_map_cols1, most_int_map_cols2;

        for(int n = 0; n < length; n ++)
        {
            unsigned short temp_rows1 = img_1.at<unsigned short>(m, n);
            unsigned short temp_rows2 = img_2.at<unsigned short>(m, n);
            unsigned short temp_cols1 = img_1.at<unsigned short>(n, m);
            unsigned short temp_cols2 = img_2.at<unsigned short>(n, m);

            most_int_map_rows1[temp_rows1]++;
            region_sum_BS_rows1 += temp_rows1;
            most_int_map_rows2[temp_rows2]++;
            region_sum_BS_rows2 += temp_rows2;

            most_int_map_cols1[temp_cols1]++;
            region_sum_BS_cols1 += temp_cols1;
            most_int_map_cols2[temp_cols2]++;
            region_sum_BS_cols2 += temp_cols2;
        }
        region_ave_BS_rows1 = region_sum_BS_rows1/(region_length*length);
        region_ave_BS_rows2 = region_sum_BS_rows2/(region_length*length);
        region_ave_BS_cols1 = region_sum_BS_cols1/(region_length*length);
        region_ave_BS_cols2 = region_sum_BS_cols2/(region_length*length);


        for(int n = 0; n < length; n ++)
        {
            unsigned short temp_rows1 = img_1.at<unsigned short>(m, n);
            unsigned short temp_rows2 = img_2.at<unsigned short>(m, n);
            unsigned short temp_cols1 = img_1.at<unsigned short>(n, m);
            unsigned short temp_cols2 = img_2.at<unsigned short>(n, m);

            region_diff_sum_rows1 += (temp_rows1-region_ave_BS_rows1)*(temp_rows1-region_ave_BS_rows1);
            region_diff_sum_rows2 += (temp_rows2-region_ave_BS_rows2)*(temp_rows2-region_ave_BS_rows2);
            region_diff_sum_cols1 += (temp_cols1-region_ave_BS_cols1)*(temp_cols1-region_ave_BS_cols1);
            region_diff_sum_cols2 += (temp_cols2-region_ave_BS_cols2)*(temp_cols2-region_ave_BS_cols2);
        }

        thres_rows1 = region_ave_BS_rows1 + 0.5*sqrt(region_diff_sum_rows1);
        region_threshold_rows1.push_back(thres_rows1);
        thres_rows2 = region_ave_BS_rows2 + 0.5*sqrt(region_diff_sum_rows2);
        region_threshold_rows2.push_back(thres_rows2);

        thres_cols1 = region_ave_BS_cols1 + 0.5*sqrt(region_diff_sum_cols1);
        region_threshold_cols1.push_back(thres_cols1);
        thres_cols2 = region_ave_BS_cols2 + 0.5*sqrt(region_diff_sum_cols2);
        region_threshold_cols2.push_back(thres_cols2);


        pair<unsigned short, int> most_int_rows1 = make_pair(most_int_map_rows1.begin()->first, most_int_map_rows1.begin()->second);
        unordered_map<unsigned short, int>::iterator it_rows1 = most_int_map_rows1.begin();

        pair<unsigned short, int> most_int_cols1 = make_pair(most_int_map_cols1.begin()->first, most_int_map_cols1.begin()->second);
        unordered_map<unsigned short, int>::iterator it_cols1 = most_int_map_cols1.begin();

        while (it_rows1 != most_int_map_rows1.end())
        {
            if(most_int_rows1.second<it_rows1->second)
            {
                most_int_rows1.first = it_rows1->first;
                most_int_rows1.second = it_rows1->second;
            }
            ++it_rows1;
        }
        unsigned short region_back_rows1 = most_int_rows1.first;
        region_most_vec_rows1.push_back(region_back_rows1);

        while (it_cols1 != most_int_map_cols1.end())
        {
            if(most_int_cols1.second<it_cols1->second)
            {
                most_int_cols1.first = it_cols1->first;
                most_int_cols1.second = it_cols1->second;
            }
            ++it_cols1;
        }
        unsigned short region_back_cols1 = most_int_cols1.first;
        region_most_vec_cols1.push_back(region_back_cols1);

        pair<unsigned short, int> most_int_rows2 = make_pair(most_int_map_rows2.begin()->first, most_int_map_rows2.begin()->second);
        unordered_map<unsigned short, int>::iterator it_rows2 = most_int_map_rows2.begin();
        while (it_rows2 != most_int_map_rows2.end())
        {
            if(most_int_rows2.second<it_rows2->second)
            {
                most_int_rows2.first = it_rows2->first;
                most_int_rows2.second = it_rows2->second;
            }
            ++it_rows2;
        }
        unsigned short region_back_rows2 = most_int_rows2.first;
        region_most_vec_rows2.push_back(region_back_rows2);

        pair<unsigned short, int> most_int_cols2 = make_pair(most_int_map_cols2.begin()->first, most_int_map_cols2.begin()->second);
        unordered_map<unsigned short, int>::iterator it_cols2 = most_int_map_cols2.begin();
        while (it_cols2 != most_int_map_cols2.end())
        {
            if(most_int_cols2.second<it_cols2->second)
            {
                most_int_cols2.first = it_cols2->first;
                most_int_cols2.second = it_cols2->second;
            }
            ++it_cols2;
        }
        unsigned short region_back_cols2 = most_int_cols2.first;
        region_most_vec_cols2.push_back(region_back_cols2);
    }

    cout<<__LINE__<<endl;
    maxBackVary_rows1 = region_most_vec_rows1[0]; minBackVary_rows1 = region_most_vec_rows1[0];
    for(int i = 1; i < region_most_vec_rows1.size(); i ++)
    {
        if(region_most_vec_rows1[i]>maxBackVary_rows1)
            maxBackVary_rows1 = region_most_vec_rows1[i];
        if(region_most_vec_rows1[i]<minBackVary_rows1)
            minBackVary_rows1 = region_most_vec_rows1[i];
    }
    maxBackVary_rows2 = region_most_vec_rows2[0]; minBackVary_rows2 = region_most_vec_rows2[0];
    for(int i = 1; i < region_most_vec_rows2.size(); i ++)
    {
        if(region_most_vec_rows2[i]>maxBackVary_rows2)
            maxBackVary_rows2 = region_most_vec_rows2[i];
        if(region_most_vec_rows2[i]<minBackVary_rows2)
            minBackVary_rows2 = region_most_vec_rows2[i];
    }

    maxBackVary_cols1 = region_most_vec_cols1[0]; minBackVary_cols1 = region_most_vec_cols1[0];
    for(int i = 1; i < region_most_vec_cols1.size(); i ++)
    {
        if(region_most_vec_cols1[i]>maxBackVary_cols1)
            maxBackVary_cols1 = region_most_vec_cols1[i];
        if(region_most_vec_cols1[i]<minBackVary_cols1)
            minBackVary_cols1 = region_most_vec_cols1[i];
    }
    maxBackVary_cols2 = region_most_vec_cols2[0]; minBackVary_cols2 = region_most_vec_cols2[0];
    for(int i = 1; i < region_most_vec_cols2.size(); i ++)
    {
        if(region_most_vec_cols2[i]>maxBackVary_cols2)
            maxBackVary_cols2 = region_most_vec_cols2[i];
        if(region_most_vec_cols2[i]<minBackVary_cols2)
            minBackVary_cols2 = region_most_vec_cols2[i];
    }

    cout<<__LINE__<<endl;
    for(int m = 0; m < length; m ++)
    {
        unsigned int region_sum_rows11=0, region_ave_rows11=0;
        unsigned int region_sum_rows12=0, region_ave_rows12=0;
        unsigned int region_sum_rows13=0, region_ave_rows13=0;
        unsigned int region_sum_rows21=0, region_ave_rows21=0;
        unsigned int region_sum_rows22=0, region_ave_rows22=0;
        unsigned int region_sum_rows23=0, region_ave_rows23=0;
        int count_rows11=0, count_rows12=0, count_rows13=0;
        int count_rows21=0, count_rows22=0, count_rows23=0;

        unsigned int region_sum_cols11=0, region_ave_cols11=0;
        unsigned int region_sum_cols12=0, region_ave_cols12=0;
        unsigned int region_sum_cols13=0, region_ave_cols13=0;
        unsigned int region_sum_cols21=0, region_ave_cols21=0;
        unsigned int region_sum_cols22=0, region_ave_cols22=0;
        unsigned int region_sum_cols23=0, region_ave_cols23=0;
        int count_cols11=0, count_cols12=0, count_cols13=0;
        int count_cols21=0, count_cols22=0, count_cols23=0;

        for(int n = 0; n < length; n ++)
        {
            unsigned short temp_rows1 = img_1.at<unsigned short>(m, n);
            unsigned short temp_rows2 = img_2.at<unsigned short>(m, n);
            unsigned short temp_cols1 = img_1.at<unsigned short>(n, m);
            unsigned short temp_cols2 = img_2.at<unsigned short>(n, m);

            if(temp_rows1>minBack && temp_rows1<maxBack)
            {
                count_rows11 ++;
                region_sum_rows11 += temp_rows1;
            }
            if(temp_rows1>minBackVary_rows1 && temp_rows1<maxBackVary_rows1)
            {
                count_rows12 ++;
                region_sum_rows12 += temp_rows1;
            }
            if(temp_rows1<region_threshold_rows1[m])
            {
                count_rows13 ++;
                region_sum_rows13 += temp_rows1;
            }

            if(temp_rows2>minBack && temp_rows2<maxBack)
            {
                count_rows21 ++;
                region_sum_rows21 += temp_rows2;
            }
            if(temp_rows2>minBackVary_rows2 && temp_rows2<maxBackVary_rows2)
            {
                count_rows22 ++;
                region_sum_rows22 += temp_rows2;
            }
            if(temp_rows2<region_threshold_rows2[m])
            {
                count_rows23 ++;
                region_sum_rows23 += temp_rows2;
            }

            if(temp_cols1>minBack && temp_cols1<maxBack)
            {
                count_cols11 ++;
                region_sum_cols11 += temp_cols1;
            }
            if(temp_cols1>minBackVary_cols1 && temp_cols1<maxBackVary_cols2)
            {
                count_cols12 ++;
                region_sum_cols12 += temp_cols1;
            }
            if(temp_cols1<region_threshold_cols1[m])
            {
                count_cols13 ++;
                region_sum_cols13 += temp_cols1;
            }

            if(temp_cols2>minBack && temp_cols2<maxBack)
            {
                count_cols21 ++;
                region_sum_cols21 += temp_cols2;
            }
            if(temp_cols2>minBackVary_cols2&& temp_cols2<maxBackVary_cols2)
            {
                count_cols22 ++;
                region_sum_cols22 += temp_cols2;
            }
            if(temp_cols2<region_threshold_cols2[m])
            {
                count_cols23 ++;
                region_sum_cols23 += temp_cols2;
            }
        }

        cout<<__LINE__<<": count_rows11 = "<<count_rows11<<endl;
        cout<<__LINE__<<": count_rows12 = "<<count_rows12<<endl;
        cout<<__LINE__<<": count_rows13 = "<<count_rows13<<endl;
        cout<<__LINE__<<": count_cols11 = "<<count_cols11<<endl;
        cout<<__LINE__<<": count_cols12 = "<<count_cols12<<endl;
        cout<<__LINE__<<": count_cols13 = "<<count_cols13<<endl;

        region_ave_rows11 = region_sum_rows11/count_rows11;
        back_ave_vec_rows11.push_back(region_ave_rows11);
        max_back_ave_rows_1 = ((max_back_ave_rows_1<region_ave_rows11) ? region_ave_rows11 : max_back_ave_rows_1);
        region_ave_rows12 = region_sum_rows12/count_rows12;
        back_ave_vec_rows12.push_back(region_ave_rows12);
        max_back_ave_rows_2 = ((max_back_ave_rows_2<region_ave_rows12) ? region_ave_rows12 : max_back_ave_rows_2);
        region_ave_rows13 = region_sum_rows13/count_rows13;
        back_ave_vec_rows13.push_back(region_ave_rows13);
        max_back_ave_rows_3 = ((max_back_ave_rows_3<region_ave_rows13) ? region_ave_rows13 : max_back_ave_rows_3);

        region_ave_rows21 = region_sum_rows21/count_rows21;
        back_ave_vec_rows21.push_back(region_ave_rows21);
        max_back_ave_rows_1 = ((max_back_ave_rows_1<region_ave_rows21) ? region_ave_rows21 : max_back_ave_rows_1);
        region_ave_rows22 = region_sum_rows22/count_rows22;
        back_ave_vec_rows22.push_back(region_ave_rows22);
        max_back_ave_rows_2 = ((max_back_ave_rows_2<region_ave_rows22) ? region_ave_rows22 : max_back_ave_rows_2);
        region_ave_rows23 = region_sum_rows23/count_rows23;
        back_ave_vec_rows23.push_back(region_ave_rows23);
        max_back_ave_rows_3 = ((max_back_ave_rows_3<region_ave_rows23) ? region_ave_rows23 : max_back_ave_rows_3);

        region_ave_cols11 = region_sum_cols11/count_cols11;
        back_ave_vec_cols11.push_back(region_ave_cols11);
        max_back_ave_cols_1 = ((max_back_ave_cols_1<region_ave_cols11) ? region_ave_cols11 : max_back_ave_cols_1);
        region_ave_cols12 = region_sum_cols12/count_cols12;
        back_ave_vec_cols12.push_back(region_ave_cols12);
        max_back_ave_cols_2 = ((max_back_ave_cols_2<region_ave_cols12) ? region_ave_cols12 : max_back_ave_cols_2);
        region_ave_cols13 = region_sum_cols13/count_cols13;
        back_ave_vec_cols13.push_back(region_ave_cols13);
        max_back_ave_cols_3 = ((max_back_ave_cols_3<region_ave_cols13) ? region_ave_cols13 : max_back_ave_cols_3);

        region_ave_cols21 = region_sum_cols21/count_cols21;
        back_ave_vec_cols21.push_back(region_ave_cols21);
        max_back_ave_cols_1 = ((max_back_ave_cols_1<region_ave_cols21) ? region_ave_cols21 : max_back_ave_cols_1);
        region_ave_cols22 = region_sum_cols22/count_cols22;
        back_ave_vec_cols22.push_back(region_ave_cols22);
        max_back_ave_cols_2 = ((max_back_ave_cols_2<region_ave_cols22) ? region_ave_cols22 : max_back_ave_cols_2);
        region_ave_cols23 = region_sum_cols23/count_cols23;
        back_ave_vec_cols23.push_back(region_ave_cols23);
        max_back_ave_cols_3 = ((max_back_ave_cols_3<region_ave_cols23) ? region_ave_cols23 : max_back_ave_cols_3);
    }

    Mat poly_A_rows11, poly_A_rows12, poly_A_rows13, poly_A_rows21, poly_A_rows22, poly_A_rows23;
    Mat poly_A_cols11, poly_A_cols12, poly_A_cols13, poly_A_cols21, poly_A_cols22, poly_A_cols23;
    polynomial_curve_fit(back_ave_vec_rows11, 1, poly_A_rows11);
    polynomial_curve_fit(back_ave_vec_rows12, 1, poly_A_rows12);
    polynomial_curve_fit(back_ave_vec_rows13, 1, poly_A_rows13);
    polynomial_curve_fit(back_ave_vec_rows21, 1, poly_A_rows21);
    polynomial_curve_fit(back_ave_vec_rows22, 1, poly_A_rows22);
    polynomial_curve_fit(back_ave_vec_rows23, 1, poly_A_rows23);
    polynomial_curve_fit(back_ave_vec_cols11, 1, poly_A_cols11);
    polynomial_curve_fit(back_ave_vec_cols12, 1, poly_A_cols12);
    polynomial_curve_fit(back_ave_vec_cols13, 1, poly_A_cols13);
    polynomial_curve_fit(back_ave_vec_cols21, 1, poly_A_cols21);
    polynomial_curve_fit(back_ave_vec_cols22, 1, poly_A_cols22);
    polynomial_curve_fit(back_ave_vec_cols23, 1, poly_A_cols23);

    Mat img_res_600_rows1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_600_rows2 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_600_cols1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_600_cols2 = Mat::zeros(1880, 1880, CV_16UC1);

    Mat img_res_min_rows1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_min_rows2 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_min_cols1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_min_cols2 = Mat::zeros(1880, 1880, CV_16UC1);

    Mat img_res_thres_rows1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_thres_rows2 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_thres_cols1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_thres_cols2 = Mat::zeros(1880, 1880, CV_16UC1);

    for(int i = 0; i < length; i ++)
    {
        double R_temp11, R_temp12, R_temp13;
        double y_fit11 = poly_A_rows11.at<double>(0, 0) + poly_A_rows11.at<double>(1, 0)*(i+1);
        double y_fit12 = poly_A_rows12.at<double>(0, 0) + poly_A_rows12.at<double>(1, 0)*(i+1);
        double y_fit13 = poly_A_rows13.at<double>(0, 0) + poly_A_rows13.at<double>(1, 0)*(i+1);
        R_temp11 = (double)max_back_ave_rows_1/y_fit11;
        R_temp12 = (double)max_back_ave_rows_2/y_fit12;
        R_temp13 = (double)max_back_ave_rows_3/y_fit13;

        double R_temp21, R_temp22, R_temp23;
        double y_fit21 = poly_A_rows21.at<double>(0, 0) + poly_A_rows21.at<double>(1, 0)*(i+1);
        double y_fit22 = poly_A_rows22.at<double>(0, 0) + poly_A_rows22.at<double>(1, 0)*(i+1);
        double y_fit23 = poly_A_rows23.at<double>(0, 0) + poly_A_rows23.at<double>(1, 0)*(i+1);
        R_temp21 = (double)max_back_ave_rows_1/y_fit21;
        R_temp22 = (double)max_back_ave_rows_2/y_fit22;
        R_temp23 = (double)max_back_ave_rows_3/y_fit23;

        for(int j = 0; j < length; j ++)
        {
            img_res_600_rows1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_1.at<unsigned short>(i, j)))*R_temp11);
            img_res_min_rows1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_1.at<unsigned short>(i, j)))*R_temp12);
            img_res_thres_rows1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_1.at<unsigned short>(i, j)))*R_temp13);

            img_res_600_rows2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_2.at<unsigned short>(i, j)))*R_temp21);
            img_res_min_rows2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_2.at<unsigned short>(i, j)))*R_temp22);
            img_res_thres_rows2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_2.at<unsigned short>(i, j)))*R_temp23);
        }
    }

    for(int j = 0; j < length; j ++)
    {
        double R_temp11, R_temp12, R_temp13;
        double y_fit11 = poly_A_cols11.at<double>(0, 0) + poly_A_cols11.at<double>(1, 0)*(j+1);
        double y_fit12 = poly_A_cols12.at<double>(0, 0) + poly_A_cols12.at<double>(1, 0)*(j+1);
        double y_fit13 = poly_A_cols13.at<double>(0, 0) + poly_A_cols13.at<double>(1, 0)*(j+1);
        R_temp11 = (double)max_back_ave_cols_1/y_fit11;
        R_temp12 = (double)max_back_ave_cols_2/y_fit12;
        R_temp13 = (double)max_back_ave_cols_3/y_fit13;

        double R_temp21, R_temp22, R_temp23;
        double y_fit21 = poly_A_cols21.at<double>(0, 0) + poly_A_cols21.at<double>(1, 0)*(j+1);
        double y_fit22 = poly_A_cols22.at<double>(0, 0) + poly_A_cols22.at<double>(1, 0)*(j+1);
        double y_fit23 = poly_A_cols23.at<double>(0, 0) + poly_A_cols23.at<double>(1, 0)*(j+1);
        R_temp21 = (double)max_back_ave_cols_1/y_fit21;
        R_temp22 = (double)max_back_ave_cols_2/y_fit22;
        R_temp23 = (double)max_back_ave_cols_3/y_fit23;


        for(int i = 0; i < length; i ++)
        {
            img_res_600_cols1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_600_rows1.at<unsigned short>(i, j)))*R_temp11);
            img_res_min_cols1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_min_rows1.at<unsigned short>(i, j)))*R_temp12);
            img_res_thres_cols1.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_thres_rows1.at<unsigned short>(i, j)))*R_temp13);

            img_res_600_cols2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_600_rows2.at<unsigned short>(i, j)))*R_temp21);
            img_res_min_cols2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_min_rows2.at<unsigned short>(i, j)))*R_temp22);
            img_res_thres_cols2.at<unsigned short>(i, j) = (unsigned short)(((double)(img_res_thres_rows2.at<unsigned short>(i, j)))*R_temp23);
        }
    }

    QString saveName11 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_600_14.tif";
    imwrite(saveName11.toStdString(), img_res_600_cols1);
    QString saveName12 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_min_14.tif";
    imwrite(saveName12.toStdString(), img_res_min_cols1);
    QString saveName13 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_thres_14.tif";
    imwrite(saveName13.toStdString(), img_res_thres_cols1);
    QString saveName21 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_600_17.tif";
    imwrite(saveName21.toStdString(), img_res_600_cols2);
    QString saveName22 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_min_17.tif";
    imwrite(saveName22.toStdString(), img_res_min_cols2);
    QString saveName23 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_fit_thres_17.tif";
    imwrite(saveName23.toStdString(), img_res_thres_cols2);

    return 1;
}


int correction_tile_via_ave_region_peak()
{
    int length = 1880, region_length = 40;
    Mat img_1 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images_cut/ZW2-UC3R1(14)_z0516_Cam1-T3_ORG_after_cut.tif", -1);
    Mat img_2 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/images_cut/ZW2-UC3R1(17)_z0517_Cam1-T3_ORG_after_cut.tif", -1);
    vector<unsigned short> back_peak_ave_rows1, back_peak_ave_rows2;
    vector<unsigned short> back_peak_ave_cols1, back_peak_ave_cols2;
    unsigned short maxValue_row = 0, maxValue_col = 0;

    for(int i = 0; i < length/region_length; i ++)
    {
        unsigned int sum_most_row_region1 = 0, sum_most_row_region2 = 0;
        unsigned short ave_most_row_region1 = 0, ave_most_row_region2 = 0;
        for(int j = 0; j < length/region_length; j ++)
        {
            unordered_map<unsigned short, int> most_int_map1, most_int_map2;
            for(int m = 0; m < region_length; m ++)
            {
                for(int n = 0; n < region_length; n ++)
                {
                    unsigned short temp1 = img_1.at<unsigned short>(i*region_length+m, j*region_length+n);
                    unsigned short temp2 = img_2.at<unsigned short>(i*region_length+m, j*region_length+n);
                    most_int_map1[temp1]++;
                    most_int_map2[temp2]++;
                }
            }

            pair<unsigned short, int> most_int1 = make_pair(most_int_map1.begin()->first, most_int_map1.begin()->second);
            unordered_map<unsigned short, int>::iterator it1 = most_int_map1.begin();
            while (it1 != most_int_map1.end())
            {
                if(most_int1.second<it1->second)
                {
                    most_int1.first = it1->first;
                    most_int1.second = it1->second;
                }
                ++it1;
            }
            unsigned short region_back1 = most_int1.first;
            sum_most_row_region1 += region_back1;

            pair<unsigned short, int> most_int2 = make_pair(most_int_map2.begin()->first, most_int_map2.begin()->second);
            unordered_map<unsigned short, int>::iterator it2 = most_int_map2.begin();
            while (it2 != most_int_map2.end())
            {
                if(most_int2.second<it2->second)
                {
                    most_int2.first = it2->first;
                    most_int2.second = it2->second;
                }
                ++it2;
            }
            unsigned short region_back2 = most_int2.first;
            sum_most_row_region2 += region_back2;
        }
        ave_most_row_region1 = sum_most_row_region1/(length/region_length);
        back_peak_ave_rows1.push_back(ave_most_row_region1);
        maxValue_row = ((maxValue_row>ave_most_row_region1) ? maxValue_row : ave_most_row_region1);

        ave_most_row_region2 = sum_most_row_region2/(length/region_length);
        back_peak_ave_rows2.push_back(ave_most_row_region2);
        maxValue_row = ((maxValue_row>ave_most_row_region2) ? maxValue_row : ave_most_row_region2);
    }

    for(int j = 0; j < length/region_length; j ++)
    {
        unsigned int sum_most_col_region1 = 0, sum_most_col_region2 = 0;
        unsigned short ave_most_col_region1 = 0, ave_most_col_region2 = 0;
        for(int i = 0; i < length/region_length; i ++)
        {
            unordered_map<unsigned short, int> most_int_map1, most_int_map2;
            for(int m = 0; m < region_length; m ++)
            {
                for(int n = 0; n < region_length; n ++)
                {
                    unsigned short temp1 = img_1.at<unsigned short>(i*region_length+m, j*region_length+n);
                    unsigned short temp2 = img_2.at<unsigned short>(i*region_length+m, j*region_length+n);
                    most_int_map1[temp1]++;
                    most_int_map2[temp2]++;
                }
            }

            pair<unsigned short, int> most_int1 = make_pair(most_int_map1.begin()->first, most_int_map1.begin()->second);
            unordered_map<unsigned short, int>::iterator it1 = most_int_map1.begin();
            while (it1 != most_int_map1.end())
            {
                if(most_int1.second<it1->second)
                {
                    most_int1.first = it1->first;
                    most_int1.second = it1->second;
                }
                ++it1;
            }
            unsigned short region_back1 = most_int1.first;
            sum_most_col_region1 += region_back1;

            pair<unsigned short, int> most_int2 = make_pair(most_int_map2.begin()->first, most_int_map2.begin()->second);
            unordered_map<unsigned short, int>::iterator it2 = most_int_map2.begin();
            while (it2 != most_int_map2.end())
            {
                if(most_int2.second<it2->second)
                {
                    most_int2.first = it2->first;
                    most_int2.second = it2->second;
                }
                ++it2;
            }
            unsigned short region_back2 = most_int2.first;
            sum_most_col_region2 += region_back2;
        }
        ave_most_col_region1 = sum_most_col_region1/(length/region_length);
        back_peak_ave_cols1.push_back(ave_most_col_region1);
        maxValue_col = ((maxValue_col>ave_most_col_region1) ? maxValue_col : ave_most_col_region1);

        ave_most_col_region2 = sum_most_col_region2/(length/region_length);
        back_peak_ave_cols2.push_back(ave_most_col_region2);
        maxValue_col = ((maxValue_col>ave_most_col_region2) ? maxValue_col : ave_most_col_region2);
    }


    Mat img_res_rows1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_rows2 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_cols1 = Mat::zeros(1880, 1880, CV_16UC1);
    Mat img_res_cols2 = Mat::zeros(1880, 1880, CV_16UC1);

    for(int i = 0; i < length/region_length; i ++)
    {
        double R_temp1, R_temp2;
        R_temp1 = (double)maxValue_row/back_peak_ave_rows1[i];
        R_temp2 = (double)maxValue_row/back_peak_ave_rows2[i];

        for(int m = 0; m < region_length; m ++)
        {
            for(int j = 0; j < length; j ++)
            {
                img_res_rows1.at<unsigned short>(i*region_length+m, j) = (unsigned short)(((double)(img_1.at<unsigned short>(i*region_length+m, j)))*R_temp1);
                img_res_rows2.at<unsigned short>(i*region_length+m, j) = (unsigned short)(((double)(img_2.at<unsigned short>(i*region_length+m, j)))*R_temp2);
            }
        }
    }

    for(int j = 0; j < length/region_length; j ++)
    {
        double R_temp1, R_temp2;
        R_temp1 = (double)maxValue_col/back_peak_ave_cols1[j];
        R_temp2 = (double)maxValue_col/back_peak_ave_cols2[j];

        for(int m = 0; m < region_length; m ++)
        {
            for(int i = 0; i < length; i ++)
            {
                img_res_cols1.at<unsigned short>(i, j*region_length+m) = (unsigned short)(((double)(img_res_rows1.at<unsigned short>(i, j*region_length+m)))*R_temp1);
                img_res_cols2.at<unsigned short>(i, j*region_length+m) = (unsigned short)(((double)(img_res_rows2.at<unsigned short>(i, j*region_length+m)))*R_temp2);
            }
        }

    }

    QString saveName1 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_ave_14.tif";
    imwrite(saveName1.toStdString(), img_res_cols1);
    QString saveName2 = "C:\\Users\\yanya\\Desktop\\To_do_list\\20200706\\F_correction\\correction_ave_17.tif";
    imwrite(saveName2.toStdString(), img_res_cols2);

    return 1;
}


void stitch_one_plane()
{
    QString folderFile = QFileDialog::getExistingDirectory(nullptr, "Select images folder", "D:\\");
    cout<<"Input folder: "<<folderFile.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", folderFile);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;


    QDir folderDir(folderFile);
    QStringList filters;
    filters << "*.tif"<<"*.tiff"<<"*.jpg";
    QFileInfoList imagesList = folderDir.entryInfoList(filters, QDir::Files);
    cout<<__LINE__<<": imagesList.size = "<<imagesList.size()<<endl;

    vector<Mat> tiles_vec;
    for(int image_i = 0; image_i < imagesList.size(); image_i ++)
    {
        Mat img_tmp = imread(imagesList[image_i].filePath().toStdString(), -1);

        tiles_vec.push_back(img_tmp);

    }

    // result += 60
    Mat result = Mat::zeros(8857, 7518, CV_16UC1);
    //Mat result = Mat::zeros(3104, 3876, CV_16UC1);


    for(int i = 0; i < imagesList.size(); i ++)
    {
        tms.push_back(tifMapScribe());
    }
    cout<<__LINE__<<endl;


    tms[0].set(0, 0);
    tms[1].set(1783, 0);
    tms[2].set(3584, -1);
    tms[3].set(5384, -1);
    tms[4].set(0, 1440);
    tms[5].set(1798, 1438);
    tms[6].set(3599, 1436);
    tms[7].set(5400, 1434);
    tms[8].set(15, 2876);
    tms[9].set(1812, 2875);
    tms[10].set(3611, 2875);
    tms[11].set(5411, 2875);
    tms[12].set(28, 4318);
    tms[13].set(1826, 4318);
    tms[14].set(3626, 4317);
    tms[15].set(5426, 4317);
    tms[16].set(44, 5758);
    tms[17].set(1842, 5757);
    tms[18].set(3642, 5756);
    tms[19].set(5442, 5756);
    tms[20].set(60, 7196);
    tms[21].set(1858, 7195);
    tms[22].set(3658, 7194);
    tms[23].set(5458, 7193);

/*
    tms[0].set(0, 0);
    tms[1].set(1802, -1);
    tms[2].set(15, 1443);
    tms[3].set(1816, 1442);
*/

    int lowx = 0;
    int lowy = 0;
    int highx = 0;
    int highy = 0;

    for(unsigned int i = 0; i < tms.size(); i ++)
    {
        if(lowx > tms[i].leftDownX) lowx = tms[i].leftDownX;
        if(lowy > tms[i].leftDownY) lowy = tms[i].leftDownY;
        if(highx < tms[i].rightUpX) highx = tms[i].rightUpX;
        if(highy < tms[i].rightUpY) highy = tms[i].rightUpY;
    }

    if(lowx < 0)
    {
        for(vector<tifMapScribe>::size_type i = 0; i < imagesList.size(); i ++)
        {
            tms[i].move(-lowx, 0);
        }
        highx -= lowx;
    }

    if(lowy < 0)
    {
        for(vector<tifMapScribe>::size_type i = 0; i < imagesList.size(); i ++)
        {
            tms[i].move(0, -lowy);
        }
        highy -= lowy;
    }

    for(vector<tifMapScribe>::size_type i = 0; i < imagesList.size(); i ++)
    {
        tms[i].move(30, 30);
    }

    highx += 60;
    highy += 60;
    int **r = new int *[highy + 1];

    for(int i = 0; i < highy + 1; i ++)
    {
        r[i] = new int[highx + 1];
    }

   for(int i = 0; i < highy+1; i ++)
   {
       for(int j = 0; j < highx+1; j++)
       {
          r[i][j] = -1;
       }
   }

   //for(int j = 0; j < 3104; j ++)
   for(int j = 0; j < 8857; j ++)
   {
       //cout<<__LINE__<<"j = "<<j<<endl;
       //for(int k = 0; k < 3876; k ++)
       for(int k = 0; k < 7518; k ++)
       {
           int sum = 0;
           for(vector<tifMapScribe>::size_type i = 0; i < imagesList.size(); i ++)
               sum += tms[i].distance(k, j);

           r[j][k] = 0;
           for(vector<tifMapScribe>::size_type i = 0; i < imagesList.size(); i ++)
               r[j][k] += (int)((tms[i].distance(k, j)+0.0)/sum * getdeep(k, j, i));
        }
    }

    cout<<__LINE__<<endl;
    for(int i = 0; i <= highy; i ++)
    {
        for(int j = 0; j <= highx; j ++)
        {
            if(r[i][j] == -1)
                r[i][j] = 0;
        }
    }
    cout<<__LINE__<<": highy + 1 = "<<highy+1<<endl;
    cout<<__LINE__<<": highx + 1 = "<<highx+1<<endl;
//    Mat result = Mat::zeros(highy+1, highx+1, CV_16UC1);
//    10071, 8444
    for(int i = 0; i <= highy; i ++)
    {
        for(int j = 0; j <= highx; j ++)
        {
            result.at<unsigned short>(i, j) = (unsigned short)r[i][j];
        }
    }

    QString saveName = saveFolder + "\\" + "mixing_image_24tiles.tif";
    imwrite(saveName.toStdString(), result);

    cout<<"Finished!"<<endl;
}

int stitch_all_planes_24tiles()
{
    QString allTileName = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"allTileName: "<<allTileName.toStdString().c_str()<<endl;
    QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save",allTileName);
    cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;

    QDir allTileDir(allTileName);
    allTileDir.setFilter(QDir::Dirs);
    QFileInfoList tilesListCols = allTileDir.entryInfoList();

    vector<QFileInfoList> tilesListRows;
    for(int i = 0; i < tilesListCols.size()-2; i ++)
    {
        QDir rowsTileDir(tilesListCols[i+2].filePath());
        rowsTileDir.setFilter(QDir::Dirs);
        tilesListRows.push_back(rowsTileDir.entryInfoList());
    }

    vector<QFileInfoList> allTileImagesList;
    int num_planes = INT_MAX;
    for(int i = 0; i < tilesListRows.size(); i ++)
    {
        for(int j = 0; j <tilesListRows[i].size()-2; j ++)
        {
            cout<<__LINE__<<": tileList["<<i*tilesListRows.size()+j<<"] Name: "<<tilesListRows[i][j+2].fileName().toStdString().c_str()<<endl;
            QDir folderDir(tilesListRows[i][j+2].filePath());
            QStringList filters;
            filters << "*.tif"<<"*.tiff"<<"*.jpg";
            QFileInfoList imagesList = folderDir.entryInfoList(filters, QDir::Files);
            cout<<__LINE__<<": imagesList.size() = "<<imagesList.size()<<endl;
            num_planes = (imagesList.size()<num_planes) ? imagesList.size() : num_planes;
            allTileImagesList.push_back(imagesList);
        }
    }
    cout<<__LINE__<<": num_planes = "<<num_planes<<endl;

    for(int image_i = 0; image_i < num_planes; image_i ++)
    {
        vector<Mat> tiles_vec_tmp;
        for(int i = 0; i < allTileImagesList.size(); i ++)
        {
            Mat img_tmp = imread(allTileImagesList[i][image_i].filePath().toStdString(), -1);

            tiles_vec_tmp.push_back(img_tmp);
        }
        tiles_vec_vec.push_back(tiles_vec_tmp);

        Mat result = Mat::zeros(8857, 7518, CV_16UC1);


        vector<tifMapScribe> tms_tmp;
        for(int i = 0; i < allTileImagesList.size(); i ++)
        {
            tms_tmp.push_back(tifMapScribe());
        }
        tms_vec_vec.push_back(tms_tmp);

        cout<<__LINE__<<endl;
        tms_vec_vec[image_i][0].set(0, 0);
        tms_vec_vec[image_i][6].set(1783, 0);
        tms_vec_vec[image_i][12].set(3584, -1);
        tms_vec_vec[image_i][18].set(5384, -1);
        tms_vec_vec[image_i][1].set(0, 1440);
        tms_vec_vec[image_i][7].set(1798, 1438);
        tms_vec_vec[image_i][13].set(3599, 1436);
        tms_vec_vec[image_i][19].set(5400, 1434);
        tms_vec_vec[image_i][2].set(15, 2876);
        tms_vec_vec[image_i][8].set(1812, 2875);
        tms_vec_vec[image_i][14].set(3611, 2875);
        tms_vec_vec[image_i][20].set(5411, 2875);
        tms_vec_vec[image_i][3].set(28, 4318);
        tms_vec_vec[image_i][9].set(1826, 4318);
        tms_vec_vec[image_i][15].set(3626, 4317);
        tms_vec_vec[image_i][21].set(5426, 4317);
        tms_vec_vec[image_i][4].set(44, 5758);
        tms_vec_vec[image_i][10].set(1842, 5757);
        tms_vec_vec[image_i][16].set(3642, 5756);
        tms_vec_vec[image_i][22].set(5442, 5756);
        tms_vec_vec[image_i][5].set(60, 7196);
        tms_vec_vec[image_i][11].set(1858, 7195);
        tms_vec_vec[image_i][17].set(3658, 7194);
        tms_vec_vec[image_i][23].set(5458, 7193);


        int lowx = 0;
        int lowy = 0;
        int highx = 0;
        int highy = 0;

        for(unsigned int i = 0; i < tms_vec_vec[image_i].size(); i ++)
        {
            if(lowx > tms_vec_vec[image_i][i].leftDownX) lowx = tms_vec_vec[image_i][i].leftDownX;
            if(lowy > tms_vec_vec[image_i][i].leftDownY) lowy = tms_vec_vec[image_i][i].leftDownY;
            if(highx < tms_vec_vec[image_i][i].rightUpX) highx = tms_vec_vec[image_i][i].rightUpX;
            if(highy < tms_vec_vec[image_i][i].rightUpY) highy = tms_vec_vec[image_i][i].rightUpY;
        }

        if(lowx < 0)
        {
            for(vector<tifMapScribe>::size_type i = 0; i < allTileImagesList.size(); i ++)
            {
                tms_vec_vec[image_i][i].move(-lowx, 0);
            }
            highx -= lowx;
        }

        if(lowy < 0)
        {
            for(vector<tifMapScribe>::size_type i = 0; i < allTileImagesList.size(); i ++)
            {
                tms_vec_vec[image_i][i].move(0, -lowy);
            }
            highy -= lowy;
        }

        for(vector<tifMapScribe>::size_type i = 0; i < allTileImagesList.size(); i ++)
        {
            tms_vec_vec[image_i][i].move(30, 30);
        }

        highx += 60;
        highy += 60;
        int **r = new int *[highy + 1];

        for(int i = 0; i < highy + 1; i ++)
        {
            r[i] = new int[highx + 1];
        }

       for(int i = 0; i < highy+1; i ++)
       {
           for(int j = 0; j < highx+1; j++)
           {
              r[i][j] = -1;
           }
       }

       cout<<__LINE__<<endl;
       for(int j = 0; j < 8857; j ++)
       {
           for(int k = 0; k < 7518; k ++)
           {
               cout<<__LINE__<<": j = "<<j<<"; k = "<<k<<endl;
               int sum = 0;
               for(vector<tifMapScribe>::size_type i = 0; i < allTileImagesList.size(); i ++)
                   sum += tms_vec_vec[image_i][i].distance(k, j);

               cout<<__LINE__<<": sum = "<<sum<<endl;
               r[j][k] = 0;
               for(vector<tifMapScribe>::size_type i = 0; i < allTileImagesList.size(); i ++)
               {
                   //cout<<"value1 = "<<tms_vec_vec[image_i][i].distance(k, j)<<endl;
                   //cout<<"value2 = "<<getdeepImageI(k, j, i, image_i)<<endl;
                   r[j][k] += (int)((tms_vec_vec[image_i][i].distance(k, j)+0.0)/sum * getdeepImageI(k, j, i, image_i));
               }

            }
        }

       cout<<__LINE__<<endl;
        for(int i = 0; i <= highy; i ++)
        {
            for(int j = 0; j <= highx; j ++)
            {
                if(r[i][j] == -1)
                    r[i][j] = 0;
            }
        }

        cout<<__LINE__<<endl;
        for(int i = 0; i <= highy; i ++)
        {
            for(int j = 0; j <= highx; j ++)
            {
                result.at<unsigned short>(i, j) = (unsigned short)r[i][j];
            }
        }

        cout<<__LINE__<<endl;
        string num_str = to_string(image_i);
        while(num_str.size() < 4)
        {
            num_str = "0" + num_str;
        }

        QString saveName = savePath + "\\" + "stitched_Z"+QString::fromStdString(num_str)+".tif";
        imwrite(saveName.toStdString(), result);
        cout<<__LINE__<<": Saved "<<saveName.toStdString().c_str()<<endl;
    }

    return 1;
}

int stitch_2img_down_cover_up()
{
    //down covers up
    int overlap = 259, shift = 14, length = 1880;
    Mat img_1 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/F_correction/correction_ave_14.tif", -1);
    Mat img_2 = imread("C:/Users/yanya/Desktop/To_do_list/20200706/F_correction/correction_ave_17.tif", -1);
    Mat result = Mat::zeros(length*2-overlap, length+shift, CV_16UC1);

    cout<<__LINE__<<endl;
    for(int i = 0; i < length-overlap; i ++)
    {
        for(int j = 0; j < length; j ++)
            result.at<unsigned short>(i, j) = img_1.at<unsigned short>(i, j);
    }

    for(int i = length-overlap; i < length-overlap+10; i ++)
    {
        for(int j = 0; j < shift; j ++)
            result.at<unsigned short>(i, j) = img_1.at<unsigned short>(i, j);
        for(int j = shift; j < shift+10; j ++)
        {
            double res1 = img_1.at<unsigned short>(i, j)*(10-i+length-overlap+0.0)/10 + img_2.at<unsigned short>(i-length+overlap, j-shift)*(i+overlap-length+0.0)/10;
            double res2 = img_1.at<unsigned short>(i, j)*(shift-j+10+0.0)/10 + img_2.at<unsigned short>(i-length+overlap, j-shift)*(-shift+j+0.0)/10;
            result.at<unsigned short>(i, j) = (unsigned short)((res1+res2)/2);
        }
        for(int j = shift+10; j < length; j ++)
            result.at<unsigned short>(i, j) = img_1.at<unsigned short>(i, j)*(10-i+length-overlap+0.0)/10 + img_2.at<unsigned short>(i-length+overlap, j-shift)*(i+overlap-length+0.0)/10;
        for(int j = length; j < length+shift; j ++)
            result.at<unsigned short>(i, j) = img_2.at<unsigned short>(i-length+overlap, j-shift);
    }

    for(int i = length-overlap+10; i < length; i ++)
    {
        for(int j = 0; j < shift; j ++)
            result.at<unsigned short>(i, j) = img_1.at<unsigned short>(i, j);
        for(int j = shift; j < shift+10; j ++)
            result.at<unsigned short>(i, j) = img_1.at<unsigned short>(i, j)*(shift-j+10+0.0)/10 + img_2.at<unsigned short>(i-length+overlap, j-shift)*(-shift+j+0.0)/10;
        for(int j = shift+10; j < shift+length; j ++)
            result.at<unsigned short>(i, j) = img_2.at<unsigned short>(i-length+overlap, j-shift);
 //           result.at<uchar>(i, j) = 0.5 * img_1.at<uchar>(i-10, j) + 0.5 * img_2.at<uchar>(i, j-1920+297);
    }
    for(int i = length; i < 2*length-overlap; i ++)
    {
        for(int j = shift; j < length+shift; j ++)
            result.at<unsigned short>(i, j) = img_2.at<unsigned short>(i-length+overlap, j-shift);
    }

    imwrite("C:/Users/yanya/Desktop/To_do_list/20200706/F_correction/mixing_image_ave_14_17.tif", result);

    return 1;
}
