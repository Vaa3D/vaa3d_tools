/* ex_push.h
 * an example program to test the push function in the plugin interface
* 2010-08-3: by Hanchuan Peng
 */


#ifndef __EX_CALL_H__
#define __EX_CALL_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class ExCallPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT;
    Q_INTERFACES(V3DPluginInterface2_1);

public:

	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent) {return true;}
        float getPluginVersion() const {return 1.1f;}
};

class matrixPanel : public QDialog
{
	Q_OBJECT;
public:
	QDoubleSpinBox* a[3][3];
	QDoubleSpinBox* b[3][3];
	QLabel* c[3][3];
	V3DPluginCallback2 &v3d;
	double A[3][3], B[3][3], C[3][3];

	virtual ~matrixPanel() {};
	matrixPanel(V3DPluginCallback2 &_v3d, QWidget *parent);
	void begin();
	void end();

public slots:
	void rand_a();
	void rand_b();

	void add();
	void mul();
	void at();
	void bt();
};


#endif

