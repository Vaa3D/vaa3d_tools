/**@file tz_apo.h
 * @brief Apo file
 * @author Linqing Feng
 * @date 15-Feb-2012
 */

#include "tz_apo.h"
#include <string.h>
#include <float.h>
#include "tz_workspace.h"
#include "tz_string.h"
#include "tz_workspace.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_graph.h"
#include "tz_math.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_stack_io.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"



void Apo_Node_Fprint(FILE *fp, Apo_Node *apo)
{
  fprintf(fp, "%d,%s,%s,%s,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,%s,%s,%s,%s,%s,%s\n",
          apo->no, "", apo->name, apo->comment, apo->z, apo->x, apo->y, apo->maxintensity, apo->intensity,
          apo->sdev, apo->volsize, apo->mass, "", "", "",apo->colorr, apo->colorg, apo->colorb);
}

void Apo_Marker_Fprint(FILE *fp, Apo_Node *apo)
{
  fprintf(fp, "%5.3f,%5.3f,%5.3f,%5.3f,1,,,255,146,0\n", apo->x, apo->y, apo->z, apo->r);
}

Apo_Node* Read_Apo_File(char *file_path, int *napo)
{
  FILE *fp = GUARDED_FOPEN(file_path, "r");

  String_Workspace *sw = New_String_Workspace();

  int n = 0;
  while (Read_Line(fp, sw) != NULL) {
    n++;
  }

  Apo_Node *apos = (Apo_Node*)malloc(sizeof(Apo_Node)*n);
  *napo = 0;

  fseek(fp, 0, SEEK_SET);

  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    apos[*napo].no = atoi(line);

    char *head1 = strsplit(line, ',', 2);
    if (head1 == NULL) {
      continue;
    }

    char *head2 = strsplit(head1, ',', 1);
    if (head2 == NULL) {
      continue;
    }

    char *token = strsplit(head2, ',', 1);
    if (token != NULL) {
      strcpy(apos[*napo].name, head1);
      strcpy(apos[*napo].comment, head2);
      apos[*napo].z = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].x = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].y = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].maxintensity = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].intensity = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].sdev = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].volsize = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    if (token != NULL) {
      apos[*napo].mass = atof(token);
    } else {
      continue;
    }

    token = strsplit(token, ',', 1);
    token = strsplit(token, ',', 1);
    token = strsplit(token, ',', 1);
    token = strsplit(token, ',', 1);
    if (token == NULL) {
      apos[*napo].r = Cube_Root(0.75 / TZ_PI * apos[*napo].volsize);

      ++(*napo);
      continue;
    }

    head1 = strsplit(token, ',', 1);
    if (head1 == NULL) {
      continue;
    }

    head2 = strsplit(head1, ',', 1);
    if (head2 == NULL)
      continue;
    else {
      strcpy(apos[*napo].colorr, token);
      strcpy(apos[*napo].colorg, head1);
      strcpy(apos[*napo].colorb, head2);
    }

    // token = strsplit(token, ',', 1);
    // if (token != NULL) {
    //   strcpy(apos[*napo].colorb, token);
    // } else {
    //   continue;
    // }

    apos[*napo].r = Cube_Root(0.75 / TZ_PI * apos[*napo].volsize);

    ++(*napo);
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return apos;
}

double Point_Frustum_Cone_Distance(double x, double y, double z, Swc_Node* tn, Swc_Node* ptn, double zscale)
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

double Point_Tree_Distance(double x, double y, double z, Swc_Tree* tree, double zScale, Swc_Tree_Node** refNode)
{
  double min_dist = DBL_MAX;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (!Swc_Tree_Node_Is_Virtual(tn) && !Swc_Tree_Node_Is_Root(tn)) {
      double dist = Point_Frustum_Cone_Distance(x, y, z, &(tn->node), &(tn->parent->node), zScale);
      if (dist < min_dist) {
        min_dist = dist;
        if (refNode)
          *refNode = tn;
      }
    }
  }
  return min_dist;
}

