#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include "fun.h"
#include <algorithm>

#include "svds.h"
using namespace std;
const double EPS=1e-15;
void printt(vector<vector<double> > &X){
    cout.precision(6);
    cout.setf(ios::fixed);
    for(int i=0;i<X.size();i++){
        for(int j=0;j<X[i].size();j++)
            cout<<X[i][j]<<' ';
        cout<<endl;
    }
    cout<<endl;
}

void transpose(vector<vector<double> > &A, vector<vector<double> > &T){
    if(A.empty()||A[0].empty()) return;
    int m=A.size();
    int n=A[0].size();
    T.clear();
    T.resize(n,vector<double>(m,0));
    for(int i=0;i<m;i++)
        for(int j=0;j<n;j++)
            T[j][i]=A[i][j];
}
void transpose(vector<vector<double> > &A){
    int m=A.size();
    for(int i=0;i<m;i++)
        for(int j=i+1;j<m;j++)
            swap(A[i][j],A[j][i]);
}

void randUnitVector(int n, vector<double> &v){
    srand(time(NULL));
    v.clear();v.resize(n);
    while(true){
        double r=0;
        for(int i=0;i<n;i++){
            v[i]=rand()*1.0/RAND_MAX-0.5;
            r+=v[i]*v[i];
        }
        r=sqrt(r);
        if(r>EPS){
            for(int i=0;i<n;i++)
                v[i]/=r;
            break;
        }
    }
}
void multiply(vector<vector<double> > &A, vector<vector<double> > &B, vector<vector<double> > &C){
    //C=A*B
    C.clear();
    if(A.empty() || A[0].empty() || B.empty() || B[0].empty()) return ;
    C.resize(A.size(),vector<double>(B[0].size(),0));
    for(int i=0;i<A.size();i++){
        for(int j=0;j<B[0].size();j++){
            C[i][j]=0;
            for(int k=0;k<A[0].size();k++){
                C[i][j]+=A[i][k]*B[k][j];
            }
        }
    }
}

void multiply(const vector<vector<double> > &X, const vector<double> & v, vector<double> & res){
    res.clear();
    if(X.empty() || v.empty()) return;
    int m=X[0].size();
    res.resize(m,0);
    for(int i=0;i<m;i++){
        for(int j=0;X[i].size();j++){
            res[i]+=X[i][j]*v[j];
        }
    }
}
double dotProduct(const vector<double> & a, const vector<double> & b){
    double res=0;
    for(int i=0;i<a.size();i++)
        res+=a[i]*b[i];
    return res;
}


void rightMultiply(SparseMatrix &A, const vector<double> & v, vector<double> & res){
    //res= A*A'*v
    int m=A.rows;
    int n=A.cols;
    res.clear();
    res.resize(m,0);
    vector<double> w(n,0);
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        w[c.col]+=c.value*v[c.row];
    }
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        res[c.row]+=c.value*w[c.col];
    }
}
void leftMultiply(SparseMatrix &A, const vector<double> & v, vector<double> & res){
    //res= A'*A*v
    int m=A.rows;
    int n=A.cols;
    res.clear();
    res.resize(n,0);
    vector<double> w(m,0);
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        w[c.row]+=c.value*v[c.col];
    }
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        res[c.col]+=c.value*w[c.row];
    }
}

void rightMultiply(const vector<vector<double> > & B,SparseMatrix &A, vector<vector<double> > & C){
    //C= B'*A
    int m=B[0].size();
    int k=B.size();
    int n=A.cols;
    for(int i=0;i<C.size();i++) fill(C[i].begin(),C[i].end(),0);
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        for(int i=0;i<m;i++){
            C[c.col][i]+=c.value*B[c.row][i];
        }
    }
}
void leftMultiply(const vector<vector<double> > & B,SparseMatrix &A, vector<vector<double> > & C){
    //C <- A B
    int r=B[0].size();
    int n=B.size();
    int m=A.rows;
    C.clear();
    C.resize(m,vector<double>(r,0));
    A.moveFirst();
    while(A.hasNext()){
        Cell c=A.next();
        for(int i=0;i<r;i++){
            C[c.row][i]+=c.value*B[c.col][i];
        }
    }
}


double norm(const vector<double> &v){
    double r=0;
    for(int i=0;i<v.size();i++)
        r+=v[i]*v[i];
    return sqrt(r);
}

