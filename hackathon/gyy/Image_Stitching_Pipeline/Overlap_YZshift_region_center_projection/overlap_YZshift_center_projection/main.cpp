#include "mainwindow.h"
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

using namespace std;
using namespace cv;

void calculate_project(QFileInfoList imageList, vector<Mat> &pdataCPlane, Mat &pdataPro, QString orientation, int &rows, int &cols, int count);
void calculate_Z_shift_MIP(vector<Mat> pdataPro1, vector<Mat> pdataPro2, QString orientation, vector<int> &XY_shift_MIP_vec, vector<int> &Z_shift_MIP_vec);
void calculate_overlap_shift(vector<vector<Mat>> pdataAllPlane1, vector<vector<Mat> > &pdata_CPlane1, vector<vector<Mat>> pdataAllPlane2, vector<vector<Mat>> &pdata_CPlane2, vector<vector<int>> &XY_overlap_plane, vector<vector<int>> &XY_shift_plane, QString orientation, int Z_shift_MIP);
void calculate_expansion_ratio(vector<vector<int>> XY_overlap_plane, vector<vector<int>> XY_shift_plane, vector<vector<double>> &overlap_shift_Ratio, int Z_shift_MIP, QString orientation, int rows, int cols);
void save_tiles_shift(QString SaveFolder, string s1, string s2, QString orientation, vector<int> XY_shift_MIP_vec, vector<int> Z_shift_MIP_vec, vector<vector<int>> XY_overlap_plane, vector<vector<int>> XY_shift_plane, vector<vector<double>> overlap_shift_Ratio, int Z_shift);
void calculate_2Img_overlap_shift(Mat pdata_CP1, Mat pdata_CP2, QString orientation, int &XY_overlap_p, int &XY_shift_p, int num, int c);
QAxObject * appendSheet(QAxObject * &worksheets, const QString sheetName);
QString setExcelName(QString saveFolder, string s1, string s2, QString orientation);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cout<<"Start!"<<endl;

    QString tileFolderFiles1 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
    cout<<"Tile1: "<<tileFolderFiles1.toStdString().c_str()<<endl;
    QString tileFolderFiles2 = QFileDialog :: getExistingDirectory(nullptr, "Select tile2 folder", tileFolderFiles1);
    cout<<"Tile2: "<<tileFolderFiles2.toStdString().c_str()<<endl;
    QString SaveFolder = QFileDialog :: getExistingDirectory(nullptr, "Select folder to save", "D:\\");
    cout<<"SaveFolder: "<<SaveFolder.toStdString().c_str()<<endl;

    QDir tileDir1(tileFolderFiles1);
    QDir tileDir2(tileFolderFiles2);
    if(!tileDir1.exists() || !tileDir2.exists())
        return 0;

    string sname1 = tileDir1.dirName().toStdString();
    //string s1 = sname1.substr(10, 2);
    string sname2 = tileDir2.dirName().toStdString();
    //string s2 = sname2.substr(10, 2);


    QString orientation1, orientation2;
    orientation1 = "left";
    orientation2 = "right";
    /*
    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        qDebug()<<__LINE__;
        return 0;
    }
    */
    cout<<__LINE__<<endl;
    cout<<"Orientation1 = "<<orientation1.toStdString().c_str()<<endl;
    cout<<"Orientation2 = "<<orientation2.toStdString().c_str()<<endl;

    tileDir1.setFilter(QDir::Dirs);
    QFileInfoList channelList1 = tileDir1.entryInfoList();
    tileDir2.setFilter(QDir::Dirs);
    QFileInfoList channelList2 = tileDir2.entryInfoList();

    vector<vector<Mat>> pdataAllPlane1;
    vector<vector<Mat>> pdataAllPlane2;
    vector<Mat> pdataPro1;
    vector<Mat> pdataPro2;

    vector<QDir> dir1, dir2;
    vector<QFileInfoList> imageList1, imageList2;
    vector<QString> tileSave1, tileSave2;
    int count = 0;
    int rows = 0, cols = 0;

    for(int i = 0; i < channelList1.size()-2; i ++)
    {
        cout<<__LINE__<<": i = "<<i<<endl;
        dir1.push_back(channelList1[i+2].filePath());
        dir2.push_back(channelList2[i+2].filePath());

        QStringList filters;
        filters << "*.tif" << "*.v3draw";
        imageList1.push_back(dir1[i].entryInfoList(filters, QDir::Files));
        imageList2.push_back(dir2[i].entryInfoList(filters, QDir::Files));

        vector<Mat> pdataPlane1, pdataPlane2;
        count ++;
        calculate_project(imageList1[i], pdataPlane1, pdataPro1[i], orientation1, rows, cols, count);
        count ++;
        calculate_project(imageList2[i], pdataPlane2, pdataPro2[i], orientation2, rows, cols, count);

        pdataAllPlane1.push_back(pdataPlane1);
        pdataAllPlane2.push_back(pdataPlane2);

    }

    vector<int> XY_shift_MIP_vec, Z_shift_MIP_vec;
    calculate_Z_shift_MIP(pdataPro1, pdataPro2, orientation1, XY_shift_MIP_vec, Z_shift_MIP_vec);

    int sum_z_shift = 0;
    for(int i = 0; i < Z_shift_MIP_vec.size(); i ++)
        sum_z_shift += Z_shift_MIP_vec[i];
    int Z_shift_MIP = (int)((sum_z_shift+0.0)/Z_shift_MIP_vec.size()+0.5);

    vector<vector<Mat>> pdata_CPlane1(channelList1.size(), vector<Mat>(imageList1.size() - Z_shift_MIP));
    vector<vector<Mat>> pdata_CPlane2(channelList1.size(), vector<Mat>(imageList1.size() - Z_shift_MIP));
    vector<vector<int>> XY_overlap_plane(channelList1.size(), vector<int>(imageList1.size(), 0));
    vector<vector<int>> XY_shift_plane(channelList1.size(), vector<int>(imageList1.size(), 0));
    vector<vector<double>> overlap_shift_Ratio(channelList1.size(), vector<double>(imageList1.size(), 0));

    calculate_overlap_shift(pdataAllPlane1, pdata_CPlane1, pdataAllPlane2, pdata_CPlane2, XY_overlap_plane, XY_shift_plane, orientation1, Z_shift_MIP);

    calculate_expansion_ratio(XY_overlap_plane, XY_shift_plane, overlap_shift_Ratio, Z_shift_MIP, orientation1, rows, cols);

    save_tiles_shift(SaveFolder, sname1, sname2, orientation1, XY_shift_MIP_vec, Z_shift_MIP_vec, XY_overlap_plane, XY_shift_plane, overlap_shift_Ratio, Z_shift_MIP);


    cout<<"Successful!"<<endl;
    return a.exec();
}

