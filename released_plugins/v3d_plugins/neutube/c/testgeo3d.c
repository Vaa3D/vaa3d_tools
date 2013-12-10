/* testgeo3d.c
 *
 * 22-Jan-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <math.h>
#include <utilities.h>
#include <time.h>
#include <unistd.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_3dgeom.h"
#include "tz_constant.h"
#include "tz_geo3d_utils.h"
#include "tz_geometry.h"
#include "tz_geo3d_vector.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_darray.h"
#include "tz_random.h"
#include "tz_geo3d_ellipse.h"
#include "tz_geo3d_point_array.h"
#include "tz_geoangle_utils.h"
#include "tz_random.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    coordinate_3d_t coord = {0, 0, 0};
    
    /* Translate coordinates. */
    Geo3d_Translate_Coordinate(coord, coord+1, coord+2, 1.0, 2.0, 3.0);

    if ((coord[0] != 1.0) || (coord[1] != 2.0) || (coord[2] != 3.0)) {
      Print_Coordinate_3d(coord);
      PRINT_EXCEPTION(":( Bug?", "Unexpected coordinate values.");
      return 1;      
    }
    
    coordinate_3d_t coord2 = {0, 0, 0};
    double dx, dy, dz;

    Geo3d_Coordinate_Offset(coord[0], coord[1], coord[2], coord2[0], coord2[1],
			    coord2[2], &dx, &dy, &dz);
    if ((dx != -coord[0]) || (dy != -coord[1]) || (dz != -coord[2])) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected offset values.");
      return 1;      
    }
    
    Coordinate_3d_Copy(coord2, coord);

    /* Rotate coordinates. */
    Geo3d_Rotate_Coordinate(coord, coord+1, coord+2, 0.1, 0.5, 0);
    
    if (fabs(Geo3d_Orgdist_Sqr(coord[0], coord[1], coord[2]) - 
	     Geo3d_Orgdist_Sqr(coord2[0], coord2[1], coord2[2])) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Rotation failed.");
      return 1;            
    }

    /* inverse rotation */
    Geo3d_Rotate_Coordinate(coord, coord+1, coord+2, 0.1, 0.5, 1);
    if ((fabs(coord[0] - coord2[0]) > 0.01) || 
	(fabs(coord[1] - coord2[1]) > 0.01) ||
	(fabs(coord[2] - coord2[2]) > 0.01)){
      PRINT_EXCEPTION(":( Bug?", "Rotation failed.");
      return 1;            
    }

    /* Normal vector of an orientation. This is a canonical representaion
     * of an orientation. */ 
    Geo3d_Orientation_Normal(0.1, 0.5, coord, coord+1, coord+2);

    /* We can also turn the normal vector into orientation. */
    double theta, psi;
    Geo3d_Normal_Orientation(coord[0], coord[1], coord[2], &theta, &psi);
    if (fabs(theta - 0.1) > 0.01 || fabs(Normalize_Radian(psi) - 0.5) > 0.01) {
      printf("%g, %g\n", theta, psi);
      PRINT_EXCEPTION(":( Bug?", "Unexpected orientation values.");
      return 1; 
    }

    /* If we rotate the vector back, we should get (0, 0, 1). */
    Geo3d_Rotate_Coordinate(coord, coord+1, coord+2, 0.1, 0.5, 1);
    
    if ((fabs(coord[0]) > 0.01) || (fabs(coord[1]) > 0.01) ||
	(fabs(coord[2] - 1.0) > 0.01)){
      PRINT_EXCEPTION(":( Bug?", "Rotation failed.");
      return 1;            
    }

    Geo3d_Coord_Orientation(1, 0, 0, &theta, &psi);
    if (fabs(theta - TZ_PI_2) > 0.01 || 
	fabs(Normalize_Radian(psi) - TZ_PI_2) > 0.01) {
      printf("%g, %g\n", theta, psi);
      PRINT_EXCEPTION(":( Bug?", "Unexpected orientation values.");
      return 1; 
    }

    Geo3d_Coord_Orientation(5, 0, 0, &theta, &psi);
    if (fabs(theta - TZ_PI_2) > 0.01 || 
	fabs(Normalize_Radian(psi) - TZ_PI_2) > 0.01) {
      printf("%g, %g\n", theta, psi);
      PRINT_EXCEPTION(":( Bug?", "Unexpected orientation values.");
      return 1; 
    }

    /* Rotate the orientation. */
    theta = TZ_PI_2;
    psi = TZ_PI;
    Geo3d_Rotate_Orientation(-TZ_PI, TZ_PI_2, &theta, &psi);
    if (fabs(theta + TZ_PI_2) > 0.01 || 
	fabs(psi + TZ_PI_2) > 0.01) {
      printf("%g, %g\n", theta, psi);
      PRINT_EXCEPTION(":( Bug?", "Unexpected orientation values.");
      return 1; 
    }
    
    /* More extensive test on rotation */
    Random_Seed(time(NULL) - getpid());
    int i;
    for (i = 0; i < 100; i++) {
      double x = Unifrnd() * (1 - Bernrnd(0.5) * 2.0);
      double y = Unifrnd() * (1 - Bernrnd(0.5) * 2.0);
      double z = Unifrnd() * (1 - Bernrnd(0.5) * 2.0);
      double theta, psi;
      Geo3d_Coord_Orientation(x, y, z, &theta, &psi);

      double x2 = 0.0;
      double y2 = 0.0;
      double z2 = Geo3d_Orgdist(x, y, z);
      Geo3d_Rotate_Coordinate(&x2, &y2, &z2, theta, psi, 0);

      if (fabs(x - x2) > 0.01 || fabs(y - y2) > 0.01 || fabs(z - z2) > 0.01) {
        printf("%g, %g\n", theta, psi);
        printf("%g, %g, %g\n", x, y, z);
        printf("%g, %g, %g\n", x2, y2, z2);
        PRINT_EXCEPTION(":( Bug?", "Unexpected orientation values.");
        return 1; 
      }
    }

    /* Dot product. */
    if (Geo3d_Dot_Product(1.0, 2.0, 3.0, 3.0, 2.0, 1.0) != 10.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected dot product.");
      return 1; 
    }

    /* Cross product. */
    Geo3d_Cross_Product(1.0, 2.0, 3.0, 3.0, 1.0, 2.0, coord, coord+1, coord+2);

    if (fabs(Geo3d_Dot_Product(1.0, 2.0, 3.0, coord[0], coord[1], coord[2])) >
	0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected dot product.");
      return 1; 
    }

    if (fabs(Geo3d_Dot_Product(3.0, 1.0, 2.0, coord[0], coord[1], coord[2])) >
	0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected dot product.");
      return 1; 
    }

    /* Distance calculations. */
    if (Geo3d_Orgdist_Sqr(1.0, 2.0, 3.0) != 14.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected value.");
      return 1;       
    }

    if (fabs(Geo3d_Orgdist(1.0, 2.0, 3.0) - sqrt(14.0)) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected value.");
      return 1;       
    }

    if (Geo3d_Dist(1.0, 2.0, 3.0, 1.0, 2.0, 3.0) != 0.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected value.");
      return 1;       
    }

    if (Geo3d_Dist_Sqr(1.0, 2.0, 3.0, 0.0, 0.0, 0.0) != 14.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected value.");
      return 1;       
    }

    /* Angle between two vectors. */
    if (fabs(Geo3d_Angle2(1, 0, 0, 0, 0, 1) - TZ_PI_2) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected value.");
      return 1;             
    }

    /* Distance between two lines */
    coordinate_3d_t line1_start = {0, 0, 0};
    coordinate_3d_t line1_end = {1, 0, 0};
    coordinate_3d_t line2_start = {0, 1, 1}; 
    coordinate_3d_t line2_end = {0, 2, 1}; 

    double d = Geo3d_Line_Line_Dist(line1_start, line1_end,
				    line2_start, line2_end);

    if (fabs(d - 1.0) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected distance.");
      return 1;             
    }
    
    /* Distance between two line segments. */
    double intersect1, intersect2;
    int cond;
    d = Geo3d_Lineseg_Lineseg_Dist(line1_start, line1_end,
				   line2_start, line2_end, 
				   &intersect1, &intersect2, &cond);
    if (fabs(d - sqrt(2.0)) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected distance.");
      return 1;             
    }

    /* Distance between a point and a line segment. */
    coord[0] = 0.5;
    coord[1] = 1.0;
    coord[2] = 1.0;
    d = Geo3d_Point_Lineseg_Dist(coord, line1_start, line1_end, &intersect1);
    if (fabs(d - sqrt(2.0)) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected distance.");
      return 1;             
    }

    /* Get a break point of a line segment. */
    Geo3d_Lineseg_Break(line1_start, line1_end, 0.1, coord);
    if ((coord[0] != 0.1) || (coord[1] != 0.0) || (coord[2] != 0.0)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected break point.");
      return 1;                   
    }

    /* Orientations of a set of points */

    /* Orientation of a covariance matrix */

    /* 3D coordinates routines */
    /* Unitize a point. */
    Coordinate_3d_Unitize(coord);
    
    if (fabs(Coordinate_3d_Norm(coord) - 1.0) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected norm.");
      return 1;                   
    }

    Set_Coordinate_3d(coord, 0.0, 0.0, 0.0);

    /* origin point is not affected by unitization */
    Coordinate_3d_Unitize(coord);
    
    if (Coordinate_3d_Norm(coord) != 0.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected norm.");
      return 1;                   
    }
    
    Set_Coordinate_3d(coord, 1.0, 2.0, 3.0);

    /* Square length */
    if (Coordinate_3d_Length_Square(coord) != 14) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected square length");
    }

    /* Angle between two vectors */
    coordinate_3d_t coord_a1 = { 1, 0, 0 }; 
    coordinate_3d_t coord_a2 = { 0, 0, 1 };

    if (fabs(Coordinate_3d_Angle2(coord_a1, coord_a2) - TZ_PI_2) > 0.01) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected angle");
    }


    /* Scale coordinates */
    Coordinate_3d_Scale(coord, 2.0);
    
    if ((coord[0] != 2.0) || (coord[1] != 4.0) || (coord[2] != 6.0)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected coordinate value.");
      return 1;                         
    }

    /* Normalizd dot product */
    if (Coordinate_3d_Normalized_Dot(coord, coord2) > 1.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected dot product.");
      return 1;                               
    }

    Set_Coordinate_3d(coord, 1.0, 2.0, 3.0);    

    Coordinate_3d_Copy(coord2, coord);
    Coordinate_3d_Scale(coord2, 2.0);

    coordinate_3d_t coord3;
    Coordinate_3d_Copy(coord3, coord);
    Coordinate_3d_Scale(coord3, 3.0);

    /* cos value of an angle formed by 3 points */
    if (Coordinate_3d_Cos3(coord, coord2, coord3) != 1.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected cos value.");
      return 1;      
    }

    /* cos value of an angle formed by 4 points */
    coordinate_3d_t coord4;
    Coordinate_3d_Copy(coord4, coord);
    Coordinate_3d_Scale(coord4, 4.0);

    if (Coordinate_3d_Cos4(coord, coord2, coord3, coord4) != 1.0) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected cos value.");
      return 1;      
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  Geo3d_Vector v1 = {1.1, 0, 0};
  Geo3d_Vector v2 = {5, 0, 0};

  double theta1 = 3.83812;
  double psi1 = 3.53202;
  double theta2 = 0.72657;
  double psi2 = 3.61214;

  Geo3d_Orientation_Normal(theta1, psi1, &(v1.x), &(v1.y), &(v1.z));
  Geo3d_Orientation_Normal(theta2, psi2, &(v2.x), &(v2.y), &(v2.z));
 
  Print_Geo3d_Vector(&v1);
  Print_Geo3d_Vector(&v2);

  printf("%g\n", Geo3d_Vector_Dot(&v1, &v2));
  printf("%g\n", Geo3d_Vector_Angle2(&v1, &v2));

  
  double angle = Vector_Angle(-1.0, -1.0);
  
  theta1 = -3.0;
  psi1 = -5.0;
  
  double x, y, z;
  Geo3d_Orientation_Normal(theta1, psi1, &x, &y, &z);
  printf("%g, %g, %g\n", x, y, z);

  Geo3d_Normal_Orientation(x, y, z, &theta1, &psi1);

  printf("%g\n", angle * 180.0 / TZ_PI);

  printf("%g, %g\n", theta1, psi1);
  Geo3d_Orientation_Normal(theta1, psi1, &x, &y, &z);
  printf("%g, %g, %g\n", x, y, z);
