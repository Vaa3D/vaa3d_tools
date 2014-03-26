/* tz_vrml_io.c
 *
 * 17-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include <math.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_string.h"
#include "tz_3dgeom.h"
#include "tz_geoangle_utils.h"
#include "tz_geo3d_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_geo3d_point_array.h"
#include "tz_vrml_material.h"
#include "tz_vrml_io.h"

void Vrml_Head_Fprint(FILE *fp, const char *version, const char *charset)
{
  fprintf(fp, "#VRML %s %s\n", version, charset);
}

void Vrml_SFFloat_Fprint(FILE *fp, SFFloat value)
{
  fprintf(fp, "%g", value);
}

void Vrml_SFColor_Fprint(FILE *fp, const SFColor color)
{
  fprintf(fp, "%g %g %g", color[0], color[1], color[2]);
}

void Vrml_SFBool_Fprint(FILE *fp, SFBool value)
{
  if (value == FALSE) {
    fprintf(fp, "FALSE");
  } else {
    fprintf(fp, "TRUE");
  }
}

void Vrml_SFVec3f_Fprint(FILE *fp, const SFVec3f vec)
{
  fprintf(fp, "%g %g %g", vec[0], vec[1], vec[2]);
}

void Vrml_SFVec3f_Set(SFVec3f vec, double x, double y, double z)
{
  vec[0] = x;
  vec[1] = y;
  vec[2] = z;
}

void Vrml_SFFloat_Field_Fprint(FILE *fp, const char *field, SFFloat value)
{
  fprintf(fp, "%s", field);
  fprintf(fp, " ");
  Vrml_SFFloat_Fprint(fp, value);
}

void Vrml_SFColor_Field_Fprint(FILE *fp, const char *field, const SFColor color)
{
  fprintf(fp, "%s", field);
  fprintf(fp, " ");
  Vrml_SFColor_Fprint(fp, color);
}

void Vrml_SFBool_Field_Fprint(FILE *fp, const char *field, SFBool value)
{
  fprintf(fp, "%s", field);
  fprintf(fp, " ");
  Vrml_SFBool_Fprint(fp, value);
}

void Vrml_SFVec3f_Field_Fprint(FILE *fp, const char *field, const SFVec3f vec)
{
  fprintf(fp, "%s", field);
  fprintf(fp, " ");
  Vrml_SFVec3f_Fprint(fp, vec);
}

void Vrml_SFFloat_Field_Fprintln(FILE *fp, const char *field, SFFloat value,
				 int indent)
{
  fprint_space(fp, indent);
  Vrml_SFFloat_Field_Fprint(fp, field, value);
  fprintf(fp, "\n");
}

void Vrml_SFColor_Field_Fprintln(FILE *fp, const char *field, 
				 const SFColor color, int indent)
{
  fprint_space(fp, indent);
  Vrml_SFColor_Field_Fprint(fp, field, color);
  fprintf(fp, "\n");
}

void Vrml_SFBool_Field_Fprintln(FILE *fp, const char *field, SFBool value, 
				int indent)
{
  fprint_space(fp, indent);
  Vrml_SFBool_Field_Fprint(fp, field, value);
  fprintf(fp, "\n");  
}

void Vrml_SFVec3f_Field_Fprintln(FILE *fp, const char *field, 
				 const SFVec3f vec, int indent)
{
  fprint_space(fp, indent);
  Vrml_SFVec3f_Field_Fprint(fp, field, vec);
  fprintf(fp, "\n");  
}

void Vrml_Sphere_Fprint(FILE *fp, const Vrml_Sphere *sphere, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "Sphere", indent);

  if (Bitmask_Get_Bit(sphere->default_mask, 0) == FALSE) {
    Vrml_SFFloat_Field_Fprintln(fp, "radius", sphere->radius, 
				indent + VRML_INDENT);
  }
  Vrml_Node_End_Fprint(fp, "Sphere", indent);
}

void Vrml_Cone_Fprint(FILE *fp, const Vrml_Cone *cone, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "Cone", indent);
  if (Bitmask_Get_Bit(cone->default_mask, 0) == FALSE) {
    Vrml_SFFloat_Field_Fprintln(fp, "bottomRadius", cone->bottom_radius,
				indent + VRML_INDENT);
  }
  if (Bitmask_Get_Bit(cone->default_mask, 1) == FALSE) {
    Vrml_SFFloat_Field_Fprintln(fp, "height", cone->height, 
				indent + VRML_INDENT);
  }
  if (Bitmask_Get_Bit(cone->default_mask, 2) == FALSE) {
    Vrml_SFBool_Field_Fprintln(fp, "side", cone->side, indent + VRML_INDENT);
  }
  if (Bitmask_Get_Bit(cone->default_mask, 3) == FALSE) {
    Vrml_SFBool_Field_Fprintln(fp, "bottom", cone->bottom, 
			       indent + VRML_INDENT);
  }
  Vrml_Node_End_Fprint(fp, "Cone", indent);
}

void Vrml_Box_Fprint(FILE *fp, const Vrml_Box *box, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "Box", indent);
  Vrml_SFVec3f_Field_Fprintln(fp, "size", box->size, 
			      indent + VRML_INDENT);
  Vrml_Node_End_Fprint(fp, "Box", indent);  
}

void Vrml_Cylinder_Fprint(FILE *fp, const Vrml_Cylinder *cylinder, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "Cylinder", indent);
  int indent2 = indent + VRML_INDENT;
  Vrml_SFBool_Field_Fprintln(fp, "bottom", cylinder->bottom, indent2);
  Vrml_SFFloat_Field_Fprintln(fp, "height", cylinder->height, indent2);
  Vrml_SFFloat_Field_Fprintln(fp, "radius", cylinder->radius, indent2);
  Vrml_SFBool_Field_Fprintln(fp, "side", cylinder->side, indent2);
  Vrml_SFBool_Field_Fprintln(fp, "top", cylinder->top, indent2);
  Vrml_Node_End_Fprint(fp, "Cylinder", indent);
}

void Vrml_Node_Begin_Fprint(FILE *fp, const char *node_name, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "%s {\n", node_name);
}

void Vrml_Node_End_Fprint(FILE *fp, const char *node_name, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "}");
  if (node_name != NULL) {
    fprintf(fp, " # end %s", node_name);
  }
  fprintf(fp, "\n");
}

void Vrml_Children_Begin_Fprint(FILE *fp, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "children [\n");
}

void Vrml_Children_End_Fprint(FILE *fp, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "] # end children\n");
}

void Vrml_Geometry_Fprint(FILE *fp, const Vrml_Geometry *geom, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "geometry\n");
  switch (geom->id) {
  case VRML_SPHERE:
    Vrml_Sphere_Fprint(fp, (Vrml_Sphere *) geom->geometry, indent);
    break;
  case VRML_CONE:
    Vrml_Cone_Fprint(fp, (Vrml_Cone *) geom->geometry, indent);
    break;  
  case VRML_BOX:
    Vrml_Box_Fprint(fp, (Vrml_Box *) geom->geometry, indent);
    break;  
  case VRML_CYLINDER:
    Vrml_Cylinder_Fprint(fp, (Vrml_Cylinder *) geom->geometry, indent);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }
}

void Vrml_Appearance_Fprint(FILE *fp, const Vrml_Appearance *app, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent);
  fprint_space(fp, indent);
  fprintf(fp, "material");
  Vrml_Material_Fprint(fp, &(app->material), indent + VRML_INDENT);
  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent);
}
	
void Vrml_Shape_Fprint(FILE *fp, const Vrml_Shape *shape, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Appearance_Fprint(fp, &(shape->appearance), indent + VRML_INDENT);
  Vrml_Geometry_Fprint(fp, &(shape->geometry), indent + VRML_INDENT);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
}
		    
#if 0
void Vrml_Sphere_Fprint( FILE *fp, double radius, int indent)
{
  fprint_space(fp, indent);
  fprintf(fp, "geometry Sphere {\n");
  fprint_space(fp, indent);
  fprintf(fp, "  radius %g\n", radius);
  fprint_space(fp, indent);
  fprintf(fp, "}\n");
}
#endif

void Vrml_Polygon_Fprint(FILE *fp, const double *coord, int n, int indent)
{
  if (n > 0) {
    fprint_space(fp, indent);
    fprintf(fp, "geometry IndexedFaceSet {\n");
    fprint_space(fp, indent);
    fprintf(fp, "  solid FALSE\n");
    fprint_space(fp, indent);
    fprintf(fp, "  coord Coordinate {\n");
    fprint_space(fp, indent);
    fprintf(fp, "    point [");
    int i;
    for (i = 0; i < n; i++) {
      fprintf(fp, "%g %g %g", coord[0], coord[1], coord[2]);
      if (i != n - 1) {
	fprintf(fp, ", ");
      }
      coord += 3;
    }
    fprintf(fp, "]\n");
    fprint_space(fp, indent);
    fprintf(fp, "  }\n");
    fprint_space(fp, indent);
    fprintf(fp, "  coordIndex [");
    for (i = 0; i < n; i++) {
      fprintf(fp, "%d, ", i);
    }
    fprintf(fp, "-1]\n");
    fprint_space(fp, indent);
    fprintf(fp, "}\n");  
  }
}

static void circle_points(double radius, double z, int nsample, 
			  double *coord)
{
  int i;
  double w = 0.0;
  double step = TZ_2PI / nsample;
  for (i = 0; i < nsample; i++) {
    coord[0] = cos(w) * radius;
    coord[1] = sin(w) * radius;
    coord[2] = z;
    coord += 3;
    w += step;
  }
}

void Vrml_Disk_Fprint(FILE *fp, double radius, int indent)
{
  int nsample = 20;
  double *coord = (double *) Guarded_Malloc(sizeof(double) * nsample * 3,
					    "Vrml_Disk_Fprint");
  circle_points(radius, 0.0, nsample, coord);
  Vrml_Polygon_Fprint(fp, coord, nsample, indent);

  free(coord);
}

			  
#define VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM			\
  Vrml_Node_Begin_Fprint(fp, "Transform", indent);		\
  fprint_space(fp, indent);					\
  fprintf(fp, "translation %g %g %g\n",				\
	  translation[0], translation[1], translation[2]);	\
  fprint_space(fp, indent);					\
  fprintf(fp, "children\n");

void Vrml_Platform_Fprint(FILE *fp, const double *bottom_coord, 
			  const double * top_coord, int nsample,
			  const double *translation, int indent)
{
  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Node_Begin_Fprint(fp, "material Material", indent + 4);
  fprint_space(fp, indent + 6);
  double color[3] = {0.6314, 0.6472, 0.8667};
  fprintf(fp, "diffuseColor %g %g %g\n", color[0], color[1], color[2]);
  fprintf(fp, "emissiveColor %g %g %g\n", color[0], color[1], color[2]);
  Vrml_Node_End_Fprint(fp, "material Material", indent + 4);
  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Polygon_Fprint(fp, bottom_coord, nsample, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }

  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Node_Begin_Fprint(fp, "material Material", indent + 4);
  fprint_space(fp, indent + 6);
  fprintf(fp, "diffuseColor %g %g %g\n", color[0], color[1], color[2]);
  Vrml_Node_End_Fprint(fp, "material Material", indent + 4);
  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Polygon_Fprint(fp, top_coord, nsample, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }

  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Node_Begin_Fprint(fp, "material Material", indent + 4);
  fprint_space(fp, indent + 6);
  fprintf(fp, "diffuseColor %g %g %g\n", color[0], color[1], color[2]);
  Vrml_Node_End_Fprint(fp, "material Material", indent + 4);
  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Node_Begin_Fprint(fp, "geometry IndexedFaceSet", indent + 2);
  fprint_space(fp, indent + 4);
  fprintf(fp, "solid FALSE\n");
  Vrml_Node_Begin_Fprint(fp, "coord Coordinate", indent + 4);
  fprint_space(fp, indent + 6);
  fprintf(fp, "point [");
  int i;
  for (i = 0; i < nsample; i++) {
    fprintf(fp, "%g %g %g, ", 
	    bottom_coord[0], bottom_coord[1], bottom_coord[2]);
    bottom_coord += 3;
    fprintf(fp, "%g %g %g", 
	    top_coord[0], top_coord[1], top_coord[2]);
    top_coord += 3;
    if (i != nsample - 1) {
      fprintf(fp, ", ");
    }
  }
  bottom_coord -= nsample * 3;
  top_coord -= nsample * 3;

  fprintf(fp, "]\n");
  Vrml_Node_End_Fprint(fp, "coord Coordinate", indent + 4);
  
  fprint_space(fp, indent + 4);
  fprintf(fp, "coordIndex [");

  int ntriangle = nsample * 2;
  for (i = 0; i < ntriangle; i++) {
    if (i % 2 == 1) {
      if (i < ntriangle - 2) {
	fprintf(fp, "%d, %d, %d, -1, ", i, i + 1, i + 2);
      } else if (i < ntriangle - 1) {
	fprintf(fp, "%d, %d, %d, -1 ", 0, i + 1, i);
      } else {
	fprintf(fp, "%d, %d, %d, -1", i, 0, 1);
      }
    } else {
      if (i < ntriangle - 2) {
	fprintf(fp, "%d, %d, %d, -1, ", i + 2, i + 1, i);
      } else if (i < ntriangle - 1) {
	fprintf(fp, "%d, %d, %d, -1 ", i + 1, i, 0);
      } else {
	fprintf(fp, "%d, %d, %d, -1", 1, 0, i);
      }
    }
  }
  fprintf(fp, "]\n");

  Vrml_Node_End_Fprint(fp, "geometry IndexedFaceSet", indent + 2);    
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }  
}

void Vrml_Platform_Fprint_M(FILE *fp, const double *bottom_coord, 
			    const double * top_coord, int nsample,
			    const double *translation, 
			    const Vrml_Material *material,
			    int indent)
{
  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  
  if (material != NULL) {
    Vrml_Material_Fprint(fp, material, indent + 2);
  }

  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Polygon_Fprint(fp, bottom_coord, nsample, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }

  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  
  if (material != NULL) {
    Vrml_Material_Fprint(fp, material, indent + 4);
  }

  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Polygon_Fprint(fp, top_coord, nsample, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }

  if (translation != NULL) {
    VRML_NEUROSEG_SHAPE_FPRINT_TRANSFORM
  }
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  
  if (material != NULL) {
    Vrml_Material_Fprint(fp, material, indent + 4);
  }

  Vrml_Node_End_Fprint(fp, "appearance Appearance", indent + 2);
  Vrml_Node_Begin_Fprint(fp, "geometry IndexedFaceSet", indent + 2);
  fprint_space(fp, indent + 4);
  fprintf(fp, "solid FALSE\n");
  Vrml_Node_Begin_Fprint(fp, "coord Coordinate", indent + 4);
  fprint_space(fp, indent + 6);
  fprintf(fp, "point [");
  int i;
  for (i = 0; i < nsample; i++) {
    fprintf(fp, "%g %g %g, ", 
	    bottom_coord[0], bottom_coord[1], bottom_coord[2]);
    bottom_coord += 3;
    fprintf(fp, "%g %g %g", 
	    top_coord[0], top_coord[1], top_coord[2]);
    top_coord += 3;
    if (i != nsample - 1) {
      fprintf(fp, ", ");
    }
  }
  bottom_coord -= nsample * 3;
  top_coord -= nsample * 3;

  fprintf(fp, "]\n");
  Vrml_Node_End_Fprint(fp, "coord Coordinate", indent + 4);
  
  fprint_space(fp, indent + 4);
  fprintf(fp, "coordIndex [");

  int ntriangle = nsample * 2;
  for (i = 0; i < ntriangle; i++) {
    if (i % 2 == 1) {
      if (i < ntriangle - 2) {
	fprintf(fp, "%d, %d, %d, -1, ", i, i + 1, i + 2);
      } else if (i < ntriangle - 1) {
	fprintf(fp, "%d, %d, %d, -1 ", 0, i + 1, i);
      } else {
	fprintf(fp, "%d, %d, %d, -1", i, 0, 1);
      }
    } else {
      if (i < ntriangle - 2) {
	fprintf(fp, "%d, %d, %d, -1, ", i + 2, i + 1, i);
      } else if (i < ntriangle - 1) {
	fprintf(fp, "%d, %d, %d, -1 ", i + 1, i, 0);
      } else {
	fprintf(fp, "%d, %d, %d, -1", 1, 0, i);
      }
    }
  }
  fprintf(fp, "]\n");

  Vrml_Node_End_Fprint(fp, "geometry IndexedFaceSet", indent + 2);    
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  if (translation != NULL) {
    Vrml_Node_End_Fprint(fp, "Transform", indent);
  }  
}

void Vrml_Neuroseg_Fprint(FILE *fp, const Neuroseg *seg, 
			  const double *translation, int indent)
{
  int nsample = 20;

  double *bottom_coord = (double *) 
    Guarded_Malloc(sizeof(double) * nsample * 3, "Vrml_Neuroseg_Fprint");
  circle_points(seg->r1, 0.0, nsample, bottom_coord);
  Rotate_XZ(bottom_coord, bottom_coord, nsample, seg->theta, seg->psi, 0);
  
  double *top_coord = (double *) 
    Guarded_Malloc(sizeof(double) * nsample * 3, "Vrml_Neuroseg_Fprint");
  circle_points(NEUROSEG_R2(seg), seg->h, nsample, top_coord);
  Rotate_XZ(top_coord, top_coord, nsample, seg->theta, seg->psi, 0);
  
  Vrml_Platform_Fprint(fp, bottom_coord, top_coord, nsample, translation,
		       indent);

  free(top_coord);
  free(bottom_coord);
}

void Vrml_Local_Neuroseg_Fprint(FILE *fp, const Local_Neuroseg *locseg, 
				int indent)
{
  double bottom[3];
  Local_Neuroseg_Bottom(locseg, bottom);
  Vrml_Neuroseg_Fprint(fp, &(locseg->seg), bottom, indent);
}

#if 0
static void local_neuroseg_to_geo3d_circle(const Local_Neuroseg *locseg,
					   Geo3d_Circle *sc,
					   Neuropos_Reference_e ref) 
{
  switch (ref) {
  case NEUROSEG_BOTTOM:
    sc->radius = locseg->seg.r1;
    Local_Neuroseg_Bottom(locseg, sc->center);
    break;
  case NEUROSEG_TOP:
    sc->radius = NEUROSEG_R2(&(locseg->seg));
    Local_Neuroseg_Top(locseg, sc->center);
    break;
  case NEUROSEG_CENTER:
    sc->radius = NEUROSEG_RC(&(locseg->seg));//(locseg->seg.r1 + locseg->seg.r2) / 2.0;
    Local_Neuroseg_Center(locseg, sc->center);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  Set_Xz_Orientation(sc->orientation, locseg->seg.theta, locseg->seg.psi); 
}

static void vrml_neurochain_get_locseg(const Neurochain *chain,
				       Local_Neuroseg *locseg)
{
  Local_Neuroseg_Copy(locseg, &(chain->locseg));
  if (chain->hist != NULL) {
    if (Bitmask_Get_Bit(chain->hist->mask, TRACE_HIST_DIRECTION_MASK)
	== TRUE) {
      if (chain->hist->direction == BACKWARD) {
	Flip_Local_Neuroseg(locseg);
      }
    }
  }
}

static Geo3d_Circle* neurochain_to_geo3d_circle_array(const Neurochain *chain,
						      Geo3d_Circle *circle)
{
  int length = Neurochain_Length(chain, FORWARD);
  if (circle == NULL) {
    circle = Make_Geo3d_Circle_Array(length + 2);
  }

  Geo3d_Circle *circle_head = circle;
  Local_Neuroseg locseg;
  
  vrml_neurochain_get_locseg(chain, &locseg);
  local_neuroseg_to_geo3d_circle(&locseg, circle, NEUROSEG_BOTTOM);
  circle++;
  
  while (chain != NULL) {
    vrml_neurochain_get_locseg(chain, &locseg);
    local_neuroseg_to_geo3d_circle(&locseg, circle, NEUROSEG_CENTER);
    circle++;
    chain = chain->next;
  }
  
  local_neuroseg_to_geo3d_circle(&locseg, circle, NEUROSEG_TOP);
  
  return circle_head;
}

void Vrml_Neurochain_Get_Locseg(const Neurochain *chain,
				Local_Neuroseg *locseg)
{
  vrml_neurochain_get_locseg(chain, locseg);
}

void Vrml_Neurochain_Fprint(FILE *fp, const Neurochain *chain, int indent)
{
  if ((chain == NULL) || (fp == NULL)) {
    return;
  }

  Local_Neuroseg *locseg = New_Local_Neuroseg();

  Geo3d_Circle bottom;
  Geo3d_Circle top;

  if (chain != NULL) {
    vrml_neurochain_get_locseg(chain, locseg);
    local_neuroseg_to_geo3d_circle(locseg, &bottom, NEUROSEG_BOTTOM);
    local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_CENTER);
    Vrml_Circle_Platform_Fprint(fp, &bottom, &top, indent);
  }

  while(chain != NULL) {
    //Vrml_Local_Neuroseg_Fprint(fp, &(chain->locseg), indent);
    vrml_neurochain_get_locseg(chain, locseg);
    local_neuroseg_to_geo3d_circle(locseg, &bottom, NEUROSEG_CENTER);
    if (chain->next != NULL) {
      if (chain->locseg.seg.h < 10.0) {
	chain = chain->next;
      }
    }
    if (chain->next != NULL) {
      vrml_neurochain_get_locseg(chain->next, locseg);
      local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_CENTER);
      Vrml_Circle_Platform_Fprint(fp, &bottom, &top, indent);
    } else {
      local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_TOP);
      Vrml_Circle_Platform_Fprint(fp, &bottom, &top, indent);
    }
    chain = chain->next;
  }

  Delete_Local_Neuroseg(locseg);
}

void Vrml_Neurochain_Fprint_M(FILE *fp, const Neurochain *chain, 
			      const Vrml_Material *material, int indent)
{
  if ((chain == NULL) || (fp == NULL)) {
    return;
  }

  Local_Neuroseg *locseg = New_Local_Neuroseg();

  Geo3d_Circle bottom;
  Geo3d_Circle top;

  if (chain != NULL) {
    vrml_neurochain_get_locseg(chain, locseg);
    local_neuroseg_to_geo3d_circle(locseg, &bottom, NEUROSEG_BOTTOM);
    local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_CENTER);
    Vrml_Circle_Platform_Fprint_M(fp, &bottom, &top, material, indent);
  }

  while(chain != NULL) {
    //Vrml_Local_Neuroseg_Fprint(fp, &(chain->locseg), indent);
    vrml_neurochain_get_locseg(chain, locseg);
    local_neuroseg_to_geo3d_circle(locseg, &bottom, NEUROSEG_CENTER);
    if (chain->next != NULL) {
      if (chain->locseg.seg.h < 10.0) {
	chain = chain->next;
      }
    }
    if (chain->next != NULL) {
      vrml_neurochain_get_locseg(chain->next, locseg);
      local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_CENTER);
      Vrml_Circle_Platform_Fprint_M(fp, &bottom, &top, material, indent);
    } else {
      local_neuroseg_to_geo3d_circle(locseg, &top, NEUROSEG_TOP);
      Vrml_Circle_Platform_Fprint_M(fp, &bottom, &top, material, indent);
    }
    chain = chain->next;
  }

  Delete_Local_Neuroseg(locseg);  
}

void Vrml_Neurochain_Fprint_C(FILE *fp, const Neurochain *chain, 
			      const Vrml_Material *material, int indent)
{
  Geo3d_Circle *circle =  
    neurochain_to_geo3d_circle_array(chain, NULL);
  int ncircle = Neurochain_Length(chain, FORWARD) + 2;
  int nsample = 20;
  coordinate_3d_t *pts = Geo3d_Circle_Array_Points(circle, ncircle, 
						   nsample, NULL);
  
  Vrml_Node_Begin_Fprint(fp, "Shape", indent);
  Vrml_Node_Begin_Fprint(fp, "appearance Appearance", indent + 2);
  if (material == NULL) {
    Vrml_Material *material2 = New_Vrml_Material();
    Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material2->default_mask));
    Vrml_Material_Fprint(fp, material2, indent + 4);
    Delete_Vrml_Material(material2);
  } else {
    Vrml_Material_Fprint(fp, material, indent + 4);
  }
  Vrml_Node_End_Fprint(fp, "Appearance", indent + 2);
  Vrml_Pm_Rectangle_Fprint(fp, (const coordinate_3d_t *) pts, nsample, 
			   ncircle, indent + 2);
  Vrml_Node_End_Fprint(fp, "Shape", indent);
  
  free(circle);
  free(pts);
}
#endif

#if 0
static double polygon_match_score(const double *coord1, const double *coord2,
				  int n, int offset)
{
  const double *start_coord2 = coord2;
  int i;
  double score = 0.0;
  if (offset >= 0) {
    coord2 += offset * 3;
    for (i = 0; i < n; i++) {
      if (i + offset == n) {
	coord2 = start_coord2;
      }
      score += sqrt((coord1[0] - coord2[0]) * (coord1[0] - coord2[0]) +
		    (coord1[1] - coord2[1]) * (coord1[1] - coord2[1]) +
		    (coord1[2] - coord2[2]) * (coord1[2] - coord2[2]));
      coord1 += 3;
      coord2 += 3;
    }
  } else {
    coord2 += -offset * 3;
    for (i = 0; i < n; i++) {
      if (i > -offset) {
	coord2 = start_coord2 + n - 1;
      }
      score += sqrt((coord1[0] - coord2[0]) * (coord1[0] - coord2[0]) +
		    (coord1[1] - coord2[1]) * (coord1[1] - coord2[1]) +
		    (coord1[2] - coord2[2]) * (coord1[2] - coord2[2]));
      coord1 += 3;
      coord2 -= 3;
    }
  }
  
  return score;
}

static void register_polygon(const double *coord1, const double *coord2,
			     int n, int *offset)
{
  int offset_trial;
  double max_score = 0.0;
  for (offset_trial = -n + 1; offset_trial < n; offset_trial++) {
    if (max_score > polygon_match_score(coord1, coord2, n, offset_trial)) {
      
    }
  }
}
#endif


#if 0
static void space_circle_points(const Space_Circle *sc, int nsample,
				double *coord)
{
  circle_points(sc->r, 0.0, nsample, coord);
  double theta = Radian_Normdiff(sc->theta, 0.0);
  if (theta > TZ_PI_2) {
    theta -= TZ_PI;
  }
  double psi = Radian_Normdiff(sc->psi, 0.0);
  if (psi > TZ_PI_2) {
    psi -= TZ_PI;
  }

  Rotate_XZ(coord, coord, nsample, theta, psi, 0);
  Geo3d_Point_Array_Translate((coordinate_3d_t *) coord, nsample,
			      sc->c[0], sc->c[1], sc->c[2]);
}
#endif

#if 0
double static norm_radian(double a)
{
  Normalize_Radian(a);
  if (a > TZ_PI) {
    a -= TZ_PI;
  }
  if (a > TZ_PI_2) {
    a -= TZ_PI;
  }
  return a;
}

static void norm_theta_psi(double theta, double psi, 
			   double *theta2, double *psi2)
{
  double x, y, z;
  Geo3d_Orientation_Normal(theta, psi, &x, &y, &z);
  Geo3d_Normal_Orientation(x, y, z, theta2, psi2);
}
#endif

void Vrml_Circle_Platform_Fprint(FILE *fp, const Geo3d_Circle *bottom,
				 const Geo3d_Circle *top, int indent)
{
  int nsample = 20;

  coordinate_3d_t *bottom_coord = (coordinate_3d_t *) 
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, "Vrml_Neuroseg_Fprint");
  coordinate_3d_t *top_coord = (coordinate_3d_t *) 
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, "Vrml_Neuroseg_Fprint");

  coordinate_3d_t plane_normal;
  Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
  Coordinate_3d_Unitize(plane_normal);

  xz_orientation_t plane_ort;
  /* 0.001 is picked to test if two vectors are parallel */
  if (Coordinate_3d_Norm(plane_normal) < 0.001) {
    /* pick a plane that is not parallel to the circles */
    plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
    plane_ort[1] = bottom->orientation[1];
  } else {
    Geo3d_Normal_Orientation(plane_normal[0], plane_normal[1], 
			     plane_normal[2], plane_ort, plane_ort + 1);
    //printf("plane: ");
    //Print_Coordinate_3d(plane_normal);
  }

  //printf("%g\n", Xz_Orientation_Dot(bottom->orientation, top->orientation));

  coordinate_3d_t start1;
  Geo3d_Circle_Plane_Point(bottom, plane_ort, 1, start1);
  Geo3d_Circle_Points(bottom, nsample, start1, bottom_coord);
  //Print_Coordinate_3d(start1);
  
  coordinate_3d_t start2;
  Geo3d_Circle_Plane_Point(top, plane_ort, 1, start2);
  Geo3d_Circle_Points(top, nsample, start2, top_coord);

  //printf("%g\n", Coordinate_3d_Dot(start1, start2));
 
  Vrml_Platform_Fprint(fp, Coordinate_3d_Double_Array(bottom_coord), 
		       Coordinate_3d_Double_Array(top_coord), nsample, 
		       NULL, indent);

  free(bottom_coord);
  free(top_coord);
}

