#include "data_training.h"
#include <iostream>
#include<stdio.h>
#include<math.h>
#include<ostream>
///////////////////////////////////线性
//parameter list
#define tol 0.001
#define tolJ 0.00001
#define maxcirl 50
#define max 10000
#define dem 2
#define C 0.6
#define file_drec "E:/vs_workplace/SVM/testSet.txt"
#define result_dec "E:/vs_workplace/SVM/result_alpha_b_5.txt"
#define result_classify  "E:/vs_workplace/SVM/result_classify_5.txt"
#define ifKernel  1
#define d 0.2

using namespace std;

struct elemeter
{
    double Error[max][2];
    double alpha[max];
    double b;
    double X[max][dem];
    double Y[max];
    int length;
    int x_dem;
}Elemter;
double getE(int i);
void UpdataEk(int k)
{
    double Ek = getE(k);
    Elemter.Error[k][0] = 1;
    Elemter.Error[k][1] = Ek;
}
double dot_product(int length,double x1[],double x2[])
{
    double result = 0;
    if(!ifKernel)
    {
        for(int i = 0;i<length;i++)
           result += x1[i]*x2[i];
    }
    else
    {
        for(int i = 0;i<length;i++)
            result += (x1[i] - x2[i]) * (x1[i] - x2[i]);
        result = exp(-result / (2 * d * d));
    }
    return result;
}
double getE(int i)
{
    int length = Elemter.length;
    int x_dem = Elemter.x_dem;
    double re = 0;
    for(int k = 0;k<length;k++)
    {
        re += Elemter.Y[k] * Elemter.alpha[k] * dot_product(x_dem,Elemter.X[i],Elemter.X[k]);
    }
    re = re + Elemter.b - Elemter.Y[i];
    return re;
}
void  SelectJ(int *j,double *Ej,int i,double Ei)
{
    Elemter.Error[i][0] = 1;
    Elemter.Error[i][1] = Ei;
    double maxE = 0;
    int length = Elemter.length;
    int flag = 0;
    for(int k = 0;k<length;k++)
    {
        if(Elemter.Error[k][0] == 1 && Elemter.Error[k][1] != 0)
        {
            if(k == i)
            continue;
            double Ek = getE(k);
            if(maxE < abs(Ek - Ei))
            {
                maxE = abs(Ek - Ei);
                *Ej = Ek;
                *j = k;
                flag = 1;
             }
        }
    }
    if(flag == 0)
    {
        *j = rand()%Elemter.length ;
        *Ej = getE(*j);
    }
}
double Clipalphaj(double L,double H,double alphaj)
{
    if(alphaj < L)
        return L;
    if(alphaj > H)
        return H;
    else
        return alphaj;
}

