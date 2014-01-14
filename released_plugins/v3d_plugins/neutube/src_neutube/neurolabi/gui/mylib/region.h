/*****************************************************************************************\
*                                                                                         *
*  Region Data Abstractions                                                               *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  March 2009                                                                    *
*                                                                                         *
\*****************************************************************************************/

#ifndef _REGION_LIB

#define _REGION_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "mylib.h"
#include "array.h"
#include "level.set.h"
#include "water.shed.h"

typedef void *Contour;

Contour *G(Copy_Contour)(Contour *cont);
Contour *Pack_Contour(Contour *R(M(cont)));
Contour *Inc_Contour(Contour *R(I(cont)));
void     Free_Contour(Contour *F(cont));
void     Kill_Contour(Contour *K(cont));
void     Reset_Contour();
int      Contour_Usage();
void     Contour_List(void (*handler)(Contour *));
int      Contour_Refcount(Contour *cont);
Contour *G(Read_Contour)(FILE *input);
void     Write_Contour(Contour *cont, FILE *output);

Contour *G(Trace_Contour)(APart *image, boolean iscon4, Indx_Type seed,
                          void *arg, boolean test(Indx_Type p, void *arg));

Contour *G(Basic_Contour)(APart *source, boolean iscon4, Indx_Type leftmost,
                          Comparator cmprsn, Value level);

Contour *G(Level_Set_Contour)(Level_Tree *t, Level_Set *r);
Contour *G(P_Vertex_Contour) (Partition  *p, int        v);

boolean    Get_Contour_Connectivity(Contour *tour);
Dimn_Type *Get_Contour_Dimensions(Contour *tour);
boolean    Boundary_Contour(Contour *tour);

void For_Contour(Contour *tour, void *arg, void handler(Indx_Type p, Indx_Type q, void *arg));

typedef struct
  { Size_Type  rastlen;  // Length of the raster pair vector (always even)
    Indx_Type *raster;   // Pixels are [raster[i],raster[i+1]] for i in [0,rastlen) and i even
  } Region;

Region *G(Copy_Region)(Region *reg);
Region *Pack_Region(Region *R(M(reg)));
Region *Inc_Region(Region *R(I(reg)));
void    Free_Region(Region *F(reg));
void    Kill_Region(Region *K(reg));
void    Reset_Region();
int     Region_Usage();
void    Region_List(void (*handler)(Region *));
int     Region_Refcount(Region *reg);
Region *G(Read_Region)(FILE *input);
void    Write_Region(Region *reg, FILE *output);

Region *G(Record_Region)(APart *source, int share, boolean iscon2n, Indx_Type leftmost,
                         boolean with_holes, void *arg, boolean (*test)(Indx_Type p, void *arg));

Region *G(Record_Basic)(APart *source, int share, boolean iscon2n, Indx_Type leftmost,
                        boolean with_holes, Comparator cmprsn, Value level);

boolean    Get_Region_Connectivity(Region *reg);
int        Get_Region_Dimensionality(Region *reg);
Dimn_Type *Get_Region_Dimensions(Region *reg);

void For_Region(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg));
void For_Region_Outline(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg));
void For_Region_Holes(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg));
void For_Region_Exterior(Region *reg, void *arg, void (*handler)(Indx_Type p, void *arg));

Region *G(Record_Level_Set)(Level_Tree *t, Level_Set *r, boolean with_holes, int share);
Region *G(Record_P_Vertex) (Partition  *p, int        v, boolean with_holes, int share);

Region *Fill_Region_Holes(Region *R(M(reg)));

Range_Bundle  *Region_Range(Range_Bundle *R(M(rng)), APart *part, Region *reg);
Size_Type      Region_Volume(Region *reg);
Size_Type      Region_Area(Region *reg);
Double_Vector *Region_COM(Double_Vector *R(O(com)), Region *reg);
Double_Vector *Region_Select_COM(Double_Vector *R(O(com)), Region *reg,
                                 APart *source, Comparator cmprsn, Value level);

Extent_Bundle *Region_Extent(Extent_Bundle *R(O(bundle)), Region *reg);
boolean        Touches_Boundary(Extent_Bundle *bundle, APart *part);

#ifdef __cplusplus
}
#endif

#endif
