/**@file tz_vrml_defs.h
 * @brief definitions for vrml
 * @author Ting Zhao
 * @date 07-May-2008
 */

#ifndef _TZ_VRML_DEFS_H_
#define _TZ_VRML_DEFS_H_ 

#include "tz_cdefs.h"
#include "tz_unipointer_linked_list.h"
#include "tz_bitmask.h"

__BEGIN_DECLS

/*
 * VRML definitions according to 
 * www.ici.unifei.edu.br/ramos/download/SistemasMultimidia/vrml2tut.pdf
 * There is also a local copy ../doc/vrml2tut.pdf
 */

#define VRML_INDENT 2

typedef BOOL SFBool;
typedef double SFFloat;
typedef void* SFNode;
typedef Unipointer_List MFNode;
typedef double SFVec3f[3];
typedef double SFColor[3];
typedef double SFRotation[4];

typedef enum {
  VRML_SPHERE = 1, VRML_CONE, VRML_BOX, VRML_CYLINDER, VRML_INDEXED_FACESET
} Vrml_Geometry_e;

typedef enum {
  SFNODE_APPEARANCE = 1, SFNODE_GEOMETRY, SFNODE_MATERIAL, SFNODE_TEXTURE,
  SFNODE_TEXTURE_TRANSFORM, SFNODE_TRANSFORM
} SFNode_Id_e;

typedef struct _Vrml_SFNode {
  SFNode_Id_e id;
  SFNode node;
} Vrml_SFNode;

typedef struct _Vrml_Geometry {
  Vrml_Geometry_e id;
  void *geometry;
} Vrml_Geometry;

typedef struct _Vrml_Sphere {
  Bitmask_t default_mask;
  SFFloat radius;
} Vrml_Sphere;

typedef struct _Vrml_Cone {
  Bitmask_t default_mask;
  SFFloat bottom_radius;
  SFFloat height;
  SFBool side;
  SFBool bottom;
} Vrml_Cone;

typedef struct _Vrml_Box {
  SFVec3f size;
} Vrml_Box;

typedef struct _Vrml_Cylinder {
  SFBool bottom;
  SFFloat height;
  SFFloat radius;
  SFBool side;
  SFBool top;
} Vrml_Cylinder;

enum {
  AMBIENT_INTENSITY = 0, DIFFUSE_COLOR, EMISSIVE_COLOR, SHININESS,
  SPECULAR_COLOR, TRANSPARENCY
};
typedef struct _Vrml_Material {
  Bitmask_t default_mask;
  SFFloat ambient_intensity;
  SFColor diffuse_color;
  SFColor emissive_color;
  SFFloat shininess;
  SFColor specular_color;
  SFFloat transparency;
} Vrml_Material;

typedef void* Vrml_Texture;
typedef void* Vrml_Texture_Transform;

typedef struct _Vrml_Appearance {
  Vrml_Material material;
  Vrml_Texture texture;
  Vrml_Texture_Transform textureTransform;
} Vrml_Appearance;

typedef struct _Vrml_Shape {
  Vrml_Appearance appearance;
  Vrml_Geometry geometry;
} Vrml_Shape;

typedef struct _Vrml_Transform {
  SFVec3f center;
  MFNode children;
  SFRotation rotation;
  SFVec3f scale;
  SFRotation  scaleOrientation;
  SFVec3f translation;
  SFVec3f bbox_center;
  SFVec3f bbox_size;
} Vrml_Transform;

__END_DECLS

#endif
