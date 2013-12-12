#include <stdlib.h>
#include <stdio.h>
#include "tz_vrml_io.h"
#include "tz_geo3d_circle.h"
#include "tz_coordinate_3d.h"
#include "tz_geo3d_point_array.h"
#include "tz_bitmask.h"
#include "tz_vrml_material.h"
#include "tz_error.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
#if 0
  FILE *fp = fopen("../x3d/testvrml.wrl", "w");

  //Vrml_Sphere_Fprint(stdout, 1.0, 2);

  Vrml_Head_Fprint(fp, "v2.0", "utf8");
  /*
  Vrml_Node_Begin_Fprint(fp, "Shape", 0);
  double coord[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 3.0, 1.0, 2.2};
  Vrml_Polygon_Fprint(fp, coord, 3, 2);
  //Vrml_Disk_Fprint(fp, 1.0, 2);
  
  Vrml_Node_End_Fprint(fp, "Shape", 0);
  */


  Neuroseg *seg = New_Neuroseg();
  Set_Neuroseg(seg, 1.0, 2.0, 1.0, 1.0, 1.0);
  double pos[] = {1.0, 1.0, 1.0};
  Vrml_Neuroseg_Fprint(fp, seg, pos, 0);

  Free_Neuroseg(seg);
  fclose(fp);
#endif

#if 1
  //FILE *fp2  = fopen ("../data/fly_neuron/chain7.bn", "r");
  FILE *fp2  = fopen ("../data/mouse_neuron/chain96.bn", "r");
  Neurochain *chain = New_Neurochain();
  Neurochain_Fread(chain, fp2);
  //Neurochain_Remove_Last(chain, 12);
  Print_Neurochain(chain->next);
  fclose(fp2);

  FILE *fp = fopen("../data/testvrml.wrl", "w");
  //FILE *fp = stdout;
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Vrml_Neurochain_Fprint_C(fp, chain, NULL, 0);
  fclose(fp);
#endif

#if 0
  Geo3d_Circle bottom;
  bottom.radius = 2.8;
  bottom.center[0] = 0.0;
  bottom.center[1] = 0.0;
  bottom.center[2] = 0.0;
  bottom.orientation[0] = 4.4;
  bottom.orientation[1] = 4.3;
  Geo3d_Circle top;
  top.radius = 2.9;
  top.center[0] = 2.0;
  top.center[1] = 2.0;
  top.center[2] = 0.0;
  top.orientation[0] = 1.5;
  top.orientation[1] = 1.9;

  FILE *fp = fopen("../data/testvrml.wrl", "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Vrml_Circle_Platform_Fprint(fp, &bottom, &top, 0);
  fclose(fp);
#endif

#if 0
#  define CIRCLE_NUMBER 4
  Geo3d_Circle circle[CIRCLE_NUMBER];
  circle[0].radius = 2.8;
  circle[0].center[0] = 0.0;
  circle[0].center[1] = 0.0;
  circle[0].center[2] = 0.0;
  circle[0].orientation[0] = 4.4;
  circle[0].orientation[1] = 4.3;
  circle[1].radius = 2.9;
  circle[1].center[0] = 2.0;
  circle[1].center[1] = 2.0;
  circle[1].center[2] = 0.0;
  circle[1].orientation[0] = 1.5;
  circle[1].orientation[1] = 1.9;
  circle[2].radius = 2.5;
  circle[2].center[0] = 4.0;
  circle[2].center[1] = 4.0;
  circle[2].center[2] = 0.0;
  circle[2].orientation[0] = 2.1;
  circle[2].orientation[1] = 2.1;

  circle[3].radius = 2.5;
  circle[3].center[0] = 6.0;
  circle[3].center[1] = 6.0;
  circle[3].center[2] = 1.0;
  circle[3].orientation[0] = 2.1;
  circle[3].orientation[1] = 2.5;

  int nsample = 20;
  coordinate_3d_t *pts = Geo3d_Circle_Array_Points(circle, CIRCLE_NUMBER, 
						   nsample, NULL);

  Print_Geo3d_Point_Array(pts, 40);

  FILE *fp = fopen("../data/testvrml.wrl", "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Vrml_Node_Begin_Fprint(fp, "Shape", 0);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", 2);
  Vrml_Material *material = New_Vrml_Material();
  Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material->default_mask));
  Vrml_Material_Fprint(fp, material, 4);
  Vrml_Node_End_Fprint(fp, "Appearance", 2);
  Vrml_Pm_Rectangle_Fprint(fp, (const coordinate_3d_t *) pts, nsample, 
			   CIRCLE_NUMBER, 2);
  Vrml_Node_End_Fprint(fp, "Shape", 0);
  fclose(fp);
#endif

#if 0
  Geo3d_Circle *circle = New_Geo3d_Circle();
  Print_Geo3d_Circle(circle);
  circle->radius = 2.9;
  circle->center[0] = 0.0;
  circle->center[1] = 0.0;
  circle->center[2] = 12.0;
  circle->orientation[0] = 1.5;
  circle->orientation[1] = 1.9;
  int nsample = 20;
  coordinate_3d_t start;
  xz_orientation_t ort;
  Set_Xz_Orientation(ort, 1.0, 0.5);
  if (Geo3d_Circle_Plane_Point(circle, ort, 1, start) == 0) {
    printf("parallel\n");
    return 1;
  }

  Print_Coordinate_3d(start);
  printf("%g\n", Coordinate_3d_Norm(start));

  //Set_Coordinate_3d(start, 1.0, 0.0, 0.0);
  coordinate_3d_t *coord = 
    (coordinate_3d_t *) malloc(sizeof(coordinate_3d_t) * nsample);
  Geo3d_Circle_Points(circle, nsample, start, coord);
  
  int i;
  for (i = 0; i < nsample; i++) {
    printf("%g\n", Coordinate_3d_Distance(coord[i], circle->center));
  }

  FILE *fp = fopen("../x3d/testvrml.wrl", "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Vrml_Node_Begin_Fprint(fp, "Shape", 0);
  Vrml_Polygon_Fprint(fp, Coordinate_3d_Double_Array(coord), nsample, 2);
  Vrml_Node_End_Fprint(fp, "Shape", 0);
  fclose(fp);
#endif

  return 1;
}
