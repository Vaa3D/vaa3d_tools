/**@file zstackexportable.h
 * @brief Exportable interface
 * @author Ting Zhao
 */

#ifndef _ZSWCEXPORTABLE_H_
#define _ZSWCEXPORTABLE_H_

#include <stdio.h>

#include "zinterface.h"

// abstract class for those can be drawn in an image
class ZSwcExportable : public virtual ZInterface {
public:
  ZSwcExportable() {}
  virtual ~ZSwcExportable() {}

  virtual int swcFprint(FILE *fp, int start_id = 0, int parent_id = -1,
			double z_scale = 1.0) = 0;
  virtual void swcExport(const char *filePath) = 0;
};

#endif