#endif

#if 0
  Geo3d_Scalar_Field *field = 
    Read_Geo3d_Scalar_Field("../data/mouse_neuron/seeds.bn");
  Print_Geo3d_Scalar_Field_Info(field);
  Stack *stack = Read_Stack("../data/mouse_neuron/mask.tif");
  Geo3d_Scalar_Field_Draw_Stack(field, stack, NULL, NULL);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  double line_start[3] = {2.0, 3.0, 4.0};
  double line_end[3] = {1.0, 1.0, 6.0};

  double point[3] = {10.5, 20.5, 40.7};

  double lamda;
  printf("%g\n", Geo3d_Point_Lineseg_Dist(point, line_start, line_end, &lamda));
  double line_point[3];
  double length = sqrt(Geo3d_Dist_Sqr(line_start[0], line_start[1], line_start[2], line_end[0], line_end[1], line_end[2]));
  line_point[0] = line_start[0] + lamda * (line_end[0] - line_start[0]);
  line_point[1] = line_start[1] + lamda * (line_end[1] - line_start[1]);
  line_point[2] = line_start[2] + lamda * (line_end[2] - line_start[2]);
  printf("%g\n", lamda);
  printf("%g\n", sqrt(Geo3d_Dist_Sqr(line_point[0], line_point[1], line_point[2], point[0], point[1], point[2])));

  darray_sub(point, line_point, 3);
  darray_sub(line_end, line_point, 3);
  printf("%g\n", Geo3d_Dot_Product(point[0], point[1], point[2],
				   line_end[0], line_end[1], line_end[2]));

