
#include "matrix.h"
#include "general.h"
#include <math.h>
#include <cmath>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <limits>
using namespace std;
//------------------------------ Implemenation  -----------------------------
Matrix::Matrix() //default constructor
{
    rows=columns=0;
    buf=NULL;
        cout<<"call constructor"<<endl;
}
Matrix::Matrix( int m,  int n)//declare an mxn matrix
{
    if(!this->buf)delete[] this->buf;
    this->rows=m;
    this->columns=n;
    this->buf=new double[m*n];
    cout<<"call constructor"<<endl;
}
Matrix::Matrix(const Matrix& A) //copy constructor
{
     //                          Matrix C(C); ??????????? could not pass compile, don't worry

//      if(!this->buf)delete[] this->buf;
      this->rows=A.rows;
      this->columns=A.columns;
      this->buf=new double[(A.rows)*(A.columns)];
      int i;
      for(i=0;i<((A.rows)*(A.columns));i++)
        {
          this->buf[i]=A.buf[i];
        }
      cout<<"call copy construcotor"<<endl;
}
Matrix::~Matrix()//destructor
{
  delete[] this->buf;
  this->rows=0;
  this->columns=0;
  cout<<"call destructor"<<endl;
}
//Assignment
Matrix& Matrix::operator = (const Matrix& A) //overloading =
{
        if(this==&A) return *this;
        if(!buf) delete[] buf;
        columns=A.columns;
        rows=A.rows;
        buf=new double[columns*rows];
        int i;
        for(i=0;i<(columns*rows);i++)
           {
            buf[i]=A.buf[i];
           }
    return *this;
}
//operators
bool Matrix::operator == (const Matrix& A)//overloading ==
{
    int i;
        if(!this->buf||!A.buf)
            {
              cout<<"Two Empty Matrix"<<endl;
              return true;
            }
        if(this->columns!=A.columns||this->rows!=A.rows)
              return false;
        else
            {
             for(i=0;i<columns*rows;i++)
                 {
                  if(abs(this->buf[i]-A.buf[i])>1e-10) return false;
                 }

            }
        return true;
}
bool Matrix::operator != (const Matrix& A)//overloading !=
{
    return !(*this==A); //use ==
}
Matrix& Matrix::operator += (const Matrix& A) //overloading +=
{
        if(!A.buf) return *this;
        if ( (this->rows != A.rows) || (this->columns != A.columns))
    {
        //cerr << "Size mismatch in matrix addition" << endl;
        throw logic_error ("Size mismatch in matrix addition");
    }
        for(int i=0;i<A.columns*A.rows;i++)
            this->buf[i]+=A.buf[i];

    return *this;
}
Matrix& Matrix::operator -=(const Matrix& A) //overloading -=
{
        if(!A.buf) return *this;
        if ( (this->rows != A.rows) || (this->columns != A.columns))
    {
        //cerr << "Size mismatch in matrix addition" << endl;
        throw logic_error ("Size mismatch in matrix addition");
    }
        for(int i=0;i<A.columns*A.rows;i++)
            this->buf[i]-=A.buf[i];
    return *this;
}
Matrix& Matrix::operator *=(const Matrix& A) //overloading *=
{
        if(!A.buf)    throw logic_error (" You are Multipling Empty Matrix");
        if(this->columns!=A.rows)    throw logic_error ("Size Mismatch!");
        if(A.columns==0||A.rows==0||this->columns==0||this->rows==0)  throw logic_error ("go check your weried matrix first");
       // Matrix tmp(*this);
        //delete[] this->buf;
        //this->buf= new double[this->rows*A.columns];
        Matrix tmp(this->rows, A.columns);
    for (int i=1; i<=tmp.rows; i++)
    {
        for (int j=1; j<=tmp.columns; j++)
        {
            tmp(i,j) = 0;
            for (int k=1; k<=A.rows; k++)
                tmp(i,j) += (*this)(i,k) * A(k,j);
        }
    }
    *this = tmp;
    return *this;
}
Matrix& Matrix::operator *=(double a) //overloading *=
{
        if(!this->buf)	throw logic_error ("please Check your empty Matrix first");
        for(int i=0;i<columns*rows;i++)
            {
             this->buf[i]*=a;
            }
    return *this;
}
Matrix Matrix::operator + () //unary +
{
    return *this; //good enough.
}
Matrix Matrix::operator - () //unary -
{
    if ((this->rows == 0) || (this->columns == 0) )
    {
        throw invalid_argument ("Missing matrix data");
    }
    Matrix tmp(this->rows, this->columns);
    for (int i=0; i<rows*columns; ++i)
    {
               tmp.buf[i]=-(this->buf[i]);
    }
    return tmp;
}
double& Matrix::operator ()( int i,  int j)// access (i,j)
{
        if(i>this->rows||j>this->columns)   throw logic_error ("Matrix is not this big");
        if(i<=0||j<=0)	throw logic_error ("can not access, your index is wrong");
    return buf[(i-1)*columns+(j-1)]; // is this correct? Unsafe
}
double& Matrix::operator()( int i,  int j) const //read only
{
    //return buf[i*columns+j]; // is this correct? Unsafe
    return buf[(i-1)*columns+(j-1)]; // is this correct? Unsafe

}
ostream& operator << (ostream& output, const Matrix& A)
{

    for ( int i = 1; i <= A.rows; i++)
    {
        for ( int j = 1; j <= A.columns; j++)
            output << A(i,j) << "\t   ";
        output << endl;
    }

    return output;
}