void calculate_project(QFileInfoList imageList, vector<Mat> &pdataCPlane, Mat &pdataPro, QString orientation, int &rows, int &cols, int count)
{
    for(int i_image = 0; i_image < imageList.size(); i_image ++)
    {
        cout<<"This is "<<i_image+1<<" image for "<<count<<"."<<endl;

        Mat image_i = imread(imageList[i_image].filePath().toStdString(), 2);
        rows = image_i.rows;
        cols = image_i.cols;

        pdataCPlane.push_back(image_i);
    }

    int centerOverlap = 0;
    if(orientation == "left" || orientation == "right")
    {
        if(orientation == "left")
            centerOverlap = cols-cols/10/2;
        else
            centerOverlap = cols/10/2;

        for(int i = 0; i < imageList.size(); i ++)
        {
            for(int j = 0; j < rows; j ++)
            {
                int maxValue = pdataCPlane[i].at<unsigned short>(j, centerOverlap-10);
                for(int k = centerOverlap-10; k < centerOverlap+10; k++)
                {
                    maxValue = ((maxValue >= pdataCPlane[i].at<unsigned short>(j, k)) ? maxValue : pdataCPlane[i].at<unsigned short>(j, k));
                }
                pdataPro.at<unsigned short>(i, j) = maxValue;
            }
        }

    }

    else if(orientation == "up" || orientation == "down")
    {
        if(orientation == "up")
            centerOverlap = rows - rows/10/2;
        else
            centerOverlap = rows/10/2;

        for(int i = 0; i < imageList.size(); i ++)
        {
            for(int j = 0; j < cols; j ++)
            {
                int maxValue = pdataCPlane[i].at<unsigned short>(centerOverlap-10, j);
                for(int k = centerOverlap-10; k < centerOverlap+10; k ++)
                {
                    maxValue = ((maxValue >= pdataCPlane[i].at<unsigned short>(k, j)) ? maxValue : pdataCPlane[i].at<unsigned short>(k, j));
                }
                pdataPro.at<unsigned short>(i, j) = maxValue;
            }
        }
    }

}

