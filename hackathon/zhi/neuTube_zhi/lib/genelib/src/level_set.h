/*****************************************************************************************\
*                                                                                         *
*  Component Tree Abstractions                                                            *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  June 2007 (from August 2006 version)                                          *
*                                                                                         *
\*****************************************************************************************/

#ifndef _LEVEL_SET

#define _LEVEL_SET

#include <stdio.h>
#include "image_lib.h"

/* A component tree occupies 20A bytes where A is the area of the image it is built from.
   The underlying image must remain in existence for as long as its component tree is being
   used (as it refers to it).  Build_Component_Tree makes the new tree the current one for
   the traversal routines, "Level_Set_<x>", below.  The image "frame" must be GREY or GREY16.
   The definition of connected is either 4- or 8-connected depending on the setting of the
   parameter iscon4 (on => 4-connected, off => 8-connected).                                 */

typedef void Component_Tree;         // Component tree of level sets of a GREY image

Component_Tree *Build_2D_Component_Tree(Image *image, int iscon4);
Component_Tree *Build_3D_Component_Tree(Stack *stack, int iscon6);

void            Print_Component_Tree(Component_Tree *t, int indent, FILE *file);

Component_Tree *Copy_Component_Tree(Component_Tree *tree);
void            Pack_Component_Tree(Component_Tree *tree);
void            Free_Component_Tree(Component_Tree *tree);
void            Kill_Component_Tree(Component_Tree *tree);
void            Reset_Component_Tree();
int             Component_Tree_Usage();


/* All the tree traversal and information routines below work off an implicit
   current component tree which is set with "Set_Current_Component_Tree"
   (or by "Build_Component_Tree" above).                                          */

typedef void Level_Set;        // Vertex of component tree (corresponds to a level set);

void            Set_Current_Component_Tree(Component_Tree *tree);
Component_Tree *Get_Current_Component_Tree();
Image          *Get_Component_Tree_Image(Component_Tree *tree);
Stack          *Get_Component_Tree_Stack(Component_Tree *tree);
int             Get_Component_Tree_Connectivity(Component_Tree *tree);

Level_Set *Level_Set_Root();
Level_Set *Level_Set_Child(Level_Set *r);
Level_Set *Level_Set_Sibling(Level_Set *r);
int        Level_Set_Size(Level_Set *r);
int        Level_Set_Level(Level_Set *r);
int        Level_Set_Peak(Level_Set *r);
int        Level_Set_Leftmost(Level_Set *r);
int        Level_Set_Background(Level_Set *r);
int        Level_Set_Id(Level_Set *r);

/* List_Level_Set will call "handler" with every pixel in the level set "r" passed to it.
   An integer, say p, passed to the handler is the offset into the pixel/voxel array of
   the pixel, e.g. the coordinate is (p % width, p / width) if a 2D tree where width is the
   width of the image.                                                                      */

void       List_Level_Set(Level_Set *r, void (*handler)(int));

#endif
