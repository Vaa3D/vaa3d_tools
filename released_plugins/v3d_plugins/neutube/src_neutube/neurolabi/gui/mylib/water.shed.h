/*****************************************************************************************\
*                                                                                         *
*  Watershed Partitioning                                                                 *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007                                                                     *
*  Mod   :  Aug 2008 -- Added Waterhshed Graph and Tree concepts                          *
*           Jan 2009 -- Generalized to n-dimensional arrays                               *
*                                                                                         *
\*****************************************************************************************/

#ifndef _WATERSHED

#define _WATERSHED

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"

typedef void Partition;

typedef Array       Label_Array;   //  A UINT8_, UINT16_, or UINT32_TYPE, PLAIN_KIND array

typedef struct
  { int seed;     // a left-most pixel (index into *label* array) in the region
    int depth;    // minimum pixel value in the region
    int size;     // # of pixels in the region
  } P_Vertex;

typedef struct
  { int region1;   // index of region on one side of the edge
    int region2;   // index of region on the other side of the edge
    int height;    // least height pixel separating the two regions of the edge
  } P_Edge;

APart        *Get_Partition_APart(Partition *p);
void          Set_Partition_APart(Partition *p, APart *I(part));
Label_Array  *Get_Partition_Labels(Partition *p);
void          Set_Partition_Labels(Partition *p, Label_Array *C(label));

boolean       Is_Partition_2n_Connected(Partition *p);
boolean       Is_Partition_Colored(Partition *p);
int           Get_Partition_Vertex_Count(Partition *p);
int           Get_Partition_Edge_Count(Partition *p);
int           Get_Partition_Color_Count(Partition *p);
Array        *Get_Partition_Labels(Partition *p);         //  May be null
P_Vertex     *Get_Partition_Vertex(Partition *p, int c);
P_Edge       *Get_Partition_Edge(Partition *p, int d);
int          *Get_Partition_Neighbors(Partition *p, int c, int *O(nedges));

Partition *G(Copy_Partition)(Partition *p);
Partition *Pack_Partition(Partition *R(M(p)));
Partition *Inc_Partition(Partition *R(I(p)));
void       Free_Partition(Partition *F(p));
void       Kill_Partition(Partition *K(p));
void       Reset_Partition();
int        Partition_Usage();
void       Partition_List(void (*handler)(Partition *));
int        Partition_Refcount(Partition *p);
Partition *G(Read_Partition)(FILE *input);
void       Write_Partition(Partition *p, FILE *output);

Partition *G(Make_Partition)(APart *I(image), Label_Array *C(M(labels)), int nregions,
                             boolean iscon2n, boolean color);

Partition *Color_Partition(Partition *R(M(p)));

Array     *Draw_Partition(Array *R(M(canvas)), Partition *p, double alpha);

  //  Watershed specific routines

Label_Array   *Label_Watershed(Pixel_APart *image, Integer_Array *R(M(labels)),
                               int *O(nbasins), boolean iscon2n);

Partition     *G(Build_Watershed)(Pixel_APart *I(image), boolean iscon2n, boolean color);

Partition     *G(Build_Seeded_Watershed)(Pixel_APart *I(image), boolean iscon2n,
                                         boolean color, Vector *seeds);

void           Average_Watershed_Heights(Partition *w, int *O(num), int *O(den),
                                                       int64 *O(sqr), int null);

  //  Partition collapsing routines

typedef struct
  { int  nrange;
    int *mapto;
  } Map_Bundle;

Map_Bundle *Static_Collapse(Map_Bundle *R(O(map)), Partition *shed, void *info,
                            tristate (*handler)(int a, int h, int b, void *info));

Map_Bundle *General_Collapse(Map_Bundle *R(O(map)), Partition *p, boolean dynamic, void *info,
                             tristate  (*decide)(int a, int ab, int b, void *info),
                             void      (*fuse)(int a, int ac, int c, int cb, int b, void *info),
                             boolean   (*compare)(int a, int ab, int b,
                                                  int c, int cd, int d, void *info)
                            );

Partition *G(Merge_Partition)(Partition *p, Map_Bundle *map,
                              Label_Array *C(M(label)), boolean color);

#ifdef __cplusplus
}
#endif

#endif
