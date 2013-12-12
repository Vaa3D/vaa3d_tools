/*****************************************************************************************\
*                                                                                         *
*  Histogram Data Abstraction and Array Statistics Routines                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  December 2008                                                                 *
*                                                                                         *
*  (c) December 20, '09, Dr. Gene Myers and Howard Hughes Medical Institute               *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef  _HISTOGRAM

#define  _HISTOGRAM

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "region.h"

typedef struct
  { Value_Kind kind;    //  Domain of histogram
    Value      binsize; //  Histogram has nbins bins where the domain covered by bin i
    Value      offset;  //     is [offset + i*binsize, offset + (i+1)*binsize)
    int        nbins;   //  number of bins
    Size_Type  total;   //  total number of counts in all the bins
    Size_Type *counts;  //  counts[i] is the # of values in bin i
  } Histogram;

Histogram *G(Copy_Histogram)(Histogram *h);
Histogram *Pack_Histogram(Histogram *R(M(h)));
Histogram *Inc_Histogram(Histogram *R(I(h)));
void       Free_Histogram(Histogram *F(h));
void       Kill_Histogram(Histogram *K(h));
void       Reset_Histogram();
int        Histogram_Usage();
void       Histogram_List(void (*handler)(Histogram *));
int        Histogram_Refcount(Histogram *h);
Histogram *G(Read_Histogram)(FILE *input);
void       Write_Histogram(Histogram *h, FILE *output);

Value  Bin2Value(Histogram *h, int b);
int    Value2Bin(Histogram *h, Value v);
double Bin2Percentile(Histogram *h, int b);
int    Percentile2Bin(Histogram *h, double fraction);
double Value2Percentile(Histogram *h, Value v);
Value  Percentile2Value(Histogram *h, double fraction);

Histogram *G(Make_Histogram)(Value_Kind kind, int nbins, Value binsize, Value offset);
Histogram *Empty_Histogram(Histogram *R(M(h)));

Histogram *G(Histogram_Array)(AForm *a, int nbins, Value binsize, Value offset);
Histogram *G(Histogram_Region)(Array *a, Region *r, int nbins, Value binsize, Value offset);
Histogram *G(Histogram_P_Vertex)(Array *a, Partition *p, int v,
                                 int nbins, Value binsize, Value offset);
Histogram *G(Histogram_Level_Set)(Array *a, Level_Tree *t, Level_Set *r,
                                  int nbins, Value binsize, Value offset);

Histogram *Histagain_Array(Histogram *R(M(h)), AForm *a, boolean clip);
Histogram *Histagain_Region(Histogram *R(M(h)), Array *a, Region *r, boolean clip);
Histogram *Histagain_P_Vertex(Histogram *R(M(h)), Array *a, Partition *p, int v, boolean clip);
Histogram *Histagain_Level_Set(Histogram *R(M(h)), Array *a, Level_Tree *t, Level_Set *r,
                               boolean clip);

Histogram *G(Histogram_Slice)(Histogram *h, int min, int max);

double Histogram_Mean(Histogram *h);
double Histogram_Sigma(Histogram *h);
double Histogram_Variance(Histogram *h);
double Histogram_Central_Moment(Histogram *h, int n);

double Histogram_Entropy(Histogram *h);
double Histogram_Cross_Entropy(Histogram *h, Histogram *g);
double Histogram_Relative_Entropy(Histogram *h, Histogram *g);

int  Otsu_Threshold(Histogram *h);
int  Triangle_Threshold(Histogram *h);
int  Intermeans_Threshold(Histogram *h);

#define BIN_COUNT            0x01   //  show bin counts
#define CUMULATIVE_COUNT     0x02   //  show cumulative counts
#define CUMULATIVE_PERCENT   0x04   //  show cumulative per cent of the total
#define ASCENDING_HGRAM      0x08   //  display in ascending order (descending by default)
#define   CLIP_HGRAM_HIGH    0x10   //  do not display any 0 count bins at the top
#define   CLIP_HGRAM_LOW     0x20   //  do not display any 0 count bins at the bottom
#define CLIP_HGRAM           0x30   //  do not display any 0 count bins at either extreme
#define BIN_MIDDLE           0x40   //  display the mid-value of a bin as opposed to its range

void   Print_Histogram(Histogram *h, FILE *output, int indent, int flags, Value binsize);

#ifdef __cplusplus
}
#endif

#endif