#endif

#if 0
  Geo3d_Scalar_Field *field = Read_Geo3d_Scalar_Field("../data/mouse_neuron3_org/seeds");
  //Print_Geo3d_Scalar_Field(field);

  darray_write("../data/pts.bn", (double *)field->points, field->size * 3);
  darray_write("../data/ws.bn", field->values, field->size);

  coordinate_3d_t centroid;
  Geo3d_Scalar_Field_Centroid(field, centroid);
  Print_Coordinate_3d(centroid);
  
  double cov[9];
  Geo3d_Scalar_Field_Cov(field, cov);
  darray_print2(cov, 3, 3);

  darray_write("../data/cov.bn", cov, 9);

  double vec[3];
  Geo3d_Cov_Orientation(cov, vec);
  darray_print2(vec, 3, 1);

  
#endif

#if 0
  double mu1, mu2;

#  if 0 /* parallel case */
  double line1_start[3] = {0, 0, 0};
  double line1_end[3] = {1, 1, 10};
  double line2_start[3] = {0, 1, 0};
  double line2_end[3] = {1, 2, 10};
#  endif

#  if 0 /* parallel case */
  double line1_start[3] = {0, 0, 0};
  double line1_end[3] = {1, 1, 10};
  double line2_start[3] = {-1, -1, -1};
  double line2_end[3] = {-3, -3, -21};