void  calculate_Z_shift_MIP(vector<Mat> pdataPro1, vector<Mat> pdataPro2, QString orientation, vector<int> &XY_shift_MIP_vec, vector<int> &Z_shift_MIP_vec)
{
    const int XY_shift_range = 20, Z_shift_range = 20;
    if(orientation == "left" || orientation == "right")
    {
        for(int i = 0; i < pdataPro1.size(); i ++)
        {
            double sum[XY_shift_range*Z_shift_range] = {0};
            int difference[XY_shift_range*Z_shift_range] = {0};
            int count[XY_shift_range*Z_shift_range] = {0};
            double minError = INT_MAX;
            for(int sz = 0; sz < XY_shift_range; sz ++)
            {
                for(int ss = 0; ss < Z_shift_range; ss ++)
                {
                    for(int row = 0; row < pdataPro1[i].rows-sz; row ++)
                    {
                        for(int col = 0; col < pdataPro1[i].cols-ss; col ++)
                        {
                            int diff;
                            if(orientation == "left")
                                diff = pdataPro1[i].at<unsigned short>(row+sz, col)-pdataPro2[i].at<unsigned short>(row, col+ss);
                            else
                                diff = pdataPro2[i].at<unsigned short>(row+sz, col)-pdataPro1[i].at<unsigned short>(row, col+ss);
                            sum[sz*Z_shift_range+ss] += diff * diff;
                            count[sz*Z_shift_range+ss] +=1;
                        }
                    }
                    difference[sz*Z_shift_range+ss] = sum[sz*Z_shift_range+ss]/count[sz*Z_shift_range+ss];

                    if(minError>difference[sz*Z_shift_range+ss])
                    {
                        minError = difference[sz*Z_shift_range+ss];
                        Z_shift_MIP_vec[i] = sz;
                        XY_shift_MIP_vec[i] = ss;
                    }
                }
                cout<<endl;
            }
            cout<<"Z shift = "<<Z_shift_MIP_vec[i];
            cout<<"XY shift = "<<XY_shift_MIP_vec[i];
            cout<<" Calculate Z shift in MIP is successful !";
        }
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int i = 0; i < pdataPro1.size(); i ++)
        {
            double sum[XY_shift_range*Z_shift_range] = {0};
            int difference[XY_shift_range*Z_shift_range] = {0};
            int count[XY_shift_range*Z_shift_range] = {0};
            double minError = INT_MAX;
            for(int sz = 0; sz < XY_shift_range; sz ++)
            {
                for(int ss = 0; ss < Z_shift_range; ss ++)
                {
                    for(int row = 0; row < pdataPro1[i].rows-sz; row ++)
                    {
                        for(int col = 0; col < pdataPro1[i].cols-ss; col ++)
                        {
                            int diff = pdataPro1[i].at<unsigned short>(row+sz, col)-pdataPro2[i].at<unsigned short>(row, col+ss);
                            sum[sz*Z_shift_range+ss] += diff * diff;
                            count[sz*Z_shift_range+ss] +=1;
                        }
                    }
                    difference[sz*Z_shift_range+ss] = sum[sz*Z_shift_range+ss]/count[sz*Z_shift_range+ss];

                    if(minError>difference[sz*Z_shift_range+ss])
                    {
                        minError = difference[sz*Z_shift_range+ss];
                        Z_shift_MIP_vec[i] = sz;
                        XY_shift_MIP_vec[i] = ss;
                    }
                }
                cout<<endl;
            }
            cout<<"Z shift = "<<Z_shift_MIP_vec[i];
            cout<<"XY shift = "<<XY_shift_MIP_vec[i];
            cout<<" Calculate Z shift in MIP is successful !";
        }
    }

}

