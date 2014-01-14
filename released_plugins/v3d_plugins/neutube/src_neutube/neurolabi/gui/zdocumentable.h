/**@file zdocumentable.h
 * @brief Parent interface of documentable classes
 * @author Ting Zhao
 */

#ifndef _ZDOCUMENTABLE_H_
#define _ZDOCUMENTABLE_H_

#include "zinterface.h"

// abstract class for those can be documentable
class ZDocumentable : public virtual ZInterface {
public:
  virtual void save(const char *filePath) = 0;
  virtual void load(const char *filePath) = 0;
  virtual ~ZDocumentable() {}
};

#endif
