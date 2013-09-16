//by Hanchuan Peng
//2013-09-15

#include "check_and_makedir.h"
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