void calculate_overlap_shift(vector<vector<Mat>> pdataAllPlane1, vector<vector<Mat>> &pdata_CPlane1, vector<vector<Mat>> pdataAllPlane2, vector<vector<Mat>> &pdata_CPlane2, vector<vector<int>> &XY_overlap_plane, vector<vector<int>> &XY_shift_plane, QString orientation, int Z_shift_MIP)
{
    cout<<__LINE__<<": Calculate overlap and shift!"<<endl;
    for(int c = 0; c < pdataAllPlane1.size(); c ++)
    {
        if(orientation == "left")
        {
            cout<<__LINE__<<": orientation = "<<orientation.toStdString().c_str()<<endl;
            for(int p = 0; p < pdataAllPlane1[0].size() - Z_shift_MIP; p ++)
            {
                for(int i = 0; i < pdataAllPlane1[c][p].rows; i ++)
                {
                    for(int j = 0; j < pdataAllPlane1[c][p].cols; j ++)
                    {
                        pdata_CPlane1[c][p].at<unsigned short>(i, j) = pdataAllPlane1[c][p+Z_shift_MIP].at<unsigned short>(i, j);
                    }
                }

                for(int i = 0; i < pdataAllPlane2[c][p].rows; i ++)
                {
                    for(int j = 0; j < pdataAllPlane2[c][p].cols; j ++)
                    {
                        pdata_CPlane2[c][p].at<unsigned short>(i, j) = pdataAllPlane2[c][p].at<unsigned short>(i, j);
                    }
                }
            }

            for(int i = 0; i < pdataAllPlane1[0].size() - Z_shift_MIP; i ++)
            {
                calculate_2Img_overlap_shift(pdata_CPlane1[c][i], pdata_CPlane2[c][i], orientation, XY_overlap_plane[c][i], XY_shift_plane[c][i], i, c);
            }
        }

        else if(orientation == "right" || orientation == "up")
        {
            cout<<__LINE__<<": orientation = "<<orientation.toStdString().c_str()<<endl;
            for(int p = 0; p < pdataAllPlane1[0].size() - Z_shift_MIP; p ++)
            {
                for(int i = 0; i < pdataAllPlane1[c][p].rows; i ++)
                {
                    for(int j = 0; j < pdataAllPlane1[c][p].cols; j ++)
                    {
                        pdata_CPlane1[c][p].at<unsigned short>(i, j) = pdataAllPlane1[c][p].at<unsigned short>(i, j);
                    }
                }

                for(int i = 0; i < pdataAllPlane2[c][p].rows; i ++)
                {
                    for(int j = 0; j < pdataAllPlane2[c][p].cols; j ++)
                    {
                        pdata_CPlane2[c][p].at<unsigned short>(i, j) = pdataAllPlane2[c][p+Z_shift_MIP].at<unsigned short>(i, j);
                    }
                }
            }

            for(int i = 0; i < pdataAllPlane1[0].size() - Z_shift_MIP; i ++)
            {
                calculate_2Img_overlap_shift(pdata_CPlane1[c][i], pdata_CPlane2[c][i], orientation, XY_overlap_plane[c][i], XY_shift_plane[c][i], i, c);
            }
        }

    }
}

void calculate_2Img_overlap_shift(Mat pdata_CP1, Mat pdata_CP2, QString orientation, int &XY_overlap_p, int &XY_shift_p, int num, int c)
{
    cout<<"Calculate the overlap and shift between the "<<num+1<<" pairs for "<<c+1<<" channel."<<endl;

    const int overlap_range = 200, shift_range = 20;
    double sum[overlap_range * shift_range] = {0};
    int difference[overlap_range * shift_range] = {0};
    int count[overlap_range * shift_range] = {0};
    double minError = INT_MAX;

    if(orientation == "left" || orientation == "right")
    {
        for(int ss1 = 50; ss1 < 50+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-50)+ss2;
                for(int row = 0; row < pdata_CP1.rows-ss2; row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff;
                        if(orientation == "left")
                            diff = pdata_CP1.at<unsigned short>(row, pdata_CP1.cols-ss1+col)-pdata_CP2.at<unsigned short>(row+ss2, col);
                        else
                            diff = pdata_CP2.at<unsigned short>(row, pdata_CP2.cols-ss1+col)-pdata_CP2.at<unsigned short>(row+ss2, col);
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap_p = ss1;
                    XY_shift_p = ss2;
                }
            }
        }
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int ss1 = 10; ss1 < 10+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-10)+ss2;
                for(int row = 0; row < ss1; row ++)
                {
                    for(int col = 0; col < pdata_CP1.cols-ss2; col ++)
                    {
                        int diff = pdata_CP1.at<unsigned short>(row+pdata_CP1.rows-ss1+row, ss2+col)-pdata_CP2.at<unsigned short>(row, col);
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap_p = ss1;
                    XY_shift_p = ss2;
                }
            }
        }
    }

}

