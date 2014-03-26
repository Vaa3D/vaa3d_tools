/*****************************************************************************************\
*                                                                                         *
*  Image Filters                                                                          *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _IMAGE_FILTERS

#define _IMAGE_FILTERS

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"
#include "water.shed.h"

/****************************************************************************************
 *                                                                                      *
 * GENERAL CONVOLUTION FILTERING (N-DIMENSIONAL CONVOLUTION AT A POINT)                 *
 *                                                                                      *
 ****************************************************************************************/

typedef void Convolver;

Convolver *G(Make_Convolver)(Array *I(image), Double_Array *C(filter), Coordinate *C(anchor));

Double_Array *Convolver_Filter(Convolver *c);
Coordinate   *Convolver_Anchor(Convolver *c);
Indx_Type     Convolver_Index(Convolver *c);
Coordinate   *Convolver_Coordinate(Convolver *c);

void      Place_Convolver(Convolver *M(c), Indx_Type p);
void      Move_Convolver_Forward(Convolver *M(c));
void      Move_Convolver_Backward(Convolver *M(c));

double    Convolve(Convolver *c);
double    Convolve_Segment(Convolver *c, Indx_Type offset, Size_Type span);

void      Free_Convolver(Convolver *F(c));
void      Kill_Convolver(Convolver *K(c));
void      Reset_Convolver();

/****************************************************************************************
 *                                                                                      *
 *  CONVOLUTION TEMPLATES                                                               *
 *                                                                                      *
 ****************************************************************************************/

Double_Vector *G(Gaussian_Filter)(double sigma, int radius);
Double_Vector *G(Box_Filter)(int radius);

Double_Array  *G(Filter_Power)(Double_Vector *F(filter), int n);
Double_Array  *G(Filter_Product)(Double_Array *F(filter1), Double_Array *F(filter2));
Double_Vector *G(Filter_Convolution)(Double_Vector *F(filter1), Double_Vector *F(filter2));

Double_Array  *G(LOG_Filter)(int ndims, double sigma, int radius);
Double_Array  *G(DOG_Filter)(int ndims, double sigma1, double sigma2, int radius);
Double_Array  *G(DOB_Filter)(int ndims, int radius1, int radius2);

Double_Array  *G(ALOG_Filter)(int ndims, double *sigma, int *radius);
Double_Array  *G(ADOG_Filter)(int ndims, double *sigma1, double *sigma2, int *radius);
Double_Array  *G(ADOB_Filter)(int ndims, int *radius1, int *radius2);

Double_Matrix *G(Gabor_Filter)(double aspect, double sigma, double wavelen, double phase,
                               double orient, int radius1, int radius2);

/****************************************************************************************
 *                                                                                      *
 *  SEPERABLE CONVOLUTION AND OTHER SPECIAL FILTERS                                     *
 *                                                                                      *
 ****************************************************************************************/

Array *Filter_Dimension(Array *R(M(image)), Double_Vector *F(filter), int dim);  //  O(RA)

Array *Average_Dimension(Array *R(M(image)), int radius, int dim);             //  O(A)
Array *    Sum_Dimension(Array *R(M(image)), int radius, int dim);             //  O(A)
Array * Square_Dimension(Array *R(M(image)), int radius, int dim);             //  O(A)
Array *    Min_Dimension(Array *R(M(image)), int radius, int dim);             //  O(A)
Array *    Max_Dimension(Array *R(M(image)), int radius, int dim);             //  O(A)

Array *Filter_Array(Array *R(M(image)), Double_Vector *F(filter));
Array *Filter_Array_List(Array *R(M(image)), Double_Vector **F(filters));

Array * Average_Array(Array *R(M(image)), int radius);
Array *     Sum_Array(Array *R(M(image)), int radius);
Array *Variance_Array(Array *R(M(image)), int radius);
Array *     Min_Array(Array *R(M(image)), int radius);
Array *     Max_Array(Array *R(M(image)), int radius);