#  endif

#  if 0 /* i-i case */
  double line1_start[3] = {0, 0, 0};
  double line1_end[3] = {3, 4, 5};
  double line2_start[3] = {-1, 8, 9};
  double line2_end[3] = {1, -3, -4};
#  endif

#  if 0 /* point to line case */
  double line1_start[3] = {3, 4, 5};
  double line1_end[3] = {3, 4, 5};
  double line2_start[3] = {-1, 8, 9};
  double line2_end[3] = {1, -3, -4};
#  endif

#  if 0 /* point to line case */
  double line2_start[3] = {3, 4, 5};
  double line2_end[3] = {3, 4, 5};
  double line1_start[3] = {-1, 8, 9};
  double line1_end[3] = {1, -3, -4};
#  endif

#  if 0 /* point to line case */
  double line2_start[3] = {3, 4, 5};
  double line2_end[3] = {3, 4, 5};
  double line1_start[3] = {-1, 8, 9};
  double line1_end[3] = {-1, 8, 9};
#  endif

#  if 0
  double line1_start[3] = {256.062, 328.674, 67.9029};
  double line1_end[3] = {246.162, 330.078, 67.9896};
  double line2_start[3] = {262.368, 336.609, 68.3076};
  double line2_end[3] = {259.956, 326.944, 67.4325};