int Calu(int i)
{
    double Ei = getE(i);
    if(Elemter.Y[i] * Ei < -tol && Elemter.alpha[i] < C
        || Elemter.Y[i] * Ei > tol && Elemter.alpha[i] > 0)
    {
        int j;
        double Ej;
        SelectJ(&j,&Ej,i,Ei);
        double oldalphai = Elemter.alpha[i];
        double oldalphaj = Elemter.alpha[j];
        double L,H;
        if(Elemter.Y[i] != Elemter.Y[j])
        {
            L = (oldalphaj - oldalphai > 0)?(oldalphaj - oldalphai) : 0;
            H = (C - oldalphai + oldalphaj > C)? C : (C - oldalphai + oldalphaj);
        }
        else
        {
            L = (oldalphai + oldalphaj - C > 0)?(oldalphai + oldalphaj - C) : 0;
            H = (oldalphai + oldalphaj > C)? C : (oldalphai + oldalphaj);
        }
        if(L == H)
            return 0;
        double delta = dot_product(Elemter.x_dem,Elemter.X[i],Elemter.X[i])
            + dot_product(Elemter.x_dem,Elemter.X[j],Elemter.X[j])
            - 2 * dot_product(Elemter.x_dem,Elemter.X[i],Elemter.X[j]);
        if(delta <= 0)
            return 0;
        Elemter.alpha[j] = oldalphaj + Elemter.Y[j] * (Ei - Ej) / delta;
        Elemter.alpha[j] = Clipalphaj(L,H,Elemter.alpha[j]);
        UpdataEk(j);
        if(abs(Elemter.alpha[j] - oldalphaj) < tolJ)
            return 0;
        Elemter.alpha[i]  = oldalphai + Elemter.Y[i] * Elemter.Y[j] * (oldalphaj - Elemter.alpha[j]);
        UpdataEk(i);
        double b1 = Elemter.b - Ei - Elemter.Y[i] * (Elemter.alpha[i] - oldalphai) * dot_product(Elemter.x_dem,Elemter.X[i],Elemter.X[i])
            - Elemter.Y[j] * (Elemter.alpha[j] - oldalphaj) * dot_product(Elemter.x_dem,Elemter.X[i],Elemter.X[j]);
        double b2 = Elemter.b - Ej - Elemter.Y[i] * (Elemter.alpha[i] - oldalphai) * dot_product(Elemter.x_dem,Elemter.X[i],Elemter.X[j])
            - Elemter.Y[j] * (Elemter.alpha[j] - oldalphaj) * dot_product(Elemter.x_dem,Elemter.X[j],Elemter.X[j]);
        if(Elemter.alpha[i] > 0 && Elemter.alpha[i] < C)
            Elemter.b = b1;
        else if(Elemter.alpha[j] > 0 && Elemter.alpha[j] < C)
            Elemter.b = b2;
        else
            Elemter.b = (b1 + b2) / 2.0;
        return 1;
    }
    return 0;
}
class SMO{
public:
    void getdatasAndinitial(FILE *file);
    void calu();
    void writeresult(FILE *file);
};
void SMO::getdatasAndinitial(FILE *file)
{
    int i;
    for( i = 0;;i++)
    {
        double e;
        if(fscanf(file,"%lf",&e) == EOF)
            break;
        Elemter.X[i][0] = e;
        for(int j = 1;j<dem;j++)
        {
            fscanf(file,"%lf",&Elemter.X[i][j]);
        }
        fscanf(file,"%lf",&Elemter.Y[i]);
    }

    Elemter.length = i;
    Elemter.x_dem = dem;
    for(int i = 0;i<Elemter.length;i++)
    {
        Elemter.alpha[i] = 0;
        Elemter.Error[i][0] = 0;
        Elemter.Error[i][1] = 0;
    }
    Elemter.b = 0;
}
void SMO::calu()
{
    int iter = 0;
    bool entireSet = true;
    int alphaPairsChanged = 0;
    while((entireSet < maxcirl) && (alphaPairsChanged > 0 || entireSet))
    {
        alphaPairsChanged = 0;
        if(entireSet)
        {
            for(int i = 0;i<Elemter.length;i++)
            {
                alphaPairsChanged += Calu(i);
            }
            iter++;
        }
        else
        {
            for(int i = 0;i<Elemter.length;i++)
            {
                if(Elemter.alpha[i] > 0 && Elemter.alpha[i] < C)
                    alphaPairsChanged += Calu(i);
            }
            iter++;
        }
        if(entireSet)
            entireSet = false;
        else if(alphaPairsChanged == 0)
            entireSet = true;
    }
}
void SMO::writeresult(FILE *file)
{

    for(int i = 0;i<Elemter.length;i++)
        fprintf(file,"%lf\n",Elemter.alpha[i]);
    fprintf(file,"%lf",Elemter.b);
}
double function(int i)
{
    int length = Elemter.length;
    int x_dem = Elemter.x_dem;
    double re = 0;
    for(int k = 0;k<length;k++)
        re += Elemter.Y[k] * Elemter.alpha[k] * dot_product(x_dem,Elemter.X[i],Elemter.X[k]);
    //printf("%lf\n",re);
    re = re + Elemter.b;

    return re;
}
class Test{
public:
    void getresult(FILE *file)
    {
        int right_num = 0;
        for(int i = 0;i<Elemter.length;i++)
        {
            fprintf(file,"The expected label is %lf    ",Elemter.Y[i]);
            double re = function(i);
            fprintf(file,"The output lable is %lf   ",(re>0) ? 1.0 : -1.0);
            fprintf(file,"re : %lf\n",re);
            if(re * Elemter.Y[i] > 0)
            {
                right_num++;
            }
        }
        printf("The correct rate is %lf%%\n",100.0 * right_num / Elemter.length);
    }
};

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

    vector<double> y_n,y_n2;
    vector<double> overlap_level,overlap_level2;
    vector<double> ratio_v,ratio_v2;
    vector<double> count_v,count_v2;
    vector<double> D,D2;
    vector<double> grey_mean,grey_mean2;
    vector<double> grey_std,grey_std2;


    feature_calculate(y_n,overlap_level,ratio_v,count_v,D,grey_mean,grey_std,coor_v);
    //feature_calculate(y_n2,overlap_level2,ratio_v2,count_v2,D2,grey_mean2,grey_std2,coor_v2);
    Feature feature;
    Chart chart;
    feature.y_n = y_n;
    feature.overlap_level = overlap_level;
    feature.ratio_v = ratio_v;
    feature.count_v = count_v;
    feature.D = D;
    feature.grey_mean = grey_mean;
    feature.grey_std = grey_std;
    Cov_calculate(chart,feature);
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
        cout<<"sum = "<<sum<<endl;
        cout<<"mean1 = "<<mean1<<endl;
        cout<<"mean2 = "<<mean2<<endl;
        cout<<"v1[i] = "<<v1[i]<<endl;
        cout<<"v2[i] = "<<v2[i]<<endl;
    }
    //cout<<""
    double mean_two = sum/(v1.size()-1);
    return mean_two;

}
bool Cov_calculate(Chart &chart,Feature &feature)
{
    chart.fifth_line.x1 = get_D(feature.y_n,feature.y_n);
    chart.fifth_line.x2 = get_D(feature.y_n,feature.ratio_v);
    chart.fifth_line.x3 = get_D(feature.y_n,feature.overlap_level);
    chart.fifth_line.x4 = get_D(feature.y_n,feature.D);
    chart.fifth_line.x5 = get_D(feature.y_n,feature.count_v);

    chart.second_line.x1 = get_D(feature.ratio_v,feature.y_n);
    chart.second_line.x2 = get_D(feature.ratio_v,feature.ratio_v);
    chart.second_line.x3 = get_D(feature.ratio_v,feature.overlap_level);
    chart.second_line.x4 = get_D(feature.ratio_v,feature.D);
    chart.second_line.x5 = get_D(feature.ratio_v,feature.count_v);

    chart.third_line.x1 = get_D(feature.overlap_level,feature.y_n);
    chart.third_line.x2 = get_D(feature.overlap_level,feature.ratio_v);
    chart.third_line.x3 = get_D(feature.overlap_level,feature.overlap_level);
    chart.third_line.x4 = get_D(feature.overlap_level,feature.D);
    chart.third_line.x5 = get_D(feature.overlap_level,feature.count_v);

    chart.forth_line.x1 = get_D(feature.D,feature.y_n);
    chart.forth_line.x2 = get_D(feature.D,feature.ratio_v);
    chart.forth_line.x3 = get_D(feature.D,feature.overlap_level);
    chart.forth_line.x4 = get_D(feature.D,feature.D);
    chart.forth_line.x5 = get_D(feature.D,feature.count_v);

    chart.fifth_line.x1 = get_D(feature.count_v,feature.y_n);
    chart.fifth_line.x2 = get_D(feature.count_v,feature.ratio_v);
    chart.fifth_line.x3 = get_D(feature.count_v,feature.overlap_level);
    chart.fifth_line.x4 = get_D(feature.count_v,feature.D);
    chart.fifth_line.x5 = get_D(feature.count_v,feature.count_v);
    cout<<chart.fifth_line.x1<<"  "<<chart.fifth_line.x2<<"  "<<chart.fifth_line.x3<<"  "<<chart.fifth_line.x4<<"  "<<chart.fifth_line.x5<<"  "<<endl;
    cout<<chart.second_line.x1<<"  "<<chart.second_line.x2<<"  "<<chart.second_line.x3<<"  "<<chart.second_line.x4<<"  "<<chart.second_line.x5<<"  "<<endl;
    return 0;

}
