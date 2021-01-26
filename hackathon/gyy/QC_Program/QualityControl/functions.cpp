#include <QFileDialog>
#include <iostream>
#include <vector>
#include <algorithm>
#include <QAxObject>
#include <QApplication>
#include "matsource.h"
#include "functions.h"
#include "x_y_shift.h"
#include "opencv2/opencv.hpp"
#include "tifmapscribe.h"

using namespace cv;
using namespace std;

static vector<Mat> tiles_vec;
static vector<tifMapScribe> tms;
unsigned short getdeep(int x, int y, vector<tifMapScribe>::size_type n);
void save_tiles_overlap_shift(vector<x_y_shift> XY_shift_vec, QString saveName);

void Functions:: calculate_YZ_shift_via_MIP()
{
    QString allTileName = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"allTileName: "<<allTileName.toStdString().c_str()<<endl;
    QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
    cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;
    vector<x_y_shift> y_z_shift_vec;

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

    vector<vector<String>> allTileStr;
    for(int i = 0; i < tilesListRows.size(); i ++)
    {
        for(int j = 0; j <tilesListRows[i].size()-2; j ++)
        {
            cout<<"tileList["<<i<<"] Name: "<<tilesListRows[i][j+2].fileName().toStdString().c_str()<<endl;
            vector<String> tile = MatSource::readMats(tilesListRows[i][j+2].filePath());
            allTileStr.push_back(tile);
        }
    }

    cout<<"allTileStr.size() = "<<allTileStr.size()<<endl;

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[0],  allTileStr[6],   200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[0],  allTileStr[1],    160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[6],  allTileStr[12],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[6],  allTileStr[7],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[12], allTileStr[18],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[12], allTileStr[13],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[18], allTileStr[19],  160, "down",  1600, 2000, 20));

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[1],  allTileStr[7],   200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[1],  allTileStr[2],    160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[7],  allTileStr[13],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[7],  allTileStr[8],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[13], allTileStr[19],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[13], allTileStr[14],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[19], allTileStr[20],  160, "down",  1600, 2000, 20));

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[2],  allTileStr[8],   200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[2],  allTileStr[3],    160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[8],  allTileStr[14],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[8],  allTileStr[9],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[14], allTileStr[20],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[14], allTileStr[15],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[20], allTileStr[21],  160, "down",  1600, 2000, 20));

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[3],  allTileStr[9],   200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[3],  allTileStr[4],    160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[9],  allTileStr[15],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[9],  allTileStr[10],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[15], allTileStr[21],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[15], allTileStr[16],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[21], allTileStr[22],  160, "down",  1600, 2000, 20));

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[4],  allTileStr[10],   200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[4],  allTileStr[5],    160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[10], allTileStr[16],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[10], allTileStr[11],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[16], allTileStr[22],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[16], allTileStr[17],  160, "down",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[22], allTileStr[23],  160, "down",  1600, 2000, 20));

    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[5],  allTileStr[11],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[11], allTileStr[17],  200, "left",  1600, 2000, 20));
    y_z_shift_vec.push_back(MatSource::A_B_y_z_shift (allTileStr[17], allTileStr[23],  200, "left",  1600, 2000, 20));

    QString saveName = savePath + "\\all_Tiles_Y_Z_shifts.xlsx";
    for(int i = 0; i < y_z_shift_vec.size(); i ++)
    {
        cout<<"Z_shift_vec["<<i<<"] = "<<y_z_shift_vec[i].y<<endl;
    }
    //save_tiles_overlap_shift(y_z_shift_vec, saveName);
}

void Functions:: calculate_overlap_shift_one_plane()
{
    QString TilesName = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"TileName: "<<TilesName.toStdString().c_str()<<endl;
    QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
    cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;

    QDir tileDir(TilesName);
    QStringList filters;
    filters << "*.tif" << "*.tiff"<<"*.jpg";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);
    vector<Mat> imagesMat;
    for(int i = 0; i < imageList.size(); i ++)
    {
        cout<<"image["<<i<<"]: "<<imageList[i].filePath().toStdString().c_str()<<endl;
        Mat tempImage=imread(imageList[i].filePath().toStdString(),2);
        imagesMat.push_back(tempImage);
    }

    vector<x_y_shift> XY_shift_vec;
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[0], imagesMat[1], "left",   200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[0], imagesMat[4], "down",   160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[1], imagesMat[2], "left",   200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[1], imagesMat[5], "down",  160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[2], imagesMat[3], "left",   200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[2], imagesMat[6], "down",  160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[3], imagesMat[7], "down",  160));

    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[4], imagesMat[5], "left",  200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[4], imagesMat[8], "down",  160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[5], imagesMat[6], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[5], imagesMat[9], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[6], imagesMat[7], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[6], imagesMat[10], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[7], imagesMat[11], "down", 160));

    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[8], imagesMat[9], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[8], imagesMat[12], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[9], imagesMat[10], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[9], imagesMat[13], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[10], imagesMat[11], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[10], imagesMat[14], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[11], imagesMat[15], "down", 160));

    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[12], imagesMat[13], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[12], imagesMat[16], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[13], imagesMat[14], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[13], imagesMat[17], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[14], imagesMat[15], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[14], imagesMat[18], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[15], imagesMat[19], "down", 160));

    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[16], imagesMat[17], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[16], imagesMat[20], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[17], imagesMat[18], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[17], imagesMat[21], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[18], imagesMat[19], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[18], imagesMat[22], "down", 160));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[19], imagesMat[23], "down", 160));

    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[20], imagesMat[21], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[21], imagesMat[22], "left", 200));
    XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[22], imagesMat[23], "left", 200));

    QString saveName = savePath + "\\one_plane_XY_shifts.xlsx";
    save_tiles_overlap_shift(XY_shift_vec, saveName);
}

