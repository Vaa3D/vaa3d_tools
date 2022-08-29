/* NeuroMorphoLib_plugin.h
 * functions for processing neuron morphology
 * 2022-8-9 : by SD-Jiang
 */
 
#ifndef __NEUROMORPHOLIB_PLUGIN_H__
#define __NEUROMORPHOLIB_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class NMorphoPlugin : public QObject, public V3DPluginInterface2_1
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

