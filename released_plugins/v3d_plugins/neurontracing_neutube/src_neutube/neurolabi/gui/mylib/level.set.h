/*****************************************************************************************\
*                                                                                         *
*  Level Set Tree Abstractions                                                            *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007 (from August 2006 version)                                          *
*  Mods  :  Jan 2009 -- Generalized to n-dimensional arrays                               *
*                                                                                         *
\*****************************************************************************************/

#ifndef _LEVEL_SET

#define _LEVEL_SET

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "array.h"


typedef void Level_Tree;         // Level tree of level sets of a GREY image

Level_Tree *G(Build_Level_Tree)(Pixel_APart *I(image), boolean iscon2n);
void        Print_Level_Tree(Level_Tree *t, int indent, int idwidth, int valwidth, FILE *file);

boolean     Get_Level_Tree_Connectivity(Level_Tree *t);
APart      *Get_Level_Tree_APart(Level_Tree *t);
void        Set_Level_Tree_APart(Level_Tree *t, Pixel_APart *I(image));

Level_Tree *G(Copy_Level_Tree)(Level_Tree *t);
Level_Tree *Pack_Level_Tree(Level_Tree *R(M(t)));
Level_Tree *Inc_Level_Tree(Level_Tree *R(I(t)));
void        Free_Level_Tree(Level_Tree *F(t));
void        Kill_Level_Tree(Level_Tree *K(t));
void        Reset_Level_Tree();
int         Level_Tree_Usage();
void        Level_Tree_List(void (*handler)(Level_Tree *));
int         Level_Tree_Refcount(Level_Tree *t);
Level_Tree *G(Read_Level_Tree)(FILE *input);
void        Write_Level_Tree(Level_Tree *t, FILE *output);

typedef void Level_Set;        // Vertex of level set tree (corresponds to a level set);

Level_Set *Level_Set_Root      (Level_Tree *t);
Level_Set *Level_Set_Child     (Level_Tree *t, Level_Set *r);
Level_Set *Level_Set_Sibling   (Level_Tree *t, Level_Set *r);
int        Level_Set_Size      (Level_Tree *t, Level_Set *r);
int        Level_Set_Level     (Level_Tree *t, Level_Set *r);
int        Level_Set_Peak      (Level_Tree *t, Level_Set *r);
int        Level_Set_Leftmost  (Level_Tree *t, Level_Set *r);
int        Level_Set_Background(Level_Tree *t, Level_Set *r);
int        Level_Set_Id        (Level_Tree *t, Level_Set *r);

void       List_Level_Set(Level_Tree *t, Level_Set *r,
                          void *arg, void (*handler)(Indx_Type p, void *arg));

#ifdef __cplusplus
}
#endif

#endif
