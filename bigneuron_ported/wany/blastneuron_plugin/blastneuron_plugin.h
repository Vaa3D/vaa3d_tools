/* blastneuron_plugin.h
 * BlastNeuron plugin for rapid retrieve and alignment of neuron morphologies, also includes pre-processing, inverse projection and batch feature computation
 * 2016-06-03 : by Yinan Wan
 */
 
#ifndef __BLASTNEURON_PLUGIN_H__
#define __BLASTNEURON_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

class BlastNeuronPlugin : public QObject, public V3DPluginInterface2_1
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

