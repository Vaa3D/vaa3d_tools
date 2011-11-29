
#ifndef V3D_PYTHON_CONSOLE_PLUGIN_H
#define V3D_PYTHON_CONSOLE_PLUGIN_H

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>
#include "v3d_interface.h"
class PythonConsoleWindow;

class PythonConsolePlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

public:
	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,
			QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name,
			const V3DPluginArgList & input, V3DPluginArgList & output,
			V3DPluginCallback2 & v3d,  QWidget * parent)
	{
		return true;
	}

    float getPluginVersion() const {return 0.60f;}

    static PythonConsoleWindow* pythonConsoleWindow;
};

#endif // V3D_PYTHON_CONSOLE_PLUGIN_H

