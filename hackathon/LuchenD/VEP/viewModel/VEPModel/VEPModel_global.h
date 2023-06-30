#ifndef VEPMODEL_GLOBAL_H
#define VEPMODEL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VEPMODEL_LIBRARY)
#  define VEPMODEL_EXPORT Q_DECL_EXPORT
#else
#  define VEPMODEL_EXPORT Q_DECL_IMPORT
#endif

#endif // VEPMODEL_GLOBAL_H
