/* Node_type_correction_plugin.h
 * Node_type_correction
 * 2019-6-25 : by Peng Xie
 */
 
#ifndef __NODE_TYPE_CORRECTION_PLUGIN_H__
#define __NODE_TYPE_CORRECTION_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "Node_type_correction_func.h"

class Node_type_correction : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);
};

#endif