void Vrml_Circle_Platform_Fprint_M(FILE *fp, const Geo3d_Circle *bottom,
				   const Geo3d_Circle *top, 
				   const Vrml_Material *material, int indent)
{
  int nsample = 30;
  double radius = dmax2(bottom->radius, top->radius);

  if (radius < 3.0) {
    nsample *= radius / 3.0;
  }

  if (nsample < 6) {
    nsample = 6;
  }

  coordinate_3d_t *bottom_coord = (coordinate_3d_t *) 
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, "Vrml_Neuroseg_Fprint");
  coordinate_3d_t *top_coord = (coordinate_3d_t *) 
    Guarded_Malloc(sizeof(coordinate_3d_t) * nsample, "Vrml_Neuroseg_Fprint");

  coordinate_3d_t plane_normal;
  Xz_Orientation_Cross(bottom->orientation, top->orientation, plane_normal);
  Coordinate_3d_Unitize(plane_normal);

  xz_orientation_t plane_ort;
  /* 0.001 is picked to test if two vectors are parallel */
  if (Coordinate_3d_Norm(plane_normal) < 0.001) {
    /* pick a plane that is not parallel to the circles */
    plane_ort[0] = bottom->orientation[0] + TZ_PI_2;
    plane_ort[1] = bottom->orientation[1];
  } else {
    Geo3d_Normal_Orientation(plane_normal[0], plane_normal[1], 
			     plane_normal[2], plane_ort, plane_ort + 1);
    //printf("plane: ");
    //Print_Coordinate_3d(plane_normal);
  }

  //printf("%g\n", Xz_Orientation_Dot(bottom->orientation, top->orientation));

  coordinate_3d_t start1;
  Geo3d_Circle_Plane_Point(bottom, plane_ort, 1, start1);
  Geo3d_Circle_Points(bottom, nsample, start1, bottom_coord);
  //Print_Coordinate_3d(start1);
  
  coordinate_3d_t start2;
  Geo3d_Circle_Plane_Point(top, plane_ort, 1, start2);
  Geo3d_Circle_Points(top, nsample, start2, top_coord);

  //printf("%g\n", Coordinate_3d_Dot(start1, start2));
 
  Vrml_Platform_Fprint_M(fp, Coordinate_3d_Double_Array(bottom_coord), 
			 Coordinate_3d_Double_Array(top_coord), nsample, 
			 NULL, material, indent);

  free(bottom_coord);
  free(top_coord);
}