#  endif

  int cond;
  double dist = Geo3d_Lineseg_Lineseg_Dist(line1_start, line1_end, 
					   line2_start, line2_end, 
					   &mu1, &mu2, &cond);
  printf("%d, %g\n", cond, dist);

  dist = Geo3d_Line_Line_Dist(line1_start, line1_end, line2_start, line2_end);

  printf("%g\n", dist);
#endif

#if 0
  double *d = Unifrnd_Double_Array(12000000, NULL);
  double *d2 = Unifrnd_Double_Array(12000000, NULL);
  tic();
  Rotate_XZ2(d, d, 4000000, 0.1, 0.2, 1);
  printf("%llu\n", toc());
  tic();
  Rotate_XZ(d, d, 4000000, 0.1, 0.2, 0);
  printf("%llu\n", toc());

#endif

#if 0
  double d[3] = {0.1, 0.2, 0.3};
  Rotate_XZ(d, d, 1, 0.1, 0.2, 0);
  darray_print2(d, 3, 1);
#endif

#if 0
  printf("%g\n", Ellipse_Point_Distance(5, 5, 3, 5));
#endif

#if 0
  Geo3d_Ellipse *ellipse = New_Geo3d_Ellipse();
  ellipse->scale = 0.5;
  ellipse->orientation[0] = 1.0;
  Print_Geo3d_Ellipse(ellipse);

  FILE *fp = fopen("../data/test.swc", "w");
  Swc_Node node;
  
  Geo3d_Ellipse_To_Swc_Node(ellipse, 1, -1, 1.0, 2, &node);
  Swc_Node_Fprint(fp, &node);  
  
  /*
  double pt[3] = {0.0, 0.0, 1.0};
  printf("%g\n", Geo3d_Ellipse_Point_Distance(ellipse, pt));
  */
  
  Geo3d_Ellipse *ellipse2 = Copy_Geo3d_Ellipse(ellipse);
  ellipse2->center[0] += 10.0;
  ellipse2->orientation[0] = 2.0;
  ellipse2->radius += 3.0;

  Geo3d_Ellipse *ellipse3 = Geo3d_Ellipse_Interpolate(ellipse, ellipse2, 0.2,
						      NULL);
  Geo3d_Ellipse_To_Swc_Node(ellipse3, 2, 1, 1.0, 2, &node);
  Swc_Node_Fprint(fp, &node);

  Geo3d_Ellipse_To_Swc_Node(ellipse2, 3, 2, 1.0, 2, &node);
  Swc_Node_Fprint(fp, &node);

  Print_Geo3d_Ellipse(ellipse3);

  fclose(fp);
#endif

