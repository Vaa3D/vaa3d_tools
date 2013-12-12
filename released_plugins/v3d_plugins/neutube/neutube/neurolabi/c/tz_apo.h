/**@file tz_apo.h
 * @brief Apo file
 * @author Linqing Feng
 * @date 15-Feb-2012
 */

#ifndef _TZ_APO_H_
#define _TZ_APO_H_

#include <stdio.h>
#include "tz_cdefs.h"
#include "utilities.h"
#include "tz_swc_tree.h"

__BEGIN_DECLS

typedef struct _Apo_Node {
  int no;   /**< id */
  char name[50];
  char comment[50];
  double maxintensity;
  double intensity;
  double x; /**< x */
  double y; /**< y */
  double z; /**< z */
  double sdev; 
  double volsize;
  double mass; 
  double r;
  char colorr[50];
  char colorg[50];
  char colorb[50];
} Apo_Node;

void Apo_Node_Fprint(FILE *fp, Apo_Node *apo);

void Apo_Marker_Fprint(FILE *fp, Apo_Node *apo);

Apo_Node* Read_Apo_File(char *file_path, int *npuncta);

double Point_Frustum_Cone_Distance(double x, double y, double z, Swc_Node* tn, Swc_Node* ptn, double zscale);

double Point_Tree_Distance(double x, double y, double z, Swc_Tree* tree, double zScale, Swc_Tree_Node** refNode);

double Puncta_Tree_Distance(double x, double y, double z, double r, Swc_Tree* tree, double pixelperumxy, double pixelperumz,
                            BOOL bmask,
                            double maskextendbyum, Swc_Tree_Node** refNode);

Stack* Read_Part_Raw_Stack(const char* stackfilename, int left, int right, int up,
      int down, int zup, int zdown, int channel);

void Label_Swc_Seg_In_Stack(Stack *stack, Swc_Node *node, Swc_Node *pnode, double zscale, int value);


__END_DECLS
#endif
