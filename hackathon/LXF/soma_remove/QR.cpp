
/*********************************************************
description:A=QR 矩阵A的列线性无关 Matrix A is M-by-N
Author:Robert.Tianyi
Date:2016-10-24
*********************************************************/
#include "QR.h"
#include "v3d_message.h"

//using namespace std;

void Matrix_QR_Factorization(vector<vector<float> >&A_a,vector<vector<float> >&Q_a,vector<vector<float> >&R_a){
    int M = A_a.size();
    int N = 5;
    float A[M][N];
    for(int i=0;i<M;i++)
    {
        A[i][0] = A_a[i][0];
        A[i][1] = A_a[i][1];
        A[i][2] = A_a[i][2];
        A[i][3] = A_a[i][3];
        A[i][4] = A_a[i][4];
    }

    int k=0;
    float Q[M][N];
    float R[N][N];
    float temp[M],temp_len;
    float *p=new float[M];
    int i,j;
    for(i=0;i<M;i++)
        for(j=0;j<N;j++)
            R[i][j]=0;
    for(k=0;k<N;k++){
        for(j=0;j<M;j++){
          temp[j]=A[j][k];//取出第 j 列  即X_k
        }
        //先求Q矩阵
        if(k==0){
            temp_len=Get_Norm(temp,M);
            R[0][0]=temp_len;
            p=VectorDivNum(temp,temp_len,M);
            for(j=0;j<M;j++){
                Q[j][k]=p[j];
            }

        }
        if(k>0){
            float *Tempmul=new float[M];
            float *sum=new float[M];
            float tempAj[M];
            float tempQj[M];
            for(i=0;i<M;i++){
                sum[i]=0;
            }
            for(int tempK=0;tempK<k;tempK++){
                for(i=0;i<M;i++){
                    tempQj[i]=Q[i][tempK];//从Q矩阵取出第k列 即Q_k
                }
                R[tempK][k]=VectorMUL(temp,tempQj,M);
                Tempmul= VectorMULNum(tempQj,R[tempK][k],M);//计算A_k在Q_j上的投影分量
                //Tempmul= VectorMULNum(tempQj,VectorMUL(temp,tempQj,M),M);
                sum=VectorADD(Tempmul,sum,M);//计算A_k在所有Q的列向量上的投影分量之和
                //for(i=0;i<M;i++)
                //  sum[i]+=Tempmul[i];
            }
            float * uk=new float[M];
            int uk_len;
            uk=VectorSUB(temp,sum,M);
            uk_len=Get_Norm(uk,M);
            R[k][k]=uk_len;
            p=VectorDivNum(uk,uk_len,M);
            for(j=0;j<M;j++)
                Q[j][k]=p[j];
        }
    }
    cout<<endl<<"Matrix Q:"<<endl;
    for(int i=0;i<M;i++){
        for(int j=0;j<N;j++){
            cout<<setw(8)<<Q[i][j]<<" ";
        }
        cout<<endl;
    }
    cout<<endl<<"Matrix R:"<<endl;
    for(int i=0;i<M;i++){
        for(int j=0;j<N;j++){
            cout<<setw(8)<<R[i][j]<<" ";
        }
        cout<<endl;
    }
    //v3d_msg("kkkkkkkk");
    for(int i=0;i<M;i++)
    {
        vector<float> q;
        q.push_back(Q[i][0]);
        q.push_back(Q[i][1]);
        q.push_back(Q[i][2]);
        q.push_back(Q[i][3]);
        q.push_back(Q[i][4]);
        Q_a.push_back(q);
    }
    for(int i=0;i<M;i++)
    {
        vector<float> r;
        r.push_back(R[i][0]);
        r.push_back(R[i][1]);
        r.push_back(R[i][2]);
        r.push_back(R[i][3]);
        r.push_back(R[i][4]);
        R_a.push_back(r);
    }
}
/*向量求模*/
float Get_Norm(float a[],int size){
    float sum=0;
    for(int i=0;i<size;i++)
        sum+=a[i]*a[i];
    return sqrt(sum);
}
/*向量除以一个数*/
float *VectorDivNum(float a[],float num,int len){
    float *p;
    p=new float[len];
    for(int i=0;i<len;i++)
        p[i]=a[i]/num;
    return p;
}
/*向量数乘*/
float * VectorMULNum(float a[],float num,int len){
    float *p;
    p=new float[len];
    for(int i=0;i<len;i++)
        p[i]=a[i]*num;
    return p;
}
/*向量内积*/
float  VectorMUL(float a[],float b[],int len){
    float *p;
    float result=0;
    for(int i=0;i<len;i++)
        result+=a[i]*b[i];
    return result;
}
/*向量加法*/
float * VectorADD(float a[],float b[],int len){
    float *p;
    p=new float[len];
    for(int i=0;i<len;i++)
        p[i]=a[i]+b[i];
    return p;
}
/*向量减法*/
float * VectorSUB(float a[],float b[],int len){
    float *p;
    p=new float[len];
    for(int i=0;i<len;i++)
        p[i]=a[i]-b[i];
    return p;
}
