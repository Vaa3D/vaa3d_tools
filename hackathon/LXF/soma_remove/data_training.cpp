#include "data_training.h"
#include <iostream>
#include<stdio.h>
#include<math.h>
#include<ostream>
#include "QR.h"
#include "SVD.h"
#include "matrix.h"
#include<math.h>
#include "soma_remove_main.h"
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

#define N 5    //测试矩阵维数定义
vector<double> classify_glio_Y(vector<double> &S,Feature &f,Each_line &E,vector<vector<double> > &R,double &sum_log);
#define NTDIS(a,b) (sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z)))
vector<bool> classify_glio2(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method);
bool GetMatrixInverse(vector<vector<double> > &V1,int n,vector<vector<double> > &V2);
void  getAStart(vector<vector<double> > &V1,int n,vector<vector<double> > &V2);
vector<vector<double> > matrix_multiply(vector<vector<double> > &arrA, vector<vector<double> > &arrB);
double getA(vector<vector<double> > &V1,int n);
vector<Coordinate> readtxt_LXF(const QString& filename,const string &inf1);
int size_all;
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


void Split(const string &s,vector<string> &v,const string &c)
{
    string::size_type pos1,pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1,pos2-pos1));
        pos1 = pos2 + c.size();
        pos2 = s.find(c,pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}


//按第一行展开计算|A|
double getA(double arcs[N][N],int n)
{
    if(n==1)
    {
        return arcs[0][0];
    }
    double ans = 0;
    double temp[N][N]={0.0};
    int i,j,k;
    for(i=0;i<n;i++)
    {
        for(j=0;j<n-1;j++)
        {
            for(k=0;k<n-1;k++)
            {
                temp[j][k] = arcs[j+1][(k>=i)?k+1:k];

            }
        }
        double t = getA(temp,n-1);
        if(i%2==0)
        {
            ans += arcs[0][i]*t;
        }
        else
        {
            ans -=  arcs[0][i]*t;
        }
    }
    return ans;
}


void  getAStart(double arcs[N][N],int n,double ans[N][N])
{
    if(n==1)
    {
        ans[0][0] = 1;
        return;
    }
    int i,j,k,t;
    double temp[N][N];
    for(i=0;i<n;i++)
    {
        for(j=0;j<n;j++)
        {
            for(k=0;k<n-1;k++)
            {
                for(t=0;t<n-1;t++)
                {
                    temp[k][t] = arcs[k>=i?k+1:k][t>=j?t+1:t];
                }
            }


            ans[j][i]  =  getA(temp,n-1);  //此处顺便进行了转置
            if((i+j)%2 == 1)
            {
                ans[j][i] = - ans[j][i];
            }
        }
    }
}

bool GetMatrixInverse(double src[N][N],int n,double des[N][N])
{
    double flag=getA(src,n);
    double t[N][N];
    if(0==flag)
    {
        cout<< "原矩阵行列式为0，无法求逆。请重新运行" <<endl;
        return false;//如果算出矩阵的行列式为0，则不往下进行
    }
    else
    {
        getAStart(src,n,t);
        for(int i=0;i<n;i++)
        {
            for(int j=0;j<n;j++)
            {
                des[i][j]=t[i][j]/flag;
            }

        }
    }

    return true;
}









