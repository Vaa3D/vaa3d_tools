#include "data_training.h"

#include <iostream>
//#include <random>
//#include <chrono>
//#include <map>
//#include <iomanip>
using namespace std;
//uint qHash(const Coordinate key)
//{
//    return key.x + key.y + key.z;
//}
//uint qHash(const vector<Coordinate> key)
//{
//    return key[0].x + key[0].y + key[0].z;
//}
bool data_training(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    char * folder_path1;
    char * folder_path2;
    if(inlist->size() == 2)
    {
        folder_path1 = inlist->at(0);
        folder_path2 = inlist->at(1);
    }
    QStringList folderList1 = importFileList_addnumbersort(QString(folder_path1));
    QStringList folderList2 = importFileList_addnumbersort(QString(folder_path2));
    QStringList fileList1,fileList2;
    vector<Coordinate> tmp,tmp2;
    vector<vector<Coordinate> > coor_v,coor_v2;
    //first folderlist1
    for(int i=2;i<folderList1.size();i++)
    {
        fileList1 = importFileList_addnumbersort(QString(folderList1[i]));
        for(int i=2;i<fileList1.size();i++)
        {
            tmp.clear();
            if (fileList1[i].toUpper().endsWith(".TXT"))
            {

                tmp = readtxt_LXF(fileList1[i]);
            }
            if(tmp.size()==0)continue;
            coor_v.push_back(tmp);
        }
    }
    //second folderlist2
    for(int i=2;i<folderList2.size();i++)
    {
        fileList2 = importFileList_addnumbersort(QString(folderList2[i]));
        for(int i=2;i<fileList2.size();i++)
        {
            tmp2.clear();
            if (fileList2[i].toUpper().endsWith(".TXT"))
            {

                tmp2 = readtxt_LXF(fileList2[i]);
            }
            if(tmp2.size()==0)continue;
            coor_v2.push_back(tmp2);
        }
    }

    vector<bool> y_n,y_n2;
    vector<double> overlap_level,overlap_level2;
    vector<double> ratio_v,ratio_v2;
    vector<double> count_v,count_v2;
    vector<double> D,D2;
    vector<double> grey_mean,grey_mean2;
    vector<double> grey_std,grey_std2;


    feature_calculate(y_n,overlap_level,ratio_v,count_v,D,grey_mean,grey_std,coor_v);
    //feature_calculate(y_n2,overlap_level2,ratio_v2,count_v2,D2,grey_mean2,grey_std2,coor_v2);
    Feature feature;
    feature.y_n = y_n;
    feature.overlap_level = overlap_level;
    feature.ratio_v = ratio_v;
    feature.count_v = count_v;
    feature.D = D;
    feature.grey_mean = grey_mean;
    feature.grey_std = grey_std;
}



vector<Coordinate> readtxt_LXF(const QString& filename)
{
    vector<Coordinate> tmp_list;


    QFile qf(filename);
    if (! qf.open(QIODevice::ReadOnly | QIODevice::Text))
    {
#ifndef DISABLE_V3D_MSG
        v3d_msg(QString("open file [%1] failed!").arg(filename));
#endif
        return tmp_list;
    }

    V3DLONG k=0;
    while (! qf.atEnd())
    {
        char curline[2000];
        qf.readLine(curline, sizeof(curline));
        k++;
        {
            //if (curline[0]=='#' || curline[0]=='x' || curline[0]=='X' || curline[0]=='\0') continue;

            QStringList qsl = QString(curline).trimmed().split("    ");
            int qsl_count=qsl.size();
            if (qsl_count<4)   continue;

            Coordinate S;

            S.x = qsl[0].toFloat();
            S.y = qsl[1].toFloat();
            S.z = qsl[2].toFloat();
            S.bri = qsl[3].toFloat();
            S.name = filename;
            tmp_list.push_back(S);
        }
    }

    return tmp_list;
}
double get_D(vector<double> &v1,vector<double> &v2)//v1,v2 with the same size
{
    double sum=0;
    double mean1,mean2;
    for(int i=0;i<v1.size();i++)
    {
        sum = sum + v1[i];
    }
    mean1 = sum/v1.size();
    sum=0;
    for(int i=0;i<v2.size();i++)
    {
        sum = sum + v2[i];
    }
    mean2 = sum/v2.size();
    sum=0;
    for(int i=0;i<v1.size();i++)
    {
        sum = (v1[i]-mean1)*(v2[i]-mean2) + sum;
    }
    double mean_two = sum/(v1.size()-1);
    return mean_two;

}
bool Cov_calculate(Chart &chart,Feature &feature)
{

}