void Vrml_Material_Fprint(FILE *fp, const Vrml_Material *material, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "material Material", indent);
  
  if (Bitmask_Get_Bit(material->default_mask, AMBIENT_INTENSITY) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "ambientIntensity %g\n", material->ambient_intensity);
  }
  if (Bitmask_Get_Bit(material->default_mask, DIFFUSE_COLOR) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "diffuseColor %g %g %g\n", material->diffuse_color[0], 
	    material->diffuse_color[1], material->diffuse_color[2]);
  }
  if (Bitmask_Get_Bit(material->default_mask, EMISSIVE_COLOR) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "emissiveColor %g %g %g\n", material->emissive_color[0], 
	    material->emissive_color[1], material->emissive_color[2]);
  }
  if (Bitmask_Get_Bit(material->default_mask, SHININESS) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "shininess %g\n", material->shininess);
  }
  if (Bitmask_Get_Bit(material->default_mask, SPECULAR_COLOR) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "specularColor %g %g %g\n", material->specular_color[0], 
	    material->specular_color[1], material->specular_color[2]);
  }
  if (Bitmask_Get_Bit(material->default_mask, TRANSPARENCY) == FALSE) {
    fprint_space(fp, indent + 2);
    fprintf(fp, "transparency %g\n", material->transparency);
  }
  Vrml_Node_End_Fprint(fp, "material Material", indent);
}