void Functions:: calculate_overlap_shift_all_images()
{
    QString TileName1 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"TileName: "<<TileName1.toStdString().c_str()<<endl;
    QString TileName2 = QFileDialog::getExistingDirectory(nullptr, "Select tile2 folder",TileName1);
    cout<<"TileName: "<<TileName2.toStdString().c_str()<<endl;
    QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
    cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;


    QFileInfo tilesList1(TileName1), tilesList2(TileName2);


//    tile21_22
    vector<String> tile1 =MatSource::readMats(tilesList1.filePath());
    vector<String> tile2 =MatSource::readMats(tilesList2.filePath());

    vector<x_y_shift> os_result = MatSource::tile_x_y_shift(tile1,tile2,"down",160);
    //vector<x_y_shift> os_result = MatSource::tile_x_y_shift(tile1,tile2,"left",200);

    QString saveName = savePath + "\\all_Tiles_XY_shifts.xlsx";
    save_tiles_overlap_shift(os_result, saveName);
}







void Functions:: shift_global_optimization()
{
    // input shifts of neighboring tiles
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

}

void Functions:: stitch_one_plane()
{
    QString folderFile = QFileDialog::getExistingDirectory(nullptr, "Select images folder to stitch", "D:\\");
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

    // input the size of the entile image
    // result += 60
    int resultImageRows = 8857;
    int resultImageCols = 7518;
    Mat result = Mat::zeros(resultImageRows, resultImageCols, CV_16UC1);


    for(int i = 0; i < imagesList.size(); i ++)
    {
        tms.push_back(tifMapScribe());
    }


    // calculate the positions of all images
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

   for(int j = 0; j < resultImageRows; j ++)
   {
       for(int k = 0; k < resultImageCols; k ++)
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

//    10071, 8444
    for(int i = 0; i <= highy; i ++)
    {
        for(int j = 0; j <= highx; j ++)
        {
            result.at<unsigned short>(i, j) = (unsigned short)r[i][j];
        }
    }

    QString saveName = saveFolder + "\\" + "mixing_image_tiles.tif";
    imwrite(saveName.toStdString(), result);

    cout<<"Finished!"<<endl;
}

void Functions:: XZ_projection_20px_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);
    QStringList filters;
    filters << "*.tif" << "*.tiff"<<"*.jpg";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);


    Mat image_para = imread(imageList[0].filePath().toStdString(), 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res = Mat::zeros(Z_length, X_length, CV_16UC1);
    int overlapCenter = Y_length/2;

    for(int i = 0; i < imageList.size(); i ++)
    {
        Mat image_i = imread(imageList[i].filePath().toStdString(), 2);
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
    QString saveName = saveFolder + "\\" + tileInfo.baseName() + "_XZ_MIP.tif";
    imwrite(saveName.toStdString(), image_res);

}

void Functions:: YZ_projection_20px_MIP()
{
    QString imagePath = QFileDialog::getExistingDirectory(nullptr, "Select tile forder", "D:\\");
    cout<<"ImagePath: "<<imagePath.toStdString().c_str()<<endl;
    QString saveFolder = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save", imagePath);
    cout<<"Save folder: "<<saveFolder.toStdString().c_str()<<endl;
    QFileInfo tileInfo(imagePath);
    QDir tileDir(imagePath);

    // filter
    QStringList filters;
    filters << "*.tif" << "*.tiff"<<"*.jpg";
    QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);

    Mat image_para = imread(imageList[0].filePath().toStdString(), 2);
    int Z_length = imageList.size();
    int X_length = image_para.cols, Y_length = image_para.rows;
    Mat image_res = Mat::zeros(Z_length, Y_length, CV_16UC1);
    int overlapCenter = X_length/2;

    for(int i = 0; i < imageList.size(); i ++)
    {
        Mat image_i = imread(imageList[i].filePath().toStdString(), 2);
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
    QString saveName = saveFolder + "\\" + tileInfo.baseName() + "_YZ_MIP.tif";
    imwrite(saveName.toStdString(), image_res);
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

void save_tiles_overlap_shift(vector<x_y_shift> XY_shift_vec, QString saveName)
{

    QAxObject *excel1 = new QAxObject();
    excel1 -> setControl("Excel.Application"); // connect EXCEL control
    excel1 -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel1 -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel1 -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            saveName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "sheet1");

    QList<QList<QVariant>> datas_sheet;
    for(int i = 0; i < XY_shift_vec.size()+1; i ++)
    {
        QList <QVariant> rows;
        if(i == 0)
        {
            rows.append("Z");
            rows.append("shift1");
            rows.append("shift2");
        }

        else
        {
            rows.append(i);
            rows.append(XY_shift_vec[i-1].x);
            rows.append(XY_shift_vec[i-1].y);
        }
        datas_sheet.append(rows);
    }

    QList <QVariant> vars;
    for(auto v: datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);
    QString num1 = QString::fromStdString(to_string(XY_shift_vec.size()+1));
    QString big_range = "A1:C"+num1;
    QAxObject *excel_property = worksheet1 -> querySubObject("Range(const QString&)", big_range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);


    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel1->dynamicCall("Quit(void)");
    delete excel1;
}