void calculate_expansion_ratio(vector<vector<int>> XY_overlap_plane, vector<vector<int>> XY_shift_plane, vector<vector<double>> &overlap_shift_Ratio, int Z_shift_MIP, QString orientation, int rows, int cols)
{
    cout<<"Calculae expansion ratio!"<<endl;
    for(int c = 0; c < XY_overlap_plane.size(); c ++)
    {
        if(orientation == "left" || orientation == "right")
        {
            double expansion_para_1 = sqrt(pow((cols-XY_overlap_plane[c][0]), 2) + pow(XY_shift_plane[c][0], 2) + 0.0);
            for(int i = 0; i < XY_overlap_plane[0].size()-Z_shift_MIP; i ++)
            {
                //overlap_shift_Ratio[c][i] = sqrt((pow((1880-XY_overlap[c][i]), 2) + pow(XY_shift[c][i], 2) + 0.0)/(pow((1880-XY_overlap[c][i+1]), 2) + pow(XY_shift[c][i+1], 2) + 0.0));
                double expansion_para_i = sqrt(pow((cols-XY_overlap_plane[c][i]), 2) + pow(XY_shift_plane[c][i], 2) + 0.0);
                overlap_shift_Ratio[c][i] = expansion_para_i/expansion_para_1;
            }
        }
        else if(orientation == "up" || orientation == "down")
        {
            double expansion_para_1 = sqrt(pow((rows-XY_overlap_plane[c][0]), 2) + pow(XY_shift_plane[c][0], 2) + 0.0);
            for(int i = 0; i < XY_overlap_plane[0].size()-Z_shift_MIP; i ++)
            {
                //overlap_shift_Ratio[c][i] = sqrt((pow((1880-XY_overlap[c][i]), 2) + pow(XY_shift[c][i], 2) + 0.0)/(pow((1880-XY_overlap[c][i+1]), 2) + pow(XY_shift[c][i+1], 2) + 0.0));
                double expansion_para_i = sqrt(pow((rows-XY_overlap_plane[c][i]), 2) + pow(XY_shift_plane[c][i], 2) + 0.0);
                overlap_shift_Ratio[c][i] = expansion_para_i/expansion_para_1;
            }
        }
    }
}

