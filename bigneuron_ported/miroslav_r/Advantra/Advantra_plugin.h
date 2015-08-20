/* Advantra_plugin.h
 * Tool for automatic neuron reconstruction from microscopy image stacks.
 * 2015-8-19 : by Miroslav Radojevic
 */
 
#ifndef __ADVANTRA_PLUGIN_H__
#define __ADVANTRA_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class Advantra : public QObject, public V3DPluginInterface2_1
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

