#ifndef __FastMarching_H__
#define __FastMarching_H__

#include <QtGui>
#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>

class FastMarchingPlugin : public QObject, public V3DPluginInterface2
{
     Q_OBJECT
     Q_INTERFACES(V3DPluginInterface2)

public:
	FastMarchingPlugin() {}
	QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);

	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent);
};

#endif