Array *LOG_Array(Array *R(M(image)), double sigma, int radius);
Array *DOG_Array(Array *R(M(image)), double sigma1, double sigma2, int radius);
Array *DOB_Array(Array *R(M(image)), int radius1, int radius2);

Array *ALOG_Array(Array *R(M(image)), double *sigma, int *radius);
Array *ADOG_Array(Array *R(M(image)), double *sigma1, double *sigma2, int *radius);
Array *ADOB_Array(Array *R(M(image)), int *radius1, int *radius2);

Array *Median_Array(Array *R(M(image)), int radius);           //  TO BE WRITTEN!
Array * Order_Array(Array *R(M(image)), int radius, int rank);  //  TO BE WRITTEN!


/****************************************************************************************
 *                                                                                      *
 * GRADIENTS, CURVATURE, FLOW_FIELDS                                                    *
 *                                                                                      *
 ****************************************************************************************/


  /* We offer a collection of seperable filters for computing the derivative and higher order
     partials, namely the simple matlab difference (Matlab), the Sobel filter (Sobel), and the
     beautiful and more accurate filter scheme described in Farid & Simoncelli, "Differentiation
     of Discrete Multidimensional Signals", IEEE Trans. Image Proc. 13, 4 (2004), 496-507, with
     designes of span 3, 5, and 7 (Iso3, Iso5, Iso7).  Obviously the larger the span the more
     time the derivative or partial takes to compute, but also the more "accurate" the derivative.

     If you which to specify your own scheme you can do so by giving Make_Derivative_Scheme
     a basis vector and a derivative vector.  For example, for the classic Sobel filter the
     basis vector is [.25, .50, .25] and the derivative vector is [-1, 0, +1].  A scheme
     (other than the system provided ones) can be removed by calling Kill_Derivative_Scheme.
  */

typedef struct
  { Double_Vector *base;
    Double_Vector *deriv;
  } D_Scheme;

D_Scheme *Matlab;
D_Scheme *Sobel;
D_Scheme *Iso3;
D_Scheme *Iso5;
D_Scheme *Iso7;

D_Scheme *Make_Derivative_Scheme(Double_Vector *C(base), Double_Vector *C(deriv));
void      Kill_Derivative_Scheme(D_Scheme *scheme);

  /* The "Matlab" gradient is simple slope about a pixel in a given direction, say 'dim'.
       Matlab_Gradient_Dim takes an 'image' and a floating point array 'grad' of the same shape
       and places the Matlab gradient in direction 'dim' within 'grad'.  A pointer to grad is
       returned as the functions result.
     Matlab_Gradient takes an image and generates an ndims x shape(image) array containing the
       gradient in each direction where ndims is the dimensionality of image.
  */

Float_Array *G(Derivative_Array)(D_Scheme *scheme, Array *image, int dim);

Float_Array *G(Partial_Array)(D_Scheme *scheme, Array *image, int n, int x1, ... X(int xn));

Float_Array *G(Gradient_Array)(D_Scheme *scheme, Array *image);

Float_Array *G(Laplacian_Array)(D_Scheme *scheme, Array *image);   // +O(1)

Float_Array *G(Levelset_Curvature_Array)(Array *image, Array *grad);  // +O(3)
Float_Array *G(Surface_Curvature_Array)(Array *image, Array *grad);