double Puncta_Tree_Distance(double x, double y, double z, double r, Swc_Tree* tree, double pixelperumxy, double pixelperumz,
                            BOOL bmask,
                            double maskextendbyum, Swc_Tree_Node** refNode)
{
  double min_dist = DBL_MAX;
  double zscale = pixelperumxy/pixelperumz;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (!Swc_Tree_Node_Is_Virtual(tn) && !Swc_Tree_Node_Is_Root(tn) && tn->node.type != 1) {
      double dist = sqrt((tn->node.x-x)*(tn->node.x-x) + (tn->node.y-y)*(tn->node.y-y) + (tn->node.z-z)*(tn->node.z-z))-tn->node.d;
      if (dist > 1000) {
        continue;
      }
      dist = Point_Frustum_Cone_Distance(x, y, z, &(tn->node), &(tn->parent->node), zscale);
      if (bmask) {
        double maskextendxy = maskextendbyum*pixelperumxy;
        if (dist < maskextendxy + r && dist < min_dist) {   //in mask area
          min_dist = dist;
          if (refNode)
            *refNode = tn;
        }
      } else {
        if (dist < min_dist) {
          min_dist = dist;
          if (refNode)
            *refNode = tn;
        }
      }
    }
  }
  return min_dist;
}

Stack* Read_Part_Raw_Stack(const char* stackfilename, int left, int right, int up,
      int down, int zup, int zdown, int channel)
{
  FILE *fp = Guarded_Fopen(stackfilename, "r", "read part of stack");
  fseek(fp, 25L, SEEK_SET); //raw_image_stack_by_hpeng
  size_t numread, sizepixel = 0;
  size_t sizex = 0; 
  size_t sizey = 0;
  size_t sizez = 0;
  size_t sizec = 0;
  numread = fread(&sizepixel, 2, 1, fp);
  numread = fread(&sizex, 2, 1, fp);
  numread = fread(&sizey, 2, 1, fp);
  numread = fread(&sizez, 2, 1, fp);
  numread = fread(&sizec, 2, 1, fp);
  Stack *stack;
  if (sizepixel == 0 || sizex == 0 || sizey == 0 || sizez == 0 || sizec == 0) {
    fseek(fp, 25L, SEEK_SET); //raw_image_stack_by_hpeng
    numread = fread(&sizepixel, 2, 1, fp);
    numread = fread(&sizex, 4, 1, fp);
    numread = fread(&sizey, 4, 1, fp);
    numread = fread(&sizez, 4, 1, fp);
    numread = fread(&sizec, 4, 1, fp);
    printf("x:%zd y:%zd z:%zd c:%zd sizepixel:%zd\n", sizex, sizey, sizez, sizec, sizepixel);
    printf("%d %d %d %d %d %d\n", left, right, up, down, zup,zdown);
    size_t stride_x = sizepixel;
    size_t stride_y = sizepixel*sizex;
    size_t stride_z = sizepixel*sizex*sizey;
    size_t stride_c = sizepixel*sizex*sizey*sizez;
    stack = Make_Stack(sizepixel, right-left+1, down-up+1, zdown-zup+1);
    size_t i,j;
    for (i=zup; i<=zdown; i++) {
      for (j=up; j<=down; j++) {
        size_t offset = 35+8+(channel-1)*stride_c+i*stride_z+j*stride_y+left*stride_x;
        fseek(fp, offset, SEEK_SET);
        size_t desoffset = (i-zup)*sizepixel*stack->height*stack->width+(j-up)*sizepixel*stack->width;
        numread = fread(stack->array+desoffset, sizepixel, stack->width, fp);
      }
    }
  } else {
    printf("x:%zd y:%zd z:%zd c:%zd sizepixel:%zd\n", sizex, sizey, sizez, sizec, sizepixel);
    printf("%d %d %d %d %d %d\n", left, right, up, down, zup,zdown);
    size_t stride_x = sizepixel;
    size_t stride_y = sizepixel*sizex;
    size_t stride_z = sizepixel*sizex*sizey;
    size_t stride_c = sizepixel*sizex*sizey*sizez;
    stack = Make_Stack(sizepixel, right-left+1, down-up+1, zdown-zup+1);
    size_t i,j;
    for (i=zup; i<=zdown; i++) {
      for (j=up; j<=down; j++) {
        size_t offset = 35+(channel-1)*stride_c+i*stride_z+j*stride_y+left*stride_x;
        fseek(fp, offset, SEEK_SET);
        size_t desoffset = (i-zup)*sizepixel*stack->height*stack->width+(j-up)*sizepixel*stack->width;
        numread = fread(stack->array+desoffset, sizepixel, stack->width, fp);
      }
    }
  }
  fclose(fp);
  return stack;
}

void Label_Swc_Seg_In_Stack(Stack *stack, Swc_Node *node, Swc_Node *pnode, double zscale, int value)
{
  int x,y,z;
  for (z=0; z<stack->depth; z++) {
    for (y=0; y<stack->height; y++) {
      for (x=0; x<stack->width; x++) {
        if (Point_Frustum_Cone_Distance(x,y,z,node,pnode,zscale) < 0) {
          Set_Stack_Pixel(stack,x,y,z,0,value);
        }
      }
    }
  }
}