void Vrml_Pm_Triangle_Fprint(FILE *fp, const coordinate_3d_t *pts,
			     int width, int height, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "geometry IndexedFaceSet", indent);
  fprint_space(fp, indent + 2);
  fprintf(fp, "solid FALSE\n");
  Vrml_Node_Begin_Fprint(fp, "coord Coordinate", indent + 2);
  int i, j;
  int offset = 0;
  fprint_space(fp, indent + 4);
  fprintf(fp, "point [");
  for (j = 0; j < height; j ++) {
    for (i = 0; i < width; i++) {
      fprintf(fp, "%g %g %g,", pts[offset][0], pts[offset][1], pts[offset][2]);
      offset++;
    }
  }
  fprintf(fp, "]\n");
  Vrml_Node_End_Fprint(fp, "coord Coordinate", indent + 2);

  fprint_space(fp, indent + 4);
  fprintf(fp, "coordIndex [");
  offset = 0;
  for (j = 0; j < height - 1; j ++) {
    for (i = 0; i < width - 1; i++) {
      fprintf(fp, "%d,%d,%d,-1,", offset, offset + width, offset + 1);
      fprintf(fp, "%d,%d,%d,-1,", offset + width, offset + 1, offset + width + 1);
      offset++;
    }
    fprintf(fp,"%d,%d,%d,-1,", offset, offset + width, offset - width + 1);
    fprintf(fp,"%d,%d,%d,-1,", offset + width, offset - width + 1, offset + 1);
    offset++;
  }
  fprintf(fp, "]\n");

  Vrml_Node_End_Fprint(fp, "geometry IndexedFaceSet", indent);
}

