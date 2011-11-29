/* ex_matrix.h
 * an example program to test dofunc function in the plugin interface
* 2010-08-23: by Zongcai Ruan
 */


#ifndef __EX_MATRIX_H__
#define __EX_MATRIX_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class ExMatrixPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT;
    Q_INTERFACES(V3DPluginInterface2_1);

public:
        float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const {
		return QStringList() << tr("About ex_matrix");
	}
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent) {
		QMessageBox::information(parent, "Version info", 
                        QString("Example Matrix Plugin %1"
				"\nCalled by another plugin (ex_call) to do 3x3 matrix computation"
				"\n\ndeveloped by Zongcai Ruan 2010. (Janelia Farm Research Campus, HHMI)")
                        .arg(getPluginVersion()));
	}

	QStringList funclist() const;
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent);

};




#endif

