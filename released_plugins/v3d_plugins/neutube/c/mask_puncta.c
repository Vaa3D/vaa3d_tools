/**@file mask_puncta.c
 * @brief >> using traced result to mask puncta
 * @author Linqing Feng
 * @date 21-July-2011
 */

#include <stdio.h>
#include <utilities.h>
#include "tz_string.h"
#include "tz_swc_tree.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"

double point_frustum_cone_distance(double x, double y, double z, Swc_Node* tn, Swc_Node* ptn, double zscale)
{
  double dist;
  double norm21 = (ptn->x - tn->x)*(ptn->x - tn->x) + 
                  (ptn->y - tn->y)*(ptn->y - tn->y) +
                  (ptn->z - tn->z)*(ptn->z - tn->z)*zscale*zscale;
  double norm10 = (tn->x - x)*(tn->x - x) + (tn->y - y)*(tn->y - y) +
                  (tn->z - z)*(tn->z - z)*zscale*zscale;
  double norm20 = (ptn->x - x)*(ptn->x - x) + (ptn->y - y)*(ptn->y - y) +
                  (ptn->z - z)*(ptn->z - z)*zscale*zscale;
  double dot1021 = (ptn->x - tn->x)*(tn->x - x) +
                   (ptn->y - tn->y)*(tn->y - y) +
                   (ptn->z - tn->z)*(tn->z - z)*zscale*zscale;
  if ((-dot1021/norm21) < 0) {
    dist = sqrt(norm10) - tn->d;            //ball shape in end
  }
  else if ((-dot1021/norm21) > 1) {
    dist = sqrt(norm20) - ptn->d;
  }
  else {
    double radius = (1+dot1021/norm21) * tn->d + (-dot1021/norm21) * ptn->d;
    dist = sqrt(norm10-(dot1021*dot1021)/(norm21)) - radius;
  }
  return dist;
}

double point_line_distance(double x, double y, double z, Swc_Tree_Node* tn, Swc_Tree_Node* ptn, double zscale)
{
  double dist;
  double norm21 = (ptn->node.x-tn->node.x)*(ptn->node.x-tn->node.x) + 
                  (ptn->node.y-tn->node.y)*(ptn->node.y-tn->node.y) +
                  (ptn->node.z-tn->node.z)*(ptn->node.z-tn->node.z)*zscale*zscale;
  double norm10 = (tn->node.x-x)*(tn->node.x-x) + (tn->node.y-y)*(tn->node.y-y) +
                  (tn->node.z-z)*(tn->node.z-z)*zscale*zscale;
  double norm20 = (ptn->node.x-x)*(ptn->node.x-x) + (ptn->node.y-y)*(ptn->node.y-y) +
                  (ptn->node.z-z)*(ptn->node.z-z)*zscale*zscale;
  double dot1021 = (ptn->node.x-tn->node.x)*(tn->node.x-x) +
                   (ptn->node.y-tn->node.y)*(tn->node.y-y) +
                   (ptn->node.z-tn->node.z)*(tn->node.z-z)*zscale*zscale;
  if ((-dot1021/norm21) < 0) {
    dist = sqrt(norm10);            //ball shape in end
  }
  else if ((-dot1021/norm21) > 1) {
    dist = sqrt(norm20);
  }
  else {
    dist = sqrt(norm10-(dot1021*dot1021)/(norm21));
  }
  return dist;
}

