
/* C++ IMPLEMENTATION OF PRINCIPAL CURVE TRACING USING MEX INTERFACE

Code developed by: Nikhila Srikanth
                   M.S. Project
                   Cognitive Systems Laboratory
                   Electrical and Computer Engineering Department
                   Northeastern University, Boston, MA

Under the guidance of: Prof. Deniz Erdogmus and Dr. Erhan Bas

Date Created: Jan 20, 2011
Date Last Updated: April 29, 2011

This code implements Gauss-Jordan elimination technique to compute inverse of a
matrix. The code refers to the algorithm present in Numerical Receipes for C.
This code is meant to run on the Windows platform
*/

//#include <mex.h>
//#include <matrix.h>
#include <math.h>

#include "ML_Divide.h"

#define ROWCOL(row,col,tot_rows) (row + col*tot_rows)

#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

// Performs operation: AX = B, i.e X = (A inverse)*B
// Requirements:
//	A must be a square matrix.
//  Number of rows in B = number of rows in A.
void Compute_ML_Divide(double *pdInput_A, double *pdInput_B, int iNum_of_rows_of_A, int iNum_of_cols_of_A,
                       int iNum_of_rows_of_B, int iNum_of_cols_of_B, double *pdOutput)
{
    double temp;
    double *pd_Matrix_A, *pd_Matrix_B; // Matrix_A initialized to copy of A. Matrix_B to copy of B. Eventually, A will be reduced to Identity form, and B to the final output.

    int iNum_of_row_and_cols_of_A, iNum_of_elems_of_A, iNum_of_elems_of_B  ;
    int iIter;

    if (iNum_of_rows_of_A == iNum_of_cols_of_A) // confirms that A is square matrix.
    {
        iNum_of_row_and_cols_of_A = iNum_of_rows_of_A; // = iNum_of_cols_of_A
    }
    else
    {
        printf ("\nError (Compute_ML_Divide): A is not a square matrix.");
        //exit (1);
    }

    pd_Matrix_A = (double *) malloc (iNum_of_rows_of_A * iNum_of_cols_of_A * sizeof(double)); 	// Allocate memory for Matrix_A
    pd_Matrix_B = pdOutput; // Since memory for output matrix is allocated by the calling function.

    iNum_of_elems_of_A = iNum_of_rows_of_A * iNum_of_cols_of_A ;
    iNum_of_elems_of_B = iNum_of_rows_of_B * iNum_of_cols_of_B ;

    for (iIter = 0; iIter < iNum_of_elems_of_A ; iIter++)
    {
        pd_Matrix_A[iIter] = pdInput_A[iIter];
    }
    for (iIter = 0; iIter < iNum_of_elems_of_B ; iIter++)
    {
        pd_Matrix_B[iIter] = pdInput_B[iIter];
    }

    /*
    A -> 10x10 matrix
    Sample Procedure:
    In the first iteration of the main outer loop the following steps occur:
    Step 1) Finds the largest element in the matrix, which then becomes the pivot.
            If (20, 50) is the largest element in the matrix, then
            big = A(20,50)
            irow = 20, icol = 50
            ipiv[icol]++ --> ipiv[50] = 1

    Step 2) We put the pivot element on the diagonal, by performing only row operations.
            A(20, 50) becomes A(50,50) by interchanging row 20 with row 50.
            NOTE: If the pivot element was already a diagonal element (row = col), then no
                  swapping is required.
            At the of step 2, the pivot element is at location A(iCol, iCol), i.e A(50,50) in our example.
            And the row that will be normalized by the pivot elemt is iCol, i.e Row 50 in our example.

    Step 3) Save the row and column index (iRow and iCol) of the pivot to unscramble the column swap
            after the matrix has been reduced to its identity form.

    Step 4) Normalize the row in which the pivot element is present.
            i.e Divide every element in row 50 by A(50,50).
            So A(50,50) = 1.

    Step 5) Reduce all columns of the pivot row to zero, except for the pivot itself.

    Step 6) Repeat Steps 1-5 to obtain the identity matrix in scrambled form

    Step 7) Unscramble by interchanging pairs of columns in reverse order, starting with the last column that was swapped.
    */

    int *piPivot_diag_positions = (int*) malloc (sizeof(int) * iNum_of_row_and_cols_of_A);
#if 0 // not needed if no column interchanges are done.
    int *piPivot_Row_Index = (int*) malloc (sizeof(int) * iNum_of_row_and_cols_of_A);
    int *piPivot_Col_Index = (int*) malloc (sizeof(int) * iNum_of_row_and_cols_of_A);
#endif

    int iIter_Diag;
    double dGreatest_element;
    int iIter_Row, iIter_Col;
    int iPivot_Row, iPivot_Col;
    int iIter_Col_SWAP;
    double dPivot_Inverse;
    double dPivot_Col_Element;

    for (iIter = 0;iIter < iNum_of_row_and_cols_of_A; iIter++)
    {
        piPivot_diag_positions[iIter]=0;
    }


#if 1 // BIGGGGGG
    for (iIter_Diag = 0; iIter_Diag < iNum_of_row_and_cols_of_A; iIter_Diag++)
    {
        // This is the main loop over the columns to be reduced.
        dGreatest_element = 0.0;
        for (iIter_Row = 0;iIter_Row < iNum_of_row_and_cols_of_A;iIter_Row++)
        {
            // This is the outer loop of the search for a pivot element.
            if (piPivot_diag_positions[iIter_Row] != 1)
            {
                for (iIter_Col = 0;iIter_Col < iNum_of_row_and_cols_of_A; iIter_Col++)
                {
                    if (piPivot_diag_positions[iIter_Col] == 0)
                    {
                        if (fabs(pd_Matrix_A[ROWCOL(iIter_Row, iIter_Col, iNum_of_row_and_cols_of_A)]) >= dGreatest_element)
                        {
                            dGreatest_element = fabs(pd_Matrix_A[ROWCOL(iIter_Row, iIter_Col, iNum_of_row_and_cols_of_A)]);
                            iPivot_Row = iIter_Row;//20
                            iPivot_Col = iIter_Col;//50
                        }
                    }
                }
            }
        }

        ++(piPivot_diag_positions[iPivot_Col]);// ipiv[50] = 1;

      /*  if (iPivot_Row != iPivot_Col)
        {
            for (iIter_Col_SWAP = 0; iIter_Col_SWAP < iNum_of_row_and_cols_of_A; iIter_Col_SWAP++)
            {
                SWAP(pd_Matrix_A[ROWCOL(iPivot_Row, iIter_Col_SWAP, iNum_of_row_and_cols_of_A)],
                                                    pd_Matrix_A[ROWCOL(iPivot_Col, iIter_Col_SWAP, iNum_of_row_and_cols_of_A)]);
            }

            for (iIter_Col_SWAP = 0; iIter_Col_SWAP < iNum_of_cols_of_B; iIter_Col_SWAP++)
            {
                SWAP(pd_Matrix_B[ROWCOL(iPivot_Row, iIter_Col_SWAP, iNum_of_cols_of_B)],
                                                    pd_Matrix_B[ROWCOL(iPivot_Col, iIter_Col_SWAP, iNum_of_cols_of_B)]);
            }

        }*/

#if 0		 // not required because we are not performing column interchanges
        piPivot_Row_Index[iIter_Diag]=iPivot_Row; //We are now ready to divide the pivot row by the
        piPivot_Col_Index[iIter_Diag]=iPivot_Col; //pivot element, located at irow and icol.
#endif

        if ((pd_Matrix_A[ROWCOL(iPivot_Col, iPivot_Col, iNum_of_row_and_cols_of_A)]) == 0.0)
        {
            printf ("\nLoghessian Matrix seems singular: ");
            for (iIter = 0 ; iIter < iNum_of_elems_of_A ; iIter++)
            {
                printf ("%g\t", pdInput_A[iIter]);
            }
            printf ("Pivot = %g", dGreatest_element);
            //mexPrintf("\nWarning: Compute_ML_Divide: Matrix A may be singular. Results may not be accurate. ");
        }

        dPivot_Inverse = 1.0/(pd_Matrix_A[ROWCOL(iPivot_Col, iPivot_Col, iNum_of_row_and_cols_of_A)]) ;
        pd_Matrix_A[ROWCOL(iPivot_Col, iPivot_Col, iNum_of_row_and_cols_of_A)]  = 1.0;

        for (iIter_Col = 0;iIter_Col < iNum_of_row_and_cols_of_A;iIter_Col++)
        {
            if (iIter_Col != iPivot_Col)
            {
                pd_Matrix_A[ROWCOL(iPivot_Col, iIter_Col, iNum_of_row_and_cols_of_A)] *= dPivot_Inverse;
            }
        }
        for (iIter_Col = 0; iIter_Col < iNum_of_cols_of_B; iIter_Col++)
        {
            pd_Matrix_B[ROWCOL(iPivot_Col, iIter_Col, iNum_of_row_and_cols_of_A)] *= dPivot_Inverse;
        }

        for (iIter_Row = 0; iIter_Row < iNum_of_row_and_cols_of_A; iIter_Row++) //Next, we reduce the rows...
        {
            if (iIter_Row != iPivot_Col)  //...except for the pivot one, of course.
            {
                dPivot_Col_Element = pd_Matrix_A[ROWCOL(iIter_Row, iPivot_Col, iNum_of_row_and_cols_of_A)];
                pd_Matrix_A[ROWCOL(iIter_Row, iPivot_Col, iNum_of_row_and_cols_of_A)]= 0.0;

                for (iIter_Col = 0; iIter_Col < iNum_of_row_and_cols_of_A;iIter_Col++)
                {
                    if (iIter_Col != iPivot_Col)
                    {
                        pd_Matrix_A[ROWCOL(iIter_Row, iIter_Col, iNum_of_row_and_cols_of_A)] -=
                                        (pd_Matrix_A[ROWCOL(iPivot_Col, iIter_Col, iNum_of_row_and_cols_of_A)] * dPivot_Col_Element);
                    }
                }

                for (iIter_Col = 0; iIter_Col < iNum_of_cols_of_B;iIter_Col++)
                {
                    pd_Matrix_B[ROWCOL(iIter_Row, iIter_Col, iNum_of_row_and_cols_of_A)] -=
                                        (pd_Matrix_B[ROWCOL(iPivot_Col, iIter_Col, iNum_of_row_and_cols_of_A)] * dPivot_Col_Element);
                }
            }
        }
    } // end of main for loop: for (iIter_Diag = 0...
#endif // #if 0 BIGGGGG
    /*
        AX = B
        a11X + a12Y + a13Z = b1
        a21X + a22Y + a23Z = b2
        a31X + a32Y + a33Z = b3

        a11P + a12Q + a13R = c1
        a21P + a22Q + a23R = c2
        a31P + a32Q + a33R = c3

        a11L + a12M + a13N = d1
        a21L + a22M + a23N = d2
        a31L + a32M + a33N = d3

        |a11  a12  a13|   | X P L |     | b1  c1 d1|
        |a21  a22  a23| x | Y Q M |  =  | b2  c2 d2|
        |a31  a32  a33|   | Z R N |     | b3  c3 d3|

        R1 <--> R2
        |a21  a22  a23|   | X P L |     | b2  c2 d2|
        |a11  a12  a13| x | Y Q M |  =  | b1  c1 d1|
        |a31  a32  a33|   | Z R N |     | b3  c3 d3|

        MULTIPLYING...
        a21X + a22Y + a23Z = b2
        a11X + a12Y + a13Z = b1
        a31X + a32Y + a33Z = b3 AND SO ON

        C1 <---> C2 on the original matrix

        |a12  a11  a13|   | X P L |     | c1  b1 d1|
        |a22  a21  a23| x | Y Q M |  =  | c2  b2 d2|
        |a32  a31  a33|   | Z R N |     | c3  b3 d3|


        a12X + a11Y + a13Z = c1   		a11X + a12Y + a13Z = b1
        a22X + a21Y + a23Z = c2			a21X + a22Y + a23Z = b2
        a32X + a31Y + a33Z = c3 		a31X + a32Y + a33Z = b3

        In order to preserve the original equantions, Column operations must not be performed on RHS (as seen in alg)
        Also, we have to replace X and Y since we did a column operation on LHS Columns 1 and 2,
                                                        so they can be compensated by Row operations in the final RHS result

    */

#if 0
//unscrambling
    for (iIter = (iNum_of_row_and_cols_of_A-1) ; iIter >= 0; iIter--)
    {
        if(piPivot_Row_Index[iIter] != piPivot_Col_Index[iIter])
        {
            for (iIter_Col = 0;iIter_Col < iNum_of_row_and_cols_of_A; iIter_Col++)
            {
                SWAP ((pd_Matrix_B[ROWCOL(piPivot_Row_Index[iIter], iIter_Col, iNum_of_row_and_cols_of_A)]),
                                (pd_Matrix_A[ROWCOL(piPivot_Col_Index[iIter], iIter_Col, iNum_of_row_and_cols_of_A)]));
            }
        }
    }
#endif
    free (piPivot_diag_positions);
    free (pd_Matrix_A);
}
