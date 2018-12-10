#ifndef SVDS_H
#define SVDS_H
#include <vector>
#include <string>
using namespace std;
class Cell{
public:
    unsigned int row;
    unsigned int col;
    double value;
    Cell():row(0),col(0),value(0){};
    Cell(int r, int c, double v):row(r),col(c),value(v){};
};
class SparseMatrix{
public:
    unsigned int rows;
    unsigned int cols;
    vector<Cell> cells;

    int cellID;

    //read data sequentially, especially when loading large matrix from the disk.
    void moveFirst(){
        cellID=0;
    }
    bool hasNext(){
        return cellID<cells.size();
    }
    Cell next(){
        return cells[cellID++];
    }
};

void svds(SparseMatrix &A, int r, vector<vector<double> > &U, vector<double> &s, vector<vector<double> > &V, string algo="DC");
//decompose A = U * diag(s) *V'
//where A is a m*n matrix
//U is a m*r column orthogonal matrix
//s is a length r vector
//V is a n*r column orthogonal matrix

void print(vector<vector<double> > &X);
void hessenbergReduction(vector<vector<double> > &A, vector<vector<double> > &U);
void QRHessenbergBasic(vector<vector<double> > &A, vector<vector<double> > &Q);
void QRbasic(vector<vector<double> > &T, vector<vector<double> > &W);
void QRFactorization(vector<vector<double> > &A, vector<vector<double> > &Q, vector<vector<double> > &R);
void transpose(vector<vector<double> > &A, vector<vector<double> > &T);
void multiply(vector<vector<double> > &A, vector<vector<double> > &B, vector<vector<double> > &C);
void QRHessenberg(vector<vector<double> > &A, vector<vector<double> > &Q);
void QRTridiagonal(vector<vector<double> > &A, vector<vector<double> > &Q);
#endif
