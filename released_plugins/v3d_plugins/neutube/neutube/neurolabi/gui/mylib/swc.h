/*****************************************************************************************\
*                                                                                         *
*  SWC Neuron Tree Data Abstraction                                                       *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  February 2010                                                                 *
*                                                                                         *
*  (c) February 10, '10, Dr. Gene Myers and Howard Hughes Medical Institute               *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef  _SWC

#define  _SWC

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"

typedef enum
  { SWC_UNDEFINED        = 0,   //  Types of SWC vertices
    SWC_SOMA             = 1,
    SWC_AXON             = 2,
    SWC_BASAL_DENDRITE   = 3,
    SWC_APICAL_DENDRITE  = 4,
    SWC_FORK_POINT       = 5,
    SWC_END_POINT        = 6,
    SWC_CUSTOM           = 7
  } SWC_Kind;

typedef struct
  { int         kind;   //  One of the defined types above
    float       x;      //  x, y, z coordinate of vertex
    float       y;
    float       z;
    float       r;      //  radius of neuron at this point
  } SWC_Node;

typedef void SWC_Tree;

int        Get_SWC_Size(SWC_Tree *t);
SWC_Node  *Get_SWC_Node(SWC_Tree *t, int i);
SWC_Node  *Get_SWC_Root(SWC_Tree *t);
SWC_Node  *Get_SWC_Father(SWC_Node *n);
SWC_Node  *Get_SWC_Child(SWC_Node *n);
SWC_Node  *Get_SWC_Sibling(SWC_Node *n);
int        Get_SWC_Outdegree(SWC_Node *n);

SWC_Tree  *G(Copy_SWC_Tree)(SWC_Tree *t);
SWC_Tree  *Pack_SWC_Tree(SWC_Tree *R(M(t)));
SWC_Tree  *Inc_SWC_Tree(SWC_Tree *R(I(t)));
void       Free_SWC_Tree(SWC_Tree *F(t));
void       Kill_SWC_Tree(SWC_Tree *K(t));
void       Reset_SWC_Tree();
int        SWC_Tree_Usage();
void       SWC_Tree_List(void (*handler)(SWC_Tree *));
int        SWC_Tree_Refcount(SWC_Tree *t);
SWC_Tree  *G(Read_SWC_Tree)(FILE *input);
void       Write_SWC_Tree(SWC_Tree *t, FILE *output);

typedef struct
  { int        num_trees;   //  # of trees
    SWC_Tree **trees;       //  trees[0..num_trees-1] point at SWC trees
  } SWC_Bundle;

SWC_Bundle *Read_SWC_File(SWC_Bundle *R(G(forest)), string file_name);

boolean Write_SWC_File(string file_name, SWC_Bundle *forest);

void Print_SWC_Tree(SWC_Tree *tree, FILE *output, int indent, string format);

SWC_Tree *Make_SWC_Tree(int size_estimate);

SWC_Node *Add_Vertex_As_Root(SWC_Tree *tree,
                                    SWC_Kind kind, double x, double y, double z, double r);

SWC_Node *Add_Vertex_As_First_Child(SWC_Tree *tree, SWC_Node *v,
                                    SWC_Kind kind, double x, double y, double z, double r);

SWC_Node *Add_Vertex_As_Sibling(SWC_Tree *tree, SWC_Node *v,
                                    SWC_Kind kind, double x, double y, double z, double r);

SWC_Node *Add_Vertex_As_Father(SWC_Tree *tree, SWC_Node *v,
                                    SWC_Kind kind, double x, double y, double z, double r);

SWC_Bundle *Clip_SWC_Tree(SWC_Bundle *R(G(forest)),
                                 SWC_Tree *tree, Double_Vector *F(beg), Double_Vector *F(end));

Extent_Bundle *SWC_Tree_Extent(Extent_Bundle *R(M(extent)), SWC_Tree *tree);
double         SWC_Tree_Path_Length(SWC_Tree *tree);

#ifdef __cplusplus
}
#endif

#endif
