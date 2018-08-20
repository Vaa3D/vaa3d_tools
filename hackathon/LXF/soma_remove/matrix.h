#ifndef MATRIX_H
#define MATRIX_H

//Matrix.h


#include <iostream>
//#include <vector>
//#include <exception>
using namespace std;
//----------------------------Definition of a Matrix class-----------------------------------------------

class Matrix
 {
    public:
        Matrix(); //default constructor
        Matrix( int m,  int n);//declare an mxn matrix
        Matrix(const Matrix& A); //copy constructor

        ~Matrix();//destructor
        //Assignment
        Matrix& operator = (const Matrix& A); //overloading =
        //operators
        bool operator == (const Matrix& A);//overloading ==
        bool operator != (const Matrix& A);//overloading !=

        Matrix& operator += (const Matrix& A); //overloading +=
        Matrix& operator -=(const Matrix& A); //overloading -=
        Matrix& operator *=(const Matrix& A); //overloading *=
        Matrix& operator *=(double a); //overloading *=

        Matrix operator + () ; //unary +
        Matrix operator - () ; //unary -
        double & operator ()( int i,  int j);// access (i,j)
        double & operator()( int i,  int j) const; //read only

        //member functions
        Matrix Transpose() ; //transpose
        Matrix Inverse();//Inverse Matrix
        Matrix Null(int n); //make a "zero" Matrix, with a new dimension
        Matrix Null();//make a "zero" Matrix, does not change the dimension
        Matrix Identity( int n);//make a nxn identity matrix
        Matrix Identity();//make a identity matrix, does not change the dimentsion
        bool LU(Matrix& L, Matrix& U);//LU decomposition. return true if successful
        bool QR(Matrix& Q, Matrix& R); //QR decomposition. return true is successful.

        double det();//determinant(Matrix)

        Matrix Eigenvalues();//find the eigen values and store them in a vector (mx1 Matrix)
        Matrix Root(const Matrix& b);//solving linear system of equations. b is actually a vector (mx1 Matrix)

        Matrix Adjugate(); //Adjoint/Adjugate
        double Cofactor(int i, int j); //cofactor Cij
        Matrix Cofactor();//matrix of cofactors
        double Minor(int i, int j);//Minor Mij

        //
        bool IsSingular();
        bool IsSymmetric();
        //

        //Added
        const int GetRows() const; //Get the dimension of the Matrix
        const int GetColumns() const;

    private:
        int rows;
        int columns;
        double* buf;

    //<< and >>
    friend ostream& operator << (ostream& output, const Matrix& A);
    friend istream& operator >> (istream& input, Matrix& A);

       // friend iostram& operator <<(iostream& output, const Matrix& A);

};


//operators, + - * can be overloaded as global operators

Matrix operator + (const Matrix& A, const Matrix& B); //Matrix A+B, using += .....
Matrix operator - (const Matrix& A, const Matrix& B); //Matrix A+B, using -= .....
Matrix operator * (const Matrix& A, const Matrix& B); //Matrix A+B, using *= .....
Matrix operator * (double a, const Matrix& A); //Matrix a*A, using *= .....
Matrix operator * (const Matrix& A, double a ); //Matrix A*a, using *= .....

#endif // MATRIX_H