void save_tiles_shift(QString SaveFolder, string s1, string s2, QString orientation, vector<int> XY_shift_MIP_vec, vector<int> Z_shift_MIP_vec, vector<vector<int>> XY_overlap_plane, vector<vector<int>> XY_shift_plane, vector<vector<double>> overlap_shift_Ratio, int Z_shift)
{

    vector<int> shift_XY=XY_shift_MIP_vec;
    vector<int> shift_Z=Z_shift_MIP_vec;
    vector<vector<int>> XY_overlap =XY_overlap_plane;
    vector<vector<int>> XY_shift =XY_shift_plane;
    int num_channel = XY_overlap_plane.size();
    int Z_plane = XY_overlap_plane[0].size();

    string abc;
    QString excelName = setExcelName(SaveFolder, s1, s2, orientation);
    QAxObject *excel = new QAxObject();
    excel -> setControl("Excel.Application"); // connect EXCEL control
    excel -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            excelName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "MIP_shift");
    QAxObject *worksheet2 = appendSheet(worksheets, "XY_shift");

    QList <QList<QVariant>> datas_sheet1;
    QList <QList<QVariant>> datas_sheet2;

    string sname1 = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname1);
    for(int i = 0 ; i < num_channel+1; i ++)
    {
        QList<QVariant> rows1;
        if(i == 0)
        {
            rows1.append(qname1);
            if(orientation == "left" || orientation == "right")
                rows1.append("Y");
            else if(orientation == "up" || orientation == "down")
                rows1.append("X");
            rows1.append("Z");
        }
        else
        {
            QString name = "C" + QString :: fromStdString(to_string((i)));
            rows1.append(name);
            rows1.append(shift_XY[i-1]);
            rows1.append(shift_Z[i-1]);
        }
        datas_sheet1.append(rows1);
    }

    for(int i = 0; i < Z_plane - Z_shift + 2; i ++)
    {
        QList<QVariant> rows2;
        if(i == 0)
        {
            String s,b="a";
            rows2.append(qname1);
            s[0]='C';
            b[0]=s[0]+num_channel;
            QString merge_cell1 = QString::fromStdString(s+":"+b+"1");
            QAxObject *merge_range1 = worksheet2->querySubObject("Range(const QString&)", merge_cell1);
            merge_range1->setProperty("MergeCells", true);

            s[0]=b[0]++;
            b[0]=s[0]+num_channel;
            QString merge_cell2 = QString::fromStdString(s+":"+b+"1");
            QAxObject *merge_range2 = worksheet2->querySubObject("Range(const QString&)", merge_cell2);
            merge_range2->setProperty("MergeCells", true);

            s[0]=b[0]+1;
            b[0]=s[0]+num_channel;
            QString merge_cell3 = QString::fromStdString(s+":"+b+"1");
            QAxObject *merge_range3 = worksheet2->querySubObject("Range(const QString&)", merge_cell3);
            merge_range3->setProperty("MergeCells", true);

            abc=b;

            rows2.append("Z");
            rows2.append("Overlap");
            for(int ii=0;ii<num_channel+1;ii++)
            {
             rows2.append("");
            }
            rows2.append("Shift");
            for(int ii=0;ii<num_channel+1;ii++)
            {
             rows2.append("");
            }
            rows2.append("Expansion_Ratio");
            for(int ii=0;ii<num_channel+1;ii++)
            {
             rows2.append("");
            }
        }
        else if(i == 1)
        {
            rows2.append("");
            rows2.append("");
            for(int ii =1;ii< num_channel+1;ii++){
                string s="0";
                s[0]='0'+ii;
              rows2.append(QString::fromStdString("C"+s));            }
            rows2.append("Average");
            rows2.append("");
            for(int ii =1;ii< num_channel+1;ii++){
                string s="0";
                s[0]='0'+ii;
              rows2.append(QString::fromStdString("C"+s));
            }
            rows2.append("Average");
            rows2.append("");
            for(int ii =1;ii< num_channel+1;ii++){
                string s="0";
                s[0]='0'+ii;
              rows2.append(QString::fromStdString("C"+s));
            }
            rows2.append("Average");
        }

        else
        {
            int index1;
            int index2;
            if(orientation == "left")
            {
                index1 = i+Z_shift-2;
                index2 = i-2;
            }
            else if(orientation == "right" || orientation == "up")
            {
                index1 = i-2;
                index2 = i+Z_shift-2;
            }

            string s1 = to_string(index1)+"_"+to_string(index2);
            QString qs1 = QString ::fromStdString(s1);
            rows2.append(qs1);

            rows2.append(i-1);
            for(int ii =0;i< num_channel;i++){
                rows2.append(XY_overlap[ii][i-2]);
            }

            rows2.append("");
            int sum=0;
            for(int iii=0;iii<num_channel;iii++){
                sum=sum+XY_overlap[iii][i-2];
            }
            double ave1 = (sum+0.0)/num_channel;
            rows2.append(ave1);
            rows2.append("");

            for(int ii =0;i< num_channel;i++){
                rows2.append(XY_shift[ii][i-2]);
            }

            rows2.append("");
            sum=0;
            for(int iii=0;iii<num_channel;iii++){
                sum=sum+XY_shift[iii][i-2];
            }
            double ave2 = (sum+0.0)/num_channel;
            rows2.append(ave2);

            rows2.append("");

            for(int ii =0;i< num_channel;i++){
                rows2.append(overlap_shift_Ratio[ii][i-2]);
            }

            rows2.append("");
            rows2.append("");
            //double ave3 = (overlap_shift_Ratio[0][i-2]+overlap_shift_Ratio[1][i-2]+overlap_shift_Ratio[2][i-2]+0.0)/3;
            //rows2.append(ave3);
        }
        datas_sheet2.append(rows2);
    }

    QList<QVariant> vars1, vars2;
    for(auto v:datas_sheet1)
    {
        vars1.append(QVariant(v));
    }
    for(auto v:datas_sheet2)
    {
        vars2.append(QVariant(v));
    }
    QVariant var1 = QVariant(vars1);
    QVariant var2 = QVariant(vars2);


    QString range1 = "A1:C"+QString :: fromStdString(to_string(num_channel));
    cout<<"excel range: "<<range1.toStdString()<<endl;
    QAxObject *excel_property1 = worksheet1 -> querySubObject("Range(const QString&)", range1);
    excel_property1 -> setProperty("Value", var1);
    excel_property1 -> setProperty("HorizontalAlignment", -4108);

    QString num = QString :: fromStdString(to_string(XY_shift_plane[0].size()-Z_shift));
    QString range2 = "A1:"+QString::fromStdString(abc)+num;
    cout<<"excel range: "<<range2.toStdString()<<endl;
    QAxObject *excel_property2 = worksheet2 -> querySubObject("Range(const QString&)", range2);
    excel_property2 -> setProperty("Value", var2);
    excel_property2 -> setProperty("HorizontalAlignment", -4108);

    cout<<"excelName: "<<excelName.toStdString().c_str()<<endl;
    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;
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