bool data_training(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback)
{
    vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
    vector<char*>* outlist = NULL;
    vector<char*>* paralist = NULL;
    char * folder_path1;
    char * folder_path2;
    char * folder_path3;
    char * folder_path4;
    if(inlist->size() == 4)
    {
        folder_path1 = inlist->at(0);
        folder_path2 = inlist->at(1);
        folder_path3 = inlist->at(2);
        folder_path4 = inlist->at(3);
    }
    else
    {
        return false;
    }
    QStringList folderList1 = importFileList_addnumbersort(QString(folder_path1));
    QStringList folderList2 = importFileList_addnumbersort(QString(folder_path2));
    QStringList predictList1 = importFileList_addnumbersort(QString(folder_path3));
    QStringList predictList2 = importFileList_addnumbersort(QString(folder_path4));
    QStringList fileList1,fileList2,predict1,predict2;
    vector<Coordinate> tmp,tmp2,tmp3,tmp4;
    vector<vector<Coordinate> > coor_v,coor_v2,coor_v3,coor_v4;
    //first folderlist1
    for(int i=2;i<folderList1.size();i++)
    {
        fileList1 = importFileList_addnumbersort(QString(folderList1[i]));
        vector<string> v1;
        Split(folderList1[i].toStdString(),v1,"/");
        cout<<"v1 = "<<v1[7]<<endl;
        //v3d_msg("kkk");
        for(int i=2;i<fileList1.size();i++)
        {
            tmp.clear();
            if (fileList1[i].toUpper().endsWith(".TXT"))
            {

                tmp = readtxt_LXF(fileList1[i],v1[v1.size()-1]);
            }
            if(tmp.size()==0)continue;
            coor_v.push_back(tmp);
        }
    }
    //second folderlist2
    for(int i=2;i<folderList2.size();i++)
    {
        fileList2 = importFileList_addnumbersort(QString(folderList2[i]));
        vector<string> v2;
        Split(folderList2[i].toStdString(),v2,"/");
        for(int i=2;i<fileList2.size();i++)
        {
            tmp2.clear();
            if (fileList2[i].toUpper().endsWith(".TXT"))
            {

                tmp2 = readtxt_LXF(fileList2[i],v2[v2.size()-1]);
            }
            if(tmp2.size()==0)continue;
            coor_v2.push_back(tmp2);
        }
    }

    for(int i=2;i<predictList1.size();i++)
    {
        predict1 = importFileList_addnumbersort(QString(predictList1[i]));
        vector<string> v3;
        Split(predictList1[i].toStdString(),v3,"/");
        for(int i=2;i<predict1.size();i++)
        {
            tmp3.clear();
            if (predict1[i].toUpper().endsWith(".TXT"))
            {

                tmp3 = readtxt_LXF(predict1[i],v3[v3.size()-1]);
            }
            if(tmp3.size()==0)continue;
            coor_v3.push_back(tmp3);
        }

    }
    for(int i=2;i<predictList2.size();i++)
    {
        predict2 = importFileList_addnumbersort(QString(predictList2[i]));
        vector<string> v4;
        Split(predictList2[i].toStdString(),v4,"/");
        for(int i=2;i<predict2.size();i++)
        {
            tmp4.clear();
            if (predict2[i].toUpper().endsWith(".TXT"))
            {

                tmp4 = readtxt_LXF(predict2[i],v4[v4.size()-1]);
            }
            if(tmp4.size()==0)continue;
            coor_v4.push_back(tmp4);
        }

    }
//cout<<tmp[0].inf1<<endl;
//v3d_msg("kkk");
    vector<inf> inf_v,inf_v2,inf_v3,inf_v4;
    vector<double> y_n,y_n2,y_n3,y_n4;
    vector<double> overlap_level,overlap_level2,overlap_level3,overlap_level4;
    vector<double> ratio_v,ratio_v2,ratio_v3,ratio_v4;
    vector<double> count_v,count_v2,count_v3,count_v4;
    vector<double> D,D2,D3,D4;
    vector<double> grey_mean,grey_mean2,grey_mean3,grey_mean4;
    vector<double> grey_std,grey_std2,grey_std3,grey_std4;


    feature_calculate(inf_v,y_n,overlap_level,ratio_v,count_v,D,grey_mean,grey_std,coor_v);
    feature_calculate(inf_v2,y_n2,overlap_level2,ratio_v2,count_v2,D2,grey_mean2,grey_std2,coor_v2);

    feature_calculate(inf_v3,y_n3,overlap_level3,ratio_v3,count_v3,D3,grey_mean3,grey_std3,coor_v3);
    feature_calculate(inf_v4,y_n4,overlap_level4,ratio_v4,count_v4,D3,grey_mean4,grey_std4,coor_v4);

    Feature feature,feature2,feature3,feature4;
    Chart chart,chart2,chart3,chart4;
    Each_line E,E2,E3,E4;
    feature.y_n = y_n;
    feature.overlap_level = overlap_level;
    feature.ratio_v = ratio_v;
    feature.count_v = count_v;
    feature.D = D;
    feature.grey_mean = grey_mean;
    feature.grey_std = grey_std;

    feature2.y_n = y_n2;
    feature2.overlap_level = overlap_level2;
    feature2.ratio_v = ratio_v2;
    feature2.count_v = count_v2;
    feature2.D = D2;
    feature2.grey_mean = grey_mean2;
    feature2.grey_std = grey_std2;

    feature3.y_n = y_n3;
    feature3.overlap_level = overlap_level3;
    feature3.ratio_v = ratio_v3;
    feature3.count_v = count_v3;
    feature3.D = D3;
    feature3.grey_mean = grey_mean3;
    feature3.grey_std = grey_std3;

    feature4.y_n = y_n4;
    feature4.overlap_level = overlap_level4;
    feature4.ratio_v = ratio_v4;
    feature4.count_v = count_v4;
    feature4.D = D4;
    feature4.grey_mean = grey_mean4;
    feature4.grey_std = grey_std4;

    Cov_calculate(chart,feature);
    Cov_calculate(chart2,feature2);
    E = E_calculate(feature);
    E2 = E_calculate(feature2);
    E3 = E_calculate(feature3);
    E4 = E_calculate(feature4);

    QString fileSaveName = "glio.txt";
    QString fileSaveName2 = "signal.txt";
    export_TXT(E,chart,fileSaveName);
    export_TXT(E2,chart2,fileSaveName2);
    int K = y_n.size();
    int U = y_n2.size();
    vector<vector<float> >A,A2;
    for(int i=0;i<K;i++)
    {
        vector<float> a;
        a.push_back(feature.y_n[i]-E.x1);
        a.push_back(feature.ratio_v[i]-E.x2);
        a.push_back(feature.overlap_level[i]-E.x3);
        a.push_back(feature.grey_std[i]-E.x4);
        a.push_back(feature.count_v[i]-E.x5);
        A.push_back(a);

    }
    for(int i=0;i<U;i++)
    {
        vector<float> a;
        a.push_back(feature2.y_n[i]-E2.x1);
        a.push_back(feature2.ratio_v[i]-E2.x2);
        a.push_back(feature2.overlap_level[i]-E2.x3);
        a.push_back(feature2.grey_std[i]-E2.x4);
        a.push_back(feature2.count_v[i]-E2.x5);
        A2.push_back(a);

    }
    vector<vector<float> >Q,R,Q2,R2;
    vector<vector<double> >R_new,R2_new;

    cout<<"对矩阵A进行QR Factorization"<<endl;
    Matrix_QR_Factorization(A,Q,R);
    Matrix_QR_Factorization(A2,Q2,R2);
    for(int i=0;i<5;i++)
    {
        vector<double> r;
        r.push_back(R[i][0]/sqrt(K-2));
        r.push_back(R[i][1]/sqrt(K-2));
        r.push_back(R[i][2]/sqrt(K-2));
        r.push_back(R[i][3]/sqrt(K-2));
        r.push_back(R[i][4]/sqrt(K-2));
        R_new.push_back(r);
    }
    for(int i=0;i<5;i++)
    {
        vector<double> r;
        r.push_back(R2[i][0]/sqrt(U-2));
        r.push_back(R2[i][1]/sqrt(U-2));
        r.push_back(R2[i][2]/sqrt(U-2));
        r.push_back(R2[i][3]/sqrt(U-2));
        r.push_back(R2[i][4]/sqrt(U-2));
        R2_new.push_back(r);
    }
    int k=5;
    vector<vector<double> > UU,UU2;
    vector<double> S,S2;
    vector<vector<double> > V,V2;
    svd(R_new,k,UU,S,V);
    svd(R2_new,k,UU2,S2,V2);


    cout<<"S="<<endl;
    for(int i=0;i<S.size();i++)
    {
        cout<<setw(7)<<S[i]<<' ';
    }
    cout<<endl;
    double sum_log=0;
    for(int i=0;i<S.size();i++)
    {
        sum_log = sum_log + log(S[i]);
    }
    double sum_log2=0;
    for(int i=0;i<S2.size();i++)
    {
        sum_log2 = sum_log2 + log(S2[i]);
    }
    cout<<endl<<"Matrix R:"<<endl;
    for(int i=0;i<R_new.size();i++)
    {
        for(int j=0;j<5;j++)
        {
            cout<<setw(8)<<R_new[i][j]<<" ";
        }
        cout<<endl;
    }

    cout<<R_new.size()<<endl;
   // v3d_msg("Start Classification");
    vector<double> cl,cl2,cl3,cl4;


    cl = classify_glio_Y(S,feature,E,R_new,sum_log);
    cl2 = classify_glio_Y(S2,feature,E,R2_new,sum_log2);

    cl3 = classify_glio_Y(S,feature2,E2,R_new,sum_log);
    cl4 = classify_glio_Y(S2,feature2,E2,R2_new,sum_log2);

//    cl = classify_glio_Y(S,feature3,E3,R_new,sum_log);
//    cl2 = classify_glio_Y(S2,feature3,E3,R2_new,sum_log2);

//    cl3 = classify_glio_Y(S,feature4,E4,R_new,sum_log);
//    cl4 = classify_glio_Y(S2,feature4,E4,R2_new,sum_log2);
   // v3d_msg("Classification Done");
//    for(int i=0;i<cl.size();i++)
//    {
//            cout<<cl2[i]<<endl;


//    }

    vector<bool> result1,result2;
    for(int i=0;i<cl.size();i++)
    {
      //  cout<<"cl cl2 = "<<cl[i]<<"    "<<cl2[i]<<endl;
        if(cl[i]>cl2[i])
        {
            //cout<<"yes = "<<inf_v[i].inf1<<"  "<<inf_v[i].name<<"     "<<feature.y_n[i]<<"    "<<feature.ratio_v[i]<<"  "<<feature.overlap_level[i]<<"        "<<feature.count_v[i]<<"     "<<feature.grey_std[i]<<endl;
            result1.push_back(1);//is glio
        }
        else
        {

            cout<<"wrong = "<<inf_v[i].inf1<<"  "<<inf_v[i].name<<"     "<<feature.y_n[i]<<"    "<<feature.ratio_v[i]<<feature.overlap_level[i]<<"        "<<feature.count_v[i]<<"     "<<feature.grey_std[i]<<endl;
            //cout<<<<endl;

            result1.push_back(0);
        }
    }

    for(int i=0;i<cl3.size();i++)
    {
      //  cout<<"cl3 cl4 = "<<cl3[i]<<"    "<<cl4[i]<<endl;
        if(cl3[i]<cl4[i]||y_n4[i]==0)
        {
            //if(y_n[i]==1)
            result2.push_back(1);//is signal
        }
        else
        {
            result2.push_back(0);
        }
    }


//    bool method=false;
//    vector<bool> result1 = classify_glio2(chart,E,chart2,E2,chart3,E3,feature3,method);
//    vector<bool> result2 = classify_glio2(chart,E,chart2,E2,chart4,E4,feature4,method);
    double sum1=0;
    for(int i=0;i<result1.size();i++)
    {
        //cout<<"result1 = "<<result1[i]<<endl;
        sum1 = sum1 + result1[i];
    }
    cout<<"sum1 = "<<sum1<<endl;
    cout<<"result1.size = "<<result1.size()<<endl;

    double per1 = sum1/result1.size();
    double sum2=0;
    for(int i=0;i<result2.size();i++)
    {
        sum2 = sum2 + result2[i];
    }
    cout<<"sum2 = "<<sum2<<endl;
    cout<<"result2.size = "<<result2.size()<<endl;
    double per2 = sum2/result2.size();
    cout<<"per = "<<per1<<"  "<<per2<<endl;

}
bool export_TXT(Each_line &E,Chart &chart,QString fileSaveName)
{
    QFile file(fileSaveName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;
    QTextStream myfile(&file);

    cout << E.x1 <<"    "<<chart.first_line.x1<<"    "<<chart.first_line.x2<<"    "<<chart.first_line.x3<<"    "<<chart.first_line.x4<<"    "<<chart.first_line.x5<<endl;
    cout << E.x2 <<"    "<<chart.second_line.x1<<"    "<<chart.second_line.x2<<"    "<<chart.second_line.x3<<"    "<<chart.second_line.x4<<"    "<<chart.second_line.x5<<endl;
    cout << E.x3 <<"    "<<chart.third_line.x1<<"    "<<chart.third_line.x2<<"    "<<chart.third_line.x3<<"    "<<chart.third_line.x4<<"    "<<chart.third_line.x5<<endl;
    cout << E.x4 <<"    "<<chart.forth_line.x1<<"    "<<chart.forth_line.x2<<"    "<<chart.forth_line.x3<<"    "<<chart.forth_line.x4<<"    "<<chart.forth_line.x5<<endl;
    cout << E.x5 <<"    "<<chart.fifth_line.x1<<"    "<<chart.fifth_line.x2<<"    "<<chart.fifth_line.x3<<"    "<<chart.fifth_line.x4<<"    "<<chart.fifth_line.x5<<endl;


        myfile << E.x1 <<"   "<<chart.first_line.x1<<"   "<<chart.first_line.x2<<"   "<<chart.first_line.x3<<"   "<<chart.first_line.x4<<"   "<<chart.first_line.x5<<endl;
        myfile << E.x2 <<"   "<<chart.second_line.x1<<"   "<<chart.second_line.x2<<"   "<<chart.second_line.x3<<"   "<<chart.second_line.x4<<"   "<<chart.second_line.x5<<endl;
        myfile << E.x3 <<"   "<<chart.third_line.x1<<"   "<<chart.third_line.x2<<"   "<<chart.third_line.x3<<"   "<<chart.third_line.x4<<"   "<<chart.third_line.x5<<endl;
        myfile << E.x4 <<"   "<<chart.forth_line.x1<<"   "<<chart.forth_line.x2<<"   "<<chart.forth_line.x3<<"   "<<chart.forth_line.x4<<"   "<<chart.forth_line.x5<<endl;
        myfile << E.x5 <<"   "<<chart.fifth_line.x1<<"   "<<chart.fifth_line.x2<<"   "<<chart.fifth_line.x3<<"   "<<chart.fifth_line.x4<<"   "<<chart.fifth_line.x5<<endl;


    file.close();
    cout<<"txt file "<<fileSaveName.toStdString()<<" has been generated"<<endl;
    return true;
}



vector<Coordinate> readtxt_LXF(const QString& filename,const string &inf1)
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
            S.inf1 = inf1;
            S.name = filename.toStdString();
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
        //cout<<"sum = "<<sum<<endl;
       // cout<<"mean1 = "<<mean1<<endl;
      //  cout<<"mean2 = "<<mean2<<endl;
      //  cout<<"v1[i] = "<<v1[i]<<endl;
      //  cout<<"v2[i] = "<<v2[i]<<endl;
    }
    double mean_two = sum/(v1.size()-1);
    return mean_two;

}
bool Cov_calculate(Chart &chart,Feature &feature)
{
    chart.fifth_line.x1 = get_D(feature.y_n,feature.y_n);
    chart.fifth_line.x2 = get_D(feature.y_n,feature.ratio_v);
    chart.fifth_line.x3 = get_D(feature.y_n,feature.overlap_level);
    chart.fifth_line.x4 = get_D(feature.y_n,feature.grey_std);
    chart.fifth_line.x5 = get_D(feature.y_n,feature.count_v);

    chart.second_line.x1 = get_D(feature.ratio_v,feature.y_n);
    chart.second_line.x2 = get_D(feature.ratio_v,feature.ratio_v);
    chart.second_line.x3 = get_D(feature.ratio_v,feature.overlap_level);
    chart.second_line.x4 = get_D(feature.ratio_v,feature.grey_std);
    chart.second_line.x5 = get_D(feature.ratio_v,feature.count_v);

    chart.third_line.x1 = get_D(feature.overlap_level,feature.y_n);
    chart.third_line.x2 = get_D(feature.overlap_level,feature.ratio_v);
    chart.third_line.x3 = get_D(feature.overlap_level,feature.overlap_level);
    chart.third_line.x4 = get_D(feature.overlap_level,feature.grey_std);
    chart.third_line.x5 = get_D(feature.overlap_level,feature.count_v);

    chart.forth_line.x1 = get_D(feature.grey_std,feature.y_n);
    chart.forth_line.x2 = get_D(feature.grey_std,feature.ratio_v);
    chart.forth_line.x3 = get_D(feature.grey_std,feature.overlap_level);
    chart.forth_line.x4 = get_D(feature.grey_std,feature.grey_std);
    chart.forth_line.x5 = get_D(feature.grey_std,feature.count_v);

    chart.fifth_line.x1 = get_D(feature.count_v,feature.y_n);
    chart.fifth_line.x2 = get_D(feature.count_v,feature.ratio_v);
    chart.fifth_line.x3 = get_D(feature.count_v,feature.overlap_level);
    chart.fifth_line.x4 = get_D(feature.count_v,feature.grey_std);
    chart.fifth_line.x5 = get_D(feature.count_v,feature.count_v);
    //cout<<chart.fifth_line.x1<<"  "<<chart.fifth_line.x2<<"  "<<chart.fifth_line.x3<<"  "<<chart.fifth_line.x4<<"  "<<chart.fifth_line.x5<<"  "<<endl;
    //cout<<chart.second_line.x1<<"  "<<chart.second_line.x2<<"  "<<chart.second_line.x3<<"  "<<chart.second_line.x4<<"  "<<chart.second_line.x5<<"  "<<endl;
    return 0;

}
Each_line E_calculate(Feature &feature)
{
    int size = feature.y_n.size();
    Each_line sum,E;
    sum.x1=sum.x2=sum.x3=sum.x4=sum.x5=0;
    for(int i=0;i<size;i++)
    {
        sum.x1 = sum.x1 + feature.y_n[i];
        sum.x2 = sum.x2 + feature.ratio_v[i];
        sum.x3 = sum.x3 + feature.overlap_level[i];
        sum.x4 = sum.x4 + feature.grey_std[i];
        cout<<"feature.grey_std = "<<feature.grey_std[i]<<endl;
        cout<<"sum.x4 = "<<sum.x4<<endl;
        sum.x5 = sum.x5 + feature.count_v[i];
    }
  //  cout<<"sum = "<<sum.x1<<"   "<<sum.x2<<"    "<<sum.x3<<"    "<<sum.x4<<"    "<<sum.x5<<endl;
  //  cout<<"size = "<<size<<endl;
    E.x1=sum.x1/size;
    E.x2=sum.x2/size;
    E.x3=sum.x3/size;
    E.x4=sum.x4/size;
    E.x5=sum.x5/size;
 //   cout<<"sum.x4 size = "<<sum.x4<<"   "<<size<<endl;
  //  cout<<"E = "<<E.x1<<"   "<<E.x2<<"    "<<E.x3<<"    "<<E.x4<<"    "<<E.x5<<endl;
   // v3d_msg("E");
    return E;
}
vector<bool> classify_glio2(Chart &chart1,Each_line &E1,Chart &chart2,Each_line &E2,Chart &chart_curr,Each_line &E_curr,Feature &feature_curr,bool &method)
{
    //cout<<"oooooooooooooooooooooooo"<<endl;
    vector<bool> classify;
    if(method)
    {
        double dis11 = sqrt( (E1.x1 - E_curr.x1)*(E1.x1 - E_curr.x1) + (chart1.first_line.x1-chart_curr.first_line.x1)*(chart1.first_line.x1-chart_curr.first_line.x1) +
                (chart1.first_line.x2-chart_curr.first_line.x2)*(chart1.first_line.x2-chart_curr.first_line.x2) + (chart1.first_line.x3-chart_curr.first_line.x3 )*(chart1.first_line.x3-chart_curr.first_line.x3)+
                (chart1.first_line.x4-chart_curr.first_line.x4)*(chart1.first_line.x4-chart_curr.first_line.x4) + (chart1.first_line.x5-chart_curr.first_line.x5)*(chart1.first_line.x5-chart_curr.first_line.x5) );

        double dis12 = sqrt( (E1.x2 - E_curr.x2)*(E1.x2 - E_curr.x2) + (chart1.second_line.x1-chart_curr.second_line.x1)*(chart1.second_line.x1-chart_curr.second_line.x1) +
                (chart1.second_line.x2-chart_curr.second_line.x2)*(chart1.second_line.x2-chart_curr.second_line.x2) + (chart1.second_line.x3-chart_curr.second_line.x3 )*(chart1.second_line.x3-chart_curr.second_line.x3)+
                (chart1.second_line.x4-chart_curr.second_line.x4)*(chart1.second_line.x4-chart_curr.second_line.x4) + (chart1.second_line.x5-chart_curr.second_line.x5)*(chart1.second_line.x5-chart_curr.second_line.x5) );

        double dis13 = sqrt( (E1.x3 - E_curr.x3)*(E1.x3 - E_curr.x3) + (chart1.third_line.x1-chart_curr.third_line.x1)*(chart1.third_line.x1-chart_curr.third_line.x1) +
                (chart1.third_line.x2-chart_curr.third_line.x2)*(chart1.third_line.x2-chart_curr.third_line.x2) + (chart1.third_line.x3-chart_curr.third_line.x3 )*(chart1.third_line.x3-chart_curr.third_line.x3)+
                (chart1.third_line.x4-chart_curr.third_line.x4)*(chart1.third_line.x4-chart_curr.third_line.x4) + (chart1.third_line.x5-chart_curr.third_line.x5)*(chart1.third_line.x5-chart_curr.third_line.x5) );

        double dis14 = sqrt( (E1.x4 - E_curr.x4)*(E1.x4 - E_curr.x4) + (chart1.forth_line.x1-chart_curr.forth_line.x1)*(chart1.forth_line.x1-chart_curr.forth_line.x1) +
                (chart1.forth_line.x2-chart_curr.forth_line.x2)*(chart1.forth_line.x2-chart_curr.forth_line.x2) + (chart1.forth_line.x3-chart_curr.forth_line.x3 )*(chart1.forth_line.x3-chart_curr.forth_line.x3)+
                (chart1.forth_line.x4-chart_curr.forth_line.x4)*(chart1.forth_line.x4-chart_curr.forth_line.x4) + (chart1.forth_line.x5-chart_curr.forth_line.x5)*(chart1.forth_line.x5-chart_curr.forth_line.x5) );

        double dis15 = sqrt( (E1.x5 - E_curr.x5)*(E1.x5 - E_curr.x5) + (chart1.fifth_line.x1-chart_curr.fifth_line.x1)*(chart1.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart1.fifth_line.x2-chart_curr.fifth_line.x2)*(chart1.fifth_line.x2-chart_curr.fifth_line.x2) + (chart1.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart1.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart1.fifth_line.x4-chart_curr.fifth_line.x4)*(chart1.fifth_line.x4-chart_curr.fifth_line.x4) + (chart1.fifth_line.x5-chart_curr.fifth_line.x5)*(chart1.fifth_line.x5-chart_curr.fifth_line.x5) );


        double dis21 = sqrt( (E2.x1 - E_curr.x1)*(E2.x1 - E_curr.x1) + (chart2.first_line.x1-chart_curr.first_line.x1)*(chart2.first_line.x1-chart_curr.first_line.x1) +
                (chart2.first_line.x2-chart_curr.first_line.x2)*(chart2.first_line.x2-chart_curr.first_line.x2) + (chart2.first_line.x3-chart_curr.first_line.x3 )*(chart2.first_line.x3-chart_curr.first_line.x3)+
                (chart2.first_line.x4-chart_curr.first_line.x4)*(chart2.first_line.x4-chart_curr.first_line.x4) + (chart2.first_line.x5-chart_curr.first_line.x5)*(chart2.first_line.x5-chart_curr.first_line.x5) );

        double dis22 = sqrt( (E2.x2 - E_curr.x2)*(E2.x2 - E_curr.x2) + (chart2.second_line.x1-chart_curr.second_line.x1)*(chart2.second_line.x1-chart_curr.second_line.x1) +
                (chart2.second_line.x2-chart_curr.second_line.x2)*(chart2.second_line.x2-chart_curr.second_line.x2) + (chart2.second_line.x3-chart_curr.second_line.x3 )*(chart2.second_line.x3-chart_curr.second_line.x3)+
                (chart2.second_line.x4-chart_curr.second_line.x4)*(chart2.second_line.x4-chart_curr.second_line.x4) + (chart2.second_line.x5-chart_curr.second_line.x5)*(chart2.second_line.x5-chart_curr.second_line.x5) );

        double dis23 = sqrt( (E2.x3 - E_curr.x3)*(E2.x3 - E_curr.x3) + (chart2.third_line.x1-chart_curr.third_line.x1)*(chart2.third_line.x1-chart_curr.third_line.x1) +
                (chart2.third_line.x2-chart_curr.third_line.x2)*(chart2.third_line.x2-chart_curr.third_line.x2) + (chart2.third_line.x3-chart_curr.third_line.x3 )*(chart2.third_line.x3-chart_curr.third_line.x3)+
                (chart2.third_line.x4-chart_curr.third_line.x4)*(chart2.third_line.x4-chart_curr.third_line.x4) + (chart2.third_line.x5-chart_curr.third_line.x5)*(chart2.third_line.x5-chart_curr.third_line.x5) );

        double dis24 = sqrt( (E2.x4 - E_curr.x4)*(E2.x4 - E_curr.x4) + (chart2.forth_line.x1-chart_curr.forth_line.x1)*(chart2.forth_line.x1-chart_curr.forth_line.x1) +
                (chart2.forth_line.x2-chart_curr.forth_line.x2)*(chart2.forth_line.x2-chart_curr.forth_line.x2) + (chart2.forth_line.x3-chart_curr.forth_line.x3 )*(chart2.forth_line.x3-chart_curr.forth_line.x3)+
                (chart2.forth_line.x4-chart_curr.forth_line.x4)*(chart2.forth_line.x4-chart_curr.forth_line.x4) + (chart2.forth_line.x5-chart_curr.forth_line.x5)*(chart2.forth_line.x5-chart_curr.forth_line.x5) );

        double dis25 = sqrt( (E2.x5 - E_curr.x5)*(E2.x5 - E_curr.x5) + (chart2.fifth_line.x1-chart_curr.fifth_line.x1)*(chart2.fifth_line.x1-chart_curr.fifth_line.x1) +
                (chart2.fifth_line.x2-chart_curr.fifth_line.x2)*(chart2.fifth_line.x2-chart_curr.fifth_line.x2) + (chart2.fifth_line.x3-chart_curr.fifth_line.x3 )*(chart2.fifth_line.x3-chart_curr.fifth_line.x3)+
                (chart2.fifth_line.x4-chart_curr.fifth_line.x4)*(chart2.fifth_line.x4-chart_curr.fifth_line.x4) + (chart2.fifth_line.x5-chart_curr.fifth_line.x5)*(chart2.fifth_line.x5-chart_curr.fifth_line.x5) );
        double dis1 = dis11+dis12+dis13+dis14+dis15;
        double dis2 = dis21+dis22+dis23+dis24+dis25;

//        if(dis1>dis2){return true;}
//        else{return false;}

    }
    else
    {
        int size = feature_curr.count_v.size();
        for(int i=0;i<size;i++)
        {
            double dis1 = sqrt( (E1.x1 - feature_curr.y_n[i])*(E1.x1 - feature_curr.y_n[i]) + (E1.x2 - feature_curr.ratio_v[i])*(E1.x2 - feature_curr.ratio_v[i]) + (E1.x3 - feature_curr.overlap_level[i])*(E1.x3 - feature_curr.overlap_level[i])
                                + (E1.x4 - feature_curr.grey_std[i])*(E1.x4 - feature_curr.grey_std[i]) + (E1.x5 - feature_curr.count_v[i])*(E1.x5 - feature_curr.count_v[i]) );

            double dis2 = sqrt( (E2.x1 - feature_curr.y_n[i])*(E2.x1 - feature_curr.y_n[i]) + (E2.x2 - feature_curr.ratio_v[i])*(E2.x2 - feature_curr.ratio_v[i]) + (E2.x3 - feature_curr.overlap_level[i])*(E2.x3 - feature_curr.overlap_level[i])
                                + (E2.x4 - feature_curr.grey_std[i])*(E2.x4 - feature_curr.grey_std[i]) + (E2.x5 - feature_curr.count_v[i])*(E2.x5 - feature_curr.count_v[i]) );



            if(dis1>dis2)
            {
                classify.push_back(1);
            }
            else
            {
                classify.push_back(0);
            }


        }
        return classify;

    }

}
vector<double> classify_glio_Y(vector<double> &S,Feature &f,Each_line &E,vector<vector<double> > &R,double &sum_log)
{
    cout<<"in classify"<<endl;
    int size = f.count_v.size();
    double m1;
    double m2;
    double m3;
    double m4;
    double m5;
    vector<vector<double> > re;
    for(int i=0;i<size;i++)
    {
        vector<double> r;
       m1 = f.y_n[i]-E.x1;
       m2 = f.ratio_v[i]-E.x2;
       m3 = f.overlap_level[i]-E.x3;
       m4 = f.grey_std[i]-E.x4;
       m5 = f.count_v[i]-E.x5;
       cout<<E.x1<<"    "<<E.x2<<"      "<<E.x3<<"      "<<E.x4<<"      "<<E.x5<<endl;
       cout<<"f.grey_std  E.x4 = "<<f.grey_std[i]<<"    "<<E.x4<<endl;
      // cout<<"m = "<<m1<<"  "<<m2<<"    "<<m3<<"    "<<m4<<"    "<<m5<<endl;
       r.push_back(m1);
       r.push_back(m2);
       r.push_back(m3);
       r.push_back(m4);
       r.push_back(m5);
       re.push_back(r);
    }
    //v3d_msg("check");
    vector<vector<double> > result;
 //   cout<<"R.size() = "<<R.size()<<endl;

    double R_m[5][5],R_inv[5][5];
    for(int i=0;i<R.size();i++)
    {
        for(int j=0;j<5;j++)
        {
            R_m[i][j] = R[i][j];
        }
    }


    GetMatrixInverse(R_m,5,R_inv);


       vector<double> R_Inv_v;
       vector<vector<double> > R_Inv_vv;
        for(int i=0;i<R.size();i++)
        {
            R_Inv_v.clear();
            for(int j=0;j<5;j++)
            {
                R_Inv_v.push_back(R_inv[i][j]);
            }
            R_Inv_vv.push_back(R_Inv_v);
        }
        result = matrix_multiply(re,R_Inv_vv);





    vector<double> prior;
    for(int i=0;i<size;i++)
    {
        prior.push_back(0.5);
    }
   // v3d_msg("check");
    vector<double> sum;
    for(int i=0;i<size;i++)
    {
        double tmp = result[i][0]*result[i][0] + result[i][1]*result[i][1] + result[i][2]*result[i][2] + result[i][3]*result[i][3] + result[i][4]*result[i][4];
     //   cout<<"tmp = "<<tmp<<"      ";
        sum.push_back(tmp);
    }

    //v3d_msg();
    vector<double> D;
    for(int i=0;i<size;i++)
    {
        double t = log(prior[i])-0.5*sum[i]+2*sum_log;
     //   cout<<"t = "<<t<<endl;
        D.push_back(t);
    }
    return D;

}

