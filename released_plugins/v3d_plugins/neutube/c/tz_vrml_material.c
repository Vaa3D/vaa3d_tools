/* tz_vrml_material.c
 *
 * 20-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <utilities.h>
#include "tz_vrml_material.h"

Vrml_Material* New_Vrml_Material()
{
  Vrml_Material *material = 
    (Vrml_Material *) Guarded_Malloc(sizeof(Vrml_Material), 
				     "New_Vrml_Material");

  material->default_mask = ALL_ON_BIT_MASK;
  material->ambient_intensity = 0.2;
  material->diffuse_color[0] = 0.8;
  material->diffuse_color[1] = 0.8;
  material->diffuse_color[2] = 0.8;
  material->emissive_color[0] = 0.0;
  material->emissive_color[1] = 0.0;
  material->emissive_color[2] = 0.0;
  material->shininess = 0.2;
  material->specular_color[0] = 0.0;
  material->specular_color[1] = 0.0;
  material->specular_color[2] = 0.0;
  material->transparency = 0.0;

  return material;
}


void Delete_Vrml_Material(Vrml_Material *material)
{
  free(material);
}

void Print_Vrml_Material(const Vrml_Material *material)
{
  printf("VRML material: \n");
  printf("Ambient intensity: %g\n", material->ambient_intensity);
  printf("Diffuse color: %g, %g, %g\n", material->diffuse_color[0],
	 material->diffuse_color[1], material->diffuse_color[2]);
  printf("Emissive color: %g, %g, %g\n", material->emissive_color[0],
	 material->emissive_color[1], material->emissive_color[2]);
  printf("Shininess: %g\n", material->shininess);
  printf("Specular color: %g, %g, %g\n", material->specular_color[0],
	 material->specular_color[1], material->specular_color[2]);
  printf("Transparency: %g\n", material->transparency);
}