QString setExcelName(QString saveFolder, string s1, string s2, QString orientation)
{
    string ss = "T_"+s1+"_"+s2+"_";
    QString qstr;
    if(orientation == "left" || orientation == "right")
        qstr = QString :: fromStdString(ss) + "YZ_XY_shift.xlsx";
    else if(orientation == "up" || orientation == "down")
        qstr = QString :: fromStdString(ss) + "XZ_XY_shift.xlsx";
    cout<<__LINE__<<": excelName = "<<qstr.toStdString().c_str()<<endl;
    QString fullName = saveFolder + "\\" + qstr;
    return fullName;
}

bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2)
{
    cout<<"s1 = "<<s1.c_str()<<endl;
    cout<<"s2 = "<<s2.c_str()<<endl;
    int num11 = std::stoi(s1);
    int num22 = std::stoi(s2);
    cout<<"num1 = "<<num11<<endl;
    cout<<"num2 = "<<num22<<endl;
    if(num11<0 || num11>28 ||num22<0 || num22>28 || num11 == num22)
        return 0;
    int flag = (num11>num22)?1:0;
    int num1 = min(num11, num22);
    int num2 = max(num11, num22);

    if(((num1-1)/5 == (num2-1)/5) && ((num1-1)/5%2 == 0) && (num1+1 == num2))
    {
        orientation1 = "left";
        orientation2 = "right";
    }

    else if(((num1-1)/5 == (num2-1)/5) && ((num1-1)/5%2 == 1) && (num1+1 == num2))
    {
        orientation1 = "right";
        orientation2 = "left";
    }

    else if(((num1-1)/5+1 == (num2-1)/5) && (num1-1)/5<4)
    {
        if(((num1%5 == 1)&&(num2 = num1+9)) || ((num1%5 == 2)&&(num2 = num1+7)) || ((num1%5 == 3)&&(num2 = num1+5))
                || ((num1%5 == 4)&&(num2 = num1+3)) || ((num1%5 == 0)&&(num2 = num1+1)))
        {
            orientation1 = "up";
            orientation2 = "down";
        }
    }

    else if(((num1 == 22)&&(num2 == 28)) || ((num1 == 23)&&(num2 == 27))||((num1 == 24)&&(num2 == 26)))
    {
        orientation1 = "up";
        orientation2 = "down";
    }

    else
        return 0;

    if(flag == 1)
    {
        QString temp = orientation1;
        orientation1 = orientation2;
        orientation2 = temp;
    }

    return 1;
}