double normalize(vector<double> &v){
    double r=0;
    for(int i=0;i<v.size();i++)
        r+=v[i]*v[i];
    r=sqrt(r);
    if(r>EPS){
        for(int i=0;i<v.size();i++)
            v[i]/=r;
    }
    return r;
}

void multiply(vector<double> &v, double d){
    for(int i=0;i<v.size();i++)
        v[i]*=d;
}
void hessenbergReduction(vector<vector<double> > &A, vector<vector<double> > &U){
    //A: m*m matrix
    //Reduction A to Hessenberg form: H=U'AU (A=UHU'), H overwrite A to save memory
    int m=A.size();
    vector<double> v(m);
    U.clear();
    U.resize(m,vector<double>(m,0));
    for(int i=0;i<m;i++)
        U[i][i]=1;
    for(int i=1;i<m;i++){
        fill(v.begin(),v.end(),0);
        for(int j=i;j<m;j++)
            v[j]=A[j][i-1];
        v[i]-=norm(v);
        normalize(v);   //P=I-2*v*v'
        //A=PAP
        //1. PA
        for(int j=i-1;j<m;j++){
            double d=0;
            for(int k=i;k<m;k++)
                d+=A[k][j]*v[k];
            for(int k=i;k<m;k++)
                A[k][j]-=d*2*v[k];
        }
        //2. AP
        for(int j=0;j<m;j++){//row j
            double d=0;
            for(int k=i;k<m;k++)
                d+=A[j][k]*v[k];
            for(int k=i;k<m;k++)
                A[j][k]-=d*2*v[k];
        }

        //U=U*P
        for(int j=0;j<m;j++){
            double d=0;
            for(int k=i;k<m;k++)
                d+=U[j][k]*v[k];
            for(int k=i;k<m;k++)
                U[j][k]-=d*2*v[k];
        }
    }
}

void givensRotation(double a, double b, double &c, double &s){
    if(fabs(b)<EPS){
        c=1;s=0;
    }else{
        if(fabs(b)>fabs(a)){
            double r=a/b;
            s=1/sqrt(1+r*r);
            c=s*r;
        }else{
            double r=b/a;
            c=1/sqrt(1+r*r);
            s=c*r;
        }
    }
}