void Vrml_Pm_Rectangle_Fprint(FILE *fp, const coordinate_3d_t *pts,
			     int width, int height, int indent)
{
  Vrml_Node_Begin_Fprint(fp, "geometry IndexedFaceSet", indent);
  fprint_space(fp, indent + 2);
  fprintf(fp, "solid FALSE\n");
  Vrml_Node_Begin_Fprint(fp, "coord Coordinate", indent + 2);
  int i, j;
  int offset = 0;
  fprint_space(fp, indent + 4);
  fprintf(fp, "point [");
  for (j = 0; j < height; j ++) {
    for (i = 0; i < width; i++) {
      fprintf(fp, "%g %g %g,", pts[offset][0], pts[offset][1], pts[offset][2]);
      offset++;
    }
  }
  fprintf(fp, "]\n");
  Vrml_Node_End_Fprint(fp, "coord Coordinate", indent + 2);

  fprint_space(fp, indent + 4);
  fprintf(fp, "coordIndex [");
  for (i = 0; i < width; i++) {
    fprintf(fp, "%d,", i);
  }
  fprintf(fp, "0,-1,");
  offset = 0;
  for (j = 0; j < height - 1; j ++) {
    for (i = 0; i < width - 1; i++) {
      fprintf(fp, "%d,%d,%d,%d,-1,", offset, offset + width, offset + width + 1,
	      offset + 1);
      offset++;
    }
    fprintf(fp,"%d,%d,%d,%d,-1,", offset, offset + width, offset + 1,
	    offset - width + 1);
    offset++;
  }
  for (i = offset + width - 1; i >= offset; i--) {
    fprintf(fp, "%d,", i);
  }
  fprintf(fp, "%d,-1", offset + width - 1);
  fprintf(fp, "]\n");

  Vrml_Node_End_Fprint(fp, "geometry IndexedFaceSet", indent);
}