vector<vector<double> > matrix_multiply(vector<vector<double> > &arrA, vector<vector<double> > &arrB)
{
//    cout<<"ppppppppppppp"<<endl;
    //矩阵arrA的行数
    int rowA = arrA.size();
    //矩阵arrA的列数
    int colA = arrA[0].size();
    //矩阵arrB的行数
    int rowB = arrB.size();
    //矩阵arrB的列数
    int colB = arrB[0].size();
    //相乘后的结果矩阵
    vector<vector<double> >  res;
    double tmp[rowA][colB];
    double sum=0;
    cout<<colA<<"   "<<colB<<"  "<<rowA<<"   "<<rowB<<endl;
    if (colA != rowB)//如果矩阵arrA的列数不等于矩阵arrB的行数。则返回空
    {
        return res;
    }
    else
    {
        //设置结果矩阵的大小，初始化为为0
//        res.resize(rowA);
//        for (int i = 0; i < rowA; ++i)
//        {
//            res[i].resize(colB);
//        }

        //矩阵相乘
     //   cout<<"uuuuuuuuuuuu"<<endl;
        for (int i = 0; i < rowA; ++i)
        {
            for (int j = 0; j < colB; ++j)
            {
                for (int k = 0; k < colA; ++k)
                {
                    sum = arrA[i][k] * arrB[k][j] + sum;
                    tmp[i][j] = sum;
                }
            }
        }
        for (int i = 0; i < rowA; ++i)
        {
            vector<double> t;
            for (int j = 0; j < colB; ++j)
            {
                t.push_back(tmp[i][j]);
            }
            res.push_back(t);
        }
    }
    return res;
}



//int main()
//{
//    Matrix a;
//    a.setSize();
//    a.chuShi();
//    a.qiuNi();
//    return 0;
//}