void QRTridiagonal(vector<vector<double> > &A, vector<vector<double> > &Q){
    //input: A m*m symmetry tridiagonal matrix
    //output: Upper triangular R, and orthogonal Q, such that A=QRQ'
    int n=A.size();
    Q.clear();
    Q.resize(n,vector<double>(n,0));
    vector<double> cs(n-1,0);
    vector<double> ss(n-1,0);
    for(int i=0;i<n;i++) Q[i][i]=1;
    for(int m=n;m>=2;m--){
        while(1){
            fill(cs.begin(),cs.end(),0);
            fill(ss.begin(),ss.end(),0);
            double delta=(A[m-2][m-2]-A[m-1][m-1])/2;
            double sign=1;
            if(delta<0) sign=-1;
            //Wilkinson shift
            double shift=A[m-1][m-1]-sign*A[m-1][m-2]*A[m-2][m-1]/(fabs(delta)+sqrt(delta*delta+A[m-1][m-2]*A[m-2][m-1]));
            for(int i=0;i<m;i++)
                A[i][i]-=shift;
            for(int i=0;i<m-1;i++){
                double a=A[i][i];
                double b=A[i+1][i];
                givensRotation(a,b,cs[i],ss[i]);
                for(int j=i;j<=i+2 && j<m;j++){
                    a=A[i][j];
                    b=A[i+1][j];
                    A[i][j]=cs[i]*a+ss[i]*b;
                    A[i+1][j]=-ss[i]*a+cs[i]*b;
                }
            }

            for(int j=1;j<m;j++){// cols    j-1, j                         c -s
                for(int i=max(0,j-2);i<=j;i++){// rows  0 ... j     [a ,b]   s  c
                    double a=A[i][j-1];
                    double b=A[i][j];
                    A[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                    A[i][j]=-ss[j-1]*a+cs[j-1]*b;
                }
            }
            for(int i=0;i<m;i++)
                A[i][i]+=shift;
            //Q=Q*G1...Gm
            for(int j=1;j<m;j++){
                for(int i=0;i<n;i++){
                    double a=Q[i][j-1];
                    double b=Q[i][j];
                    Q[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                    Q[i][j]=-ss[j-1]*a+cs[j-1]*b;
                }
            }
            if(fabs(A[m-1][m-2])<1e-10)
                break;
        }
    }
}
vector<double> secularEquationSolver(vector<double> &z, vector<double> &D, double sigma){

    //solve equation
    // 1 + \sigma \sum_j \frac{b^2_j}{d_j-\lambda} =0
    int n=z.size();
    vector<double> res(n);

    //sort : d_0 < d_1 < ... < d_{n-1}
    vector<int> index;
    vector<double> d(n);
    merge_sort(D,index);
    if(sigma<0)
        reverse(index.begin(),index.end());
    vector<double> b(n);
    for(int i=0;i<n;i++){
        b[i]=z[index[i]];
        d[i]=D[index[i]];
    }

    vector<double> lambda(n);
    for(int i=0;i<n;i++){
        vector<double> delta(d.size());
        for(int j=0;j<delta.size();j++)
            delta[j]=(d[j]-d[i])/sigma;
        double gamma=0;
        if(i+1<n){
            //gamma>1/delta[i+1]
            double A=b[i]*b[i];
            double B=-A/delta[i+1]-1;
            for(int j=0;j<delta.size();j++)
                if(j!=i)
                    B-=b[j]*b[j]/delta[j];
            double C=1;
            for(int j=0;j<delta.size();j++)
                if(j!=i)
                    C+=b[j]*b[j]/delta[j];
            C/=delta[i+1];
            C-=b[i+1]*b[i+1]/delta[i+1];
            gamma=(-B+sqrt(B*B-4*A*C))/(2*A);
        }
        //start Newton
        double diff=1;
        while(diff*diff>EPS){
            double g=0;
            for(int j=0;j<n;j++){
                g-=b[j]*b[j]/((delta[j]*gamma-1)*(delta[j]*gamma-1));
            }
            double f=1;
            for(int j=0;j<n;j++){
                f+=b[j]*b[j]/(delta[j]-1/gamma);
            }
            //f+g(newGamma-gamma)=0
            double newGamma=-f/g+gamma;
            diff=fabs(newGamma-gamma);
            gamma=newGamma;
        }
        lambda[i]=1/gamma*sigma+d[i];
    }

    for(int i=0;i<n;i++)
        res[index[i]]=lambda[i];
    return res;
}

void DCSub(vector<double> &alpha, vector<double> &beta, vector<vector<double> > &Q, vector<double> &D, int start, int end){
    if(start==end){
        Q[start][start]=1;
        D[start]=alpha[start];
        return;
    }else{
        int mid=(start+end)/2;

        //      |   mid     mid+1
        //---------------------------------------------
        //  mid |   a(mid)      b(mid+1)
        //  mid+1   |   b(mid+1)    a(mid+1)

        alpha[mid]-=beta[mid+1];
        alpha[mid+1]-=beta[mid+1];
        DCSub(alpha,beta,Q,D,start,mid);
        DCSub(alpha,beta,Q,D,mid+1,end);
        //alpha[mid]+=beta[mid+1];
        //alpha[mid+1]+=beta[mid+1];


        int n=end-start+1;
        vector<double> z(n,0);
        for(int i=start;i<=mid;i++)
            z[i-start]=Q[mid][i];
        for(int i=mid+1;i<=end;i++)
            z[i-start]=Q[mid+1][i];

        //calculate eigen systems of matrix D+beta[mid+1]*z*z'

        vector<double> d(n,0);
        for(int i=0;i<n;i++)
            d[i]=D[i+start];

        //secular equation is:
        // 1 + \sum_j \frac{z^2_j}{d_j-\lambda} =0
        vector<double> lambda=secularEquationSolver(z, d, beta[mid+1]);


        //eigen vector:
        //P = (D-\lambda I)^{-1} z
        vector<vector<double> > P(n,vector<double>(n));
        for(int i=0;i<n;i++){//for each eigen value
            vector<double> p(n);
            for(int j=0;j<n;j++)
                p[j]=1.0/(D[j+start]-lambda[i])*z[j];
            normalize(p);
            for(int j=0;j<n;j++)
                P[j][i]=p[j];
        }

        vector<vector<double> > oldQ(n,vector<double>(n));
        for(int i=0;i<n;i++)
            for(int j=0;j<n;j++){
                oldQ[i][j]=Q[i+start][j+start];
            }

        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                Q[i+start][j+start]=0;
                for(int k=0;k<n;k++){
                    Q[i+start][j+start]+=oldQ[i][k]*P[k][j];
                }
            }
        }
        //eigen value
        for(int i=0;i<n;i++)
            D[i+start]=lambda[i];
    }
}
void DCTridiagonal(vector<double> alpha, vector<double> &beta, vector<vector<double> > &Q, vector<double> &D){
    //input: A m*m symmetry tridiagonal matrix
    //output: diagonal matrix D, and orthogonal D, such that A=QRQ'
    int m=alpha.size();
    Q.clear();
    Q.resize(m,vector<double>(m,0));
    D.clear();
    D.resize(m,0);
    DCSub(alpha, beta, Q, D, 0, m-1);
}

