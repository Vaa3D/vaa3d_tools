#ifndef _ZVRMLEXPORTABLE_H_
#define _ZVRMLEXPORTABLE_H_

#include <stdio.h>

#include "zinterface.h"
#include "tz_vrml_defs.h"

// abstract class for those can be drawn in an image
class ZVrmlExportable : public virtual ZInterface {
public:
  virtual ~ZVrmlExportable() {};
  virtual void vrmlFprint(FILE *fp, const Vrml_Material *material = NULL, 
			  int indent = 0, double z_scale = 1.0) = 0;
};

#endif