istream& operator >> (istream& input, Matrix& A)
{


    if (input.fail())
    {
        throw runtime_error ("Error reading input");
    }

    BypassComment(input);
    input >> A.rows;
    BypassComment(input);
    input >> A.columns;
        cout<<A.rows<<"*"<<A.columns<<endl; // for debug


    if (A.rows<=0 || A.columns<=0)
    {
        throw invalid_argument ("Invalid matrix data");
    }
        if (A.buf) delete[] A.buf;//check if A.buf=NULL, if not,delete A
        A.buf=new double[A.rows*A.columns];
    for (int i=1; i<=A.rows; i++)
    {
        for (int j=1; j<=A.columns; j++)
        {
            BypassComment(input);
            input >> A(i,j);
                        //cout<<A(i,j);
        }
    }

    //use BypassComment to skip comments.

    return input;
}


//------------Member Functions------------------------------
Matrix Matrix::Adjugate() //Adjoint/Adjugate
{
    Matrix tmp;
    tmp=this->Cofactor();
    tmp=tmp.Transpose();
    return tmp;
}
double Matrix::Cofactor(int i, int j) //cofactor Cij
{
        double tmp;
        tmp=this->Minor(i,j);
        tmp=pow(-1,(i+j))*tmp;
//	double tmp;
    return tmp;
}
Matrix Matrix::Cofactor()//matrix of cofactors
{

    if(!this->buf)    throw logic_error (" Empty Matrix ");
    Matrix tmp(this->rows, this->columns);
        for (int i=1;i<=this->rows;i++)
              {
                for (int j=1;j<=this->columns;j++)
                        {
                          tmp(i,j)=this->Cofactor(i,j);
                        }
              }
    return tmp;

}
double Matrix::Minor(int i, int j)//Minor Mij
{
        double tmp;
        Matrix A;
        A.rows=(this->rows)-1;
        A.columns=(this->columns)-1;
        A.buf=new double[A.rows*A.columns];
        int a=0;
          for(int m=1;m<=this->rows;m++)
              {
                for(int n=1;n<=this->columns;n++)
                   {
                      if(m==i) continue;
                      if(n==j) continue;
                      A.buf[a]=(*this)(m,n);
                      a++;
                   }
              }
        tmp=A.det();
        return tmp;

}
bool Matrix::IsSingular()
{
    //return (this->det()==0);  //may not work, because of double precision. you fix it.
         return (fabs(this->det()-0)<0.00000001);
}
bool Matrix::IsSymmetric()
{
    return ((*this)==(this->Transpose()));
}
const int Matrix::GetRows() const
{
    return rows;
};