void QRHessenberg(vector<vector<double> > &A, vector<vector<double> > &Q){
    //input: A m*m square Hessenberg Matrix
    //output: Upper triangular R, and orthogonal Q, such that A=QRQ'
    int n=A.size();
    Q.clear();
    Q.resize(n,vector<double>(n,0));
    vector<double> cs(n-1,0);
    vector<double> ss(n-1,0);
    for(int i=0;i<n;i++) Q[i][i]=1;
    for(int m=n;m>=2;m--){
        while(1){
            fill(cs.begin(),cs.end(),0);
            fill(ss.begin(),ss.end(),0);
            double delta=(A[m-2][m-2]-A[m-1][m-1])/2;
            double sign=1;
            if(delta<0) sign=-1;
            //Wilkinson shift
            double shift=A[m-1][m-1]-sign*A[m-1][m-2]*A[m-2][m-1]/(fabs(delta)+sqrt(delta*delta+A[m-1][m-2]*A[m-2][m-1]));
            for(int i=0;i<m;i++)
                A[i][i]-=shift;
            for(int i=0;i<m-1;i++){
                double a=A[i][i];
                double b=A[i+1][i];
                givensRotation(a,b,cs[i],ss[i]);
                for(int j=i;j<m;j++){
                    a=A[i][j];
                    b=A[i+1][j];
                    A[i][j]=cs[i]*a+ss[i]*b;
                    A[i+1][j]=-ss[i]*a+cs[i]*b;
                }
            }

            for(int j=1;j<m;j++){// cols    j-1, j                         c -s
                for(int i=0;i<=j;i++){// rows  0 ... j      [a ,b]   s  c
                    double a=A[i][j-1];
                    double b=A[i][j];
                    A[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                    A[i][j]=-ss[j-1]*a+cs[j-1]*b;
                }
            }
            for(int i=0;i<m;i++)
                A[i][i]+=shift;
            //Q=Q*G1...Gm
            for(int j=1;j<m;j++){
                for(int i=0;i<n;i++){
                    double a=Q[i][j-1];
                    double b=Q[i][j];
                    Q[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                    Q[i][j]=-ss[j-1]*a+cs[j-1]*b;
                }
            }
            if(fabs(A[m-1][m-2])<1e-10)
                break;
        }
    }
}
void QRHessenbergBasic(vector<vector<double> > &A, vector<vector<double> > &Q){
    //input: A m*m square Hessenberg Matrix
    //output: Upper triangular R such taht A=QRQ' for orthogonal matrix Q
    int m=A.size();
    vector<double> cs(m-1,0);
    vector<double> ss(m-1,0);
    double diff=1;
    Q.clear();
    Q.resize(m,vector<double>(m,0));
    for(int i=0;i<m;i++) Q[i][i]=1;
    while(1){
        for(int i=0;i<m-1;i++){
            double a=A[i][i];
            double b=A[i+1][i];
            givensRotation(a,b,cs[i],ss[i]);
            for(int j=i;j<m;j++){
                a=A[i][j];
                b=A[i+1][j];
                A[i][j]=cs[i]*a+ss[i]*b;
                A[i+1][j]=-ss[i]*a+cs[i]*b;
            }
        }

        for(int j=1;j<m;j++){// cols    j-1, j              c -s
            for(int i=0;i<=j;i++){// rows  0 ... j      [a ,b]  s  c
                double a=A[i][j-1];
                double b=A[i][j];
                A[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                A[i][j]=-ss[j-1]*a+cs[j-1]*b;
            }
        }
        //Q=Q*G1...Gm
        for(int j=1;j<m;j++){
            for(int i=0;i<m;i++){
                double a=Q[i][j-1];
                double b=Q[i][j];
                Q[i][j-1]=cs[j-1]*a+ss[j-1]*b;
                Q[i][j]=-ss[j-1]*a+cs[j-1]*b;
            }
        }
        diff=0;
        for(int i=0;i+1<m;i++){
            diff+=A[i+1][i]*A[i+1][i];
        }
        diff=sqrt(diff);
        if(diff<1e-10)
            break;
    }
}


void QRFactorization(vector<vector<double> > &A, vector<vector<double> > &Q, vector<vector<double> > &R){
    //A=Q*R
    //A: m*n matrix
    //Q: m*m matrix
    //R: m*n matrix
    int m=A.size();
    int n=A[0].size();


    for(int i=0;i<Q.size();i++)
    {
        fill(Q[i].begin(),Q[i].end(),0);
    }
    for(int i=0;i<R.size();i++)
        fill(R[i].begin(),R[i].end(),0);



    vector<double> q(m);
    vector<double> r(n);
    for(int i=0;i<n;i++){
        for(int j=0;j<m;j++)
            q[j]=A[j][i];
        fill(r.begin(),r.end(),0);
        for(int j=0;j<i && j<m;j++)
        {
            r[j]=0;
            for(int k=0;k<q.size();k++)
                r[j]+=Q[k][j]*q[k];
            for(int k=0;k<q.size();k++)
                q[k]-=Q[k][j]*r[j];
        }
        if(i<m)
        {
            r[i]=normalize(q);
            for(int j=0;j<m;j++)
            {
                Q[j][i]=q[j];
            }
        }

        for(int j=0;j<m;j++){
            R[j][i]=r[j];
        }
    }

}


void lanczos(SparseMatrix &A, vector<vector<double> > &P, vector<double> &alpha, vector<double> &beta, unsigned int rank){
    //P'*A*A'*P = T = diag(alpha) + diag(beta,1) + diag(beta, -1)
    //P=[p1,p2, ... , pk]
    rank=min(A.cols,min(A.rows,rank));
    vector<double> p;
    unsigned int m=A.rows;
    unsigned int n=A.cols;
    vector<double> prevP(m,0);
    randUnitVector(m,p);
    P.clear();
    P.resize(m,vector<double>(rank,0));
    vector<double> v;
    alpha.clear();alpha.resize(rank);
    beta.clear();beta.resize(rank);
    beta[0]=0;
    for(int i=0;i<rank;i++){
        for(int j=0;j<p.size();j++){
            P[j][i]=p[j];
        }
        rightMultiply(A, p, v);
        alpha[i]=dotProduct(p,v);
        if(i+1<rank){
            for(int j=0;j<m;j++)
                v[j]=v[j]-beta[i]*prevP[j]-alpha[i]*p[j];
            beta[i+1]=norm(v);
            prevP=p;
            for(int j=0;j<m;j++)
                p[j]=v[j]/beta[i+1];
        }
    }
}

void lanczosT(SparseMatrix &A, vector<vector<double> > &P, vector<double> &alpha, vector<double> &beta, unsigned int rank){
    //P'*A'*A*P = T = diag(alpha) + diag(beta,1) + diag(beta, -1)
    //P=[p1,p2, ... , pk]
    rank=min(A.cols,min(A.rows,rank));
    vector<double> p;
    unsigned int m=A.rows;
    unsigned int n=A.cols;
    vector<double> prevP(n,0);
    randUnitVector(n,p);
    P.clear();
    P.resize(n,vector<double>(rank,0));
    vector<double> v;
    alpha.clear();alpha.resize(rank);
    beta.clear();beta.resize(rank);
    beta[0]=0;
    for(int i=0;i<rank;i++){
        for(int j=0;j<p.size();j++){
            P[j][i]=p[j];
        }
        leftMultiply(A, p, v);
        alpha[i]=dotProduct(p,v);
        if(i+1<rank){
            for(int j=0;j<n;j++)
                v[j]=v[j]-beta[i]*prevP[j]-alpha[i]*p[j];
            beta[i+1]=norm(v);
            prevP=p;
            for(int j=0;j<n;j++)
                p[j]=v[j]/beta[i+1];
        }
    }
}

void QRbasic(vector<vector<double> > &T, vector<vector<double> > &W){
    int l=T.size();
    W.clear();
    W.resize(l,vector<double>(l,0));
    vector<vector<double> > Q(l,vector<double>(l,0));
    vector<vector<double> > R(l,vector<double>(l,0));
    vector<vector<double> > nextW(l,vector<double>(l,0));
    for(int i=0;i<l;i++)
        W[i][i]=1;
    while(true){
        //T=Q*R
        QRFactorization(T,Q,R);
        //T=R*Q
        multiply(R,Q,T);
        //W=W*Q;
        multiply(W,Q,nextW);
        double diff=0;
        for(int i=0;i<l;i++)
            for(int j=0;j<l;j++)
                diff+=(W[i][j]-nextW[i][j]) * (W[i][j]-nextW[i][j]);
        W=nextW;
        if(diff<EPS*EPS)
            break;
    }
}
void svds(SparseMatrix &A, int r, vector<vector<double> > &U, vector<double> &s, vector<vector<double> > &V, string algo){
    //A=U*diag(s)*V'
    //A:m*n matrix sparse matrix
    //U:m*r matrix, U[i]=i th left singular vector
    //s:r vector
    //V:n*r matrix, V[i]=i th right singular vector
    int m=A.rows;
    int n=A.cols;

    //lanczos: A*A'=P*T*P'
    if(m<=n){
        int l=m;
        vector<vector<double> > P(m,vector<double>(l,0));
        vector<double> alpha(l,0);
        vector<double> beta(l,0);
        lanczos(A,P,alpha,beta,l);
        vector<vector<double> > W;
        if(algo=="QR"){
            vector<vector<double> > T(l,vector<double>(l,0));
            for(int i=0;i<l;i++){
                T[i][i]=alpha[i];
                if(i)
                    T[i-1][i]=T[i][i-1]=beta[i];
            }
            QRTridiagonal(T,W);
        }else if(algo=="DC"){
            vector<double> D(l,0);
            vector<vector<double> > Q;
            DCTridiagonal(alpha,beta,Q,D);
            //need sort
            vector<int> index;
            merge_sort(D,index);
            reverse(index.begin(),index.end());
            W.resize(l,vector<double>(l));
            for(int i=0;i<l;i++)
                for(int j=0;j<l;j++)
                    W[i][j]=Q[i][index[j]];
        }
        U.clear();U.resize(m,vector<double>(l));
        multiply(P,W,U);
        for(int i=0;i<U.size();i++)
            U[i].resize(r);
        V.clear();V.resize(n,vector<double>(r));
        rightMultiply(U,A,V);
        s.clear();s.resize(r,0);
        for(int i=0;i<r;i++){
            for(int j=0;j<n;j++)
                s[i]+=V[j][i]*V[j][i];
            s[i]=sqrt(s[i]);
            if(s[i]>EPS){
                for(int j=0;j<n;j++)
                V[j][i]/=s[i];
            }
        }
    }else{
        int l=n;
        vector<vector<double> > P(n,vector<double>(l,0));
        vector<double> alpha(l,0);
        vector<double> beta(l,0);
        lanczosT(A,P,alpha,beta,l);

        vector<vector<double> > W;
        if(algo=="QR"){
            vector<vector<double> > T(l,vector<double>(l,0));
            for(int i=0;i<l;i++){
                T[i][i]=alpha[i];
                if(i)
                    T[i-1][i]=T[i][i-1]=beta[i];
            }
            QRTridiagonal(T,W);
        }else if(algo=="DC"){
            vector<double> D(l,0);
            vector<vector<double> > Q;
            DCTridiagonal(alpha,beta,Q,D);
            //need sort
            vector<int> index;
            merge_sort(D,index);
            reverse(index.begin(),index.end());
            W.resize(l,vector<double>(l));
            for(int i=0;i<l;i++)
                for(int j=0;j<l;j++)
                    W[i][j]=Q[i][index[j]];

        }
        V.clear();V.resize(n,vector<double>(l,0));
        U.clear();U.resize(m,vector<double>(r,0));
        multiply(P,W,V);
        for(int i=0;i<V.size();i++)
            V[i].resize(r);
        leftMultiply(V,A,U);
        s.clear();s.resize(r,0);
        for(int i=0;i<r;i++){
            for(int j=0;j<m;j++)
                s[i]+=U[j][i]*U[j][i];
            s[i]=sqrt(s[i]);
            if(s[i]>EPS){
                for(int j=0;j<m;j++)
                U[j][i]/=s[i];
            }
        }
    }
}
