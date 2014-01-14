/**@file tz_vrml_io.h
 * @brief vrml io routines
 * @author Ting Zhao
 * @date 17-Mar-2008
 */

#ifndef _TZ_VRML_IO_H_
#define _TZ_VRML_IO_H_ 

#include "tz_cdefs.h"
#include "tz_geo3d_circle.h"
#include "tz_neuroseg.h"
#include "tz_local_neuroseg.h"
#include "tz_vrml_defs.h"

__BEGIN_DECLS

/**@addtogroup vrml_io vrml I/O (tz_vrml_io.h) 
 * @{
 */

/**@addtogroup vrml_head_print_ vrml head print I/O */
/**@{ */

/**@brief print the head line of VRML
 *
 * Vrml_Head_Fprint() prints the head line of VRML to the file stream <fp>.
 * <version> specifies the version number of VRML (e.g. "V2.0") and charset 
 * specifies the character set of the VRML file (e.g. "utf8").
 */
void Vrml_Head_Fprint(FILE *fp, const char *version, const char *charset);

/*@}*/

/*
 * Vrml data structure IO. <indent> specifies indent of printing style if it
 * is available.
 */

/**@addtogroup vrml_e_print_ Vrml elementary data type io
 * @brief Print non-structured vrml data
 * @{ 
 */

/**@brief Print a SFFloat \a value into \a fp.
 */
void Vrml_SFFloat_Fprint(FILE *fp, SFFloat value);
void Vrml_SFColor_Fprint(FILE *fp, const SFColor color);
void Vrml_SFBool_Fprint(FILE *fp, SFBool value);
void Vrml_SFVec3f_Fprint(FILE *fp, const SFVec3f vec);

/*@}*/

/**@addtogroup Vrml_SFVec3d_Set_
 * @brief Set SFVec3f
 * @{
 */
void Vrml_SFVec3f_Set(SFVec3f vec, double x, double y, double z);

/*@}*/

/*
 * Print vrml data along with a field name.
 */
void Vrml_SFFloat_Field_Fprint(FILE *fp, const char *field, SFFloat value);
void Vrml_SFColor_Field_Fprint(FILE *fp, const char *field, 
			       const SFColor color);
void Vrml_SFBool_Field_Fprint(FILE *fp, const char *field, SFBool value);
void Vrml_SFVec3f_Field_Fprint(FILE *fp, const char *field, const SFVec3f vec);

/*
 * Print a field as a line.
 */
void Vrml_SFFloat_Field_Fprintln(FILE *fp, const char *field, SFFloat value,
				 int indent);
void Vrml_SFColor_Field_Fprintln(FILE *fp, const char *field, 
				 const SFColor color, int indent);
void Vrml_SFBool_Field_Fprintln(FILE *fp, const char *field, SFBool value, 
				int indent);
void Vrml_SFVec3f_Field_Fprintln(FILE *fp, const char *field, 
				 const SFVec3f vec, int indent);

/*
 * Vrml_Sphere_Fprint() prints a sphere to the file stream <fp>.
 * Vrml_Cone_Fprint() prints a cone to the stream <fp>.
 * Vrml_Box_Fprint() prints a box to the file stream <fp>.
 * Vrml_Cylinder_Fprint() prints a cylinder to the file stream <fp>.
 */
void Vrml_Sphere_Fprint(FILE *fp, const Vrml_Sphere *sphere, int indent);
void Vrml_Cone_Fprint(FILE *fp, const Vrml_Cone *cone, int indent);
void Vrml_Box_Fprint(FILE *fp, const Vrml_Box *box, int indent);
void Vrml_Cylinder_Fprint(FILE *fp, const Vrml_Cylinder *cylinder, int indent);
void Vrml_Shape_Fprint(FILE *fp, const Vrml_Shape *shape, int indent);

/*
 * Vrml_Node_Begin_Fprint() prints the beginning of a node of the name 
 * <node_name> and Vrml_Node_Begin_Fprint() prints the end of a node.
 */
void Vrml_Node_Begin_Fprint(FILE *fp, const char *node_name, int indent);
void Vrml_Node_End_Fprint(FILE *fp, const char *node_name, int indent);

void Vrml_Children_Begin_Fprint(FILE *fp, int indent);
void Vrml_Children_End_Fprint(FILE *fp, int indent);

void Vrml_Geometry_Fprint(FILE *fp, const Vrml_Geometry *geom, int indent);
void Vrml_Appearance_Fprint(FILE *fp, const Vrml_Appearance *app, int indent);

void Vrml_SFNode_Fprint(FILE *fp, const Vrml_SFNode *sfnode, int indent);

/*
 * Miscellaneous routines.
 */
void Vrml_Polygon_Fprint(FILE *fp, const double *coord, int n, int indent);
void Vrml_Disk_Fprint(FILE *fp, double radius, int indent);
void Vrml_Neuroseg_Fprint(FILE *fp, const Neuroseg *seg, 
			  const double *translation, int indent);
void Vrml_Local_Neuroseg_Fprint(FILE *fp, const Local_Neuroseg *seg, 
				int indent);

void Vrml_Circle_Platform_Fprint(FILE *fp, const Geo3d_Circle *bottom,
				 const Geo3d_Circle *top, int indent);

void Vrml_Circle_Platform_Fprint_M(FILE *fp, const Geo3d_Circle *bottom,
				   const Geo3d_Circle *top, 
				   const Vrml_Material *material, int indent);

void Vrml_Round_Tube_Fprint(FILE *fp, const Geo3d_Circle *bottom,
			    const Geo3d_Circle *top, int indent);

void Vrml_Material_Fprint(FILE *fp, const Vrml_Material *material, int indent);

void Vrml_Platform_Fprint(FILE *fp, const double *bottom_coord, 
			  const double * top_coord, int nsample,
			  const double *translation, int indent);

void Vrml_Platform_Fprint_M(FILE *fp, const double *bottom_coord, 
			    const double * top_coord, int nsample,
			    const double *translation, 
			    const Vrml_Material *material,
			    int indent);

void Vrml_Pm_Triangle_Fprint(FILE *fp, const coordinate_3d_t *pts,
			     int width, int height, int indent);

void Vrml_Pm_Rectangle_Fprint(FILE *fp, const coordinate_3d_t *pts,
			      int width, int height, int indent);

/**@}*/
__END_DECLS

#endif
