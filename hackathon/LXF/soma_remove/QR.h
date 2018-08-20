#ifndef QR_H
#define QR_H
#include<iostream>
#include<math.h>
#include <iomanip>
#include <vector>
using namespace std;
//#define M  500 //A矩阵的行
//#define  N  5 //A矩阵的列
float Get_Norm(float a[],int size);/*向量求模*/
float *VectorDivNum(float a[],float num,int len);/*向量除以一个数*/
float * VectorMULNum(float a[],float num,int len);/*向量数乘*/
float  VectorMUL(float a[],float b[],int len);/*向量内积*/
float * VectorADD(float a[],float b[],int len); /*向量加法*/
float * VectorSUB(float a[],float b[],int len);/*向量减法*/
void Matrix_QR_Factorization(vector<vector<float> >&A_a,vector<vector<float> >&Q_a,vector<vector<float> >&R_a);

#endif // QR_H
