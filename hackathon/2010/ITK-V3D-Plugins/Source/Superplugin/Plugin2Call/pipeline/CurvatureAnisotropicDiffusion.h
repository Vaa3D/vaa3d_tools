#ifndef __CurvatureAnisotropicDiffusion_H__
#define __CurvatureAnisotropicDiffusion_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <v3d_interface.h>


class CurvatureAnisotropicDiffusionPlugin : public QObject, public V3DPluginInterface2
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2)

public:
	CurvatureAnisotropicDiffusionPlugin() {}
	QStringList menulist() const;
	QStringList funclist() const;

	void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);

	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent);

};

#endif