Float_Array *G(Gradient_Flow_Field_Array)(Array *image, double mu);

  /* The routines above are quite efficient as they operate over the entire array, but they
       are also space intensive as they require copies of the original image.  For example,
       for a 400Mb uint8 3D stack, Matlab_Gradient uses 400Mb * 3 * 4 = 4.8Gb!  Often it
       suffices to have the gradient or other quantity at a pixel in one or more directions
       in a sweep of the image as a temporary quantity.  To acheive this we introduce 'iterators'
       that you setup on an image, and then you call them for the gradient(s) or other quantities
       as an index 'p' progresses through any slice of the underlying array (see array.h for the
       definition of a slice).  At the end of the sweep you need to kill the iterator.  The idea
       is that the iterator is setup to make the sweep computation as efficient as possible,
       albeit not quite as efficient as the code for the array-based routines above.

     For a given quantity there is a routine that sets up an iterator, which can be either a
       Value_Iterator of a Vector_Iterator, and then there are macro-implemented routines to
       get the next value or vector, and to kill the value or vector iterator as follows:

       double         NEXT_VALUE(Value_Iterator *g);
       void           KILL_VALUE(Value_Iterator *g);

       Double_Vector *NEXT_VECTOR(Vector_Iterator *g);
       void           KILL_VECTOR(Vector_Iterator *g);

     For example, below, Matlab_Gradient(slice,dim), returns a value iterator that will advance
       over each pixel of 'slice' returning the Matlab gradient in direction dim.  If one wants
       the gradient in every direction simultaneously, then Matlab_Gradient_Vector(slice), builds
       a vector iterator that returns a vector iterator that returns a vector of the derivatives
       in each direction for each successive pixel of slice.  Moreover, iterators can be built
       over iterators.  For example, Magnitute_Iterator is a value iterator that given a vector
       iterator returns the magnitude of said vector for each pixel in sequence!
  */

typedef struct _Value_Iterator                 //  Only filters.p sees these macros
  { double (*next)(struct _Value_Iterator *);
    void   (*kill)(struct _Value_Iterator *);
  } Value_Iterator;

#define NEXT_VALUE(g)  ((g)->next(g))
#define KILL_VALUE(g)  ((g)->kill(g))

typedef struct _Vector_Iterator
  { Double_Vector *(*next)(struct _Vector_Iterator *);
    void           (*kill)(struct _Vector_Iterator *);
  } Vector_Iterator;

#define NEXT_VECTOR(g)  ((g)->next(g))
#define KILL_VECTOR(g)  ((g)->kill(g))

Value_Iterator  *General_Convolver(AForm *I(image), Double_Vector **F(filters));
Vector_Iterator *General_Vector(int n, Value_Iterator *C(v1), ... X( Value_Iterator *C(vn)));

Value_Iterator  *Derivative(D_Scheme *scheme, AForm *I(image), int dim);

Value_Iterator  *Partial(D_Scheme *scheme, AForm *I(image), int n, int x1, ... X(int xn));

Vector_Iterator *Gradient(D_Scheme *scheme, AForm *I(image));

double           Magnitude(Double_Vector *v);
Value_Iterator  *Magnitude_Iterator(Vector_Iterator *C(it));

Value_Iterator  *Laplacian(D_Scheme *scheme, AForm *I(image));

/*
Value_Iterator  *Levelset_Curvature(AForm *image, Value_Iterator *gradient);
Value_Iterator  *Surface_Curvature(AForm *image, Value_Iterator *gradient);
Value_Iterator  *Gradient_Flow_Field(AForm *image, Value_Iterator *gradient, double mu);
*/


/****************************************************************************************
 *                                                                                      *
 * DISTANCE TRANSFORMS, VORONOI DIAGRAMS, DELAUNEY TRIANGULATION                        *
 *                                                                                      *
 ****************************************************************************************/

Float_Array *G(Squared_Distance)(APart *image);
Float_Array *G(Manhattan_Distance)(APart *image);

Float_Array *G(Binary_Euclidean_Distance)(APart *image, int boundary);

Label_Array *G(Voronoi_Labeling)(int nseed, Indx_Type *seeds, Coordinate *F(shape));

Partition   *G(Delauney_Partition)(int nseed, Indx_Type *seeds, Coordinate *F(shape));

Label_Array *G(Geodesic_Labeling)(APart *image, boolean iscon2n, int *nregions);
Partition   *G(Geodesic_Partition)(APart *image, boolean iscon2n);

#ifdef __cplusplus
}
#endif

#endif
