/* ex_push.h
 * an example program to test the push function in the plugin interface
* 2010-08-3: by Hanchuan Peng
 */


#ifndef __EX_PUSH_H__
#define __EX_PUSH_H__

#include <QtGui>
#include <stdio.h>
#include <stdlib.h>

#include "v3d_interface.h"

class ExPushPlugin : public QObject, public V3DPluginInterface2_1
{
    Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1);

private:
        // static const float pluginVersion = 1.1; // not in msvc

public:
//	QStringList menulist() const;
//	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);
//
//	QStringList funclist() const {return QStringList();}
//	void dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent) {}
//

	QStringList menulist() const;
	void domenu(const QString & menu_name, V3DPluginCallback2 & v3d,  QWidget * parent);

	QStringList funclist() const {return QStringList();}
	bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output,
				 V3DPluginCallback2 & v3d,  QWidget * parent) {return true;}

        // plugin interface version 2.1 requires plugin version
        float getPluginVersion() const {return 1.1f;}
};




#endif

