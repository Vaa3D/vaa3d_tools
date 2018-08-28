#ifndef SVD_H
#define SVD_H


#include <cmath>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>
using namespace std;
const int MAX_ITER=100000;
const double eps=0.0000001;
double get_norm(double *x, int n){
    double r=0;
    for(int i=0;i<n;i++)
        r+=x[i]*x[i];
    return sqrt(r);
}
double normalize(double *x, int n){
    double r=get_norm(x,n);
    if(r<eps)
        return 0;
    for(int i=0;i<n;i++)
        x[i]/=r;
    return r;
}

inline double product(double*a, double *b,int n){
    double r=0;
    for(int i=0;i<n;i++)
        r+=a[i]*b[i];
    return r;
}

void orth(double *a, double *b, int n){//|a|=1
    double r=product(a,b,n);
    for(int i=0;i<n;i++)
        b[i]-=r*a[i];

}

bool svd(vector<vector<double> > A, int K, vector<vector<double> > &U, vector<double> &S, vector<vector<double> > &V){
    int M=A.size();
    int N=A[0].size();
    U.clear();
    V.clear();
    S.clear();
    S.resize(K,0);
    U.resize(K);
    for(int i=0;i<K;i++)
        U[i].resize(M,0);
    V.resize(K);
    for(int i=0;i<K;i++)
        V[i].resize(N,0);


    srand(time(0));
    double *left_vector=new double[M];
    double *next_left_vector=new double[M];
    double *right_vector=new double[N];
    double *next_right_vector=new double[N];
    int col=0;
    for(int col=0;col<K;col++){
        double diff=1;
        double r=-1;
        while(1){
            for(int i=0;i<M;i++)
                left_vector[i]= (float)rand() / RAND_MAX;
            if(normalize(left_vector, M)>eps)
                break;
        }

        for(int iter=0;diff>=eps && iter<MAX_ITER;iter++){
            memset(next_left_vector,0,sizeof(double)*M);
            memset(next_right_vector,0,sizeof(double)*N);
            for(int i=0;i<M;i++)
                for(int j=0;j<N;j++)
                    next_right_vector[j]+=left_vector[i]*A[i][j];

            r=normalize(next_right_vector,N);
            if(r<eps) break;
            for(int i=0;i<col;i++)
                orth(&V[i][0],next_right_vector,N);
            normalize(next_right_vector,N);

            for(int i=0;i<M;i++)
                for(int j=0;j<N;j++)
                    next_left_vector[i]+=next_right_vector[j]*A[i][j];
            r=normalize(next_left_vector,M);
            if(r<eps) break;
            for(int i=0;i<col;i++)
                orth(&U[i][0],next_left_vector,M);
            normalize(next_left_vector,M);
            diff=0;
            for(int i=0;i<M;i++){
                double d=next_left_vector[i]-left_vector[i];
                diff+=d*d;
            }

            memcpy(left_vector,next_left_vector,sizeof(double)*M);
            memcpy(right_vector,next_right_vector,sizeof(double)*N);
        }
        if(r>=eps){
            S[col]=r;
            memcpy((char *)&U[col][0],left_vector,sizeof(double)*M);
            memcpy((char *)&V[col][0],right_vector,sizeof(double)*N);
        }else{
            cout<<r<<endl;
            break;
        }
    }
    delete [] next_left_vector;
    delete [] next_right_vector;
    delete [] left_vector;
    delete [] right_vector;

    return true;
}

void print(vector<vector<double> > &A){

}
int main(){
    int m=1000;
    int n=800;
    int k=5;
    //分解一个10*8的矩阵A，求其前5个奇异值和奇异向量
    srand(time(0));
    vector<vector<double> > A;
    A.resize(m);

    for(int i=0;i<m;i++){
        A[i].resize(n);
        for(int j=0;j<n;j++)
            A[i][j]=(float)rand()/RAND_MAX-0.5;
    }



    cout<<"A="<<endl;
    for(int i=0;i<A.size();i++){
        for(int j=0;j<A[i].size();j++){
            cout<<setw(12)<<A[i][j]<<' ';
        }
        cout<<endl;
    }
    cout<<endl;

    vector<vector<double> > U;
    vector<double> S;
    vector<vector<double> > V;
    svd(A,k,U,S,V);
    cout<<"U="<<endl;
    for(int i=0;i<U[0].size();i++){
        for(int j=0;j<U.size();j++){
            cout<<setw(12)<<U[j][i]<<' ';
        }
        cout<<endl;
    }
    cout<<endl;
    cout<<"S="<<endl;
    for(int i=0;i<S.size();i++){
        cout<<setw(7)<<S[i]<<' ';
    }
    cout<<endl;
    cout<<"V="<<endl;
    for(int i=0;i<V[0].size();i++){
        for(int j=0;j<V.size();j++){
            cout<<setw(12)<<V[j][i]<<' ';
        }
        cout<<endl;
    }
    return 0;
}

#endif // SVD_H
