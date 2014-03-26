/*****************************************************************************************\
*                                                                                         *
*  Connectivity and floodfill routines                                                    *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2008                                                                  *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _FLOODFILL

#define _FLOODFILL

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"

// GRID ROUTINES
     
typedef int Grid_Id;

Grid_Id    Setup_Grid(APart *a, string routine);

int        Grid_Size(Grid_Id id, boolean iscon2n);
Offs_Type *Grid_Neighbors(Grid_Id id, boolean iscon2n);
double    *Grid_Distances(Grid_Id id, boolean iscon2n);
int       *Grid_Backtrack(Grid_Id id, boolean iscon2n);

boolean   *Boundary_Pixels(Grid_Id id, Indx_Type p, boolean iscon2n);
boolean   *Boundary_Pixels_2d(Grid_Id id, Indx_Type p, boolean iscon4);
boolean   *Boundary_Pixels_3d(Grid_Id id, Indx_Type p, boolean iscon6);

void       Release_Grid(Grid_Id id);

void       Reset_Grids();

// FLOOD FILL ROUTINES

void Flood_Object(APart *source, int share, boolean iscon2n, Indx_Type seed,
                  void *argt, boolean (*test)(Indx_Type p, void *argt),
                  void *arge, void    (*eval)(Indx_Type p, void *arge),
                  void *argc, boolean (*check)(Size_Type n, void *argc),
                  void *arga, void    (*act)(Indx_Type p, void *arga));

void Flood_All(APart *source, int share, boolean iscon2n,
               void *argt, boolean (*test)(Indx_Type p, void *argt),
               void *arge, void    (*eval)(Indx_Type p, void *arge),
               void *argc, boolean (*check)(Size_Type n, void *argc),
               void *arga, void    (*act)(Indx_Type p, void *arga));

void Flood_Surface(APart *source, int share, boolean iscon2n, Indx_Type leftmost_seed,
                   void *argt, boolean (*test) (Indx_Type p, void *argt),
                   void *arge, void    (*eval)(Indx_Type p, void *arge),
                   void *argc, boolean (*check) (Size_Type n, Size_Type a, Size_Type d, void *argc),
                   void *arga, void    (*act)(Indx_Type p, int x, void *arga)
                  );

Indx_Type Find_Leftmost_Seed(APart *source, int share, boolean iscon2n, Indx_Type seed,
                             void *argt, int (*test)(Indx_Type p, void *argt));

void Reset_Flood();

#ifdef __cplusplus
}
#endif

#endif
