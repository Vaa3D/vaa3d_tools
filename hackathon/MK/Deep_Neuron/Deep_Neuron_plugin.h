/* Deep_Neuron_plugin.h
 * This is a test plugin, you can use it as a demo.
 * 2017-12-12 : by MK
 */
 
#ifndef __DEEP_NEURON_PLUGIN_H__
#define __DEEP_NEURON_PLUGIN_H__

#define BOOST_DATE_TIME_NO_LIB
#define BOOST_FILESYSTEM_NO_LIB
#define BOOST_SYSTEM_NO_LIB

#include <QtGui>

#include <v3d_interface.h>

class DeepNeuron_plugin : public QObject, public V3DPluginInterface2_1
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

