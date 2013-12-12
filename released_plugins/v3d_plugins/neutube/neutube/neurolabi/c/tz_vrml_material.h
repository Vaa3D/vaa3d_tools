/**@file tz_vrml_material.h
 * @brief vrml matrerial
 * @author Ting Zhao
 * @date 20-Mar-2008
 */

#ifndef _TZ_VRML_MATERIAL_H_
#define _TZ_VRML_MATERIAL_H_ 

#include "tz_cdefs.h"
#include "tz_vrml_defs.h"

__BEGIN_DECLS

/* see tz_vrml_io.h for printing Vrml_Material in a VRML file */

/*
 * Constructors and destructors.
 */
Vrml_Material* New_Vrml_Material();
void Delete_Vrml_Material(Vrml_Material *material);

/*
 * Print_Vrml_Material() prints a vrml material to standard output.
 */
void Print_Vrml_Material(const Vrml_Material *material);

__END_DECLS

#endif
