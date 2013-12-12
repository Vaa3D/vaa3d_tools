/*****************************************************************************************\
*                                                                                         *
*  Shortest Path Algorithms                                                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  March 2010                                                                    *
*                                                                                         *
*  (c) February 10, '10, Dr. Gene Myers and Howard Hughes Medical Institute               *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef  _SHORTEST_PATHS

#define  _SHORTEST_PATHS

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"
#include "region.h"

typedef void SP_Item;

SP_Item  *SP_Value(double val);
SP_Item  *SP_Test(boolean (*cond)(Indx_Type p, double val));
SP_Item  *SP_Edge(double (*func)(float *w, Indx_Type p, Indx_Type q, double d));

SP_Item  *Seed_Zone(Indx_Type seed);
SP_Item  *Slice_Zone(Slice *I(slice));
SP_Item  *Region_Zone(Region *I(region));
SP_Item  *Array_Zone();
SP_Item  *Image_Zone(SP_Item *K(which), SP_Item *K(zone));

void      Kill_SP(SP_Item *p);
SP_Item  *Inc_SP(SP_Item *p);

Float_Array *G(Shortest_Paths)(Float_Array *w, boolean iscon2n, SP_Item *K(cost),
                               SP_Item *K(source), SP_Item *K(zone));

Vector *G(Find_Path)(Float_Array *w, boolean iscon2n, SP_Item *K(cost),
                     Float_Array *d, Indx_Type end);

Vector *G(Shortest_Between)(Float_Array *w, boolean iscon2n, SP_Item *K(cost),
                            SP_Item *K(source), SP_Item *K(target), double *O(score));

#ifdef __cplusplus
}
#endif

#endif