#if 0
  Geo3d_Ellipse *ellipse = New_Geo3d_Ellipse();
  ellipse->scale = 0.5;

  FILE *fp = fopen("../data/test.swc", "w");
  coordinate_3d_t *pts = Geo3d_Ellipse_Sampling(ellipse, 20, 0, NULL);
  Geo3d_Point_Array_Swc_Fprint(fp, pts, 20, 1, -1, 1.0, 2);

  ellipse->orientation[0] = 1.0;
  pts = Geo3d_Ellipse_Sampling(ellipse, 20, 0, NULL);
  Geo3d_Point_Array_Swc_Fprint(fp, pts, 20, 21, -1, 1.0, 3);

  ellipse->center[2] = 5.0;
  pts = Geo3d_Ellipse_Sampling(ellipse, 20, 0, NULL);
  Geo3d_Point_Array_Swc_Fprint(fp, pts, 20, 41, -1, 1.0, 4);

  coordinate_3d_t vec[2];
  Coordinate_3d_Copy(vec[0], ellipse->center);
  Geo3d_Ellipse_First_Vector(ellipse, vec[1]);
  Coordinate_3d_Add(vec[0], vec[1], vec[1]);
  Geo3d_Point_Array_Swc_Fprint(fp, vec, 2, 101, -1, 1.0, 5);
 
  Geo3d_Ellipse_Second_Vector(ellipse, vec[1]);
  Coordinate_3d_Add(vec[0], vec[1], vec[1]);
  Geo3d_Point_Array_Swc_Fprint(fp, vec, 2, 111, -1, 1.0, 6);
 
  Geo3d_Ellipse_Normal_Vector(ellipse, vec[1]);
  Coordinate_3d_Add(vec[0], vec[1], vec[1]);
  Geo3d_Point_Array_Swc_Fprint(fp, vec, 2, 121, -1, 1.0, 7);
 
  fclose(fp);
#endif

#if 0
  Geo3d_Ellipse ep_array[10];

  Geo3d_Ellipse *ellipse = New_Geo3d_Ellipse();
  ellipse->scale = 0.5;

  Geo3d_Ellipse_Copy(ep_array, ellipse);

  FILE *fp = fopen("../data/test.swc", "w");

  ellipse->orientation[0] += 0.5;
  ellipse->center[2] += 3.0;
  ellipse->center[0] += 1.0;
  Geo3d_Ellipse_Copy(ep_array + 1, ellipse);

  ellipse->center[2] += 3.0;
  Geo3d_Ellipse_Copy(ep_array + 2, ellipse);

  coordinate_3d_t *pts = Geo3d_Ellipse_Array_Sampling(ep_array, 3, 20, NULL);
  Geo3d_Point_Array_Swc_Fprint(fp, pts, 60, 1, -1, 1.0, 2);

  fclose(fp);
#endif

#if 0
  printf("%g\n", Vector_Angle(1.0, 1.0) * 180 / TZ_PI);
#endif

#if 0
  double theta, psi;
  coordinate_3d_t coord;
  coord[0] = 0.0822;
  coord[1] = 0.1515;
  coord[2] = 0.1369;
  Geo3d_Coord_Orientation(coord[0], coord[1], coord[2], &theta, &psi);
  printf("%g, %g\n", theta, psi);

  Geo3d_Orientation_Normal(theta, psi, coord, coord+1, coord+2); 
  Print_Coordinate_3d(coord);
#endif

#if 0
  double theta, psi;
  double x = -1;
  double y = 1.83697019872103e-16;
  double z = 3;
  
  Geo3d_Coord_Orientation(x, y, z, &theta, &psi);
#endif

#if 1
  double theta = Vector_Angle2(0, 1, 1, 0, TRUE);
  printf("angle: %g\n", theta);

  theta = Vector_Angle2(0, 1, -1, 0, FALSE);
  printf("angle: %g\n", theta);

  theta = Vector_Angle2(-1, 0, -1, -1, TRUE);
  printf("angle: %g\n", theta);
#endif

  return 0;
}
