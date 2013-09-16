//by Hanchuan Peng
//2013-09-15

#include "check_and_makedir.h"

#ifndef __STAND_ALONE__

#include <QtGui>

bool check_and_make_dir(const char *dirname)
{
  if (!dirname)
    return false;
  QString d = dirname;
  d = d.trimmed();

  if (QDir(d).exists())
    return true;
  else
  {
    return QDir().mkdir(d);
  }
}

#else // stand alone version

#include "../ImageManager/VM_config.h"

bool check_and_make_dir(const char *dirname)
{
  return (make_dir(dirname) == 0);
}

#endif