const int Matrix::GetColumns() const
{
    return columns; //
};
Matrix Matrix::Transpose()  //transpose
{
    //check size
    if ((this->GetRows() == 0) || (this->GetColumns() == 0) )
    {
        //cerr << "Missing matrix data" << endl;
        throw invalid_argument ("Missing matrix data");
    }

    Matrix tmp(this->GetColumns(), this->GetRows());
    for (int i=1; i<=tmp.GetRows(); ++i)
    {
        for (int j=1; j<=tmp.GetColumns(); ++j)
            tmp(i,j) = (*this)(j,i);
    }
    return tmp;
}
Matrix Matrix::Inverse()//Inverse Matrix
{
    Matrix tmp;
        if((*this).GetRows()!=(*this).GetColumns())
           {throw logic_error ("Solving for Inverse fail: Not a square    matrix!");
            //return (*this);
           }
        if(fabs(this->det()-0)<0.000000001)
           {
             throw logic_error ("determinant equal to zero, can not do inverse");
           }
        Matrix A;
        A=this->Adjugate();
        tmp=(1/this->det())*A;
    return tmp;
}
Matrix Matrix::Null(int n) //make a "zero" Matrix, with a new dimension, change "this"
{
/*
        if ( n==0 || this->GetRows==0 || this->GetColumns==0)
    {
        //cerr << "Size mismatch in matrix addition" << endl;
        throw logic_error ("can not do Null");
    }

*/

//	if(n==0||this->GetRows==0||this->GetRows==0) return false;
    this->rows=n;
    this->columns=n;
    this->buf=new double[n*n];
    for(int a=0;a<n;a++)
    {
        for(int b=0;b<n;b++)
            this->buf[a*this->columns+b]=0;
    }

    return *this;
}
Matrix Matrix::Null()//make a "zero" Matrix, does not change the dimension, change "this"
{
    //if(this->GetRows()=0||this->GetColumns()=0) return false;
    for(int a=0;a< this->GetRows();a++)
    {
        for(int b=0;b< this->GetColumns();b++)
            this->buf[a*this->GetColumns()+b]=0;
    }
    return *this;
}
Matrix Matrix::Identity( int n)//make a nxn identity matrix,change "this"
{
       // if(n==0) return false;

        if(n<=0)
         throw invalid_argument ("argument has to be larger than 0");

    this->rows=n;
    this->columns=n;
    this->buf=new double[n*n];
    int a;
    int b;
    for(a=0;a< this->GetRows();a++)
    {
        for(b=0;b< this->GetRows();b++)
        {
            if(a==b)
                this->buf[a*n+b]=1;
        else
            this->buf[a*n+b]=0;
        }
        }
    return *this;
}
Matrix Matrix::Identity()//make a identity matrix, does not change the dimentsion, change "this"
{
    if(this->GetRows()!=this->GetColumns() || this->GetRows()==0)
             throw logic_error ("Check your matrix . Matrix has to be squre ");
    int a;
    int b;
    for(a=0;a<this->GetRows();a++)
    {
        for(b=0; b< this->GetColumns();b++)
        {
            if(a==b)
                this->buf[a*this->columns+b]=1;
        else
            this->buf[a*this->columns+b]=0;
        }
    }
    return *this;
}
bool Matrix::LU(Matrix& L, Matrix& U)//LU decomposition. return true if successful
{
/*
//check size
        if ((this->GetRows() == 0) || (this->GetColumns() == 0) || (this->GetRows() != this->GetColumns()))
            return false; //cannot do it.
        Matrix B = *this;
        B.Null();//zero out
        L=B;
        U=B;
        //check for pivot point (normalized to 1), record row change permutation and row scaling factor
        int* rowIndex; double* rowScale;
        try
        {
            rowIndex = new int [B.rows];
            rowScale = new double [B.rows];
        }
        catch (...)	//mostly bad_alloc&
        {
            cout << "Error allocating memory" << "\n";
            if (rowIndex) delete[] rowIndex; rowIndex = NULL;
            if (rowScale) delete[] rowScale; rowScale = NULL;
            return false;
        }

        int detSign = 1;
        double tol = numeric_limits<double>::min();
                cout<<tol;
        double tmp, eMax; int rowMax;
        for (int i=0; i<B.rows; ++i)
        {
            for (int j=0; j<B.columns; ++j)
            {
                tmp = fabs(B.buf[i*B.columns+j]);
                if (tmp>eMax) eMax = tmp;
            }
            if (eMax<tol)
            {
                cout << "Matrix is singular" << "\n";
                detSign = 0;
                return false;
            }

            rowScale[i] = 1/eMax;
        }

        for (int j=0; j<B.columns; ++j)
        {
            eMax = 0;
            for (int i=j; i<B.rows; ++i)
            {
                tmp = fabs(B.buf[i*B.columns+j]) * rowScale[i];
                if (tmp>eMax)
                {
                    eMax = tmp;
                    rowMax = i;
                }
            }
            if (eMax<tol)
            {
                cout << "Matrix is singular" << "\n";
                detSign = 0;
                return false;
            }

            //if row interchange is performed, record new detSign and row scaling factor
            if (rowMax != j)
            {
                for (int k=0; k<B.columns; ++k)
                {
                    tmp = B.buf[j*B.columns+k];
                    B.buf[j*B.columns+k] = B.buf[rowMax*B.columns+k];
                    B.buf[rowMax*B.columns+k] =tmp;
                }
                detSign = -detSign;
                tmp = rowScale[j];
                rowScale[j] = rowScale[rowMax];
                rowScale[rowMax] = tmp;
            }
            rowIndex[j] = rowMax; //used for root solving (need to modify variables declaration of LU function)
        }

        //same procedure as Crout's algorithm without pivoting
        for (int j=0; j<B.columns; ++j)
        {
            for (int i=0; i<=j; ++i)
            {
                U.buf[i*U.columns+j] = B.buf[i*B.columns+j];
                for (int k=0; k<i; ++k)
                {
                    U.buf[i*U.columns+j] -= L.buf[i*L.columns+k] * U.buf[k*U.columns+j];
                }
                if (i==j) L.buf[i*L.columns+j] = 1;
                else L.buf[i*L.columns+j] = 0;
            }

            for (int i=j+1; i<B.rows; ++i)
            {
                if (fabs(U.buf[j*U.columns+j]) < tol)
                {
                    cout << "Matrix is singular" << "\n";
                    detSign = 0;
                    return false;
                }
                L.buf[i*L.columns+j] = B.buf[i*B.columns+j] / U.buf[j*U.columns+j];
                for (int k=0; k<j; ++k)
                {
                    L.buf[i*L.columns+j] -= L.buf[i*L.columns+k] * U.buf[k*U.columns+j] / U.buf[j*U.columns+j];
                }
                U.buf[i*U.columns+j] = 0;
            }
        }

        return true;
*/


/*

    if(this->GetRows()==0 || this->GetRows()!=this->GetColumns()) return false;
    Matrix B=*this;
    B.Null();
    L=B;
    U=B;
    int i,j,n,k,m=1;
    for(i=0;i<this->GetColumns();i++)
        U.buf[i]=this->buf[i];
    for(i=0;i<this->GetColumns();i++)
        L.buf[i*columns+1]=this->buf[i*columns+1];
    while(m<this->GetColumns())
        {
            for(j=m;j<this->GetRows();j++)
                {
                    double sum=0;
                    for(k=0;k<m-1;k++)
                        sum+=L.buf[m*columns+k]*U.buf[k*columns+j];
                    U.buf[m*columns+j]=this->buf[m*columns+j]-sum;
                    if(U.buf[m*columns+j]==0) return false;
                }
            for(j=m;j<this->GetRows();j++)
                {
                    double sum2=0;
                    for(k=0;k<m-1;k++)
                        sum2+=L.buf[j*columns+k]*U.buf[k*columns+m];
                    L.buf[j*columns+m]=this->buf[j*columns+m]-sum2;
                }
            m++;
        }

return true;

*/




/*
double m=0;
        //check this Matrix
        if(!this->buf)
            cout<<"Matrix A has no data"<<endl;
        if(this->columns!=this->rows)
            cout<<"Matrix has to be square matrix for LU decomposition"<<endl;
        if(L.buf)
          {
            L.rows=L.columns=0;
            delete[] L.buf;
            L.buf=NULL;
           }
        L.columns=this->columns;
        L.rows=this->rows;
        L.buf=new double[L.rows*L.columns];

        if(U.buf)
          {
            U.rows=U.columns=0;
            delete[] U.buf;
            U.buf=NULL;
           }
        U.columns=this->columns;
        U.rows=this->rows;
        U.buf=new double[U.rows*U.columns];

        // initialize Matrix U
        for(long i=0;i< this->rows*this->columns;i++)
        {
            // *(U.buf+i)=*(A.buf+i);
            U.buf[i]=this->buf[i];
        }
        // initialize Matrix L
        for(long i=0;i< this->rows;i++)
        {
            for (long k=0;k< this->columns;k++)
            {
               if(i==k)               // ==,==,==,==,==
                   // *(L.buf+i*A.columns+k)=1;
                   L.buf[i*this->columns+k]=1;
               else
                   // *(L.buf+i*A.columns+k)=0;
                   L.buf[i*this->columns+k]=1;
            }

        }

      //LU decomposition start from here
        for(long i=0; i< this->columns;i++)
        {
            for(long j=i+1;j< this->columns;j++)
            {
                //if(*(U.buf+i*U.columns+i)==0)
                if(U.buf[i*this->columns+i]==0)
                {   cout<<"Matrix can not be divided by LU decomposition"<<endl;
                    return false;
                }
                else
                {
                   //  m=*(U.buf+j*U.columns+i)/(*(U.buf+i*U.columns+i));
                      m=U.buf[j*this->columns+i]/U.buf[i*this->columns+i];
                    // *(L.buf+j*L.columns+i)=*(U.buf+j*U.columns+i)/(*(U.buf+i*U.columns+i));// get Matrix L
                      L.buf[j*this->columns+i]=U.buf[j*this->columns+i]/U.buf[i*this->columns+i];
                    for(long k=i;k<this->columns;k++)
                   {
                     // *(U.buf+j*A.columns+k)=*(U.buf+j*A.columns+k)-m*(*(U.buf+i*A.columns+k));//get Marrix U
                         U.buf[j*this->columns+k]=U.buf[j*this->columns+k]-m*U.buf[i*this->columns+k];
                   }
                }

            }

        }

        return true;
*/

if(this->rows!=this->columns)   //LU decomposition only can be operated on square Matrix
    {
       cout<<"Matrix A is not a square Matrix! Please check the data again."<<endl;
        return false;
    }


    double bigflag=0.0; // bigflag is used for storing the biggest value date in the matrix A to judge whether all of the items in a rows equals 0( in this case, |A|=0, ;
    for(long i=0;i< this->rows;i++)  //primary check the prerequisite for LU decomposition.
    {
        bigflag=0.0;
        for(long j=0;j<this->columns;j++)   //Becareful A.rows=A.columns! here is just for easy reading and understanding
         {
           //if(fabs(*(A.buf+A.rows*i+j))>bigflag)
             if(fabs(this->buf[i*this->rows+j])>bigflag)

                //bigflag=*(A.buf+A.rows*i+j);    //to find the biggest value data in one row
                 bigflag=this->buf[i*this->rows+j];
         }
        if(bigflag==0.0)
          {
            cout<<"No nonzero largest element in the"<<i+1<<"th rows.(The det(A)=0, which does not meet the requestment of LU decomposition.)"<<endl;
            //" Matrix A may be a SINGULAR Matrix, which maybe can't be decomposed. Should be check again???"
          return false;
          }

    }

    if(L.buf)  //if L have data, delete the data
    {
        L.rows=L.columns=0;
        delete[] L.buf;
        L.buf=NULL;
    }   //End of if L.buf...
    if(U.buf)  //if U have data, delete the data
    {
        U.rows=U.columns=0;
        delete[] U.buf;
        U.buf=NULL;
    }   //End of if U.buf ...

    L.rows=U.rows=this->rows;
    L.columns=U.columns=this->columns;  //set the L,U's rows and columns the same as the A's
    L.buf=new double[L.rows*L.columns];  //creat a buffer for storing the data
    U.buf=new double[U.rows*U.columns];
    for(long i=0;i<L.rows;i++)
        for(long j=0;j<L.columns;j++)
        {
        *(L.buf+L.columns*i+j)=0.0;
        *(U.buf+U.columns*i+j)=0.0;
        }    //init L,U=0;

    double sum=0.0; //temp varity used for store A[i][j] during the loop but init it to 0.0 here..

    cout<<"LU decomposing.";    //if n=very large, then should to tell user I am running.
    for(long n=0;n<this->rows;n++)  //n used for A[n][n]
    {
        // all L[n][n] is assumed =1.0;
        for(long j=n;j<U.columns;j++)   // for caculating U[][j], the nth rows
        {
            sum=*(this->buf+n*this->rows+j);    //here sum store A[n][j]
            for(long k=0;k<n;k++)
                sum-=(*(L.buf+n*L.columns+k))*(*(U.buf+k*U.columns+j) );
            *(U.buf+n*U.columns+j)=sum;
        }

        for(long i=0;i<L.rows;i++)   //for caculating L[i][], the nth columns
        {
            sum=*(this->buf+i*this->rows+n);    //here sum store A[n][j]
            for(long k=0;k<n;k++)
                sum-=(*(L.buf+i*L.rows+k))*(*(U.buf+k*U.rows+n) );
            if(*(U.buf+n*U.rows+n)==0)
                {   //if U[n][n]==0, then it means that A can not decomposed, for if U[n][n]=0, |A|=|L|*|U|=|L[n][n]|*|U[n][n]|=0, which can not meet the prerequisite of decomposing A;
                    cout<<"OOps, Zero in U["<<n<<"]["<<n<<"] is found, the matrix can not be decomposed. "<<endl;
                    L.rows=L.columns=0;
                    U.rows=U.columns=0;
                    delete[] L.buf;
                    L.buf=NULL;
                    delete[] U.buf;
                    U.buf=NULL; //Clear all of the value is useful for other function who call LU(), and detect whether the operation of LU is successful.Also free all the memory.
                    return false;
                }
            else
                {
                    *(L.buf+i*L.rows+n)=sum*1.0/(*(U.buf+n*U.rows+n));
                }
        }
        cout<<".";
    }

    return true;

}
bool Matrix::QR(Matrix& Q, Matrix& R)
{
    if ((this->GetRows() == 0) || (this->GetRows() != this->GetColumns()))
                return false;
    Matrix tmp,tmp2,B;
    B = (*this);
    R = B.Identity((*this).GetRows());
    Q = B.Identity((*this).GetRows());
    tmp2 = (*this);
    for(int j=1;j<=(*this).GetColumns();j++)//main loop by column
        for(int i=1;i<=(*this).GetRows();i++)
        {
            if(i>j)//under the diagonal
                {
                    //tmp = tmp2.MakeQij(i,j);
            tmp = B.Identity((*this).GetRows());
            tmp(i,i) = tmp2(j,j)/pow(tmp2(i,j)*tmp2(i,j)+tmp2(j,j)*tmp2(j,j),0.5);// c
            tmp(j,j) = tmp(i,i);// c
            tmp(i,j) = (-1) * tmp2(i,j)/pow(tmp2(i,j)*tmp2(i,j)+tmp2(j,j)*tmp2(j,j),0.5);// -s
                tmp(j,i) = (-1) * tmp(i,j); // s
                    tmp2=tmp*tmp2;

                    Q = tmp * Q;
                }
        }

    R = Q*(*this);//get R
    Q = Q.Transpose();//get Q

        return true;

}
double Matrix::det()//determinant(Matrix)
{
    cout<<"tttttt"<<endl;
//        if(this->rows!=this->columns)
//             throw logic_error ("Matrix has to be square to find det");
        cout<<"uuuuuuuu"<<endl;
    int r=this->GetRows();
    double tmp=1;

    Matrix L,U;

    this->LU(L,U);
    for(int i=0;i<r;i++)
        tmp=tmp*L.buf[i*r+i]*U.buf[i*r+i];
    return tmp;


/*        this->tmpMat;
        Matrix L1;
        Matrix U1;
        tmpMat.LU(L1,U1);
  */


/*if(this->rows!=this->columns)
             throw logic_error ("Matrix has to be square to find det");
    double tmp;
    if(this->rows==1)
        tmp=this->buf[1];
    else
        {
            Matrix sub;
            int i=0;
            for(int j=0;j<columns;j++)
            {
                sub=this->submatrix(i,j);
                tmp+=(-1)^(i+j)*this->buf[i*columns+j]*sub.det();
            }
        }
    return tmp;
*/



}
Matrix Matrix::Eigenvalues()//find the eigen values and store them in a vector (mx1 Matrix)
{
    if(this->IsSingular() || this->rows!=this->columns)
        throw logic_error ("Matrix has to be square and not singular");
    Matrix tmp,B,Q,R;
    int i,j,m=0;
    B=*this;
    tmp.rows=this->GetRows();
    tmp.columns=1;
    tmp.buf=new double(tmp.rows);
    while(m<=1)
    {
        B.QR(Q,R);
        B=R*Q;
        for(j=0;j<this->GetColumns();j++)
        {
            for(i=j+1;i<this->GetRows();i++)
            {
                if(B.buf[i*columns+j]<0.0000000001) continue;
                else
                {
                    m=0;
                    break;
                }
            }
        }
    m++;
    }
    for(i=0;i<this->GetRows();i++)
    tmp.buf[i]=B.buf[i*columns+i];
    return tmp;

}
Matrix Matrix::Root(const Matrix& b)//solving linear system of equations. b is actually a vector (mx1 Matrix)
{
        if(b.rows!=this->GetRows() || this->det()==0)
        throw logic_error ("two matrices should have same rows and cannot be Singular");
    Matrix tmp;
    tmp=(this->Inverse());
    tmp=tmp*b;

    return tmp;
}
//------------------------------------------------------------------------------------------------
//operators, + - * can be overloaded as global operators

Matrix operator + (const Matrix& A, const Matrix& B) //Matrix A+B, using += .....
{
    Matrix tmp=A;
    tmp+=B;//use "+="
    return tmp;//   done
}

Matrix operator - (const Matrix& A, const Matrix& B) //Matrix A+B, using -= .....
{
    Matrix tmp=A;
    tmp-=B;//use "-="
    return tmp;//    done
}

Matrix operator * (const Matrix& A, const Matrix& B) //Matrix A+B, using *= .....
{
    Matrix tmp=A;
    tmp*=B;//use "*="
    return tmp;//    done
}


Matrix operator * (double a, const Matrix& A) //Matrix a*A, using *= .....
{
    Matrix tmp=A;
    //do a*A
        tmp*=a;
    return tmp;   //done
};


Matrix operator * (const Matrix& A, double a ) //Matrix A*a, using *= .....
{
    Matrix tmp=A;
    //do A*a
        tmp*=a;
    return tmp;
};