Stack* make_mask_stack(const char* traceresult, int dim1, int dim2, int dim3, double z_scale, double pixelperumxy,
                        double pixelperumz, double maskextendbyum)
{
  Stack *mask = Make_Stack(GREY, dim1, dim2, dim3);
  Zero_Stack(mask);

  if (String_Ends_With(traceresult, ".swc")) {
    //build mask from swc file
    Swc_Tree *tree = Read_Swc_Tree(traceresult);
    Swc_Tree_Iterator_Leaf(tree);
    Swc_Tree_Node *tn = tree->begin;
    while (tn != NULL) {
      Swc_Tree_Node *tmptn = tn;
      while (!Swc_Tree_Node_Is_Root(tmptn) && tmptn->node.type != 1) {  /*without soma area*/
        Swc_Tree_Node *pNode = tmptn->parent;
        double maxradius = MAX2(tmptn->node.d, pNode->node.d);
        
        double extendxy = pixelperumxy*maskextendbyum;
        double extendz = pixelperumz*maskextendbyum;
        double xhigh = MAX2(pNode->node.x, tmptn->node.x);
        xhigh = ((xhigh+extendxy+maxradius)>(dim1-1)) ? (dim1-1) : (xhigh+extendxy+maxradius);
        double xlow = MIN2(pNode->node.x, tmptn->node.x);
        xlow = ((xlow-extendxy-maxradius)<0) ? 0 : (xlow-extendxy-maxradius);
        double yhigh = MAX2(pNode->node.y, tmptn->node.y);
        yhigh = ((yhigh+extendxy+maxradius)>(dim2-1)) ? (dim2-1) : (yhigh+extendxy+maxradius);
        double ylow = MIN2(pNode->node.y, tmptn->node.y);
        ylow = ((ylow-extendxy-maxradius)<0) ? 0 : (ylow-extendxy-maxradius);
        double zhigh = MAX2(pNode->node.z, tmptn->node.z);
        zhigh = ((zhigh+extendz+maxradius)>(dim3-1)) ? (dim3-1) : (zhigh+extendz+maxradius);
        double zlow = MIN2(pNode->node.z, tmptn->node.z);
        zlow = ((zlow-extendz-maxradius)<0) ? 0 : (zlow-extendz-maxradius);
        double zscale = pixelperumxy/pixelperumz;
        int i,j,k;
        for (i=iround(xlow); i<=iround(xhigh); i++) {
          for (j=iround(ylow); j<=iround(yhigh); j++) {
            for (k=iround(zlow); k<=iround(zhigh); k++) {
              //double dist = point_line_distance(i, j, k, tmptn, pNode, zscale);
              double dist = point_frustum_cone_distance(i,j,k,&(tmptn->node),&(pNode->node),zscale);
//            double norm21 = (pNode->node.x-tmptn->node.x)*(pNode->node.x-tmptn->node.x) +
//              (pNode->node.y-tmptn->node.y)*(pNode->node.y-tmptn->node.y) +
//              (pNode->node.z-tmptn->node.z)*(pNode->node.z-tmptn->node.z)*zscale*zscale;
//            double norm10 = (tmptn->node.x-i)*(tmptn->node.x-i) + (tmptn->node.y-j)*(tmptn->node.y-j) +
//              (tmptn->node.z-k)*(tmptn->node.z-k)*zscale*zscale;
//            double norm20 = (pNode->node.x-i)*(pNode->node.x-i) + (pNode->node.y-j)*(pNode->node.y-j) +
//              (pNode->node.z-k)*(pNode->node.z-k)*zscale*zscale;
//            double dot1021 = (pNode->node.x-tmptn->node.x)*(tmptn->node.x-i) +
//              (pNode->node.y-tmptn->node.y)*(tmptn->node.y-j) +
//              (pNode->node.z-tmptn->node.z)*(tmptn->node.z-k)*zscale*zscale;
//            if ((-dot1021/norm21) < 0) {
//              dist = sqrt(norm10);            //ball shape in end
//            } else if ((-dot1021/norm21) > 1) {
//              dist = sqrt(norm20);
//            } else {
//              dist = sqrt(norm10-(dot1021*dot1021)/(norm21));
//            }

              if (dist < extendxy) {
                *(mask->array + ((k*mask->height + j)*mask->width + i)) = 255;
              }
            }
          }
        }

        tmptn = tmptn->parent;
      }  
      tn = tn->next;
    }
  } else {
    //build mask from .tb file
    int n;
    Locseg_Chain* chains = Dir_Locseg_Chain_N(traceresult, "^chain[[:digit:]]*\\.tb", &n, NULL);

    printf("found %d tb file\n", n);
    int i=0;
    Locseg_Label_Workspace* ws = New_Locseg_Label_Workspace();
    double maskextendbypixel = maskextendbyum * pixelperumxy;
    for (i=0; i<n; i++) {
      Default_Locseg_Label_Workspace(ws);
      ws->option = 1;
      ws->sdiff = maskextendbypixel;
      ws->value = 255;
      ws->slimit = 50;
      Locseg_Chain_Label_W(&(chains[i]), mask, z_scale, 0, Locseg_Chain_Length(&(chains[i])), ws);
    }
  }
  return mask;
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string>", "[-o <string>] [-maskstackinput <string>]",
    "[-traceresult <string>]", "[-dimension <int> <int> <int>]", "[-maskextendbyum <double(2.5)>]",
    "[-pixelperumxy <double(9.66)>]", "[-pixelperumz <double(2)>]", "[-zscale <double(1)>]",
    "[-maskstackoutput <string>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Stack *mask;
  if (Is_Arg_Matched("-maskstackinput")) {
    mask = Read_Stack(Get_String_Arg("-maskstackinput"));
  } else {
    char* traceresult = Get_String_Arg("-traceresult");
    int dim1 = Get_Int_Arg("-dimension", 1);
    int dim2 = Get_Int_Arg("-dimension", 2);
    int dim3 = Get_Int_Arg("-dimension", 3);
    double z_scale = Get_Double_Arg("-zscale");
    double pixelperumxy = Get_Double_Arg("-pixelperumxy");
    double pixelperumz = Get_Double_Arg("-pixelperumz"); 
    double maskextendbyum = Get_Double_Arg("-maskextendbyum");
    mask = make_mask_stack(traceresult, dim1, dim2, dim3, z_scale, pixelperumxy, pixelperumz, maskextendbyum);
    if (Is_Arg_Matched("-maskstackoutput")) {
      if (fhasext(Get_String_Arg("-maskstackoutput"), "raw")) {
        Write_Raw_Stack(Get_String_Arg("-maskstackoutput"), mask);
      } else {
        Write_Stack(Get_String_Arg("-maskstackoutput"), mask);
      }
    }
  }


  int n;
  Swc_Node *punctas = Read_Swc_File(Get_String_Arg("input"), &n);
  printf("found %d puncta\n", n);

  FILE *fp = Guarded_Fopen(Get_String_Arg("-o"), "w", "open output file");
  int i;
  for (i=0; i<n; i++) {
    if (Stack_Neighbor_Mean(mask, 26, iround(punctas[i].x), iround(punctas[i].y), iround(punctas[i].z)) > 0) {
      Swc_Node_Fprint(fp, &(punctas[i]));
    }
  }
  fclose(fp);


  return 0;
}

